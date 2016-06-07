#ifndef RIR_FUNCTION_H
#define RIR_FUNCTION_H

#include "BC.h"
#include "Code.h"

#include "RDefs.h"

#include <vector>

namespace rjit {
namespace rir {

class OpenFunction;

// Function is an array of code objects. Usually contained in a BCClosure
class Function {
  public:
    std::vector<Code*> code;

    Function() {}

    friend OpenFunction;
};
}
}

#endif
