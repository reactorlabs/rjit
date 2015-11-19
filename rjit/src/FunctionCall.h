#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H

#include "llvm.h"

#define GETFUNCTION_NAME "getFunction"

#define ICSTUB_NAME "icStub"

#define IS_NAMED(x, y) ((x)->getName().str().compare((y)) == 0)

#define IS_GET_FUNCTION(x)                                                     \
    IS_NAMED(((x)->getCalledFunction()), GETFUNCTION_NAME)

#define NAME_CONTAINS(x, y)                                                    \
    ((((x)->getName().str()).find((y))) != std::string::npos)

#define ADVANCE(I, n)                                                          \
    for (unsigned int i = 0; i < n; ++i, ++I) {                                \
    }

namespace osr {

typedef std::vector<llvm::Instruction*> Inst_Vector;
class FunctionCall;
typedef std::vector<FunctionCall*> FunctionCalls;
typedef std::pair<llvm::inst_iterator, Inst_Vector*> Pos_N_Args;

class FunctionCall {
  public:
    FunctionCall(llvm::CallInst* getFunc, Inst_Vector args,
                 llvm::CallInst* icStub)
        : getFunc(getFunc), args(args), icStub(icStub) {}

    static FunctionCalls* getFunctionCalls(llvm::Function* f);

    static Pos_N_Args extractArguments(llvm::Function* f, unsigned int pos);

    Inst_Vector* getCallArguments();

    void printFunctionCall() {
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

  private:
    llvm::CallInst* getFunc;
    Inst_Vector args;
    llvm::CallInst* icStub;
};
} // namespace osr
#endif