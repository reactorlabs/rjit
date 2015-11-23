#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/IR/CFG.h> // the successor function

#include "FunctionCloner.h"

using namespace llvm;
namespace osr {

llvm::Function* FunctionCloner::cloneF() {
    llvm::ValueToValueMapTy VMap;
    llvm::Function* duplicateFunction =
        llvm::CloneFunction(this->f, VMap, false);
    this->f->getParent()->getFunctionList().push_back(duplicateFunction);
    return duplicateFunction;
}

llvm::Function* FunctionCloner::insertValues(FunctionCall* fc) {
    int nArgs = fc->getNumbArguments();
    // Really not the best way of doing it but we'll try.
    llvm::ValueToValueMapTy VMap;
    llvm::Function* duplicateFunction =
        llvm::CloneFunction(this->f, VMap, false);
    this->f->getParent()->getFunctionList().push_back(duplicateFunction);
    Inst_Vector getVars;
    int counter = 0;
    // TODO kind of hack here. Find a better of identifying param vars.
    for (inst_iterator it = inst_begin(duplicateFunction),
                       e = inst_end(duplicateFunction);
         it != e && counter < nArgs; ++it) {
        llvm::CallInst* call = dynamic_cast<llvm::CallInst*>(&(*it));
        if (call != NULL && IS_GET_VAR(call)) {
            counter++;
            getVars.push_back(&(*it));
        }
    }
    // TODO kind of a hack here. Put the arguments inside of the function
    Inst_Vector args = *(fc->getArgs());
    for (Inst_Vector::iterator it = getVars.begin(), ait = args.begin();
         (it != getVars.end()) && (ait != args.end()); ++it, ++ait) {

        llvm::Instruction* ci = (*ait)->clone();
        ci->insertBefore(*it);
        (*it)->replaceAllUsesWith(ci);
        (*it)->removeFromParent();
        // VMap[(*it)] = (*ait);
    }

    // Split the block to insert the code
    llvm::Value* parent = fc->getGetFunc()->getParent();
    // check that this is a basic block
    llvm::BasicBlock* bb = dynamic_cast<llvm::BasicBlock*>(parent);
    printf("BEFORE THE POSSIBLE RETURN \n");
    if (bb == NULL)
        return nullptr; // failure

    llvm::BasicBlock* dead = bb->splitBasicBlock(fc->getGetFunc(), "DEAD");
    BB_Vector* blocks = this->getBBs(duplicateFunction);
    // TODO this is wrong, correct it
    llvm::Function* pf = dynamic_cast<llvm::Function*>(bb->getParent());
    for (BB_Vector::iterator it = blocks->begin(); it != blocks->end(); ++it) {
        (*it)->removeFromParent();
        (*it)->insertInto(pf, dead);
    }

    bb->getTerminator()->setSuccessor(0, (*(blocks->begin())));
    duplicateFunction->removeFromParent();

    // take away the rest of the code after the call

    printf("Let's see the result\n");
    pf->dump();
    // optional: remove the rest in a clean way if possible.
    // if not tail call, must set the continuation correctly.
    return nullptr;
}
}