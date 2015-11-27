#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H

#include "llvm.h"
#include <llvm/IR/InstIterator.h>

#define GETFUNCTION_NAME "getFunction"

#define ICSTUB_NAME "icStub"

#define IS_NAMED(x, y) ((x)->getName().str().compare((y)) == 0)

#define IS_GET_FUNCTION(x)                                                     \
    IS_NAMED(((x)->getCalledFunction()), GETFUNCTION_NAME)

#define NAME_CONTAINS(x, y)                                                    \
    ((((x)->getName().str()).find((y))) != std::string::npos)

#define GET_VAR_NAME "genericGetVar"
#define IS_GET_VAR(x) IS_NAMED((x)->getCalledFunction(), GET_VAR_NAME)

#define CONSTANT_NAME "constant"

namespace osr {

typedef std::vector<llvm::Instruction*> Inst_Vector;
class FunctionCall;
typedef std::vector<FunctionCall*> FunctionCalls;
typedef std::pair<llvm::inst_iterator, Inst_Vector*> Pos_N_Args;
typedef struct {
    llvm::inst_iterator it;
    Inst_Vector* args;
    llvm::CallInst* consts;
} Pos_Args_Consts;

/**
 * @brief      Wrapper for a function call in rjit. Enables to expose
 *             all parts easily. Also provides a function to get all calls
 *             from a function.
 */
class FunctionCall {
  public:
    FunctionCall(llvm::CallInst* getFunc, Inst_Vector args,
                 llvm::CallInst* consts, llvm::CallInst* icStub)
        : getFunc(getFunc), args(args), consts(consts), icStub(icStub) {}

    static FunctionCalls* getFunctionCalls(llvm::Function* f);

    static Pos_Args_Consts extractArguments(llvm::Function* f,
                                            unsigned int pos);

    void printFunctionCall();

    int getNumbArguments();

    llvm::CallInst* getGetFunc() { return getFunc; }
    Inst_Vector* getArgs() { return &args; }
    llvm::CallInst* getConsts() { return consts; }
    llvm::CallInst* getIcStub() { return icStub; }
    llvm::Function* getFunction() {
        if (consts && consts->getParent() && consts->getParent()->getParent()) {
            llvm::Function* func =
                dynamic_cast<llvm::Function*>(consts->getParent()->getParent());
            return func;
        }
        return nullptr;
    }

  private:
    llvm::CallInst* getFunc;
    Inst_Vector args;
    llvm::CallInst* consts;
    llvm::CallInst* icStub;
};
} // namespace osr
#endif