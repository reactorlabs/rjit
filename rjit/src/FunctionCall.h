#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H

#include "llvm.h"
#include <llvm/IR/InstIterator.h>
#include <Rinternals.h>
#include "Compiler.h"
#include "JITCompileLayer.h"

#define GETFUNCTION_NAME "getFunction"
#define CONSTANT_NAME "constant"
#define ICSTUB_NAME "icStub"
#define GET_VAR_NAME "genericGetVar"
#define USR_LIT "userLiteral"

#define IS_NAMED(x, y) ((x)->getName().str().compare((y)) == 0)
#define NAME_CONTAINS(x, y)                                                    \
    ((((x)->getName().str()).find((y))) != std::string::npos)

#define IS_GET_FUNCTION(x)                                                     \
    IS_NAMED(((x)->getCalledFunction()), GETFUNCTION_NAME)
#define IS_STUB(x) NAME_CONTAINS((x)->getCalledFunction(), ICSTUB_NAME)

using namespace llvm;
namespace osr {

typedef std::vector<Instruction*> Inst_Vector;
class FunctionCall;
typedef std::vector<FunctionCall*> FunctionCalls;

/**
 * @brief      Wrapper for a function call in rjit. Enables to expose
 *             all parts easily. Also provides a function to get all calls
 *             from a function.
 */
class FunctionCall {
  public:
    FunctionCall(CallInst* icStub);

    static FunctionCalls* getFunctionCalls(Function* f);

    unsigned int getNumbArguments();

    CallInst* getGetFunc() { return getFunc; }
    Inst_Vector* getArgs() { return &args; }
    CallInst* getIcStub() { return icStub; }
    Instruction* getConsts() { return consts; }
    Function* getFunction();
    void fixPromises(SEXP cp, SEXP inFun, rjit::Compiler* c);

    int getFunctionSymbol();

    void setInPtr(rjit::Compiler* c, SEXP addr);
    Value* getInPtr();
    Instruction* getArg_back();

  private:
    CallInst* getFunc;
    Inst_Vector args;
    CallInst* consts;
    CallInst* icStub;
    Value* inPtr;

    Value* getRho();
};
} // namespace osr
#endif