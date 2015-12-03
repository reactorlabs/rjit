#ifndef ABINLINER_H
#define ABINLINER_H

#include "llvm.h"
#include <Rinternals.h>
#include <algorithm>
#include <llvm/IR/InstIterator.h>

#include <string>
#include <sstream>
#include "FunctionCall.h"

namespace osr {
typedef std::vector<llvm::Instruction*> Inst_Vector;
typedef std::vector<llvm::ReturnInst*> RInst_Vector;

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

class ABInliner {
  public:
    static ABInliner& getInstance() {
        static ABInliner instance;
        return instance;
    }

    // Inlining functions

    /**
     * @brief      { function_description }
     *
     * @param      args     { parameter_description }
     * @param      getVars  { parameter_description }
     * @param[in]  n        { parameter_description }
     */
    static void replaceArgs(Inst_Vector* args, Inst_Vector* getVars, int n);

    /**
     * @brief      { function_description }
     *
     * @param      call    { parameter_description }
     * @param[in]  offset  { parameter_description }
     */
    static void updateCPAccess(llvm::CallInst* call, int offset);

    /**
     * @brief      { function_description }
     *
     * @param      inner   { parameter_description }
     * @param      fc      { parameter_description }
     * @param[in]  offset  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    static Function_N_RInsts getBody2Inline(llvm::Function* inner,
                                            FunctionCall* fc, int offset);

    /**
     * @brief      Inlines inner calls into outter
     *
     * @param      outter  llvm::Function*
     * @param      inner   llvm::Function*
     *
     * @return     outter with the inlined calls
     */
    static llvm::Function* inlineThisInThat(llvm::Function* outter,
                                            llvm::Function* inner);

    /**
     * @brief      Inlines fc call to inner into outter
     *
     * @param      fc        FunctionCall*
     * @param      outter    llvm::Function*
     * @param      inner     llvm::Function*
     *
     * @return     outter with the inlined call to inner
     */
    static llvm::Function* inlineFunctionCall(FunctionCall* fc,
                                              llvm::Function* outter,
                                              llvm::Function* inner,
                                              llvm::ReturnInst* iRet);
    // TODO this is useful only for now.
    void activate() { active = true; }
    void deactivate() {
        active = false;
        contexts.clear();
    }
    bool isActive() { return active; }

    std::vector<ContextWrapper*> contexts;

  private:
    ABInliner() : active(false) {}
    bool active;
};
} // namespace osr

#endif