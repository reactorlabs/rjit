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
    llvm::Function* outter = fc->getFunction();
    Inst_Vector getVars;
    llvm::ReturnInst* returnInst = nullptr;

    if (!outter)
        printf("Error: getBody2Inline failure.\n"); // TODO fail or stop

    // Replace the rho, consts and useCache
    llvm::Function::arg_iterator OAI = outter->arg_begin();
    llvm::Function::arg_iterator AI = clone->arg_begin();
    for (; AI != clone->arg_end() && OAI != outter->arg_end(); ++AI, ++OAI) {
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

llvm::Function* ABInliner::inlineFunctionCall(FunctionCall* fc,
                                              llvm::Function* outter,
                                              llvm::Function* toInline,
                                              llvm::ReturnInst* iRet) {
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
        (*it)->insertInto(outter, deadBlock);
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
            printf("ERROR parent of return is null\n"); // TODO
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
    outter->dump();
    return outter;
}

llvm::Function*
ABInliner::inlineThisInThat(llvm::Function* outter, // TODO outer
                            llvm::Function* inner) {
    ABInliner u = ABInliner::getInstance();
    if (outter == nullptr || inner == nullptr || u.contexts.empty()) {
        return outter;
    }

    // TODO select only the calls that correspond to inner
    FunctionCalls* calls = FunctionCall::getFunctionCalls(outter);

    Function_N_RInsts bodyNRet;
    for (FunctionCalls::iterator it = calls->begin(); it != calls->end();
         ++it) {
        bodyNRet =
            ABInliner::getBodyToInline(inner, *it, u.contexts.at(0)->cp.size());
        ABInliner::inlineFunctionCall(*it, outter, bodyNRet.f, bodyNRet.rInsts);
    }
    return nullptr;
}

} // namespace osr