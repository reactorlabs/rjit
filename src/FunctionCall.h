#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H

#include "llvm.h"
#include <llvm/IR/InstIterator.h>

#define GETFUNCTION_NAME "getFunction"
#define CONSTANT_NAME "constant"
#define ICSTUB_NAME "icStub"
#define LITERAL_NAME "userLiteral"
#define GET_VAR_NAME "genericGetVar"

#define IS_NAMED(x, y) ((x)->getName().str().compare((y)) == 0)
#define NAME_CONTAINS(x, y)                                                    \
    ((((x)->getName().str()).find((y))) != std::string::npos)

#define IS_GET_FUNCTION(x)                                                     \
    IS_NAMED(((x)->getCalledFunction()), GETFUNCTION_NAME)
#define IS_GET_VAR(x) IS_NAMED((x)->getCalledFunction(), GET_VAR_NAME)
#define IS_CALL_NAMED(x, y) IS_NAMED((x)->getCalledFunction(), (y))
#define IS_STUB(x) NAME_CONTAINS((x)->getCalledFunction(), ICSTUB_NAME)
#define IS_CONSTANT_CALL(x) IS_CALL_NAMED((x), CONSTANT_NAME)

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
                 llvm::CallInst* icStub)
        : getFunc(getFunc), args(args), icStub(icStub) {}

    static FunctionCalls* getFunctionCalls(llvm::Function* f);

    static Inst_Vector* extractArguments(llvm::Function* f,
                                         llvm::inst_iterator it,
                                         llvm::Instruction* end);

    void printFunctionCall();

    int getNumbArguments();

    llvm::CallInst* getGetFunc() { return getFunc; }
    Inst_Vector* getArgs() { return &args; }
    llvm::CallInst* getIcStub() { return icStub; }
    llvm::Function* getFunction() {
        if (getFunc && getFunc->getParent() &&
            getFunc->getParent()->getParent()) {
            llvm::Function* func = dynamic_cast<llvm::Function*>(
                getFunc->getParent()->getParent());
            return func;
        }
        return nullptr;
    }

    int getFunctionSymbol();

  private:
    llvm::CallInst* getFunc;
    Inst_Vector args;
    llvm::CallInst* icStub;
};
} // namespace osr
#endif