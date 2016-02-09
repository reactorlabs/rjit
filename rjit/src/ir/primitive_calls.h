#ifndef INTRINSICS_H_
#define INTRINSICS_H_

#include "Ir.h"
#include "Builder.h"

namespace rjit {
namespace ir {

class EndClosureContext : public PrimitiveCall {
  public:
    llvm::Value* cntxt() { return getValue(0); }
    llvm::Value* resul() { return getValue(1); }

    EndClosureContext(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::EndClosureContext) {}

    static EndClosureContext* create(Builder& b, llvm::Value* cntxt, llvm::Value* result) {
        llvm::CallInst* ins = llvm::CallInst::Create(b.intrinsic<EndClosureContext>(), arguments(cntxt, result), "", b);
        b.markSafepoint(ins);
        return new EndClosureContext(ins);
    }

    static char const* intrinsicName() { return "endClosureContext"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {t::cntxtPtr, t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::EndClosureContext;
    }

private:
    static std::vector<llvm::Value *> arguments(llvm::Value * cntxt, llvm::Value * result) {
        std::vector<llvm::Value*> args_;
        args_.push_back(cntxt);
        args_.push_back(result);
        return args_;
    }

};

class ClosureQuickArgumentAdaptor : public PrimitiveCall {
  public:
    llvm::Value* op() { return getValue(0); }
    llvm::Value* arglis() { return getValue(1); }

    ClosureQuickArgumentAdaptor(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::ClosureQuickArgumentAdaptor) {}

    static ClosureQuickArgumentAdaptor* create(Builder& b, llvm::Value* op,
                                               llvm::Value* arglis) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(arglis);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<ClosureQuickArgumentAdaptor>(), args_, "", b);

        b.markSafepoint(ins);
        ClosureQuickArgumentAdaptor* result =
            new ClosureQuickArgumentAdaptor(ins);
        return result;
    }

    static char const* intrinsicName() { return "closureQuickArgumentAdaptor"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::ClosureQuickArgumentAdaptor;
    }
};

class CallNative : public PrimitiveCall {
  public:
    llvm::Value* native() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }

    CallNative(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::CallNative) {}

    static CallNative* create(Builder& b, llvm::Value* native,
                              llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(native);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CallNative>(), args_, "", b);

        b.markSafepoint(ins);
        CallNative* result = new CallNative(ins);
        return result;
    }

    static char const* intrinsicName() { return "callNative"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::CallNative;
    }
};

class ClosureNativeCallTrampoline : public PrimitiveCall {
  public:
    llvm::Value* cntxt() { return getValue(0); }
    llvm::Value* native() { return getValue(1); }
    llvm::Value* rh() { return getValue(2); }

    ClosureNativeCallTrampoline(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::ClosureNativeCallTrampoline) {}

    static ClosureNativeCallTrampoline* create(Builder& b, llvm::Value* cntxt,
                                               llvm::Value* native,
                                               llvm::Value* rh) {

        std::vector<llvm::Value*> args_;
        args_.push_back(cntxt);
        args_.push_back(native);
        args_.push_back(rh);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<ClosureNativeCallTrampoline>(), args_, "", b);

        ClosureNativeCallTrampoline* result =
            new ClosureNativeCallTrampoline(ins);
        return result;
    }

    static char const* intrinsicName() { return "closureNativeCallTrampoline"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::cntxtPtr, t::SEXP, t::SEXP},
                                       false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::ClosureNativeCallTrampoline;
    }
};

// Replacement for GETSTACK_LOGICAL_NO_NA_PTR The call is used only for
// error reporting.
class ConvertToLogicalNoNA : public PrimitiveCall {
  public:
    llvm::Value* what() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int call() { return getValueInt(2); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    ConvertToLogicalNoNA(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::ConvertToLogicalNoNA) {}

    static ConvertToLogicalNoNA* create(Builder& b, llvm::Value* what,
                                        SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(what);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<ConvertToLogicalNoNA>(), args_, "", b);

        b.markSafepoint(ins);
        ConvertToLogicalNoNA* result = new ConvertToLogicalNoNA(ins);
        return result;
    }

    static char const* intrinsicName() { return "convertToLogicalNoNA"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Int, {t::SEXP, t::SEXP, t::Int},
                                       false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::ConvertToLogicalNoNA;
    }
};

class PrintValue : public PrimitiveCall {
  public:
    llvm::Value* value() { return getValue(0); }

    PrintValue(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::PrintValue) {}

    static PrintValue* create(Builder& b, llvm::Value* value) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<PrintValue>(), args_, "", b);

        b.markSafepoint(ins);
        PrintValue* result = new PrintValue(ins);
        return result;
    }

    static char const* intrinsicName() { return "printValue"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::PrintValue;
    }
};

