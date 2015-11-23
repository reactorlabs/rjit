#ifndef OSRINLINER_H
#define OSRINLINER_H

#include "llvm.h"

namespace osr {
class OSRInliner {
  public:
    // TODO do a per call function
    static llvm::Function* inlineThisInThat(llvm::Function* outter,
                                            llvm::Function* inner);
};
} // namespace osr
#endif