#include <llvm/IR/InstIterator.h>

#include "FunctionCall.h"
#include "Utils.h"

namespace osr {

// TODO modify to use the use of getFunction instead.
Inst_Vector* FunctionCall::extractArguments(llvm::Function* f,
                                            llvm::inst_iterator it,
                                            llvm::Instruction* ic) {
    Inst_Vector* args = new Inst_Vector();
    llvm::inst_iterator end = inst_end(f);
    ++it; // skip the getFunction
    for (; it != end && (&(*it) != ic); ++it) {
        args->push_back(&(*it));
    }
    return args;
}

// TODO this is bad and I should feel bad
FunctionCalls* FunctionCall::getFunctionCalls(llvm::Function* f) {

    FunctionCalls* result = new FunctionCalls();
    llvm::CallInst* gf = nullptr;
    llvm::CallInst* ics = nullptr;

    for (llvm::inst_iterator it = inst_begin(f), e = inst_end(f); it != e;
         ++it) {
        gf = dynamic_cast<llvm::CallInst*>(&(*it));
        // TODO we assume there is only one use for the getFunc
        if (gf && IS_GET_FUNCTION(gf) && gf->getNumUses() == 1) {
            ics = dynamic_cast<llvm::CallInst*>(gf->user_back());
            if (ics && IS_STUB(ics)) {
                llvm::inst_iterator argsIt = it;
                Inst_Vector* args =
                    extractArguments(f, argsIt, gf->user_back());
                result->push_back(new FunctionCall(gf, *args, ics));
            } else {
                // TODO error malformed IR or wrong assumptions on my side
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