// startFor returns the sequence over which the loop will iterate. No
// need for all the other things here because we do not support other
// than generic variable loads and stores.
class StartFor : public PrimitiveCall {
  public:
    llvm::Value* seq() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }

    StartFor(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::StartFor) {}

    static StartFor* create(Builder& b, llvm::Value* seq, llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(seq);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<StartFor>(), args_, "", b);

        b.markSafepoint(ins);
        StartFor* result = new StartFor(ins);
        return result;
    }

    static char const* intrinsicName() { return "startFor"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::StartFor;
    }
};

// Loop sequence length returns the length of the sequence the loop will
// iterate over and errors if the sequence is of wrong type.
class LoopSequenceLength : public PrimitiveCall {
  public:
    llvm::Value* seq() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int call() { return getValueInt(2); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    LoopSequenceLength(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::LoopSequenceLength) {}

    static LoopSequenceLength* create(Builder& b, llvm::Value* seq, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(seq);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<LoopSequenceLength>(), args_, "", b);

        b.markSafepoint(ins);
        LoopSequenceLength* result = new LoopSequenceLength(ins);
        return result;
    }

    static char const* intrinsicName() { return "loopSequenceLength"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Int, {t::SEXP, t::SEXP, t::Int},
                                       false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::LoopSequenceLength;
    }
};

// Given the for loop sequence, and index, returns the index-th value of
// the sequence. TODO Note that this always allocates for vectors.
class GetForLoopValue : public PrimitiveCall {
  public:
    llvm::Value* seq() { return getValue(0); }
    llvm::Value* index() { return getValue(1); }

    GetForLoopValue(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GetForLoopValue) {}

    static GetForLoopValue* create(Builder& b, llvm::Value* seq,
                                   llvm::Value* index) {

        std::vector<llvm::Value*> args_;
        args_.push_back(seq);
        args_.push_back(index);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetForLoopValue>(), args_, "", b);

        b.markSafepoint(ins);
        GetForLoopValue* result = new GetForLoopValue(ins);
        return result;
    }

    static char const* intrinsicName() { return "getForLoopValue"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GetForLoopValue;
    }
};

class MarkVisible : public PrimitiveCall {
  public:
    MarkVisible(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::MarkVisible) {}

    static MarkVisible* create(Builder& b) {

        std::vector<llvm::Value*> args_;

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<MarkVisible>(), args_, "", b);

        b.markSafepoint(ins);
        MarkVisible* result = new MarkVisible(ins);
        return result;
    }

    static char const* intrinsicName() { return "markVisible"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {

                                                }, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::MarkVisible;
    }
};

class MarkInvisible : public PrimitiveCall {
  public:
    MarkInvisible(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::MarkInvisible) {}

    static MarkInvisible* create(Builder& b) {

        std::vector<llvm::Value*> args_;

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<MarkInvisible>(), args_, "", b);

        b.markSafepoint(ins);
        MarkInvisible* result = new MarkInvisible(ins);
        return result;
    }

    static char const* intrinsicName() { return "markInvisible"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {

                                                }, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::MarkInvisible;
    }
};

// When LLVM IR creates user visible constant, this function contains all
// the code required to make the constant. Currently this means taking
// the value from the constant pool and marking it as not mutable.
class UserLiteral : public PrimitiveCall {
  public:
    llvm::Value* constantPool() { return getValue(0); }

    int index() { return getValueInt(1); }
    SEXP indexValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), index());
    }
    SEXP index(Builder const& b) { return b.constantPool(index()); }

    UserLiteral(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::UserLiteral) {}

    static UserLiteral* create(Builder& b, SEXP index) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(index)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<UserLiteral>(), args_, "", b);

        b.markSafepoint(ins);
        UserLiteral* result = new UserLiteral(ins);
        return result;
    }

    static char const* intrinsicName() { return "userLiteral"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::UserLiteral;
    }
};

class PatchIC : public PrimitiveCall {
  public:
    PatchIC(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::PatchIC) {}

    static PatchIC* create(Builder& b, llvm::Value* addr,
                           llvm::Value* stackmapId, llvm::Value* caller) {

        std::vector<llvm::Value*> args_;
        args_.push_back(addr);
        args_.push_back(stackmapId);
        args_.push_back(caller);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<PatchIC>(), args_, "", b);

        b.markSafepoint(ins);
        PatchIC* result = new PatchIC(ins);
        return result;
    }

    static char const* intrinsicName() { return "patchIC"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::Void, {t::voidPtr, t::t_i64, t::nativeFunctionPtr_t}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::PatchIC;
    }
};

class CompileIC : public PrimitiveCall {
  public:
    CompileIC(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::CompileIC) {}

