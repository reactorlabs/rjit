#ifndef INTRINSICS_H_
#define INTRINSICS_H_

#include "Ir.h"
#include "Builder.h"

namespace rjit {
namespace ir {


class InitClosureContext : public PrimitiveCall {
  public:

    llvm::Value* cntxt() { return getArgument(0); }
    llvm::Value* call() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* sysparen() { return getArgument(3); }

    InitClosureContext (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::InitClosureContext, ins) { }

    static InitClosureContext & Create(
            Builder & b,
            llvm::Value* cntxt,
            llvm::Value* call,
            llvm::Value* rho,
            llvm::Value* sysparen) {

        std::vector<llvm::Value*> args_;
        args_.push_back(cntxt);
        args_.push_back(call);
        args_.push_back(rho);
        args_.push_back(sysparen);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<InitClosureContext>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        InitClosureContext * result = new InitClosureContext(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "initClosureContext";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Void,
            {
                t::cntxt, t::SEXP, t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::InitClosureContext;
    }
};

class EndClosureContext : public PrimitiveCall {
  public:

    llvm::Value* cntxt() { return getArgument(0); }
    llvm::Value* resul() { return getArgument(1); }

    EndClosureContext (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::EndClosureContext, ins) { }

    static EndClosureContext & Create(
            Builder & b,
            llvm::Value* cntxt,
            llvm::Value* resul) {

        std::vector<llvm::Value*> args_;
        args_.push_back(cntxt);
        args_.push_back(resul);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<EndClosureContext>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        EndClosureContext * result = new EndClosureContext(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "endClosureContext";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Void,
            {
                t::cntxt, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::EndClosureContext;
    }
};

class ClosureQuickArgumentAdaptor : public PrimitiveCall {
  public:

    llvm::Value* op() { return getArgument(0); }
    llvm::Value* arglis() { return getArgument(1); }

    ClosureQuickArgumentAdaptor (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::ClosureQuickArgumentAdaptor, ins) { }

    static ClosureQuickArgumentAdaptor & Create(
            Builder & b,
            llvm::Value* op,
            llvm::Value* arglis) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(arglis);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<ClosureQuickArgumentAdaptor>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        ClosureQuickArgumentAdaptor * result = new ClosureQuickArgumentAdaptor(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "closureQuickArgumentAdaptor";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::ClosureQuickArgumentAdaptor;
    }
};

class CallNative : public PrimitiveCall {
  public:

    llvm::Value* native() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }

    CallNative (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::CallNative, ins) { }

    static CallNative & Create(
            Builder & b,
            llvm::Value* native,
            llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(native);
        args_.push_back(rho);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<CallNative>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        CallNative * result = new CallNative(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "callNative";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::CallNative;
    }
};

class ClosureNativeCallTrampoline : public PrimitiveCall {
  public:

    llvm::Value* cntxt() { return getArgument(0); }
    llvm::Value* native() { return getArgument(1); }
    llvm::Value* rh() { return getArgument(2); }

    ClosureNativeCallTrampoline (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::ClosureNativeCallTrampoline, ins) { }

    static ClosureNativeCallTrampoline & Create(
            Builder & b,
            llvm::Value* cntxt,
            llvm::Value* native,
            llvm::Value* rh) {

        std::vector<llvm::Value*> args_;
        args_.push_back(cntxt);
        args_.push_back(native);
        args_.push_back(rh);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<ClosureNativeCallTrampoline>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        ClosureNativeCallTrampoline * result = new ClosureNativeCallTrampoline(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "closureNativeCallTrampoline";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::cntxt, t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::ClosureNativeCallTrampoline;
    }
};

// Replacement for GETSTACK_LOGICAL_NO_NA_PTR The call is used only for
// error reporting.
class ConvertToLogicalNoNA : public PrimitiveCall {
  public:

    llvm::Value* what() { return getArgument(0); }
    llvm::Value* constantPool() { return getArgument(1); }
    
    int call() { return getArgumentInt(2); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    ConvertToLogicalNoNA (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::ConvertToLogicalNoNA, ins) { }

    static ConvertToLogicalNoNA & Create(
            Builder & b,
            llvm::Value* what,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(what);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<ConvertToLogicalNoNA>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        ConvertToLogicalNoNA * result = new ConvertToLogicalNoNA(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "convertToLogicalNoNA";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Int,
            {
                t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::ConvertToLogicalNoNA;
    }
};

class PrintValue : public PrimitiveCall {
  public:

    llvm::Value* value() { return getArgument(0); }

    PrintValue (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::PrintValue, ins) { }

    static PrintValue & Create(
            Builder & b,
            llvm::Value* value) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<PrintValue>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        PrintValue * result = new PrintValue(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "printValue";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Void,
            {
                t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::PrintValue;
    }
};

// startFor returns the sequence over which the loop will iterate. No
// need for all the other things here because we do not support other
// than generic variable loads and stores.
class StartFor : public PrimitiveCall {
  public:

    llvm::Value* seq() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }

    StartFor (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::StartFor, ins) { }

    static StartFor & Create(
            Builder & b,
            llvm::Value* seq,
            llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(seq);
        args_.push_back(rho);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<StartFor>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        StartFor * result = new StartFor(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "startFor";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::StartFor;
    }
};

// Loop sequence length returns the length of the sequence the loop will
// iterate over and errors if the sequence is of wrong type.
class LoopSequenceLength : public PrimitiveCall {
  public:

    llvm::Value* seq() { return getArgument(0); }
    llvm::Value* constantPool() { return getArgument(1); }
    
    int call() { return getArgumentInt(2); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    LoopSequenceLength (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::LoopSequenceLength, ins) { }

    static LoopSequenceLength & Create(
            Builder & b,
            llvm::Value* seq,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(seq);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<LoopSequenceLength>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        LoopSequenceLength * result = new LoopSequenceLength(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "loopSequenceLength";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Int,
            {
                t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::LoopSequenceLength;
    }
};

// Given the for loop sequence, and index, returns the index-th value of
// the sequence. TODO Note that this always allocates for vectors.
class GetForLoopValue : public PrimitiveCall {
  public:

    llvm::Value* seq() { return getArgument(0); }
    llvm::Value* index() { return getArgument(1); }

    GetForLoopValue (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GetForLoopValue, ins) { }

    static GetForLoopValue & Create(
            Builder & b,
            llvm::Value* seq,
            llvm::Value* index) {

        std::vector<llvm::Value*> args_;
        args_.push_back(seq);
        args_.push_back(index);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetForLoopValue>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GetForLoopValue * result = new GetForLoopValue(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "getForLoopValue";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GetForLoopValue;
    }
};

class MarkVisible : public PrimitiveCall {
  public:


    MarkVisible (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::MarkVisible, ins) { }

    static MarkVisible & Create(
            Builder & b) {

        std::vector<llvm::Value*> args_;

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<MarkVisible>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        MarkVisible * result = new MarkVisible(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "markVisible";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Void,
            {
                
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::MarkVisible;
    }
};

class MarkInvisible : public PrimitiveCall {
  public:


    MarkInvisible (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::MarkInvisible, ins) { }

    static MarkInvisible & Create(
            Builder & b) {

        std::vector<llvm::Value*> args_;

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<MarkInvisible>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        MarkInvisible * result = new MarkInvisible(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "markInvisible";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Void,
            {
                
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::MarkInvisible;
    }
};

// When LLVM IR creates user visible constant, this function contains all
// the code required to make the constant. Currently this means taking
// the value from the constant pool and marking it as not mutable.
class UserLiteral : public PrimitiveCall {
  public:

    llvm::Value* constantPool() { return getArgument(0); }
    
    int index() { return getArgumentInt(1); }
    SEXP indexValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), index());
    }
    SEXP index(Builder const & b) { return b.constantPool(index()); }

    UserLiteral (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::UserLiteral, ins) { }

    static UserLiteral & Create(
            Builder & b,
            SEXP index) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(index)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<UserLiteral>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        UserLiteral * result = new UserLiteral(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "userLiteral";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::UserLiteral;
    }
};

// Just returns the index-th constant from the constant pool.
class Constant : public PrimitiveCall {
  public:

    llvm::Value* constantPool() { return getArgument(0); }
    
    int index() { return getArgumentInt(1); }
    SEXP indexValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), index());
    }
    SEXP index(Builder const & b) { return b.constantPool(index()); }

    Constant (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::Constant, ins) { }

    static Constant & Create(
            Builder & b,
            SEXP index) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(index)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<Constant>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        Constant * result = new Constant(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "constant";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::Constant;
    }
};

// Generic getvar does not use any caches whatsoever. TODO this means we
// can get rid of the checks in getvar(), and reduce its code to this. We
// definitely want faster versions.
class GenericGetVar : public PrimitiveCall {
  public:

    llvm::Value* rho() { return getArgument(0); }
    llvm::Value* constantPool() { return getArgument(1); }
    
    int symbol() { return getArgumentInt(2); }
    SEXP symbolValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), symbol());
    }
    SEXP symbol(Builder const & b) { return b.constantPool(symbol()); }

    GenericGetVar (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericGetVar, ins) { }

    static GenericGetVar & Create(
            Builder & b,
            llvm::Value* rho,
            SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericGetVar>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericGetVar * result = new GenericGetVar(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericGetVar";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericGetVar;
    }
};

class GenericGetEllipsisArg : public PrimitiveCall {
  public:

    llvm::Value* rho() { return getArgument(0); }
    llvm::Value* constantPool() { return getArgument(1); }
    
    int symbol() { return getArgumentInt(2); }
    SEXP symbolValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), symbol());
    }
    SEXP symbol(Builder const & b) { return b.constantPool(symbol()); }

    GenericGetEllipsisArg (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericGetEllipsisArg, ins) { }

    static GenericGetEllipsisArg & Create(
            Builder & b,
            llvm::Value* rho,
            SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericGetEllipsisArg>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericGetEllipsisArg * result = new GenericGetEllipsisArg(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericGetEllipsisArg";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericGetEllipsisArg;
    }
};

class GenericSetVar : public PrimitiveCall {
  public:

    llvm::Value* value() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }
    llvm::Value* constantPool() { return getArgument(2); }
    
    int symbol() { return getArgumentInt(3); }
    SEXP symbolValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), symbol());
    }
    SEXP symbol(Builder const & b) { return b.constantPool(symbol()); }

    GenericSetVar (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericSetVar, ins) { }

    static GenericSetVar & Create(
            Builder & b,
            llvm::Value* value,
            llvm::Value* rho,
            SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericSetVar>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericSetVar * result = new GenericSetVar(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericSetVar";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Void,
            {
                t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericSetVar;
    }
};

class GenericSetVarParent : public PrimitiveCall {
  public:

    llvm::Value* value() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }
    llvm::Value* constantPool() { return getArgument(2); }
    
    int symbol() { return getArgumentInt(3); }
    SEXP symbolValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), symbol());
    }
    SEXP symbol(Builder const & b) { return b.constantPool(symbol()); }

    GenericSetVarParent (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericSetVarParent, ins) { }

    static GenericSetVarParent & Create(
            Builder & b,
            llvm::Value* value,
            llvm::Value* rho,
            SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericSetVarParent>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericSetVarParent * result = new GenericSetVarParent(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericSetVarParent";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Void,
            {
                t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericSetVarParent;
    }
};

class GetFunction : public PrimitiveCall {
  public:

    llvm::Value* rho() { return getArgument(0); }
    llvm::Value* constantPool() { return getArgument(1); }
    
    int symbol() { return getArgumentInt(2); }
    SEXP symbolValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), symbol());
    }
    SEXP symbol(Builder const & b) { return b.constantPool(symbol()); }

