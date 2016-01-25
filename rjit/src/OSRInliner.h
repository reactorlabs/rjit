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
    /**
     * @brief      Inlines calls inside outer
     *
     * @param[in]  f  the outer function.
     * @param[in]  env    the environment associated with f.
     *
     * @return     a SEXP with a compiled version of f, where calls are inlined.
     */
    SEXP inlineCalls(SEXP f, SEXP env);

  private:
    rjit::Compiler* c;
    Function* closureQuickArgumentAdaptor;
    Function* CONS_NR;

    static void setCP(SEXP firstP, SEXP secondP);

    /**
     * @brief      Updates the indexes used to access the constant pool
     *
     * @param      call    The instruction accessing the constant pool.
     * @param[in]  offset  Corrector value.
     */
    static void updateCPAccess(CallInst* call, int offset);

    static void replaceArgs(Inst_Vector* args, Inst_Vector* vars, int n);

    static SEXP getFunction(SEXP cp, int symbol, SEXP env);

    static void prepareCodeToInline(Function* toInline, FunctionCall* fc,
                                    int cpOffset, Return_List* ret);

    void insertBody(Function* toOpt, Function* toInline, Function* toInstrument,
                    FunctionCall* fc, Return_List* ret);

    static Inst_Vector* getTrueCondition();
    static Inst_Vector* getOSRCondition(FunctionCall* fc);
    Value* createNewRho(FunctionCall* fc);
};

} // namespace osr

#endif