    static CompileIC* create(Builder& b, llvm::Value* size, llvm::Value* call,
                             llvm::Value* fun, llvm::Value* rho,
                             llvm::Value* stackmapId) {

        std::vector<llvm::Value*> args_;
        args_.push_back(size);
        args_.push_back(call);
        args_.push_back(fun);
        args_.push_back(rho);
        args_.push_back(stackmapId);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CompileIC>(), args_, "", b);

        b.markSafepoint(ins);
        CompileIC* result = new CompileIC(ins);
        return result;
    }

    static char const* intrinsicName() { return "compileIC"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::voidPtr, {t::t_i64, t::SEXP, t::SEXP, t::SEXP, t::t_i64}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::CompileIC;
    }
};

class InitClosureContext : public PrimitiveCall {
  public:
    InitClosureContext(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::InitClosureContext) {}

    static InitClosureContext* create(Builder& b, llvm::Value* context,
                                      llvm::Value* call, llvm::Value* newrho,
                                      llvm::Value* rho, llvm::Value* actuals,
                                      llvm::Value* fun) {

        std::vector<llvm::Value*> args_;
        args_.push_back(context);
        args_.push_back(call);
        args_.push_back(newrho);
        args_.push_back(rho);
        args_.push_back(actuals);
        args_.push_back(fun);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<InitClosureContext>(), args_, "", b);

        b.markSafepoint(ins);
        InitClosureContext* result = new InitClosureContext(ins);
        return result;
    }

    static char const* intrinsicName() { return "initClosureContext"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::Void, {t::cntxtPtr, t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::SEXP},
            false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::InitClosureContext;
    }
};

// Call NewEnvironment
class NewEnv : public PrimitiveCall {
  public:
    NewEnv(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::NewEnv) {}

    static NewEnv* create(Builder& b, llvm::Value* names, llvm::Value* values,
                          llvm::Value* parent) {

        std::vector<llvm::Value*> args_;
        args_.push_back(names);
        args_.push_back(values);
        args_.push_back(parent);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<NewEnv>(), args_, "", b);

        b.markSafepoint(ins);
        NewEnv* result = new NewEnv(ins);
        return result;
    }

    static char const* intrinsicName() { return "Rf_NewEnvironment"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP, t::SEXP},
                                       false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::NewEnv;
    }
};

// Call CONS_NR
class ConsNr : public PrimitiveCall {
  public:
    ConsNr(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::ConsNr) {}

    static ConsNr* create(Builder& b, llvm::Value* car, llvm::Value* cdr) {

        std::vector<llvm::Value*> args_;
        args_.push_back(car);
        args_.push_back(cdr);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<ConsNr>(), args_, "", b);

        b.markSafepoint(ins);
        ConsNr* result = new ConsNr(ins);
        return result;
    }

    static char const* intrinsicName() { return "CONS_NR"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::ConsNr;
    }
};

// Just returns the index-th constant from the constant pool.
class Constant : public PrimitiveCall {
  public:
    llvm::Value* constantPool() { return getValue(0); }

    int index() { return getValueInt(1); }
    SEXP indexValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), index());
    }
    SEXP index(Builder const& b) { return b.constantPool(index()); }

    Constant(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::Constant) {}

    static Constant* create(Builder& b, SEXP index) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(index)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<Constant>(), args_, "", b);

        b.markSafepoint(ins);
        Constant* result = new Constant(ins);
        return result;
    }

    static char const* intrinsicName() { return "constant"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::Constant;
    }
};

// Generic getvar does not use any caches whatsoever. TODO this means we
// can get rid of the checks in getvar(), and reduce its code to this. We
// definitely want faster versions.
class GenericGetVar : public PrimitiveCall {
  public:
    llvm::Value* rho() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int symbol() { return getValueInt(2); }
    SEXP symbolValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), symbol());
    }
    SEXP symbol(Builder const& b) { return b.constantPool(symbol()); }

    GenericGetVar(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GenericGetVar) {}

    static GenericGetVar* create(Builder& b, llvm::Value* rho, SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericGetVar>(), args_, "", b);

        b.markSafepoint(ins);
        GenericGetVar* result = new GenericGetVar(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericGetVar"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP, t::Int},
                                       false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericGetVar;
    }
};

class GenericGetEllipsisArg : public PrimitiveCall {
  public:
    llvm::Value* rho() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int symbol() { return getValueInt(2); }
    SEXP symbolValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), symbol());
    }
    SEXP symbol(Builder const& b) { return b.constantPool(symbol()); }

    GenericGetEllipsisArg(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GenericGetEllipsisArg) {}

    static GenericGetEllipsisArg* create(Builder& b, llvm::Value* rho,
                                         SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericGetEllipsisArg>(), args_, "", b);

        b.markSafepoint(ins);
        GenericGetEllipsisArg* result = new GenericGetEllipsisArg(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericGetEllipsisArg"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP, t::Int},
                                       false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericGetEllipsisArg;
    }
};

