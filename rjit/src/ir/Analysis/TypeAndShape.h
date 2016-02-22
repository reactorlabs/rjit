#ifndef ANALYSIS_TYPEANDSHAPE_H
#define ANALYSIS_TYPEANDSHAPE_H

#include "ir/Ir.h"
#include "ir/Pass.h"
#include "ir/PassDriver.h"

#include "TypeInfo.h"

namespace rjit {
namespace analysis {

class TypeAndShapePass : public ir::Fixpoint<ir::AState<TypeInfo>> {
public:
    typedef TypeInfo Value;
    typedef ir::AState<Value> State;

    void constantLoad(SEXP c, ir::Value p) {
        state[p] = TypeInfo(c);
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
            state[dest] = Value(Value::Type::Any);
    }

    /** If we have incomming type & shape information, store it in the variable too. Otherwise do nothing (this means the variable will be assumed Top at read).
     */
    match genericSetVar(ir::GenericSetVar * p) {
        llvm::Value * src = p->value();
        SEXP symbol = p->symbolValue();
        if (state.has(src))
            state[symbol] = state[src];
    }

    /** A call to ICStub invalidates all variables.

      TODO call to ICStub should be its own pattern

     */
    match call(llvm::CallInst * ins) {

    }

    bool dispatch(llvm::BasicBlock::iterator& i) override;
};

class TypeAndShape : public ir::ForwardDriver<TypeAndShapePass> {
public:
    typedef TypeInfo Value;
    typedef ir::AState<Value> State;

} ;




} // namespace analysis
} // namespace rjit



#endif // IR_ANALYSIS_TYPEANDSHAPE_H
