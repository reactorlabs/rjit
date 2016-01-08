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
#include "OSRHandler.h"

#include <cassert>

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
    assert(outer && clone && "ABInliner: failed to clone the body");

    Inst_Vector getVars;
    llvm::ReturnInst* returnInst = nullptr;

    // Replace the rho, consts and useCache
    llvm::Function::arg_iterator OAI = outer->arg_begin();
    llvm::Function::arg_iterator AI = clone->arg_begin();
    for (; AI != clone->arg_end() && OAI != outer->arg_end(); ++AI, ++OAI)
        (*AI).replaceAllUsesWith(&(*OAI));

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
    // llvm::Function* instru = OSRHandler::getInstrumented(outer);
    // auto base = std::pair<llvm::Function*, StateMap*>(instru,
    // OSRHandler::getInstance()->maps[instru]);

    llvm::BasicBlock* callBlock =
        dynamic_cast<llvm::BasicBlock*>(fc->getGetFunc()->getParent());

    assert(callBlock && "ABInliner: Call block is null");

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
    auto landPad = dynamic_cast<llvm::Instruction*>(
        base.second->getCorrespondingOneToOneValue(fc->getGetFunc()));
    OSRLibrary::OSRPointConfig conf(true, true, -1, nullptr, outer->getParent(),
                                    nullptr, nullptr, outer->getParent(),
                                    nullptr);
    outer->getParent()->getFunctionList().push_back(base.first);
    auto res = OSRLibrary::insertResolvedOSR(
        getGlobalContext(), *outer,
        *(dynamic_cast<llvm::Instruction*>(fc->getGetFunc())), *(base.first),
        *landPad, *(ABInliner::getOSRCondition()), *(base.second), conf);

    // clean up
    fc->getGetFunc()->removeFromParent();
    deadBlock->removeFromParent();
    delete deadBlock;
    toInline->removeFromParent();
    delete toInline;
    outer->dump();

    printf("See the instrumentation\n");
    res.second->dump();
    return outer;
}

SEXP ABInliner::inlineCalls(SEXP sOuter, SEXP env) {
    rjit::Compiler c("module");
    SEXP cOuter = c.compile("outer", sOuter);
    llvm::Function* outer = reinterpret_cast<llvm::Function*>(TAG(cOuter));
    assert(outer && "ABInliner failed to compile outer function.");

    llvm::Function* inner = nullptr;
    SEXP cInner;

    // Get the function calls inside the function.
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

SEXP ABInliner::concatenateCP(SEXP first, SEXP second) {
    int sizeF = LENGTH(CDR(first));
    int sizeS = LENGTH(CDR(second));
    SEXP newPool = allocVector(VECSXP, sizeF + sizeS);

    for (int i = 0; i < sizeF; ++i)
        SET_VECTOR_ELT(newPool, i, VECTOR_ELT(CDR(first), i));

    for (int i = 0; i < sizeS; ++i)
        SET_VECTOR_ELT(newPool, i, VECTOR_ELT(CDR(second), i));
    return newPool;
}
/*TODO fixed condition for the moment*/
Inst_Vector* ABInliner::getOSRCondition() {
    Inst_Vector* res = new Inst_Vector();
    llvm::ConstantInt* ci1 = ConstantInt::get(getGlobalContext(), APInt(32, 1));
    llvm::ConstantInt* ci2 = ConstantInt::get(getGlobalContext(), APInt(32, 0));
    llvm::ICmpInst* cond =
        new llvm::ICmpInst(llvm::ICmpInst::ICMP_EQ, ci1, ci2);
    res->push_back(cond);
    return res;
}

} // namespace osr