#include "OSRInliner.h"

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

    // If there are more instructions
    // after the icStub, we break get the next inst and split the block there
    llvm::BasicBlock* continuation = nullptr;
    llvm::BasicBlock::iterator it(fc->getIcStub());
    ++it;
    if (it != fc->getIcStub()->getParent()->end()) {
        continuation = deadBlock->splitBasicBlock(*it, "CONT");
    }

    // Get the return instructions inside the callee and the caller
    RInst_Vector* calleeReturns = FunctionCloner(toInline).getReturnInsts();

    // Insert the blocks from the function to Inline
    BB_Vector* blocks = FunctionCloner::getBBs(toInline);
    for (BB_Vector::iterator it = blocks->begin(); it != blocks->end(); ++it) {
        (*it)->removeFromParent();
        // inserts the block before the deadBlock
        (*it)->insertInto(outter, deadBlock);
    }

    // Handle the returns: simple case, one return
    if (calleeReturns->size() == 1) {
        llvm::Value* theRet = calleeReturns->at(0)->getReturnValue();
        fc->getIcStub()->replaceAllUsesWith(theRet);
    }

    // TODO check that it had only one successor?
    if (!blocks->empty()) {
        // Execution continues from the callBlock to the first block of the
        // inlined function
        callBlock->getTerminator()->setSuccessor(0, *(blocks->begin()));

        // Remove the return by a jump to the continuation
        if (continuation != nullptr && calleeReturns->size() == 1) {
            llvm::BasicBlock* parentReturn = dynamic_cast<llvm::BasicBlock*>(
                calleeReturns->at(0)->getParent());
            if (parentReturn == NULL) {
                printf("ERROR parent of return is null\n");
                return nullptr;
            }
            parentReturn->splitBasicBlock(calleeReturns->at(0), "DEADRETURN");
            parentReturn->getTerminator()->setSuccessor(0, continuation);
            // calleeReturns->at(0)->removeFromParent();
        }
    }

    // Clean up the vectors
    blocks->clear();
    calleeReturns->clear();
    // remove the toInline from the module now that it has been mutilated
    toInline->removeFromParent();

    outter->dump();
    // TODO fix the return statements ..

    return outter;
}

llvm::Function* OSRInliner::inlineThisInThat(llvm::Function* outter,
                                             llvm::Function* inner) {
    if (outter == nullptr || inner == nullptr) {
        return outter;
    }

    FunctionCloner* fe = new FunctionCloner(inner);
    // TODO select only the calls that correspond to inner
    FunctionCalls* calls = FunctionCall::getFunctionCalls(outter);

    llvm::Function* withArgs = nullptr;
    for (FunctionCalls::iterator it = calls->begin(); it != calls->end();
         ++it) {
        withArgs = fe->insertValues(*it);
        OSRInliner::inlineFunctionCall(*it, outter, withArgs);
    }
    return nullptr;
}

} // namespace osr