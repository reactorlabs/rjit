#ifndef OPTIMIZATION_SCALARS_H
#define OPTIMIZATION_SCALARS_H

#include "ir/Ir.h"
#include "ir/Pass.h"
#include "ir/PassDriver.h"
#include "ir/Analysis/TypeAndShape.h"
#include "ir/IrScalars.h"


namespace rjit {
namespace optimization {


class Scalars;

class ScalarsPass : public ir::Pass, public ir::Optimization {
public:

    typedef analysis::TypeAndShape::Value Value;

    /** Converts primitive binary operator pattern to a scalar operation.

      The template argument determines the scalar pattern it should be replaced with.
     */
    template<typename T>
    void replaceWithScalar(ir::PrimitiveBinaryOperator * p, llvm::Value * lhs, llvm::Value * rhs, SEXPTYPE s, llvm::Type * scalarType) {
        auto l = ir::GetVectorElement::insertBefore(p, lhs, 0, scalarType);
        auto r = ir::GetVectorElement::insertBefore(p, rhs, 0, scalarType);
        auto op = T::insertBefore(p, l, r);
        auto result = ir::AllocVector::insertBefore(p, s, 1);
        ir::SetVectorElement::insertBefore(p, result, 0, op, scalarType);
        replaceAllUsesWith(p, result);
        eraseFromParent(p);
    }

    template<typename T>
    void replaceWithScalar(ir::PrimitiveBinaryOperator * p) {
        llvm::Value * lhs = p->lhs();
        Value l = tsa()[lhs];
        llvm::Value * rhs = p->rhs();
        Value r = tsa()[rhs];
        if (l.shape() == Value::Shape::Scalar and r.shape() == Value::Shape::Scalar) {
            if (l.type() == Value::Type::Double and r.type() == Value::Type::Double) {
                replaceWithScalar<typename T::ScalarDouble>(p, lhs, rhs, REALSXP, t::Double);
            } else if (l.type() == Value::Type::Integer and r.type() == Value::Type::Integer) {
                replaceWithScalar<typename T::ScalarInt>(p, lhs, rhs, INTSXP, t::Int);
            }
        }

    }


    match add(ir::GenericAdd * p) {
        replaceWithScalar<ir::GenericAdd>(p);
    }

    match add(ir::GenericSub * p) {
        replaceWithScalar<ir::GenericSub>(p);
    }

    match add(ir::GenericMul * p) {
        replaceWithScalar<ir::GenericMul>(p);
    }

    match add(ir::GenericDiv * p) {
        replaceWithScalar<ir::GenericDiv>(p);
    }


    bool dispatch(llvm::BasicBlock::iterator & i) override;

protected:

    friend class Scalars;

    analysis::TypeAndShapePass * tsa_ = nullptr;

    analysis::TypeAndShapePass & tsa() {
        return *tsa_;
    }

};


class Scalars : public ir::LinearDriver<ScalarsPass> {
public:
    void getAnalysisUsage(AnalysisUsage& AU) const override {
        AU.addRequired<analysis::TypeAndShape>();
    }

    bool runOnFunction(llvm::Function & f) override {
        f.dump();
        std::cout << "Running on function" << std::endl;
        pass.tsa_ =  getAnalysis<analysis::TypeAndShape>().pass();
        return ir::LinearDriver<ScalarsPass>::runOnFunction(f);
    }
};

} // namespace optimization
} // namespace rjit

#endif // OPTIMIZATION_SCALARS_H
