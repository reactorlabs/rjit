#include "OSRInliner.h"

#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include "Utils.h"

namespace osr {

llvm::Function* OSRInliner::inlineFunctionCall(FunctionCall* fc,
                                               llvm::Function* outter,
                                               llvm::Function* toInline) {
    llvm::BasicBlock* callBlock =
        dynamic_cast<llvm::BasicBlock*>(fc->getGetFunc()->getParent());
    if (callBlock == NULL) {
        printf("WARNING parent is not a block!"); // TODO probably error instead
        return nullptr;
    }

    // Split the basic block in order to put the entire content of the call
    // inside an unreachable block
    llvm::BasicBlock* deadBlock =
        callBlock->splitBasicBlock(fc->getGetFunc(), "DEADCALL");
    llvm::BasicBlock* continuation = nullptr;
    llvm::BasicBlock::iterator it(fc->getIcStub());
    ++it;
    if (it != fc->getIcStub()->getParent()->end()) {
        continuation = deadBlock->splitBasicBlock(*it, "CONT");
    }

    // Get the return instructions inside the callee
    // TODO highly inefficient
    RInst_Vector* calleeReturns = FunctionCloner(toInline).getReturnInsts();

    // Insert the blocks from the function to inline
    BB_Vector* blocks = Utils::getBBs(toInline);
    for (BB_Vector::iterator it = blocks->begin(); it != blocks->end(); ++it) {
        (*it)->removeFromParent();
        (*it)->insertInto(outter, deadBlock);
    }

    // Handle the returns: simple case, one return
    if (calleeReturns->size() == 1) {
        llvm::Value* theRet = calleeReturns->at(0)->getReturnValue();
        fc->getIcStub()->replaceAllUsesWith(theRet);
    }

    if (!blocks->empty()) {
        // It must have a unique successor since we just created the block
        callBlock->getTerminator()->setSuccessor(0, *(blocks->begin()));

        // Replace the return by a jump to the continuation
        if (continuation != nullptr && calleeReturns->size() == 1) {
            llvm::BasicBlock* parentReturn = dynamic_cast<llvm::BasicBlock*>(
                calleeReturns->at(0)->getParent());
            if (parentReturn == NULL) {
                printf("ERROR parent of return is null\n"); // TODO
                return nullptr;
            }
            llvm::BasicBlock* split = parentReturn->splitBasicBlock(
                calleeReturns->at(0), "DEADRETURN");
            parentReturn->getTerminator()->setSuccessor(0, continuation);
            llvm::DeleteDeadBlock(split);
        }
    }

    blocks->clear();
    calleeReturns->clear();
    llvm::DeleteDeadBlock(deadBlock);
    toInline->removeFromParent();
    delete toInline;

    outter->dump();

    return outter;
}

llvm::Function* OSRInliner::inlineThisInThat(llvm::Function* outter,
                                             llvm::Function* inner) {
    Utils u = Utils::getInstance();
    if (outter == nullptr || inner == nullptr || u.contexts.empty()) {
        return outter;
    }

    FunctionCloner* fe = new FunctionCloner(inner);
    // TODO select only the calls that correspond to inner
    FunctionCalls* calls = FunctionCall::getFunctionCalls(outter);

    llvm::Function* withArgs = nullptr;
    for (FunctionCalls::iterator it = calls->begin(); it != calls->end();
         ++it) {
        withArgs = fe->insertValues(*it, u.contexts.at(0)->cp.size());
        OSRInliner::inlineFunctionCall(*it, outter, withArgs);
    }
    return nullptr;
}

} // namespace osr