    GetFunction (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GetFunction, ins) { }

    static GetFunction & Create(
            Builder & b,
            llvm::Value* rho,
            SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetFunction>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GetFunction * result = new GetFunction(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "getFunction";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GetFunction;
    }
};

class GetGlobalFunction : public PrimitiveCall {
  public:

    llvm::Value* constantPool() { return getArgument(0); }
    
    int symbol() { return getArgumentInt(1); }
    SEXP symbolValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), symbol());
    }
    SEXP symbol(Builder const & b) { return b.constantPool(symbol()); }

    GetGlobalFunction (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GetGlobalFunction, ins) { }

    static GetGlobalFunction & Create(
            Builder & b,
            SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetGlobalFunction>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GetGlobalFunction * result = new GetGlobalFunction(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "getGlobalFunction";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GetGlobalFunction;
    }
};

class GetSymFunction : public PrimitiveCall {
  public:

    llvm::Value* constantPool() { return getArgument(0); }
    
    int name() { return getArgumentInt(1); }
    SEXP nameValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), name());
    }
    SEXP name(Builder const & b) { return b.constantPool(name()); }

    GetSymFunction (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GetSymFunction, ins) { }

    static GetSymFunction & Create(
            Builder & b,
            SEXP name) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(name)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetSymFunction>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GetSymFunction * result = new GetSymFunction(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "getSymFunction";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GetSymFunction;
    }
};

