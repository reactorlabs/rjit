#ifndef PASS_H
#define PASS_H

#include "llvm.h"
#include "RIntlns.h"
//#include "primitive_calls.h"
#include "JITModule.h"
#include "ir/Ir.h"

namespace rjit {
namespace ir {

class Pattern;

template <typename PASS>
class ForwardDriver;


class Pass {
public:
    typedef void match;

    Pass() {}

    /** The dispatcher method. Autogenerated in some other file.
     */
    virtual bool dispatch(llvm::BasicBlock::iterator& i);

    virtual match defaultMatch(Pattern* ins) {
        // std::cout << "default instruction match" << std::endl;
    }

    virtual match defaultMatch(llvm::Instruction* ins) {
        // ins->dump();
    }

    void setFunction(llvm::Function * f) {
        this->f = f;
        constantPool = f->arg_begin();
    }

protected:

    friend class Predicate;

    /** Returns the constant pool's value for currently analyzed function.
     */
    llvm::Value * constantPool;

    llvm::Function * f;

};

/** A fixpoint driver.
 */
template<typename ASTATE>
class Fixpoint {
public:
    typename ASTATE::Value const & operator [] (ir::Value index) const {
        return state[index];
    }

    typename ASTATE::Value & operator [] (ir::Value index) {
        return state[index];
    }

protected:
    template<typename T>
    friend class ForwardDriver;

    virtual bool runOnBlock(llvm::BasicBlock * block, ASTATE && incomming) {
        if (states_.find(block) != states_.end()) {
            if (not states_[block].mergeWith(incomming))
                return false;
        } else {
            states_[block] = incomming;
        }
        // set current state
        state = states_[block];
        return true;
    }

    virtual ASTATE initialState(llvm::Function * f) {
        return ASTATE();
    }

    ASTATE state;

private:
    std::map<llvm::BasicBlock *, ASTATE> states_;

};



/** Base class for all optimizations.

  Provides means to alter the ir.
 */
class Optimization {
  protected:
    void replaceAllUsesWith(llvm::Instruction* o, llvm::Instruction* n) {
        o->replaceAllUsesWith(n);
    }

    void replaceAllUsesWith(llvm::Instruction* o, Pattern* n);

    void replaceAllUsesWith(Pattern* o, llvm::Instruction* n);

    void replaceAllUsesWith(Pattern* o, Pattern* n);

    void removeFromParent(llvm::Instruction* ins) { ins->removeFromParent(); }

    void eraseFromParent(llvm::Instruction* ins) { ins->eraseFromParent(); }

    void eraseFromParent(Pattern* p);
};


} // namespace ir
} // namespace rjit

#endif // PASS_H
