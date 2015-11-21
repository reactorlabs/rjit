#ifndef FUNCTIONEXTRACTOR_H
#define FUNCTIONEXTRACTOR_H

#include "llvm.h"
#include "FunctionCall.h"

namespace osr {

typedef std::vector<llvm::Instruction*> Inst_Vector;
/**
 * @brief      Enables to make a copy of a function's body in order to inline
 * 						 it directly inside another
 * function.
 * TODO: 			Implement a function that takes a FunctionCall
 * to
 * f
 * and
 * inlines it.
 */
class FunctionExtractor {
  public:
    FunctionExtractor(llvm::Function* f) : f(f) {}

    llvm::Function* cloneF();
    llvm::Function* insertValues(FunctionCall* fc);

  private:
    llvm::Function* f;
};

} // namespace osr
#endif
