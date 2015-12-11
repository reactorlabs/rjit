#include <llvm/IR/Module.h>
#include "Compiler.h"

#include "api.h"

#include "RIntlns.h"

#include "ir/ir.h"
#include "ir/Builder.h"
#include "ir/intrinsics.h"
#include "ir/Handler.h"

#include "Utils.h"

#include <llvm/IR/BasicBlock.h>
#include "llvm/Analysis/TargetTransformInfo.h"

#include "R.h"

#include "ABInliner.h"

namespace osr {

void ABInliner::replaceArgs(Inst_Vector* args, Inst_Vector* getVars, int n) {
    int counter = 0;
    for (Inst_Vector::iterator it = getVars->begin(), ait = args->begin();
         (it != getVars->end()) && (ait != args->end()) && (counter < n);
         ++it, ++ait, ++counter) {
        (*it)->replaceAllUsesWith(*ait);
        (*it)->removeFromParent();
    }
}

void ABInliner::updateCPAccess(llvm::CallInst* call, int offset) {
    for (unsigned int i = 0; i < call->getNumArgOperands(); ++i) {
        llvm::ConstantInt* index =
            dynamic_cast<llvm::ConstantInt*>(call->getArgOperand(i));
        if (index) {
            int64_t value = index->getSExtValue();
            call->setArgOperand(i, ConstantInt::get(getGlobalContext(),
                                                    APInt(32, value + offset)));
        }
    }
}
// TODO the functions is highly inefficient!
Function_N_RInsts ABInliner::getBodyToInline(llvm::Function* f,
                                             FunctionCall* fc, int offset) {

    llvm::Function* clone = Utils::cloneFunction(f);
    llvm::Function* outer = fc->getFunction();
    Inst_Vector getVars;
    llvm::ReturnInst* returnInst = nullptr;

    if (!outer)
        printf("Error: getBody2Inline failure.\n"); // TODO fail or stop

    // Replace the rho, consts and useCache
    llvm::Function::arg_iterator OAI = outer->arg_begin();
    llvm::Function::arg_iterator AI = clone->arg_begin();
    for (; AI != clone->arg_end() && OAI != outer->arg_end(); ++AI, ++OAI) {
        (*AI).replaceAllUsesWith(&(*OAI));
    }

    for (inst_iterator it = inst_begin(clone), e = inst_end(clone); it != e;
         ++it) {
        llvm::CallInst* call = dynamic_cast<llvm::CallInst*>(&(*it));
        if (call) {
            updateCPAccess(call, offset);
            if (IS_GET_VAR(call))
                getVars.push_back(&(*it));
        } else {
            llvm::ReturnInst* ret = dynamic_cast<llvm::ReturnInst*>(&(*it));
            if (ret)
                returnInst = ret;
        }
    }

    replaceArgs(fc->getArgs(), &getVars, fc->getNumbArguments());
    getVars.clear();

    Function_N_RInsts result = {clone, returnInst};
    return result;
}

// TODO instrument here!
llvm::Function* ABInliner::inlineFunctionCall(FunctionCall* fc,
                                              llvm::Function* outer,
                                              llvm::Function* toInline,
                                              llvm::ReturnInst* iRet) {
    auto base = StateMap::generateIdentityMapping(outer);
    llvm::BasicBlock* callBlock =
        dynamic_cast<llvm::BasicBlock*>(fc->getGetFunc()->getParent());
    if (callBlock == NULL) {
        printf("WARNING parent is not a block!"); // TODO probably error instead
        return nullptr;
    }

    llvm::BasicBlock* deadBlock =
        callBlock->splitBasicBlock(fc->getIcStub(), "DEADCALL");
    llvm::BasicBlock::iterator it(fc->getIcStub());
    ++it;
    llvm::BasicBlock* continuation = deadBlock->splitBasicBlock(*it, "CONT");

    // Insert the blocks from the function to inline
    BB_Vector* blocks = Utils::getBBs(toInline);
    for (BB_Vector::iterator it = blocks->begin(); it != blocks->end(); ++it) {
        (*it)->removeFromParent();
        (*it)->insertInto(outer, deadBlock);
    }

    // TODO Handles only one return
    llvm::Value* theRet = iRet->getReturnValue();
    fc->getIcStub()->replaceAllUsesWith(theRet);

    if (!blocks->empty()) {
        // It must have a unique successor since we just created the block
        callBlock->getTerminator()->setSuccessor(0, *(blocks->begin()));
        llvm::BasicBlock* parentReturn =
            dynamic_cast<llvm::BasicBlock*>(iRet->getParent());
        if (parentReturn == NULL) {
            printf("ERROR  of return is null\n"); // TODO
            return nullptr;
        }
        llvm::BasicBlock* split =
            parentReturn->splitBasicBlock(iRet, "DEADRETURN");
        parentReturn->getTerminator()->setSuccessor(0, continuation);
        split->removeFromParent();
        delete split;
    }

    blocks->clear();
    fc->getGetFunc()->removeFromParent();
    deadBlock->removeFromParent();
    delete deadBlock;
    toInline->removeFromParent();
    delete toInline;
    ABInliner::OSRInstrument(base.first, outer, base.second);
    outer->dump();
    return outer;
}

SEXP ABInliner::inlineThisInThat(SEXP sOuter, SEXP env) {

    rjit::Compiler c("module");
    SEXP cOuter = c.compile("outer", sOuter);

    llvm::Function* outer = reinterpret_cast<llvm::Function*>(TAG(cOuter));
    llvm::Function* inner = nullptr;
    SEXP cInner;

    if (!outer)
        return sOuter; // TODO fix me

    FunctionCalls* calls = FunctionCall::getFunctionCalls(outer);

    Function_N_RInsts bodyNRet;
    for (FunctionCalls::iterator it = calls->begin(); it != calls->end();
         ++it) {

        // Get the function that is called
        SEXP symbol = VECTOR_ELT(CDR(cOuter), (*it)->getFunctionSymbol());
        SEXP sInner = findFun(symbol, env);
        if (TYPEOF(sInner) == NATIVESXP) {
            inner = reinterpret_cast<llvm::Function*>(TAG(sInner));
            cInner = sInner;
        } else if (TYPEOF(sInner) == CLOSXP) {
            cInner = c.compile("inner", BODY(sInner));
            inner = reinterpret_cast<llvm::Function*>(TAG(cInner));
        } else {
            continue;
        }

        bodyNRet = ABInliner::getBodyToInline(inner, *it, LENGTH(CDR(cOuter)));
        ABInliner::inlineFunctionCall(*it, outer, bodyNRet.f, bodyNRet.rInsts);

        // Set the constant pool
        int sizeO = LENGTH(CDR(cOuter));
        int sizeI = LENGTH(CDR(cInner));
        SEXP objs = allocVector(VECSXP, sizeO + sizeI);
        for (int i = 0; i < sizeO; ++i)
            SET_VECTOR_ELT(objs, i, VECTOR_ELT(CDR(cOuter), i));
        for (int i = 0; i < sizeI; ++i)
            SET_VECTOR_ELT(objs, i + sizeO, VECTOR_ELT(CDR(cInner), i));
        SETCDR(cOuter, objs);
    }
    c.jitAll();
    return cOuter;
}

void ABInliner::OSRInstrument(llvm::Function* base,
                              llvm::Function* instrumented, StateMap* map) {
    llvm::Instruction* term = instrumented->getEntryBlock().getTerminator();
    llvm::Instruction* pad = &(*inst_begin(base));
    OSRLibrary::OSRPointConfig bim(true, true, -1, nullptr,
                                   instrumented->getParent(), nullptr, nullptr,
                                   instrumented->getParent(), nullptr);
    instrumented->getParent()->getFunctionList().push_back(base);
    if (!term)
        return;
    auto res = OSRLibrary::insertResolvedOSR(
        getGlobalContext(), *instrumented, *term, *base, *pad,
        *(ABInliner::getOSRCondition()), *map, bim);
    printf("The continuation\n");
    res.second->dump();
}

Inst_Vector* ABInliner::getOSRCondition() {
    Inst_Vector* res = new Inst_Vector();
    llvm::ConstantInt* ci = ConstantInt::get(getGlobalContext(), APInt(32, 1));
    llvm::ICmpInst* cond = new llvm::ICmpInst(llvm::ICmpInst::ICMP_EQ, ci, ci);
    res->push_back(cond);
    return res;
}

llvm::Function* ABInliner::inlineCalls(llvm::Function* outer, SEXP env) {

    /*ABInliner u = ABInliner::getInstance();
    FunctionCalls* calls = FunctionCall::getFunctionCalls(outer);
    Function_N_RInsts bodyNRet;
    for (FunctionCalls::iterator it = calls->begin(); it != calls->end(); ++it)
    {
        std::string name = (*it)->getGetFunc()->getName().str();
        if (ABInliner::getInstance().pool.find(name) !=
    ABInliner::getInstance().pool.end()) {
            SEXP fun = findFun(ABInliner::getInstance().pool[name], env);
            if (fun && TYPEOF(fun) == NATIVESXP) {
                llvm::Function* inner =
    reinterpret_cast<llvm::Function*>(TAG(fun));
                bodyNRet = ABInliner::getBodyToInline(inner, *it,
    u.contexts.at(0)->cp.size());
                ABInliner::inlineFunctionCall(*it, outer, bodyNRet.f,
    bodyNRet.rInsts);
            }
        } else {
            printf("NOT IN THE POOL!\n");
        }
    } */

    return nullptr;
}

} // namespace osr