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

    static OSRInliner* getInstance() {
        static OSRInliner instance;
        return &instance;
    }

    void activate() { this->active = true; }

    void deactivate() { this->active = false; }

    bool isActive() { return this->active; }

  private:
    OSRInliner() : active(false){};
    bool active;
};
} // namespace osr
#endif