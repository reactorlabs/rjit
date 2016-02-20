#ifndef IR_STATE_H
#define IR_STATE_H

/** State. All templated.


  State must allow merge

  I want to know:

  - type:
     Raw Logical Integer Double Complex Character List Closure Top ??others
                   \        /
                     Numeric
      \              /                     /
                 Vector
  - subtype: (class?)
     array, matrix,




  */

#include <memory>

#include "llvm.h"
#include "RIntlns.h"
#include "Ir.h"

namespace rjit {
namespace ir {



template<typename AVALUE>
class AState {
public:
    typedef AVALUE Value;

    AState() = default;
    AState(AState const &) = default;
    AState(AState && other):
        registers_(std::move(other.registers_)) {
    }

    bool has(ir::Value index) const {
        return registers_.find(index) != registers_.end();
    }

    bool has(SEXP symbol) const {
        return variables_.find(symbol) != variables_.end();
    }

    AVALUE & operator [] (ir::Value index) {
        return registers_[index];
    }

    AVALUE & operator [] (SEXP symbol) {
        return variables_[symbol];
    }

    AState & operator = (AState<AVALUE> const &) = default;

    /** Merges the other state into itself. Returns true if the state changed during the merge.
     */
    bool mergeWith(AState<AVALUE> const & other) {
        bool result = mergeMaps(registers_, other.registers_);
        return mergeMaps(variables_, other.variables_) or result;
    }

protected:

    template<typename MAP>
    bool mergeMaps(MAP & myMap, MAP const & theirMap) {
        bool result = false;
        for (auto their : theirMap) {
            auto mine = myMap.find(their.first);
            if (mine == myMap.end()) {
                myMap[their.first] = their.second;
                result = true;
            } else {
                result = mine->second.mergeWith(their.second) or result;
            }
        }
        // we do not care about any values in ourselves that are not part of incomming - this should not even be possible
        return result;
    }

    std::map<llvm::Value *, AVALUE> registers_;
    std::map<SEXP, AVALUE> variables_;



};

} // namespace ir
} // namespace rjit

#endif // IR_STATE_H


