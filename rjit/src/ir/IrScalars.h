#ifndef IR_IR_SCALARS_H
#define IR_IR_SCALARS_H

#include "ir/Ir.h"

#include <climits>

namespace rjit {
namespace ir {

/** Allocates a vector of given type & size.

  Translates to a Rf_allocVector call internally.
 */
class AllocVector : public ir::PrimitiveCall {
public:

    SEXPTYPE type() {
        return Builder::integer(ins_->getOperand(0));
    }

    llvm::Value * size() {
        return ins_->getOperand(1);
    }

    static AllocVector * create(Builder & b, SEXPTYPE type, ir::Value size) {
        Sentinel s(b);
        return insertBefore(s, type, size);
    }

    static AllocVector * insertBefore(llvm::Instruction * ins, SEXPTYPE type, ir::Value size) {
        llvm::CallInst* i = llvm::CallInst::Create(
            primitiveFunction<AllocVector>(ins->getModule()), {Builder::integer(type), size }, "", ins);
        Builder::markSafepoint(i);
        return new AllocVector(i);
    }

    static AllocVector * insertBefore(Pattern * p, SEXPTYPE type, ir::Value size) {
        return insertBefore(p->first(), type, size);
    }

    static AllocVector * insertBefore(Pattern * p, SEXPTYPE type, int size) {
        return insertBefore(p->first(), type, Builder::integer(size, 64));
    }

    static char const * intrinsicName() {
        return "Rf_allocVector";
    }

    static llvm::FunctionType * intrinsicType() {
        // SEXPTYPE is unsigned in R
        return llvm::FunctionType::get(
            t::SEXP, {t::Int, t::VectorLength}, false);
    }

    static bool classof(Pattern const* s) {
        return s->kind == Kind::AllocVector;
    }

protected:
    AllocVector(llvm::Instruction * ins):
        ir::PrimitiveCall(ins, Kind::AllocVector) {
    }

};

/** Base class for native operators.

  These should have their anchor instruction the native operator itself.
*/
class NativeBinaryOperator : public ir::Pattern, ir::BinaryOperator {
public:
    llvm::Value * lhs() override {
        return ins_->getOperand(0);
    }

    llvm::Value * rhs() override {
        return ins_->getOperand(1);
    }

    llvm::Instruction * first() const override {
        return ir::Pattern::first();
    }

    llvm::Instruction * last() const override {
        return ir::Pattern::last();
    }

protected:
    NativeBinaryOperator(llvm::Instruction * ins, Kind kind):
        ir::Pattern(ins, kind) {
    }
};

/** Floating point addition of two scalars w/o NA handling.
 */
class FAdd : public ir::NativeBinaryOperator {
public:

    static FAdd * create(ir::Builder & b, ir::Value lhs, ir::Value rhs) {
        Sentinel s(b);
        return insertBefore(s, lhs, rhs);
    }

    static FAdd * insertBefore(llvm::Instruction * ins, ir::Value lhs, ir::Value rhs) {
        auto i = llvm::BinaryOperator::Create(llvm::Instruction::FAdd, lhs, rhs, "", ins);
        return new FAdd(i);
    }

    static FAdd * insertBefore(ir::Pattern * p, ir::Value lhs, ir::Value rhs) {
        return insertBefore(p->first(), lhs, rhs);
    }

    static bool classof(Pattern const * s) {
        return s->kind == Kind::FAdd;
    }

protected:
    FAdd(llvm::Instruction * ins):
        ir::NativeBinaryOperator(ins, Kind::FAdd) {
    }
};

/** Floating point subtraction of two scalars w/o NA handling.
 */
class FSub : public ir::NativeBinaryOperator {
public:

    static FSub * create(ir::Builder & b, ir::Value lhs, ir::Value rhs) {
        Sentinel s(b);
        return insertBefore(s, lhs, rhs);
    }

    static FSub * insertBefore(llvm::Instruction * ins, ir::Value lhs, ir::Value rhs) {
        auto i = llvm::BinaryOperator::Create(llvm::Instruction::FSub, lhs, rhs, "", ins);
        return new FSub(i);
    }

