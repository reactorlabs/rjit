#ifndef OSR_INLINER_H
#define OSR_INLINER_H

#include "llvm.h"
#include "Compiler.h"
#include "FunctionCall.h"
#include "RDefs.h"
#include "Types.h"
#include <Rinternals.h>
#include <list>
using namespace llvm;

namespace osr {
typedef std::vector<llvm::Instruction*> Inst_Vector;
typedef std::list<llvm::ReturnInst*> Return_List;

class OSRInliner {
  public:
    OSRInliner(rjit::Compiler* c);

    SEXP inlineCalls(SEXP f);

    static std::map<uint64_t, SEXP> exits;

  private:
    rjit::Compiler* c;
    Function* closureQuickArgumentAdaptor;
    Function* CONS_NR;
    Function* fixClosure;
    static uint64_t id;

    static void setCP(SEXP firstP, SEXP secondP);

    /**
     * @brief      Updates the indexes used to access the constant pool
     *
     * @param      call    The instruction accessing the constant pool.
     * @param[in]  offset  Corrector value.
     */
    static void updateCPAccess(CallInst* call, int offset);

    static SEXP getFunction(SEXP cp, int symbol, SEXP env);

    static bool isMissingArgs(SEXP formals, FunctionCall* fc);

    static void prepareCodeToInline(Function* toInline, FunctionCall* fc,
                                    CallInst* newrho, int cpOffset,
                                    Return_List* ret);

    void insertBody(Function* toOpt, Function* toInline, Function* toInstrument,
                    FunctionCall* fc, Return_List* ret);

    static Inst_Vector* getTrueCondition();
    static Inst_Vector* getOSRCondition(FunctionCall* fc);
    CallInst* createNewRho(FunctionCall* fc);
    Inst_Vector* createCompensation(SEXP fun, SEXP formals);
};

} // namespace osr

#endif