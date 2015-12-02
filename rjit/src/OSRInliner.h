#ifndef OSRINLINER_H
#define OSRINLINER_H

#include "llvm.h"
#include "FunctionCall.h"
#include "FunctionCloner.h"

namespace osr {
/**
 * @brief      Provides static functions to inline a function into another one.
 */
class OSRInliner {
  public:
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
                                              llvm::Function* inner);

  private:
    OSRInliner(){};
};
} // namespace osr
#endif