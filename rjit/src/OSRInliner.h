#ifndef OSR_INLINER_H
#define OSR_INLINER_H

#include "llvm.h"
#include "Compiler.h"
#include "FunctionCall.h"
#include <Rinternals.h>
using namespace llvm;

namespace osr {
typedef std::vector<llvm::Instruction*> Inst_Vector;

class OSRInliner {
  public:
    /**
     * @brief      Inlines calls inside outer
     *
     * @param[in]  f  the outer function.
     * @param[in]  env    the environment associated with f.
     *
     * @return     a SEXP with a compiled version of f, where calls are inlined.
     */
    static SEXP inlineCalls(SEXP f, SEXP env);

  private:
    OSRInliner() {}

    static void setCP(SEXP firstP, SEXP secondP);

    /**
     * @brief      Updates the indexes used to access the constant pool
     *
     * @param      call    The instruction accessing the constant pool.
     * @param[in]  offset  Corrector value.
     */
    static void updateCPAccess(CallInst* call, int offset);

    static void replaceArgs(Inst_Vector* args, Inst_Vector* vars, int n);

    static SEXP getFunction(rjit::Compiler* c, SEXP cp, int symbol, SEXP env);

    static void prepareCodeToInline(Function* toInline, FunctionCall* fc,
                                    int cpOffset, ReturnInst** ret = nullptr);

    static void insertBody(Function* toOpt, Function* toInline,
                           Function* toInstrument, FunctionCall* fc,
                           ReturnInst* ret);
};

} // namespace osr

#endif