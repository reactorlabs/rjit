#ifndef OPTIMIZATION_BOXINGREMOVAL_H
#define OPTIMIZATION_BOXINGREMOVAL_H

#include "ir/Analysis/ScalarsTracking.h"


namespace rjit {
namespace optimization {

class BoxingRemoval;


class BoxingRemovalPass : public ir::Pass, public ir::Optimization {
public:
    typedef analysis::ScalarsTracking::Value Value;


    /** Getting a variable can be replaced with register holding its value.
     */
    match variableGet(ir::GenericGetVar * p) {
        Value & v = st()[p->symbolValue()];
        //std::cout << CHAR(PRINTNAME(p->symbolValue())) << std::endl;
        if (v.ptr() != nullptr and v.ptr() != p->result()) {
            //std::cout << "removed" << std::endl;
            assert(v.type() == Value::Type::ValuePtr and "Tracking type of variables should be value.");
            replaceAllUsesWith(p, v.ptr());
            eraseFromParent(p);
        }
    }

    /** Getting a scalar we already know can be replaced with the original register.
     */
    match getElement(ir::GetVectorElement * p) {
        Value & v = st()[p->vector()];
        if (v.ptr() != nullptr and v.ptr() != p->result()) {
            assert(v.type() == Value::Type::ScalarPtr and "Tracking type of non variables should be scalar.");
            replaceAllUsesWith(p, v.ptr());
            eraseFromParent(p);
        }
    }

    bool dispatch(llvm::BasicBlock::iterator & i) override;

protected:

    friend class BoxingRemoval;

    analysis::ScalarsTrackingPass * st_ = nullptr;

    analysis::ScalarsTrackingPass & st() {
        return *st_;
    }

};

class BoxingRemoval : public ir::OptimizationDriver<BoxingRemovalPass, analysis::ScalarsTracking> {
protected:
    void setFunction(llvm::Function * f) override {
        ir::OptimizationDriver<BoxingRemovalPass, analysis::ScalarsTracking>::setFunction(f);
        pass.st_ = getAnalysis<analysis::ScalarsTracking>().pass();
    }
};




} // namespace optimization
} // namespace rjit

#endif // OPTIMIZATION_BOXINGREMOVAL_H
