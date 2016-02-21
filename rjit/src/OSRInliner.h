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
typedef std::pair<SEXP, SEXP> ExitEntry;
typedef std::list<FunctionCall*> Call_List;
typedef std::map<SEXP, Call_List> Call_Map;

class OSRInliner {
  public:
    OSRInliner(rjit::Compiler* c);

    /**
     * @brief      Inlines all the function calls inside the function contained
     * in closure f.
     *
     * @param[in]  f     A SEXP closure.
     *
     * @return     Closure f containing a compiled function with function calls
     * inlined
     */
    SEXP inlineCalls(SEXP f);

    /**
     * Enables to register function SEXP to be used when the OSR exit is taken.
     */
    static std::map<uint64_t, ExitEntry> exits;

  private:
    rjit::Compiler* c;
    /**
     * Intrinsics used to create a new rho
     */
    Function* closureQuickArgumentAdaptor;
    Function* CONS_NR;
    /**
     * LLVM function corresponding to the function called in OSR ENTRY to
     * replace
     * the incorrect optimized version with the toInstrument version.
     */
    Function* fixClosure;

    /**
     * Unique id counter. Used for registering exits.
     */
    static uint64_t id;

    static void setCP(SEXP firstP, SEXP secondP);
    static void updateCPAccess(CallInst* call, int offset);
    static SEXP getFunction(SEXP cp, int symbol, SEXP env);
    Call_Map sortCalls(FunctionCalls* calls, SEXP outer);
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