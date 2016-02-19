#ifndef PASS_DRIVER_H
#define PASS_DRIVER_H

#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/BasicBlock.h"
#include "ir/Pass.h"
#include "llvm.h"
#include "ir/State.h"

namespace rjit {
namespace ir {

template <typename aPass>
class PassDriver : public FunctionPass {
  public:
    static char ID;

    template <typename T>
    struct RegisterMe {
        RegisterMe() : X(aPass::getPassName(), "", false, false) {
            std::cout << aPass::getPassName() << "Registered\n";
        }
        RegisterPass<T> X;
    };
    static RegisterMe<PassDriver<aPass>> Registered;

    PassDriver() : FunctionPass(ID) {}

    void getAnalysisUsage(AnalysisUsage& AU) const override {}

};

template <typename aPass>
char PassDriver<aPass>::ID = 0;

template <typename aPass>
PassDriver<aPass>::RegisterMe<PassDriver<aPass>> PassDriver<aPass>::Registered;

template <typename Pass>
class LinearDriver : public PassDriver<Pass> {
  public:
    Pass pass;

    bool runOnFunction(Function& f) override {
        if (f.isDeclaration() || f.empty())
            return false;

        pass.setFunction(&f);
        return runOnFunction_(f);
    }

protected:

    virtual bool runOnFunction_(Function& f) { return dispatch_(f); }

    virtual bool dispatch_(Function& f) {
        for (auto& b : f) {
            BasicBlock::iterator i = b.begin();
            while (i != b.end()) {
                if (!pass.dispatch(i))
                    i++;
            }
        }

        return false;
    }
};

/** Forward driver for passes.

 The forward driver does

 */
template <typename PASS>
class ForwardDriver : public PassDriver<PASS> {
    typedef std::pair<llvm::BasicBlock *, typename PASS::State> QueueItem;
public:
    bool runOnFunction(llvm::Function & f) override {
        if (f.isDeclaration() or f.empty())
            return false;
        pass_.setFunction(&f);
        q_.push_back(QueueItem(f.begin(), pass_.initialState(&f)));
        while (not q_.empty()) {
            auto & i = q_.front();
            runOnBlock(i.first, std::move(i.second));
            q_.pop_front();
        }
        // forward driver for state passes is only for analyses
        return false;
    }

    PASS * pass() {
        return &pass_;
    }

protected:

    /** Executes the analysis pass on given block, checking whether a fixpoint has been reached, updating the current state and enqueuing its successors.
     */
    void runOnBlock(llvm::BasicBlock * block, typename PASS::State && incomming) {
        if (not pass_.runOnBlock(block, std::move(incomming)))
            return;
        // iterate over all instructions in the block
        BasicBlock::iterator i = block->begin();
        while (i != block->end())
            pass_.dispatch(i);
        // enqueue next block(s)
        llvm::TerminatorInst * t = block->getTerminator();
        for (unsigned ii = 1, end = t->getNumSuccessors(); ii < end; ++ii)
            q_.push_back(std::pair<llvm::BasicBlock *, typename PASS::State>(t->getSuccessor(ii), pass_.state));
        // enque first successor with move semantics
        if (t->getNumSuccessors() > 0)
            q_.push_back(std::pair<llvm::BasicBlock *, typename PASS::State>(t->getSuccessor(0), std::move(pass_.state)));
    }

    PASS pass_;
private:
    std::deque<QueueItem> q_;
};



} // namespace ir
} // namespace rjit

#endif
