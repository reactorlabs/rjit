#ifndef OSRINLINER_H
#define OSRINLINER_H

#include "llvm.h"
#include "FunctionCall.h"
#include "FunctionCloner.h"

namespace osr {
class OSRInliner {
  public:
    // TODO do a per call function
    static llvm::Function* inlineThisInThat(llvm::Function* outter,
                                            llvm::Function* inner);
    static llvm::Function* inlineFunctionCall(FunctionCall* fc,
                                              llvm::Function* outter,
                                              llvm::Function* toInline);
};
} // namespace osr
#endif