class GenericSetVar : public PrimitiveCall {
  public:
    llvm::Value* value() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int symbol() { return getValueInt(3); }
    SEXP symbolValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), symbol());
    }
    SEXP symbol(Builder const& b) { return b.constantPool(symbol()); }

    GenericSetVar(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GenericSetVar) {}

    static GenericSetVar* create(Builder& b, llvm::Value* value,
                                 llvm::Value* rho, SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericSetVar>(), args_, "", b);

        b.markSafepoint(ins);
        GenericSetVar* result = new GenericSetVar(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericSetVar"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::Void, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericSetVar;
    }
};

class GenericSetVarParent : public PrimitiveCall {
  public:
    llvm::Value* value() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int symbol() { return getValueInt(3); }
    SEXP symbolValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), symbol());
    }
    SEXP symbol(Builder const& b) { return b.constantPool(symbol()); }

    GenericSetVarParent(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GenericSetVarParent) {}

    static GenericSetVarParent* create(Builder& b, llvm::Value* value,
                                       llvm::Value* rho, SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericSetVarParent>(), args_, "", b);

        b.markSafepoint(ins);
        GenericSetVarParent* result = new GenericSetVarParent(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericSetVarParent"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::Void, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericSetVarParent;
    }
};

class GetFunction : public PrimitiveCall {
  public:
    llvm::Value* rho() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int symbol() { return getValueInt(2); }
    SEXP symbolValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), symbol());
    }
    SEXP symbol(Builder const& b) { return b.constantPool(symbol()); }

    GetFunction(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GetFunction) {}

    static GetFunction* create(Builder& b, llvm::Value* rho, SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GetFunction>(), args_, "", b);

        b.markSafepoint(ins);
        GetFunction* result = new GetFunction(ins);
        return result;
    }

    static char const* intrinsicName() { return "getFunction"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP, t::Int},
                                       false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GetFunction;
    }
};

class GetGlobalFunction : public PrimitiveCall {
  public:
    llvm::Value* constantPool() { return getValue(0); }

    int symbol() { return getValueInt(1); }
    SEXP symbolValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), symbol());
    }
    SEXP symbol(Builder const& b) { return b.constantPool(symbol()); }

    GetGlobalFunction(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GetGlobalFunction) {}

    static GetGlobalFunction* create(Builder& b, SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetGlobalFunction>(), args_, "", b);

        b.markSafepoint(ins);
        GetGlobalFunction* result = new GetGlobalFunction(ins);
        return result;
    }

    static char const* intrinsicName() { return "getGlobalFunction"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GetGlobalFunction;
    }
};

class GetSymFunction : public PrimitiveCall {
  public:
    llvm::Value* constantPool() { return getValue(0); }

    int name() { return getValueInt(1); }
    SEXP nameValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), name());
    }
    SEXP name(Builder const& b) { return b.constantPool(name()); }

    GetSymFunction(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GetSymFunction) {}

    static GetSymFunction* create(Builder& b, SEXP name) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(name)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GetSymFunction>(), args_, "", b);

        b.markSafepoint(ins);
        GetSymFunction* result = new GetSymFunction(ins);
        return result;
    }

    static char const* intrinsicName() { return "getSymFunction"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GetSymFunction;
    }
};

class GetBuiltinFunction : public PrimitiveCall {
  public:
    llvm::Value* constantPool() { return getValue(0); }

    int name() { return getValueInt(1); }
    SEXP nameValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), name());
    }
    SEXP name(Builder const& b) { return b.constantPool(name()); }

    GetBuiltinFunction(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GetBuiltinFunction) {}

    static GetBuiltinFunction* create(Builder& b, SEXP name) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(name)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetBuiltinFunction>(), args_, "", b);

        b.markSafepoint(ins);
        GetBuiltinFunction* result = new GetBuiltinFunction(ins);
        return result;
    }

    static char const* intrinsicName() { return "getBuiltinFunction"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GetBuiltinFunction;
    }
};

class GetInternalBuiltinFunction : public PrimitiveCall {
  public:
    llvm::Value* constantPool() { return getValue(0); }

    int name() { return getValueInt(1); }
    SEXP nameValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), name());
    }
    SEXP name(Builder const& b) { return b.constantPool(name()); }

    GetInternalBuiltinFunction(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GetInternalBuiltinFunction) {}

    static GetInternalBuiltinFunction* create(Builder& b, SEXP name) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(name)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetInternalBuiltinFunction>(), args_, "", b);

        b.markSafepoint(ins);
        GetInternalBuiltinFunction* result =
            new GetInternalBuiltinFunction(ins);
        return result;
    }

    static char const* intrinsicName() { return "getInternalBuiltinFunction"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GetInternalBuiltinFunction;
    }
};

