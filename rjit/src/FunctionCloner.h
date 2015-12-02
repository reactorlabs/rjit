#ifndef FunctionCloner_H
#define FunctionCloner_H

#include "llvm.h"
#include "FunctionCall.h"

namespace osr {

typedef std::vector<llvm::Instruction*> Inst_Vector;
typedef std::vector<llvm::ReturnInst*> RInst_Vector;
/**
 * @brief      Enables to make a copy of a function's body, or replace the
 *             arguments with their actual values.
 *
 */
class FunctionCloner {
  public:
    FunctionCloner(llvm::Function* f) : f(f) {}

    /**
     * @brief      returns a clone of f
     *
     * @return     llvm::Function*
     */
    llvm::Function* cloneF();

    /**
     * @brief      Replaces arguments provided by fc into a clone of f
     *
     * @param      fc      FunctionCall to f
     * @param[in]  offset  Size of the constant pool of the caller
     *
     * @return     A clone of f with arguments replaced by their values
     */
    llvm::Function* insertValues(FunctionCall* fc, int offset);

    /**
     * @brief      Returns all the ReturnInst of f
     *
     * @return     A vector of the ReturnInst in f
     */
    RInst_Vector* getReturnInsts();

  private:
    llvm::Function* f;
};

} // namespace osr
#endif
