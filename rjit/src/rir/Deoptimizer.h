#ifndef RIR_DEOPTIMIZER
#define RIR_DEOPTIMIZER

namespace rjit {
namespace rir {

class Deoptimizer {
    static Code* deoptimize(Function* fun, Code* cur, BC_t* pc);
}
}
}

#endif