class CheckFunction : public PrimitiveCall {
  public:
    llvm::Value* f() { return getValue(0); }

    CheckFunction(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::CheckFunction) {}

    static CheckFunction* create(Builder& b, llvm::Value* f) {

        std::vector<llvm::Value*> args_;
        args_.push_back(f);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CheckFunction>(), args_, "", b);

        b.markSafepoint(ins);
        CheckFunction* result = new CheckFunction(ins);
        return result;
    }

    static char const* intrinsicName() { return "checkFunction"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::CheckFunction;
    }
};

// Creates a promise out of the given code and environment and returns
// its value.
class CreatePromise : public PrimitiveCall {
  public:
    llvm::Value* fun() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }

    CreatePromise(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::CreatePromise) {}

    static CreatePromise* create(Builder& b, llvm::Value* fun,
                                 llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(fun);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CreatePromise>(), args_, "", b);

        b.markSafepoint(ins);
        CreatePromise* result = new CreatePromise(ins);
        return result;
    }

    static char const* intrinsicName() { return "createPromise"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::CreatePromise;
    }
};

// Given a SEXP, returns its type. We can perfectly do this in LLVM, but
// having an function for it simplifies the analysis on our end.
class SexpType : public PrimitiveCall {
  public:
    llvm::Value* value() { return getValue(0); }

    SexpType(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::SexpType) {}

    static SexpType* create(Builder& b, llvm::Value* value) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<SexpType>(), args_, "", b);

        b.markSafepoint(ins);
        SexpType* result = new SexpType(ins);
        return result;
    }

    static char const* intrinsicName() { return "sexpType"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Int, {t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::SexpType;
    }
};

class AddArgument : public PrimitiveCall {
  public:
    llvm::Value* args() { return getValue(0); }
    llvm::Value* arg() { return getValue(1); }

    AddArgument(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::AddArgument) {}

    static AddArgument* create(Builder& b, llvm::Value* args,
                               llvm::Value* arg) {

        std::vector<llvm::Value*> args_;
        args_.push_back(args);
        args_.push_back(arg);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<AddArgument>(), args_, "", b);

        b.markSafepoint(ins);
        AddArgument* result = new AddArgument(ins);
        return result;
    }

    static char const* intrinsicName() { return "addArgument"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::AddArgument;
    }
};

class AddKeywordArgument : public PrimitiveCall {
  public:
    llvm::Value* args() { return getValue(0); }
    llvm::Value* arg() { return getValue(1); }
    llvm::Value* name() { return getValue(2); }

    AddKeywordArgument(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::AddKeywordArgument) {}

    static AddKeywordArgument* create(Builder& b, llvm::Value* args,
                                      llvm::Value* arg, llvm::Value* name) {

        std::vector<llvm::Value*> args_;
        args_.push_back(args);
        args_.push_back(arg);
        args_.push_back(name);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<AddKeywordArgument>(), args_, "", b);

        b.markSafepoint(ins);
        AddKeywordArgument* result = new AddKeywordArgument(ins);
        return result;
    }

    static char const* intrinsicName() { return "addKeywordArgument"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP, t::SEXP},
                                       false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::AddKeywordArgument;
    }
};

class AddEllipsisArgumentHead : public PrimitiveCall {
  public:
    llvm::Value* args() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* eager() { return getValue(2); }

    AddEllipsisArgumentHead(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::AddEllipsisArgumentHead) {}

    static AddEllipsisArgumentHead* create(Builder& b, llvm::Value* args,
                                           llvm::Value* rho,
                                           llvm::Value* eager) {

        std::vector<llvm::Value*> args_;
        args_.push_back(args);
        args_.push_back(rho);
        args_.push_back(eager);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<AddEllipsisArgumentHead>(), args_, "", b);

        b.markSafepoint(ins);
        AddEllipsisArgumentHead* result = new AddEllipsisArgumentHead(ins);
        return result;
    }

    static char const* intrinsicName() { return "addEllipsisArgumentHead"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP, t::Bool},
                                       false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::AddEllipsisArgumentHead;
    }
};

class AddEllipsisArgumentTail : public PrimitiveCall {
  public:
    llvm::Value* args() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* eager() { return getValue(2); }

    AddEllipsisArgumentTail(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::AddEllipsisArgumentTail) {}

    static AddEllipsisArgumentTail* create(Builder& b, llvm::Value* args,
                                           llvm::Value* rho,
                                           llvm::Value* eager) {

        std::vector<llvm::Value*> args_;
        args_.push_back(args);
        args_.push_back(rho);
        args_.push_back(eager);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<AddEllipsisArgumentTail>(), args_, "", b);

        b.markSafepoint(ins);
        AddEllipsisArgumentTail* result = new AddEllipsisArgumentTail(ins);
        return result;
    }

    static char const* intrinsicName() { return "addEllipsisArgumentTail"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP, t::Bool},
                                       false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::AddEllipsisArgumentTail;
    }
};

