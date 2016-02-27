#ifndef ANALYSIS_SCALARS_TRACKING_H
#define ANALYSIS_SCALARS_TRACKING_H

#include "ir/Builder.h"
#include "ir/Ir.h"
#include "ir/Pass.h"
#include "ir/PassDriver.h"
#include "ir/IrScalars.h"
#include "ir/primitive_calls.h"



namespace rjit {
namespace analysis {

/** Scalar tracking

  1) for each variable, keep llvm register in which the value already exists
     - genericGetVar and genericSetVar update this info
     - ICStub clears it
  2) for each register that holds a variable or a sexp, keep register that has its unboxed scalar
     - getvectorelement and setvectorelement update this as well as CreateAndSetScalar

  Merging preserves the values if they are same, otherwise sets them to null.

  Updating aleady existing value is a noop, i.e. getvar of a var we already have will keep the mirroring

 */


class TrackingValue {
public:
    enum class Type {
        Uninitialized,
        ValuePtr,
        ScalarPtr
    };

    TrackingValue():
        type_(Type::Uninitialized),
        ptr_(nullptr) {
    }

    static TrackingValue valuePtr(ir::Value ptr) {
        return TrackingValue(Type::ValuePtr, ptr);
    }

    static TrackingValue scalarPtr(ir::Value ptr) {
        return TrackingValue(Type::ScalarPtr, ptr);
    }

    Type type() const {
        return type_;
    }

    llvm::Value * ptr() const {
        return ptr_;
    }

    bool mergeWith(TrackingValue const & other) {
        // merging uninitialized with anything else is always unintitialized
        if (type_ == Type::Uninitialized)
            return false;
        if (type_ == other.type_ and ptr_ == other.ptr_)
            return false;
        type_ = Type::Uninitialized;
        ptr_ = nullptr;
        return true;
    }

    void update(Type type, ir::Value ptr) {
        if (ptr_ == nullptr) {
            type_ = type;
            ptr_ = ptr;
        } else {
            assert(type_ == type and "It should be impossible to change tracking type");
        }
    }

private:

    TrackingValue(Type type, llvm::Value * ptr):
        type_(type),
        ptr_(ptr) {
    }

    Type type_;

    llvm::Value * ptr_;

};


class ScalarsTrackingPass: public ir::Fixpoint<ir::AState<TrackingValue>> {
public:
    typedef TrackingValue Value;
    typedef ir::AState<Value> State;

    /** getvar makes the link between its result and the variable it was read from.
     */
    match getVariable(ir::GenericGetVar * p) {
        SEXP v = p->symbolValue();
        state[v].update(Value::Type::ValuePtr, p);
    }

    /** setvar links the target variable t the source register.
     */
    match setVariable(ir::GenericSetVar * p) {
        SEXP v = p->symbolValue();
        state[v] = Value::valuePtr(p->value());
    }

    /** user call invalidates all variable links since we can't assume they will hold the same values after the call.
     */
    match userCall(ir::ICStub * p) {
        state.invalidateVariables(Value());
    }

    /** we are only interested in getting 0th element of a vector that is sexp of either double or integer.

      TODO this can be relaxed for any types as the optimization would still work but I am keeping it simple for now.
     */
    match getVectorElement(ir::GetVectorElement * p) {
        llvm::Value * index = p->index();
        llvm::Type * type = p->type();
        if (llvm::isa<llvm::ConstantInt>(index) and ir::Builder::integer(index) == 0)
            if (type == t::Double or type == t::Int)
                state[p->vector()].update(Value::Type::ScalarPtr, p->result());
    }


    /** Setting scalar value to a vector associates the vector with the new scalar overriding any possible existing associations.
     */
    match setVectorElement(ir::SetVectorElement * p) {
        llvm::Value * index = p->index();
        llvm::Type * type = p->type();
        if (llvm::isa<llvm::ConstantInt>(index) and ir::Builder::integer(index) == 0)
            if (type == t::Double or type == t::Int)
                state[p->vector()] = Value::scalarPtr(p->value());
    }

    /** Links the result SEXP to the scalar it is initialized with.
     */
    match createAndSetScalar(ir::CreateAndSetScalar * p) {
        state[p].update(Value::Type::ScalarPtr, p->scalar());
    }


    bool dispatch(llvm::BasicBlock::iterator & i) override;
};

class ScalarsTracking : public ir::ForwardDriver<ScalarsTrackingPass> {
public:
    typedef TrackingValue Value;
    typedef ir::AState<Value> State;
};





} // namespace analysis
} // namespace rjit

#endif // ANALYSIS_SCALARS_TRACKING_H