    static FSub * insertBefore(ir::Pattern * p, ir::Value lhs, ir::Value rhs) {
        return insertBefore(p->first(), lhs, rhs);
    }

    static bool classof(Pattern const * s) {
        return s->kind == Kind::FSub;
    }

protected:
    FSub(llvm::Instruction * ins):
        ir::NativeBinaryOperator(ins, Kind::FSub) {
    }
};

/** Floating point multiplication of two scalars w/o NA handling.
 */
class FMul : public ir::NativeBinaryOperator {
public:

    static FMul * create(ir::Builder & b, ir::Value lhs, ir::Value rhs) {
        Sentinel s(b);
        return insertBefore(s, lhs, rhs);
    }

    static FMul * insertBefore(llvm::Instruction * ins, ir::Value lhs, ir::Value rhs) {
        auto i = llvm::BinaryOperator::Create(llvm::Instruction::FMul, lhs, rhs, "", ins);
        return new FMul(i);
    }

    static FMul * insertBefore(ir::Pattern * p, ir::Value lhs, ir::Value rhs) {
        return insertBefore(p->first(), lhs, rhs);
    }
    static bool classof(Pattern const * s) {
        return s->kind == Kind::FMul;
    }

protected:
    FMul(llvm::Instruction * ins):
        ir::NativeBinaryOperator(ins, Kind::FMul) {
    }
};

/** Floating point division of two scalars w/o NA handling.
 */
class FDiv : public ir::NativeBinaryOperator {
public:

    static FDiv * create(ir::Builder & b, ir::Value lhs, ir::Value rhs) {
        Sentinel s(b);
        return insertBefore(s, lhs, rhs);
    }

    static FDiv * insertBefore(llvm::Instruction * ins, ir::Value lhs, ir::Value rhs) {
        auto i = llvm::BinaryOperator::Create(llvm::Instruction::FDiv, lhs, rhs, "", ins);
        return new FDiv(i);
    }

    static FDiv * insertBefore(ir::Pattern * p, ir::Value lhs, ir::Value rhs) {
        return insertBefore(p->first(), lhs, rhs);
    }

    static bool classof(Pattern const * s) {
        return s->kind == Kind::FDiv;
    }

protected:
    FDiv(llvm::Instruction * ins):
        ir::NativeBinaryOperator(ins, Kind::FDiv) {
    }
};

/** Integer addition of two scalars without NA handling.
 */
class Add : public ir::NativeBinaryOperator {
public:

    static Add * create(ir::Builder & b, ir::Value lhs, ir::Value rhs) {
        Sentinel s(b);
        return insertBefore(s, lhs, rhs);
    }

    static Add * insertBefore(llvm::Instruction * ins, ir::Value lhs, ir::Value rhs) {
        auto i = llvm::BinaryOperator::Create(llvm::Instruction::Add, lhs, rhs, "", ins);
        return new Add(i);
    }

    static Add * insertBefore(ir::Pattern * ins, ir::Value lhs, ir::Value rhs) {
        return insertBefore(ins->first(), lhs, rhs);
    }

    static bool classof(Pattern const * s) {
        return s->kind == Kind::Add;
    }

protected:
    Add(llvm::Instruction * ins):
        ir::NativeBinaryOperator(ins, Kind::Add) {
    }

};

/** Integer subtraction of two scalars without NA handling.
 */
class Sub : public ir::NativeBinaryOperator {
public:

    static Sub * create(ir::Builder & b, ir::Value lhs, ir::Value rhs) {
        Sentinel s(b);
        return insertBefore(s, lhs, rhs);
    }

    static Sub * insertBefore(llvm::Instruction * ins, ir::Value lhs, ir::Value rhs) {
        auto i = llvm::BinaryOperator::Create(llvm::Instruction::Sub, lhs, rhs, "", ins);
        return new Sub(i);
    }

    static Sub * insertBefore(ir::Pattern * p, ir::Value lhs, ir::Value rhs) {
        return insertBefore(p->first(), lhs, rhs);
    }