class CallBuiltin : public PrimitiveCall {
  public:
    llvm::Value* call() { return getValue(0); }
    llvm::Value* closure() { return getValue(1); }
    llvm::Value* arguments() { return getValue(2); }
    llvm::Value* rho() { return getValue(3); }

    CallBuiltin(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::CallBuiltin) {}

    static CallBuiltin* create(Builder& b, llvm::Value* call,
                               llvm::Value* closure, llvm::Value* arguments,
                               llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(call);
        args_.push_back(closure);
        args_.push_back(arguments);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CallBuiltin>(), args_, "", b);

        b.markSafepoint(ins);
        CallBuiltin* result = new CallBuiltin(ins);
        return result;
    }

    static char const* intrinsicName() { return "callBuiltin"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::CallBuiltin;
    }
};

class CallSpecial : public PrimitiveCall {
  public:
    llvm::Value* call() { return getValue(0); }
    llvm::Value* closure() { return getValue(1); }
    llvm::Value* arguments() { return getValue(2); }
    llvm::Value* rho() { return getValue(3); }

    CallSpecial(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::CallSpecial) {}

    static CallSpecial* create(Builder& b, llvm::Value* call,
                               llvm::Value* closure, llvm::Value* arguments,
                               llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(call);
        args_.push_back(closure);
        args_.push_back(arguments);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CallSpecial>(), args_, "", b);

        b.markSafepoint(ins);
        CallSpecial* result = new CallSpecial(ins);
        return result;
    }

    static char const* intrinsicName() { return "callSpecial"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::CallSpecial;
    }
};

class CallClosure : public PrimitiveCall {
  public:
    llvm::Value* call() { return getValue(0); }
    llvm::Value* closure() { return getValue(1); }
    llvm::Value* arguments() { return getValue(2); }
    llvm::Value* rho() { return getValue(3); }

    CallClosure(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::CallClosure) {}

    static CallClosure* create(Builder& b, llvm::Value* call,
                               llvm::Value* closure, llvm::Value* arguments,
                               llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(call);
        args_.push_back(closure);
        args_.push_back(arguments);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CallClosure>(), args_, "", b);

        b.markSafepoint(ins);
        CallClosure* result = new CallClosure(ins);
        return result;
    }

    static char const* intrinsicName() { return "callClosure"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::CallClosure;
    }
};

class CreateClosure : public PrimitiveCall {
  public:
    llvm::Value* rho() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int forms() { return getValueInt(2); }
    SEXP formsValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), forms());
    }
    SEXP forms(Builder const& b) { return b.constantPool(forms()); }

    int body() { return getValueInt(3); }
    SEXP bodyValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), body());
    }
    SEXP body(Builder const& b) { return b.constantPool(body()); }

    CreateClosure(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::CreateClosure) {}

    static CreateClosure* create(Builder& b, llvm::Value* rho, SEXP forms,
                                 SEXP body) {

        std::vector<llvm::Value*> args_;
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(forms)));
        args_.push_back(Builder::integer(b.constantPoolIndex(body)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CreateClosure>(), args_, "", b);

        b.markSafepoint(ins);
        CreateClosure* result = new CreateClosure(ins);
        return result;
    }

    static char const* intrinsicName() { return "createClosure"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::Int, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::CreateClosure;
    }
};

class GenericUnaryMinus : public PrimitiveCall {
  public:
    llvm::Value* op() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int call() { return getValueInt(3); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericUnaryMinus(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GenericUnaryMinus) {}

    static GenericUnaryMinus* create(Builder& b, llvm::Value* op,
                                     llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericUnaryMinus>(), args_, "", b);

        b.markSafepoint(ins);
        GenericUnaryMinus* result = new GenericUnaryMinus(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericUnaryMinus"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericUnaryMinus;
    }
};

class GenericUnaryPlus : public PrimitiveCall {
  public:
    llvm::Value* op() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int call() { return getValueInt(3); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericUnaryPlus(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GenericUnaryPlus) {}

    static GenericUnaryPlus* create(Builder& b, llvm::Value* op,
                                    llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericUnaryPlus>(), args_, "", b);

        b.markSafepoint(ins);
        GenericUnaryPlus* result = new GenericUnaryPlus(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericUnaryPlus"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericUnaryPlus;
    }
};

class GenericAdd : public PrimitiveCall {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericAdd(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::GenericAdd) {}

    static GenericAdd* create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                              llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericAdd>(), args_, "", b);

        b.markSafepoint(ins);
        GenericAdd* result = new GenericAdd(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericAdd"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericAdd;
    }
};

class GenericSub : public PrimitiveCall {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericSub(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::GenericSub) {}

