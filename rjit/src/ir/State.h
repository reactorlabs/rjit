#ifndef IR_STATE_H
#define IR_STATE_H

/** State. All templated.
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
    AState & operator = (AState const &) = default;
    AState & operator = (AState &&) = default;

    AState(AState && other):
        registers_(std::move(other.registers_)) {
    }

    bool has(ir::Value index) const {
        return registers_.find(index) != registers_.end();
    }

    AVALUE & operator [] (ir::Value index) {
        return registers_[index];
    }

    bool has(SEXP symbol) const {
        return variables_.find(symbol) != variables_.end();
    }

    AVALUE & operator [] (SEXP symbol) {
        return  variables_[symbol];
    }

    /** Merges the other state into itself. Returns true if the state changed during the merge.
     */
    bool mergeWith(AState<AVALUE> const & other) {
        bool result = false;
        for (auto their : other.registers_) {
            auto mine = registers_.find(their.first);
            if (mine == registers_.end()) {
                registers_[their.first] = their.second;
                result = true;
            } else {
                result = mine->second.mergeWith(their.second) or result;
            }
        }
        // we do not care about any values in ourselves that are not part of incomming - this should not even be possible
        return result;
    }

protected:
    std::map<llvm::Value *, AVALUE> registers_;
    std::map<SEXP, AVALUE> variables_;



};

} // namespace ir
} // namespace rjit

#endif // IR_STATE_H


