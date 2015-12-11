#ifndef ABINLINER_H
#define ABINLINER_H

#include "llvm.h"
#include <Rinternals.h>
#include <algorithm>
#include <llvm/IR/InstIterator.h>

#include <string>
#include <sstream>
#include "FunctionCall.h"
#include "Liveness.hpp"
#include "OSRLibrary.hpp"

namespace osr {
typedef std::vector<llvm::Instruction*> Inst_Vector;
typedef std::vector<llvm::ReturnInst*> RInst_Vector;
typedef std::map<std::string, SEXP> FunctionPool;

typedef struct {
    llvm::Function* f;
    llvm::ReturnInst* rInsts;
} Function_N_RInsts;

typedef struct ContextWrapper {
    std::vector<SEXP> cp;
    llvm::Function* f;
    ContextWrapper(std::vector<SEXP> CP, llvm::Function* F) {
        cp = CP;
        f = F;
    }
} ContextWrapper;

/**
 * @brief      THE API IS NOT FIXED; JUST PROTOTYPING FOR THE MOMENT
 */
class ABInliner {
  public:
    static ABInliner& getInstance() {
        static ABInliner instance;
        return instance;
    }

    // Inlining functions

    /**
     * @brief      Replaces the argument accesses by their values
     *
     * @param      args     contains the arguments in order of appearance
     * @param      getVars  contains the corresponding getVar calls
     * @param[in]  n        how many of them we want to replace
     */
    static void replaceArgs(Inst_Vector* args, Inst_Vector* getVars, int n);

    /**
     * @brief      Updates the indexes used to access the constant pool
     *
     * @param      call    The instruction that accesses the constant pool
     * @param[in]  offset  The new start of the constant pool
     */
    static void updateCPAccess(llvm::CallInst* call, int offset);

    /**
     * @brief      Create a clone of inner ready to be inlined at fc.
     *
     * @param      inner   The callee function
     * @param      fc      The caller call to inner
     * @param[in]  offset  Size of the caller's constant pool
     *
     * @return     The clone of inner with the correct access to the CP and
     *             the values for the arguments.
     */
    static Function_N_RInsts getBodyToInline(llvm::Function* inner,
                                             FunctionCall* fc, int offset);

    /**
     * @brief      Inlines inner calls into outer
     *
     * @param      outer  llvm::Function*
     *
     * @return     outer with the inlined calls
     */
    static SEXP inlineThisInThat(SEXP sOuter, SEXP env);

    /**
     * @brief      Inlines fc call to inner into outer and requires the return
     *             instruction of the inner function.
     *
     * @param      fc        FunctionCall*
     * @param      outer    llvm::Function*
     * @param      inner     llvm::Function*
     * @param      iRet      llvm::ReturnInst*
     *
     * @return     outer with the inlined call to inner
     */
    static llvm::Function* inlineFunctionCall(FunctionCall* fc,
                                              llvm::Function* outer,
                                              llvm::Function* inner,
                                              llvm::ReturnInst* iRet);

    static void OSRInstrument(llvm::Function* base,
                              llvm::Function* instrumented, StateMap* map);

    llvm::Function* inlineCalls(llvm::Function* outer, SEXP env);

    // TODO for testing
    static Inst_Vector* getOSRCondition();

    FunctionPool pool;

  private:
    ABInliner() {}
};
} // namespace osr

#endif