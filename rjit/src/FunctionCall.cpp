#include <llvm/IR/InstIterator.h>

#include "FunctionCall.h"
#include "Utils.h"

namespace osr {

FunctionCall::FunctionCall(llvm::CallInst* getFunc, Inst_Vector args,
                           llvm::CallInst* icStub)
    : getFunc(getFunc), args(args), icStub(icStub) {}

Pos_N_Args FunctionCall::extractArguments(llvm::Function* f, unsigned int pos) {

    Inst_Vector* result = new Inst_Vector();
    llvm::inst_iterator I = Utils::advance(inst_begin(f), pos);
    llvm::inst_iterator E = inst_end(f);
    llvm::CallInst* stub = NULL;
    while (I != E) {
        stub = dynamic_cast<llvm::CallInst*>(&(*I));
        // TODO good for now but would be better to check that this is the
        // actual call
        // to the function that we took in getFunction. Probably can do that
        // through
        // the Use of the instruction.
        if (stub != NULL &&
            NAME_CONTAINS(stub->getCalledFunction(), "icStub")) {
            return Pos_N_Args(I, result);
        }

        result->push_back(&(*I));
        ++I;
    }
    return Pos_N_Args(E, result);
}

FunctionCalls* getFunctionCalls(llvm::Function* f) {

    FunctionCalls* result = new FunctionCalls();
    llvm::CallInst* gf = NULL;
    llvm::CallInst* ics = NULL;
    inst_iterator_wrap I(inst_begin(f));
    inst_iterator_wrap E(inst_end(f));

    for (; I != E; ++I) {
        gf = dynamic_cast<llvm::CallInst*>(I.get());
        if (gf != NULL && IS_GET_FUNCTION(gf)) {
            Pos_N_Args res = FunctionCall::extractArguments(f, I.getPos());
            ics = dynamic_cast<llvm::CallInst*>(&(*(res.first)));
            if (gf != NULL) {
                std::string name = Utils::getIcStubName(res.second->size());
                if (IS_NAMED(gf->getCalledFunction(), name)) {
                    ics = gf;
                    result->push_back(new FunctionCall(gf, *(res.second), ics));
                }
            }
        }
    }

    return result;
}

} // namespace osr