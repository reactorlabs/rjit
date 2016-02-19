#ifndef ANALYSIS_TYPEANDSHAPE_H
#define ANALYSIS_TYPEANDSHAPE_H

#include "ir/Ir.h"
#include "ir/Pass.h"
#include "ir/PassDriver.h"

namespace rjit {
namespace analysis {

// TODO for now the lattices are super simple
// FIXME this will go away and be replaced with int packing value
class TypeAndShapeValue_ {
public:
    // TODO other interesting types will go in here
    enum class Type {
        Raw,
        Integer,
        Double,
        Complex,
        Character,
        List,
        Closure,
        Top,
    };

    // TODO size buckets will go in here
    enum class Shape {
        Scalar,
        Top,
    };

    // TODO stuff like Array, NamedArray, etc will go in here
    enum class Attributes {
        Empty,
        Top,
    };

    static TypeAndShapeValue_ merge(TypeAndShapeValue_ a, TypeAndShapeValue_ b) {
        TypeAndShapeValue_ result(a);
        result.mergeWith(b);
        return result;
    }

    bool mergeWith(TypeAndShapeValue_ const & other) {
        bool result = false;
        Type t = sup(type_, other.type_);
        if (t != type_) {
            type_ = t;
            result = true;
        }
        Shape s = sup(shape_, other.shape_);
        if (s != shape_) {
            shape_ = s;
            result = true;
        }
        Attributes a = sup(attributes_, other.attributes_);
        if (a != attributes_) {
            attributes_ = a;
            result = true;
        }
        return result;
    }

    Type type() const {
        return type_;
    }

    Shape shape() const {
        return shape_;
    }

    Attributes attributes() const {
        return attributes_;
    }

    /** While it would in theory make sense to create bottom as default, having bottom in the no-so-lattice-ish value would only complicate the code - we use nullptrs as bottom instead.

      TODO is this really true?
     */
    TypeAndShapeValue_():
        type_(Type::Top),
        shape_(Shape::Top),
        attributes_(Attributes::Top) {
    }

    TypeAndShapeValue_(Type t, Shape s = Shape::Top, Attributes a = Attributes::Top):
        type_(t),
        shape_(s),
        attributes_(a) {
    }


private:

    static Type sup(Type t1, Type t2) {
        return t1 == t2 ? t1 : Type::Top;
    }

    static Shape sup(Shape s1, Shape s2) {
        return s1 == s2 ? s1 : Shape::Top;
    }

    static Attributes sup(Attributes a1, Attributes a2) {
        return a1 == a2 ? a1 : Attributes::Top;
    }
    Type type_;
    Shape shape_;
    Attributes attributes_;
};

class TypeAndShapePass : public ir::Pass, public ir::Fixpoint<ir::AState<TypeAndShapeValue_>> {
public:
    typedef TypeAndShapeValue_ Value;
    typedef ir::AState<Value> State;

    void constantLoad(SEXP c, ir::Value p) {
        SEXPTYPE t = TYPEOF(c);
        Value::Shape s = (LENGTH(c) == 1) ? Value::Shape::Scalar : Value::Shape::Top;
        Value::Attributes a = Rf_isNull(ATTRIB(c)) ? Value::Attributes::Empty : Value::Attributes::Top;
        switch (t) {
        case RAWSXP:
            state[p] = Value(Value::Type::Raw, s, a);
            break;
        case INTSXP:
            state[p] = Value(Value::Type::Integer, s, a);
            break;
        case REALSXP:
            state[p] = Value(Value::Type::Double, s, a);
            break;
        case CPLXSXP:
            state[p] = Value(Value::Type::Complex, s, a);
            break;
        case CHARSXP:
            state[p] = Value(Value::Type::Character, s, a);
            break;
        case VECSXP:
            state[p] = Value(Value::Type::List, s, a);
            break;
        case CLOSXP:
            state[p] = Value(Value::Type::Closure, s, a);
            break;
        default:
            state[p] = Value(Value::Type::Top);
        }
    }

    match constant(ir::UserLiteral * p) {
        constantLoad(p->indexValue(), p);
    }


    match binops(ir::BinaryOperator * p) {
        auto lhs = p->lhs();
        auto rhs = p->rhs();
        state[p->pattern()] = Value::merge(state[lhs], state[rhs]);
    }

    /** If we have information about the variable, store it to the register, otherwise initialize the register to top.
       */
    match genericGetVar(ir::GenericGetVar * p) {
        llvm::Value * dest = p->result();
        SEXP symbol = p->symbolValue();
        if (state.has(symbol))
            state[dest] = state[symbol];
        else
            state[dest] = Value();
    }

    /** If we have incomming type & shape information, store it in the variable too. Otherwise do nothing (this means the variable will be assumed Top at read).
     */
    match genericSetVar(ir::GenericSetVar * p) {
        llvm::Value * src = p->rho();
        SEXP symbol = p->symbolValue();
        if (state.has(src))
            state[symbol] = state[src];
    }

    /** A call to ICStub invalidates all variables.

      TODO call to ICStub should be its own pattern

     */
    match call(llvm::CallInst * ins) {

    }

    bool isScalar(ir::Value v) {
        if (not state.has(v))
            return false;
        return state[v].shape() == Value::Shape::Scalar;
    }


    bool dispatch(llvm::BasicBlock::iterator& i) override;
};

class TypeAndShape : public ir::ForwardDriver<TypeAndShapePass> {
public:
    typedef TypeAndShapeValue_ Value;
    typedef ir::AState<Value> State;

} ;




} // namespace analysis
} // namespace rjit



#endif // IR_ANALYSIS_TYPEANDSHAPE_H