class GetBuiltinFunction : public PrimitiveCall {
  public:

    llvm::Value* constantPool() { return getArgument(0); }
    
    int name() { return getArgumentInt(1); }
    SEXP nameValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), name());
    }
    SEXP name(Builder const & b) { return b.constantPool(name()); }

    GetBuiltinFunction (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GetBuiltinFunction, ins) { }

    static GetBuiltinFunction & Create(
            Builder & b,
            SEXP name) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(name)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetBuiltinFunction>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GetBuiltinFunction * result = new GetBuiltinFunction(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "getBuiltinFunction";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GetBuiltinFunction;
    }
};

class GetInternalBuiltinFunction : public PrimitiveCall {
  public:

    llvm::Value* constantPool() { return getArgument(0); }
    
    int name() { return getArgumentInt(1); }
    SEXP nameValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), name());
    }
    SEXP name(Builder const & b) { return b.constantPool(name()); }

    GetInternalBuiltinFunction (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GetInternalBuiltinFunction, ins) { }

    static GetInternalBuiltinFunction & Create(
            Builder & b,
            SEXP name) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(name)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetInternalBuiltinFunction>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GetInternalBuiltinFunction * result = new GetInternalBuiltinFunction(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "getInternalBuiltinFunction";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GetInternalBuiltinFunction;
    }
};