    static GenericSub* create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                              llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericSub>(), args_, "", b);

        b.markSafepoint(ins);
        GenericSub* result = new GenericSub(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericSub"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericSub;
    }
};

class GenericMul : public PrimitiveCall {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericMul(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::GenericMul) {}

    static GenericMul* create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                              llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericMul>(), args_, "", b);

        b.markSafepoint(ins);
        GenericMul* result = new GenericMul(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericMul"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericMul;
    }
};

class GenericDiv : public PrimitiveCall {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericDiv(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::GenericDiv) {}

    static GenericDiv* create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                              llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericDiv>(), args_, "", b);

        b.markSafepoint(ins);
        GenericDiv* result = new GenericDiv(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericDiv"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericDiv;
    }
};

class GenericPow : public PrimitiveCall {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericPow(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::GenericPow) {}

    static GenericPow* create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                              llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericPow>(), args_, "", b);

        b.markSafepoint(ins);
        GenericPow* result = new GenericPow(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericPow"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericPow;
    }
};

class GenericSqrt : public PrimitiveCall {
  public:
    llvm::Value* op() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int call() { return getValueInt(3); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericSqrt(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GenericSqrt) {}

    static GenericSqrt* create(Builder& b, llvm::Value* op, llvm::Value* rho,
                               SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericSqrt>(), args_, "", b);

        b.markSafepoint(ins);
        GenericSqrt* result = new GenericSqrt(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericSqrt"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericSqrt;
    }
};

class GenericExp : public PrimitiveCall {
  public:
    llvm::Value* op() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int call() { return getValueInt(3); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericExp(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::GenericExp) {}

    static GenericExp* create(Builder& b, llvm::Value* op, llvm::Value* rho,
                              SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericExp>(), args_, "", b);

        b.markSafepoint(ins);
        GenericExp* result = new GenericExp(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericExp"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericExp;
    }
};

class GenericEq : public PrimitiveCall {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericEq(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::GenericEq) {}

    static GenericEq* create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                             llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericEq>(), args_, "", b);

        b.markSafepoint(ins);
        GenericEq* result = new GenericEq(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericEq"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericEq;
    }
};

class GenericNe : public PrimitiveCall {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericNe(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::GenericNe) {}

    static GenericNe* create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                             llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericNe>(), args_, "", b);

        b.markSafepoint(ins);
        GenericNe* result = new GenericNe(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericNe"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericNe;
    }
};

class GenericLt : public PrimitiveCall {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericLt(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::GenericLt) {}

    static GenericLt* create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                             llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericLt>(), args_, "", b);

        b.markSafepoint(ins);
        GenericLt* result = new GenericLt(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericLt"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericLt;
    }
};

class GenericLe : public PrimitiveCall {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericLe(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::GenericLe) {}

    static GenericLe* create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                             llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericLe>(), args_, "", b);

        b.markSafepoint(ins);
        GenericLe* result = new GenericLe(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericLe"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericLe;
    }
};

class GenericGe : public PrimitiveCall {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericGe(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::GenericGe) {}

    static GenericGe* create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                             llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericGe>(), args_, "", b);

        b.markSafepoint(ins);
        GenericGe* result = new GenericGe(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericGe"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericGe;
    }
};

class GenericGt : public PrimitiveCall {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericGt(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::GenericGt) {}

    static GenericGt* create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                             llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericGt>(), args_, "", b);

        b.markSafepoint(ins);
        GenericGt* result = new GenericGt(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericGt"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericGt;
    }
};

class GenericBitAnd : public PrimitiveCall {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericBitAnd(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GenericBitAnd) {}

    static GenericBitAnd* create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                                 llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericBitAnd>(), args_, "", b);

        b.markSafepoint(ins);
        GenericBitAnd* result = new GenericBitAnd(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericBitAnd"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericBitAnd;
    }
};

class GenericBitOr : public PrimitiveCall {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericBitOr(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GenericBitOr) {}

    static GenericBitOr* create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                                llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericBitOr>(), args_, "", b);

        b.markSafepoint(ins);
        GenericBitOr* result = new GenericBitOr(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericBitOr"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericBitOr;
    }
};

class GenericNot : public PrimitiveCall {
  public:
    llvm::Value* op() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int call() { return getValueInt(3); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericNot(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::GenericNot) {}

    static GenericNot* create(Builder& b, llvm::Value* op, llvm::Value* rho,
                              SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericNot>(), args_, "", b);

        b.markSafepoint(ins);
        GenericNot* result = new GenericNot(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericNot"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericNot;
    }
};

class GenericGetVarMissOK : public PrimitiveCall {
  public:
    llvm::Value* symbol() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }

