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
    // TODO try to get the return instruction also.
    // TODO see how we could use the valueMap
    Inst_Vector args = *(fc->getArgs());
    for (Inst_Vector::iterator it = getVars.begin(), ait = args.begin();
         (it != getVars.end()) && (ait != args.end()); ++it, ++ait) {

        llvm::Instruction* ci = (*ait)->clone();
        ci->insertBefore(*it);
        (*it)->replaceAllUsesWith(ci);
        (*it)->removeFromParent();
    }
    return duplicateFunction;
}

RInst_Vector* FunctionCloner::getReturnInsts() {
    RInst_Vector* res = new RInst_Vector();
    llvm::ReturnInst* ret = nullptr;
    for (inst_iterator it = inst_begin(this->f), e = inst_end(this->f); it != e;
         ++it) {
        ret = dynamic_cast<llvm::ReturnInst*>(&(*it));
        if (ret != NULL)
            res->push_back(ret);
    }
    return res;
}
}