class CheckFunction : public PrimitiveCall {
  public:

    llvm::Value* f() { return getArgument(0); }

    CheckFunction (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::CheckFunction, ins) { }

    static CheckFunction & Create(
            Builder & b,
            llvm::Value* f) {

        std::vector<llvm::Value*> args_;
        args_.push_back(f);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<CheckFunction>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        CheckFunction * result = new CheckFunction(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "checkFunction";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Void,
            {
                t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::CheckFunction;
    }
};

// Creates a promise out of the given code and environment and returns
// its value.
class CreatePromise : public PrimitiveCall {
  public:

    llvm::Value* fun() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }

    CreatePromise (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::CreatePromise, ins) { }

    static CreatePromise & Create(
            Builder & b,
            llvm::Value* fun,
            llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(fun);
        args_.push_back(rho);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<CreatePromise>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        CreatePromise * result = new CreatePromise(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "createPromise";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::CreatePromise;
    }
};

// Given a SEXP, returns its type. We can perfectly do this in LLVM, but
// having an function for it simplifies the analysis on our end.
class SexpType : public PrimitiveCall {
  public:

    llvm::Value* value() { return getArgument(0); }

    SexpType (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::SexpType, ins) { }

    static SexpType & Create(
            Builder & b,
            llvm::Value* value) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<SexpType>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        SexpType * result = new SexpType(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "sexpType";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Int,
            {
                t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::SexpType;
    }
};

class AddArgument : public PrimitiveCall {
  public:

    llvm::Value* args() { return getArgument(0); }
    llvm::Value* arg() { return getArgument(1); }

    AddArgument (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::AddArgument, ins) { }

    static AddArgument & Create(
            Builder & b,
            llvm::Value* args,
            llvm::Value* arg) {

        std::vector<llvm::Value*> args_;
        args_.push_back(args);
        args_.push_back(arg);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<AddArgument>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        AddArgument * result = new AddArgument(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "addArgument";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::AddArgument;
    }
};

class AddKeywordArgument : public PrimitiveCall {
  public:

    llvm::Value* args() { return getArgument(0); }
    llvm::Value* arg() { return getArgument(1); }
    llvm::Value* name() { return getArgument(2); }

    AddKeywordArgument (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::AddKeywordArgument, ins) { }

    static AddKeywordArgument & Create(
            Builder & b,
            llvm::Value* args,
            llvm::Value* arg,
            llvm::Value* name) {

        std::vector<llvm::Value*> args_;
        args_.push_back(args);
        args_.push_back(arg);
        args_.push_back(name);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<AddKeywordArgument>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        AddKeywordArgument * result = new AddKeywordArgument(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "addKeywordArgument";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::AddKeywordArgument;
    }
};

class AddEllipsisArgumentHead : public PrimitiveCall {
  public:

    llvm::Value* args() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }
    llvm::Value* eager() { return getArgument(2); }

    AddEllipsisArgumentHead (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::AddEllipsisArgumentHead, ins) { }

    static AddEllipsisArgumentHead & Create(
            Builder & b,
            llvm::Value* args,
            llvm::Value* rho,
            llvm::Value* eager) {

        std::vector<llvm::Value*> args_;
        args_.push_back(args);
        args_.push_back(rho);
        args_.push_back(eager);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<AddEllipsisArgumentHead>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        AddEllipsisArgumentHead * result = new AddEllipsisArgumentHead(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "addEllipsisArgumentHead";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::Bool
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::AddEllipsisArgumentHead;
    }
};

class AddEllipsisArgumentTail : public PrimitiveCall {
  public:

    llvm::Value* args() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }
    llvm::Value* eager() { return getArgument(2); }

    AddEllipsisArgumentTail (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::AddEllipsisArgumentTail, ins) { }

    static AddEllipsisArgumentTail & Create(
            Builder & b,
            llvm::Value* args,
            llvm::Value* rho,
            llvm::Value* eager) {

        std::vector<llvm::Value*> args_;
        args_.push_back(args);
        args_.push_back(rho);
        args_.push_back(eager);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<AddEllipsisArgumentTail>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        AddEllipsisArgumentTail * result = new AddEllipsisArgumentTail(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "addEllipsisArgumentTail";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::Bool
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::AddEllipsisArgumentTail;
    }
};

class CallBuiltin : public PrimitiveCall {
  public:

    llvm::Value* call() { return getArgument(0); }
    llvm::Value* closure() { return getArgument(1); }
    llvm::Value* arguments() { return getArgument(2); }
    llvm::Value* rho() { return getArgument(3); }

    CallBuiltin (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::CallBuiltin, ins) { }

    static CallBuiltin & Create(
            Builder & b,
            llvm::Value* call,
            llvm::Value* closure,
            llvm::Value* arguments,
            llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(call);
        args_.push_back(closure);
        args_.push_back(arguments);
        args_.push_back(rho);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<CallBuiltin>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        CallBuiltin * result = new CallBuiltin(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "callBuiltin";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::CallBuiltin;
    }
};

class CallSpecial : public PrimitiveCall {
  public:

    llvm::Value* call() { return getArgument(0); }
    llvm::Value* closure() { return getArgument(1); }
    llvm::Value* arguments() { return getArgument(2); }
    llvm::Value* rho() { return getArgument(3); }

    CallSpecial (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::CallSpecial, ins) { }

    static CallSpecial & Create(
            Builder & b,
            llvm::Value* call,
            llvm::Value* closure,
            llvm::Value* arguments,
            llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(call);
        args_.push_back(closure);
        args_.push_back(arguments);
        args_.push_back(rho);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<CallSpecial>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        CallSpecial * result = new CallSpecial(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "callSpecial";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::CallSpecial;
    }
};

class CallClosure : public PrimitiveCall {
  public:

    llvm::Value* call() { return getArgument(0); }
    llvm::Value* closure() { return getArgument(1); }
    llvm::Value* arguments() { return getArgument(2); }
    llvm::Value* rho() { return getArgument(3); }

    CallClosure (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::CallClosure, ins) { }

    static CallClosure & Create(
            Builder & b,
            llvm::Value* call,
            llvm::Value* closure,
            llvm::Value* arguments,
            llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(call);
        args_.push_back(closure);
        args_.push_back(arguments);
        args_.push_back(rho);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<CallClosure>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        CallClosure * result = new CallClosure(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "callClosure";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::CallClosure;
    }
};

class CreateClosure : public PrimitiveCall {
  public:

    llvm::Value* rho() { return getArgument(0); }
    llvm::Value* constantPool() { return getArgument(1); }
    
    int forms() { return getArgumentInt(2); }
    SEXP formsValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), forms());
    }
    SEXP forms(Builder const & b) { return b.constantPool(forms()); }
    
    int body() { return getArgumentInt(3); }
    SEXP bodyValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), body());
    }
    SEXP body(Builder const & b) { return b.constantPool(body()); }

    CreateClosure (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::CreateClosure, ins) { }

    static CreateClosure & Create(
            Builder & b,
            llvm::Value* rho,
            SEXP forms,
            SEXP body) {

        std::vector<llvm::Value*> args_;
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(forms)));
        args_.push_back(Builder::integer(b.constantPoolIndex(body)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<CreateClosure>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        CreateClosure * result = new CreateClosure(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "createClosure";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::Int, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::CreateClosure;
    }
};

class GenericUnaryMinus : public PrimitiveCall {
  public:

    llvm::Value* op() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }
    llvm::Value* constantPool() { return getArgument(2); }
    
    int call() { return getArgumentInt(3); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericUnaryMinus (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericUnaryMinus, ins) { }

    static GenericUnaryMinus & Create(
            Builder & b,
            llvm::Value* op,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericUnaryMinus>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericUnaryMinus * result = new GenericUnaryMinus(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericUnaryMinus";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericUnaryMinus;
    }
};

class GenericUnaryPlus : public PrimitiveCall {
  public:

    llvm::Value* op() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }
    llvm::Value* constantPool() { return getArgument(2); }
    
    int call() { return getArgumentInt(3); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericUnaryPlus (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericUnaryPlus, ins) { }

    static GenericUnaryPlus & Create(
            Builder & b,
            llvm::Value* op,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericUnaryPlus>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericUnaryPlus * result = new GenericUnaryPlus(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericUnaryPlus";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericUnaryPlus;
    }
};

class GenericAdd : public PrimitiveCall {
  public:

    llvm::Value* lhs() { return getArgument(0); }
    llvm::Value* rhs() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* constantPool() { return getArgument(3); }
    
    int call() { return getArgumentInt(4); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericAdd (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericAdd, ins) { }

    static GenericAdd & Create(
            Builder & b,
            llvm::Value* lhs,
            llvm::Value* rhs,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericAdd>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericAdd * result = new GenericAdd(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericAdd";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericAdd;
    }
};

class GenericSub : public PrimitiveCall {
  public:

    llvm::Value* lhs() { return getArgument(0); }
    llvm::Value* rhs() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* constantPool() { return getArgument(3); }
    
    int call() { return getArgumentInt(4); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericSub (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericSub, ins) { }

    static GenericSub & Create(
            Builder & b,
            llvm::Value* lhs,
            llvm::Value* rhs,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericSub>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericSub * result = new GenericSub(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericSub";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericSub;
    }
};

class GenericMul : public PrimitiveCall {
  public:

    llvm::Value* lhs() { return getArgument(0); }
    llvm::Value* rhs() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* constantPool() { return getArgument(3); }
    
    int call() { return getArgumentInt(4); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericMul (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericMul, ins) { }

    static GenericMul & Create(
            Builder & b,
            llvm::Value* lhs,
            llvm::Value* rhs,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericMul>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericMul * result = new GenericMul(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericMul";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericMul;
    }
};

class GenericDiv : public PrimitiveCall {
  public:

    llvm::Value* lhs() { return getArgument(0); }
    llvm::Value* rhs() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* constantPool() { return getArgument(3); }
    
    int call() { return getArgumentInt(4); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericDiv (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericDiv, ins) { }

    static GenericDiv & Create(
            Builder & b,
            llvm::Value* lhs,
            llvm::Value* rhs,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericDiv>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericDiv * result = new GenericDiv(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericDiv";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericDiv;
    }
};

class GenericPow : public PrimitiveCall {
  public:

    llvm::Value* lhs() { return getArgument(0); }
    llvm::Value* rhs() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* constantPool() { return getArgument(3); }
    
    int call() { return getArgumentInt(4); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericPow (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericPow, ins) { }

    static GenericPow & Create(
            Builder & b,
            llvm::Value* lhs,
            llvm::Value* rhs,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericPow>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericPow * result = new GenericPow(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericPow";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericPow;
    }
};

class GenericSqrt : public PrimitiveCall {
  public:

    llvm::Value* op() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }
    llvm::Value* constantPool() { return getArgument(2); }
    
    int call() { return getArgumentInt(3); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericSqrt (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericSqrt, ins) { }

    static GenericSqrt & Create(
            Builder & b,
            llvm::Value* op,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericSqrt>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericSqrt * result = new GenericSqrt(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericSqrt";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericSqrt;
    }
};

class GenericExp : public PrimitiveCall {
  public:

    llvm::Value* op() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }
    llvm::Value* constantPool() { return getArgument(2); }
    
    int call() { return getArgumentInt(3); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericExp (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericExp, ins) { }

    static GenericExp & Create(
            Builder & b,
            llvm::Value* op,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericExp>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericExp * result = new GenericExp(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericExp";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericExp;
    }
};

class GenericEq : public PrimitiveCall {
  public:

    llvm::Value* lhs() { return getArgument(0); }
    llvm::Value* rhs() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* constantPool() { return getArgument(3); }
    
    int call() { return getArgumentInt(4); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericEq (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericEq, ins) { }

    static GenericEq & Create(
            Builder & b,
            llvm::Value* lhs,
            llvm::Value* rhs,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericEq>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericEq * result = new GenericEq(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericEq";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericEq;
    }
};

class GenericNe : public PrimitiveCall {
  public:

    llvm::Value* lhs() { return getArgument(0); }
    llvm::Value* rhs() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* constantPool() { return getArgument(3); }
    
    int call() { return getArgumentInt(4); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericNe (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericNe, ins) { }

    static GenericNe & Create(
            Builder & b,
            llvm::Value* lhs,
            llvm::Value* rhs,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericNe>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericNe * result = new GenericNe(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericNe";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericNe;
    }
};

class GenericLt : public PrimitiveCall {
  public:

    llvm::Value* lhs() { return getArgument(0); }
    llvm::Value* rhs() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* constantPool() { return getArgument(3); }
    
    int call() { return getArgumentInt(4); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericLt (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericLt, ins) { }

    static GenericLt & Create(
            Builder & b,
            llvm::Value* lhs,
            llvm::Value* rhs,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericLt>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericLt * result = new GenericLt(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericLt";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericLt;
    }
};

class GenericLe : public PrimitiveCall {
  public:

    llvm::Value* lhs() { return getArgument(0); }
    llvm::Value* rhs() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* constantPool() { return getArgument(3); }
    
    int call() { return getArgumentInt(4); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericLe (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericLe, ins) { }

    static GenericLe & Create(
            Builder & b,
            llvm::Value* lhs,
            llvm::Value* rhs,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericLe>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericLe * result = new GenericLe(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericLe";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericLe;
    }
};

class GenericGe : public PrimitiveCall {
  public:

    llvm::Value* lhs() { return getArgument(0); }
    llvm::Value* rhs() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* constantPool() { return getArgument(3); }
    
    int call() { return getArgumentInt(4); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericGe (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericGe, ins) { }

    static GenericGe & Create(
            Builder & b,
            llvm::Value* lhs,
            llvm::Value* rhs,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericGe>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericGe * result = new GenericGe(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericGe";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericGe;
    }
};

class GenericGt : public PrimitiveCall {
  public:

    llvm::Value* lhs() { return getArgument(0); }
    llvm::Value* rhs() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* constantPool() { return getArgument(3); }
    
    int call() { return getArgumentInt(4); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericGt (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericGt, ins) { }

    static GenericGt & Create(
            Builder & b,
            llvm::Value* lhs,
            llvm::Value* rhs,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericGt>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericGt * result = new GenericGt(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericGt";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericGt;
    }
};

class GenericBitAnd : public PrimitiveCall {
  public:

    llvm::Value* lhs() { return getArgument(0); }
    llvm::Value* rhs() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* constantPool() { return getArgument(3); }
    
    int call() { return getArgumentInt(4); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericBitAnd (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericBitAnd, ins) { }

    static GenericBitAnd & Create(
            Builder & b,
            llvm::Value* lhs,
            llvm::Value* rhs,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericBitAnd>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericBitAnd * result = new GenericBitAnd(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericBitAnd";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericBitAnd;
    }
};

class GenericBitOr : public PrimitiveCall {
  public:

    llvm::Value* lhs() { return getArgument(0); }
    llvm::Value* rhs() { return getArgument(1); }
    llvm::Value* rho() { return getArgument(2); }
    llvm::Value* constantPool() { return getArgument(3); }
    
    int call() { return getArgumentInt(4); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericBitOr (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericBitOr, ins) { }

    static GenericBitOr & Create(
            Builder & b,
            llvm::Value* lhs,
            llvm::Value* rhs,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericBitOr>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericBitOr * result = new GenericBitOr(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericBitOr";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericBitOr;
    }
};

class GenericNot : public PrimitiveCall {
  public:

    llvm::Value* op() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }
    llvm::Value* constantPool() { return getArgument(2); }
    
    int call() { return getArgumentInt(3); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    GenericNot (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericNot, ins) { }

    static GenericNot & Create(
            Builder & b,
            llvm::Value* op,
            llvm::Value* rho,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericNot>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericNot * result = new GenericNot(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericNot";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericNot;
    }
};

class GenericGetVarMissOK : public PrimitiveCall {
  public:

    llvm::Value* symbol() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }

    GenericGetVarMissOK (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericGetVarMissOK, ins) { }

    static GenericGetVarMissOK & Create(
            Builder & b,
            llvm::Value* symbol,
            llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(symbol);
        args_.push_back(rho);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericGetVarMissOK>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericGetVarMissOK * result = new GenericGetVarMissOK(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericGetVarMissOK";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericGetVarMissOK;
    }
};

class GenericGetEllipsisValueMissOK : public PrimitiveCall {
  public:

    llvm::Value* symbol() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }

    GenericGetEllipsisValueMissOK (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::GenericGetEllipsisValueMissOK, ins) { }

    static GenericGetEllipsisValueMissOK & Create(
            Builder & b,
            llvm::Value* symbol,
            llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(symbol);
        args_.push_back(rho);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericGetEllipsisValueMissOK>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        GenericGetEllipsisValueMissOK * result = new GenericGetEllipsisValueMissOK(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "genericGetEllipsisValueMissOK";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::SEXP,
            {
                t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::GenericGetEllipsisValueMissOK;
    }
};

class CheckSwitchControl : public PrimitiveCall {
  public:

    llvm::Value* ctrl() { return getArgument(0); }
    llvm::Value* constantPool() { return getArgument(1); }
    
    int call() { return getArgumentInt(2); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }

    CheckSwitchControl (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::CheckSwitchControl, ins) { }

    static CheckSwitchControl & Create(
            Builder & b,
            llvm::Value* ctrl,
            SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(ctrl);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<CheckSwitchControl>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        CheckSwitchControl * result = new CheckSwitchControl(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "checkSwitchControl";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Void,
            {
                t::SEXP, t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::CheckSwitchControl;
    }
};

class SwitchControlCharacter : public PrimitiveCall {
  public:

    llvm::Value* ctrl() { return getArgument(0); }
    llvm::Value* constantPool() { return getArgument(1); }
    
    int call() { return getArgumentInt(2); }
    SEXP callValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const & b) { return b.constantPool(call()); }
    
    int cases() { return getArgumentInt(3); }
    SEXP casesValue() {
        llvm::Function * f = callInst()->getParent()->getParent();
        JITModule * m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), cases());
    }
    SEXP cases(Builder const & b) { return b.constantPool(cases()); }

    SwitchControlCharacter (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::SwitchControlCharacter, ins) { }

    static SwitchControlCharacter & Create(
            Builder & b,
            llvm::Value* ctrl,
            SEXP call,
            SEXP cases) {

        std::vector<llvm::Value*> args_;
        args_.push_back(ctrl);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));
        args_.push_back(Builder::integer(b.constantPoolIndex(cases)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<SwitchControlCharacter>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        SwitchControlCharacter * result = new SwitchControlCharacter(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "switchControlCharacter";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Int,
            {
                t::SEXP, t::SEXP, t::Int, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::SwitchControlCharacter;
    }
};

class SwitchControlInteger : public PrimitiveCall {
  public:

    llvm::Value* ctrl() { return getArgument(0); }
    int numCases() { return getArgumentInt(1); }

    SwitchControlInteger (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::SwitchControlInteger, ins) { }

    static SwitchControlInteger & Create(
            Builder & b,
            llvm::Value* ctrl,
            int numCases) {

        std::vector<llvm::Value*> args_;
        args_.push_back(ctrl);
        args_.push_back(Builder::integer(numCases));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<SwitchControlInteger>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        SwitchControlInteger * result = new SwitchControlInteger(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "switchControlInteger";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Int,
            {
                t::SEXP, t::Int
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::SwitchControlInteger;
    }
};

class ReturnJump : public PrimitiveCall {
  public:

    llvm::Value* value() { return getArgument(0); }
    llvm::Value* rho() { return getArgument(1); }

    ReturnJump (llvm::CallInst* ins) :
        PrimitiveCall(PatternKind::ReturnJump, ins) { }

    static ReturnJump & Create(
            Builder & b,
            llvm::Value* value,
            llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);
        args_.push_back(rho);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<ReturnJump>(),
            args_,
            "",
            b);

        b.insertCall(ins);
        ReturnJump * result = new ReturnJump(ins);
        result->attachTo(ins);
        return *result;
    }

    static char const* primitiveName() {
        return "returnJump";
    }

    static llvm::FunctionType* primitiveType() {
        return llvm::FunctionType::get(
            t::Void,
            {
                t::SEXP, t::SEXP
            },
            false);
    }

    static bool classof(Pattern const * s) {
        return s->kind == PatternKind::ReturnJump;
    }
};

} // namespace ir
} // namespace rjit
#endif  // INTRINSICS_H_