    static bool classof(Pattern const * s) {
        return s->kind == Kind::Sub;
    }

protected:
    Sub(llvm::Instruction * ins):
        ir::NativeBinaryOperator(ins, Kind::Sub) {
    }

};

/** Integer multiplication of two scalars without NA handling.
 */
class Mul : public ir::NativeBinaryOperator {
public:

    static Mul * create(ir::Builder & b, ir::Value lhs, ir::Value rhs) {
        Sentinel s(b);
        return insertBefore(s, lhs, rhs);
    }

    static Mul * insertBefore(llvm::Instruction * ins, ir::Value lhs, ir::Value rhs) {
        auto i = llvm::BinaryOperator::Create(llvm::Instruction::Mul, lhs, rhs, "", ins);
        return new Mul(i);
    }

    static Mul * insertBefore(ir::Pattern * p, ir::Value lhs, ir::Value rhs) {
        return insertBefore(p->first(), lhs, rhs);
    }

    static bool classof(Pattern const * s) {
        return s->kind == Kind::Mul;
    }

protected:
    Mul(llvm::Instruction * ins):
        ir::NativeBinaryOperator(ins, Kind::Mul) {
    }

};

/** Integer division of two scalars without NA handling.
 */
class Div : public ir::NativeBinaryOperator {
public:

    static Div * create(ir::Builder & b, ir::Value lhs, ir::Value rhs) {
        Sentinel s(b);
        return insertBefore(s, lhs, rhs);
    }

    static Div * insertBefore(llvm::Instruction * ins, ir::Value lhs, ir::Value rhs) {
        auto i = llvm::BinaryOperator::Create(llvm::Instruction::SDiv, lhs, rhs, "", ins);
        return new Div(i);
    }

    static Div * insertBefore(ir::Pattern * p, ir::Value lhs, ir::Value rhs) {
        return insertBefore(p->first(), lhs, rhs);
    }

    static bool classof(Pattern const * s) {
        return s->kind == Kind::Div;
    }

protected:
    Div(llvm::Instruction * ins):
        ir::NativeBinaryOperator(ins, Kind::Div) {
    }

};

/** NA check for double scalars. Calls to R_IsNA internally.
 */
class FIsNA : public ir::PrimitiveCall {
public:
    llvm::Value * value() {
        return getValue(0);
    }

    static FIsNA * create(ir::Builder & b, ir::Value value) {
        Sentinel s(b);
        return insertBefore(s, value);
    }

    static FIsNA * insertBefore(llvm::Instruction * ins, ir::Value value) {
        llvm::CallInst * i = llvm::CallInst::Create(primitiveFunction<FIsNA>(ins->getModule()), { value }, "", ins);
        Builder::markSafepoint(i);
        return new FIsNA(i);
    }

    static FIsNA * insertBefore(Pattern * p, ir::Value value) {
        return insertBefore(p->first(), value);
    }

    static char const * intrinsicName() {
        return "R_IsNA";
    }

    static llvm::FunctionType * intrinsicType() {
        return llvm::FunctionType::get(
            t::Bool, { t::Double }, false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == Kind::FIsNA;
    }

protected:

    FIsNA(llvm::Instruction * ins):
        ir::PrimitiveCall(ins, Kind::FIsNA) {
    }
};

/** NA check for integer scalars.

  Compares to R's value for integer NA, which is min int.

 */
class IIsNA : public ir::Pattern {
public:

    static IIsNA * create(ir::Builder & b, ir::Value value) {
        Sentinel s(b);
        return insertBefore(s, value);
    }

    static IIsNA * insertBefore(llvm::Instruction * ins, ir::Value value) {
        auto i = new ICmpInst(ins, ICmpInst::ICMP_EQ, value, Builder::integer(INT_MIN));
        return new IIsNA(i);
    }

    static IIsNA * insertBefore(Pattern * p, ir::Value value) {
        return insertBefore(p->first(), value);
    }

    static bool classof(Pattern const * s) {
        return s->kind == Kind::IIsNA;
    }

protected:

    IIsNA(llvm::Instruction * ins):
        ir::Pattern(ins, Kind::IIsNA) {
    }
};

} // namespace ir
} // namespace rjit

#endif // IR_IR_SCALARS_H
