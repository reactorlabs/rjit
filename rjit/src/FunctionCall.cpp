#include <llvm/IR/InstIterator.h>

#include "FunctionCall.h"
#include "Utils.h"

namespace osr {

Pos_Args_Consts FunctionCall::extractArguments(llvm::Function* f,
                                               unsigned int pos) {

    Inst_Vector* args = new Inst_Vector();
    llvm::inst_iterator I = Utils::advance(inst_begin(f), pos);
    llvm::inst_iterator E = inst_end(f);
    llvm::CallInst* stub = NULL;
    llvm::CallInst* constantCall = NULL;
    // Skip the getFunction line
    if (I != E)
        ++I;

    while (I != E) {
        stub = dynamic_cast<llvm::CallInst*>(&(*I));
        // TODO good for now but would be better to check that this is the
        // actual call
        // to the function that we took in getFunction. Probably can do that
        // through
        // the Use of the instruction.
        if (stub != NULL &&
            NAME_CONTAINS(stub->getCalledFunction(), ICSTUB_NAME)) {
            Pos_Args_Consts result = {I, args, constantCall};
            return result;
        } else if (stub != NULL &&
                   NAME_CONTAINS(stub->getCalledFunction(), CONSTANT_NAME)) {
            constantCall = stub;
        }
        args->push_back(&(*I));
        ++I;
    }
    Pos_Args_Consts result = {E, args, constantCall};
    return result;
}

// TODO this is bad and I should feel bad
FunctionCalls* FunctionCall::getFunctionCalls(llvm::Function* f) {

    FunctionCalls* result = new FunctionCalls();
    llvm::CallInst* gf = NULL;
    llvm::CallInst* ics = NULL;
    inst_iterator_wrap I(inst_begin(f));
    inst_iterator_wrap E(inst_end(f));

    for (; I != E; ++I) {
        gf = dynamic_cast<llvm::CallInst*>(I.get());
        if (gf != NULL && IS_GET_FUNCTION(gf)) {
            Pos_Args_Consts res = FunctionCall::extractArguments(f, I.getPos());
            ics = dynamic_cast<llvm::CallInst*>(&(*(res.it)));
            if (ics != NULL) {
                if (NAME_CONTAINS(ics->getCalledFunction(), ICSTUB_NAME)) {
                    result->push_back(
                        new FunctionCall(gf, *(res.args), res.consts, ics));
                }
            }
        }
    }

    return result;
}

void FunctionCall::printFunctionCall() {
    printf("------------------------------------------------\n");
    printf("The getFunction:\n");
    getFunc->dump();
    printf("The intermediary args:\n");
    for (Inst_Vector::iterator it = args.begin(); it != args.end(); ++it) {
        (*it)->dump();
    }
    printf("The icStub:\n");
    icStub->dump();
    printf("------------------------------------------------\n");
}

int FunctionCall::getNumbArguments() {
    std::string name = icStub->getCalledFunction()->getName().str();
    return ((int)name.back() - '0');
}

} // namespace osr