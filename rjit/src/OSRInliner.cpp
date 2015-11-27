#include "OSRInliner.h"
#include <llvm/IR/Dominators.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

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
    // after the icStub, we get the next inst and split the block there
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
            llvm::BasicBlock* split = parentReturn->splitBasicBlock(
                calleeReturns->at(0), "DEADRETURN");
            parentReturn->getTerminator()->setSuccessor(0, continuation);
            llvm::DeleteDeadBlock(split);
            // calleeReturns->at(0)->removeFromParent();
        }
    }

    // Clean up the vectors
    blocks->clear();
    calleeReturns->clear();
    // Remove the deadblock
    llvm::DeleteDeadBlock(deadBlock);
    // remove the toInline from the module now that it has been mutilated
    toInline->removeFromParent();
    delete toInline;

    outter->dump();

    for (auto it = outter->arg_begin(); it != outter->arg_end(); ++it) {
        printf("Number of uses: %d\n", (*it).getNumUses());
    }
    /*std::string result;
   llvm::raw_string_ostream rso(result);
   if(!llvm::verifyFunction(*outter, &rso)) {
       printf("The duplicate is FINE \n");
   }else {
       printf("PROBLEMMMMMMM with duplicate %s\n", rso.str().c_str());
   }
   printf("testing the module");
   if(!llvm::verifyModule(*(outter->getParent()), &rso)) {
       printf("The module is fiiiiiine\n");
   } else {
       printf("The module is malformed\n");
   }*/
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