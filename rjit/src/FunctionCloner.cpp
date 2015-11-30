#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/IR/CFG.h> // the successor function
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

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
    // TODO not sure about that
    this->f->getParent()->getFunctionList().push_back(duplicateFunction);
    Inst_Vector getVars;
    int counter = 0;

    llvm::Function* outter = fc->getFunction();
    if (outter) {
        llvm::Function::arg_iterator OAI = outter->arg_begin();
        llvm::Function::arg_iterator AI = duplicateFunction->arg_begin();
        // Not sure this works since we replace..
        for (; AI != duplicateFunction->arg_end() && OAI != outter->arg_end();
             ++AI, ++OAI) {
            (*AI).replaceAllUsesWith(&(*OAI));
        }
    }

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
    // TODO see how we could use the valueMap
    /* Inst_Vector args = *(fc->getArgs());
     for (Inst_Vector::iterator it = getVars.begin(), ait = args.begin();
          (it != getVars.end()) && (ait != args.end()); ++it, ++ait) {

         llvm::Instruction* ci = (*ait)->clone();
         ci->insertBefore(*it);
         (*it)->replaceAllUsesWith(ci);
         (*it)->removeFromParent();
     }*/

    // Clean up
    getVars.clear();
    // args.clear();
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