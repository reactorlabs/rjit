#include <llvm/IR/InstIterator.h>

#include "FunctionCall.h"
#include "Utils.h"

namespace osr {

// TODO modify to use the use of getFunction instead.
Pos_Args_Consts FunctionCall::extractArguments(llvm::Function* f,
                                               llvm::inst_iterator it) {

    Inst_Vector* args = new Inst_Vector();
    llvm::CallInst* stub = nullptr;
    llvm::CallInst* constantCall = nullptr;
    llvm::inst_iterator end = inst_end(f);
    ++it;
    for (; it != end; ++it) {
        stub = dynamic_cast<llvm::CallInst*>(&(*it));
        if (stub && IS_STUB(stub)) {
            Pos_Args_Consts result = {it, args, constantCall};
            return result;
        } else if (stub && IS_CONSTANT_CALL(stub)) {
            constantCall = stub;
        } else {
            args->push_back(&(*it));
        }
    }
    Pos_Args_Consts result = {end, args, constantCall};
    return result;
}

// TODO this is bad and I should feel bad
FunctionCalls* FunctionCall::getFunctionCalls(llvm::Function* f) {

    FunctionCalls* result = new FunctionCalls();
    llvm::CallInst* gf = nullptr;
    llvm::CallInst* ics = nullptr;

    for (llvm::inst_iterator it = inst_begin(f), e = inst_end(f); it != e;
         ++it) {
        gf = dynamic_cast<llvm::CallInst*>(&(*it));
        if (gf && IS_GET_FUNCTION(gf)) {
            llvm::inst_iterator copy = it;
            Pos_Args_Consts res = FunctionCall::extractArguments(f, copy);
            ics = dynamic_cast<llvm::CallInst*>(&(*(res.it)));
            if (ics && IS_STUB(ics)) {
                result->push_back(
                    new FunctionCall(gf, *(res.args), res.consts, ics));
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