    GenericGetVarMissOK(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GenericGetVarMissOK) {}

    static GenericGetVarMissOK* create(Builder& b, llvm::Value* symbol,
                                       llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(symbol);
        args_.push_back(rho);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericGetVarMissOK>(), args_, "", b);

        b.markSafepoint(ins);
        GenericGetVarMissOK* result = new GenericGetVarMissOK(ins);
        return result;
    }

    static char const* intrinsicName() { return "genericGetVarMissOK"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericGetVarMissOK;
    }
};

class GenericGetEllipsisValueMissOK : public PrimitiveCall {
  public:
    llvm::Value* symbol() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }

    GenericGetEllipsisValueMissOK(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::GenericGetEllipsisValueMissOK) {}

    static GenericGetEllipsisValueMissOK*
    create(Builder& b, llvm::Value* symbol, llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(symbol);
        args_.push_back(rho);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericGetEllipsisValueMissOK>(), args_, "", b);

        b.markSafepoint(ins);
        GenericGetEllipsisValueMissOK* result =
            new GenericGetEllipsisValueMissOK(ins);
        return result;
    }

    static char const* intrinsicName() {
        return "genericGetEllipsisValueMissOK";
    }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::GenericGetEllipsisValueMissOK;
    }
};

class CheckSwitchControl : public PrimitiveCall {
  public:
    llvm::Value* ctrl() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int call() { return getValueInt(2); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    CheckSwitchControl(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::CheckSwitchControl) {}

    static CheckSwitchControl* create(Builder& b, llvm::Value* ctrl,
                                      SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(ctrl);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<CheckSwitchControl>(), args_, "", b);

        b.markSafepoint(ins);
        CheckSwitchControl* result = new CheckSwitchControl(ins);
        return result;
    }

    static char const* intrinsicName() { return "checkSwitchControl"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {t::SEXP, t::SEXP, t::Int},
                                       false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::CheckSwitchControl;
    }
};

class SwitchControlCharacter : public PrimitiveCall {
  public:
    llvm::Value* ctrl() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int call() { return getValueInt(2); }
    SEXP callValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), call());
    }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    int cases() { return getValueInt(3); }
    SEXP casesValue() {
        llvm::Function* f = ins()->getParent()->getParent();
        JITModule* m = static_cast<JITModule*>(f->getParent());
        return VECTOR_ELT(m->constPool(f), cases());
    }
    SEXP cases(Builder const& b) { return b.constantPool(cases()); }

    SwitchControlCharacter(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::SwitchControlCharacter) {}

    static SwitchControlCharacter* create(Builder& b, llvm::Value* ctrl,
                                          SEXP call, SEXP cases) {

        std::vector<llvm::Value*> args_;
        args_.push_back(ctrl);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));
        args_.push_back(Builder::integer(b.constantPoolIndex(cases)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<SwitchControlCharacter>(), args_, "", b);

        b.markSafepoint(ins);
        SwitchControlCharacter* result = new SwitchControlCharacter(ins);
        return result;
    }

    static char const* intrinsicName() { return "switchControlCharacter"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::Int, {t::SEXP, t::SEXP, t::Int, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::SwitchControlCharacter;
    }
};

class SwitchControlInteger : public PrimitiveCall {
  public:
    llvm::Value* ctrl() { return getValue(0); }
    int numCases() { return getValueInt(1); }

    SwitchControlInteger(llvm::Instruction* ins)
        : PrimitiveCall(ins, Kind::SwitchControlInteger) {}

    static SwitchControlInteger* create(Builder& b, llvm::Value* ctrl,
                                        int numCases) {

        std::vector<llvm::Value*> args_;
        args_.push_back(ctrl);
        args_.push_back(Builder::integer(numCases));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<SwitchControlInteger>(), args_, "", b);

        b.markSafepoint(ins);
        SwitchControlInteger* result = new SwitchControlInteger(ins);
        return result;
    }

    static char const* intrinsicName() { return "switchControlInteger"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Int, {t::SEXP, t::Int}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::SwitchControlInteger;
    }
};

class ReturnJump : public PrimitiveCall {
  public:
    llvm::Value* value() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }

    ReturnJump(llvm::Instruction* ins) : PrimitiveCall(ins, Kind::ReturnJump) {}

    static ReturnJump* create(Builder& b, llvm::Value* value,
                              llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<ReturnJump>(), args_, "", b);

        b.markSafepoint(ins);
        ReturnJump* result = new ReturnJump(ins);
        return result;
    }

    static char const* intrinsicName() { return "returnJump"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {t::SEXP, t::SEXP}, false);
    }

    static bool classof(Pattern const* s) {
        return s->getKind() == Kind::ReturnJump;
    }
};

} // namespace ir
} // namespace rjit
#endif // INTRINSICS_H_
