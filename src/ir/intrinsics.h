#ifndef INTRINSICS_H_
#define INTRINSICS_H_

#include "ir.h"
#include "Builder.h"

namespace rjit {
namespace ir {

class InitClosureContext : public Intrinsic {
  public:
    llvm::Value* cntxt() { return getValue(0); }
    llvm::Value* call() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* sysparen() { return getValue(3); }

    InitClosureContext(llvm::Instruction* ins) : Intrinsic(ins) {}

    static InitClosureContext create(Builder& b, llvm::Value* cntxt,
                                     llvm::Value* call, llvm::Value* rho,
                                     llvm::Value* sysparen) {

        std::vector<llvm::Value*> args_;
        args_.push_back(cntxt);
        args_.push_back(call);
        args_.push_back(rho);
        args_.push_back(sysparen);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<InitClosureContext>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::InitClosureContext);
        return ins;
    }

    static char const* intrinsicName() { return "initClosureContext"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::Void, {t::cntxt, t::SEXP, t::SEXP, t::SEXP}, false);
    }
};

class EndClosureContext : public Intrinsic {
  public:
    llvm::Value* cntxt() { return getValue(0); }
    llvm::Value* resul() { return getValue(1); }

    EndClosureContext(llvm::Instruction* ins) : Intrinsic(ins) {}

    static EndClosureContext create(Builder& b, llvm::Value* cntxt,
                                    llvm::Value* resul) {

        std::vector<llvm::Value*> args_;
        args_.push_back(cntxt);
        args_.push_back(resul);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<EndClosureContext>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::EndClosureContext);
        return ins;
    }

    static char const* intrinsicName() { return "endClosureContext"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {t::cntxt, t::SEXP}, false);
    }
};

class ClosureQuickArgumentAdaptor : public Intrinsic {
  public:
    llvm::Value* op() { return getValue(0); }
    llvm::Value* arglis() { return getValue(1); }

    ClosureQuickArgumentAdaptor(llvm::Instruction* ins) : Intrinsic(ins) {}

    static ClosureQuickArgumentAdaptor create(Builder& b, llvm::Value* op,
                                              llvm::Value* arglis) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(arglis);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<ClosureQuickArgumentAdaptor>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::ClosureQuickArgumentAdaptor);
        return ins;
    }

    static char const* intrinsicName() { return "closureQuickArgumentAdaptor"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }
};

class CallNative : public Intrinsic {
  public:
    llvm::Value* native() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }

    CallNative(llvm::Instruction* ins) : Intrinsic(ins) {}

    static CallNative create(Builder& b, llvm::Value* native,
                             llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(native);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CallNative>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::CallNative);
        return ins;
    }

    static char const* intrinsicName() { return "callNative"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }
};

class ClosureNativeCallTrampoline : public Intrinsic {
  public:
    llvm::Value* cntxt() { return getValue(0); }
    llvm::Value* native() { return getValue(1); }
    llvm::Value* rh() { return getValue(2); }

    ClosureNativeCallTrampoline(llvm::Instruction* ins) : Intrinsic(ins) {}

    static ClosureNativeCallTrampoline create(Builder& b, llvm::Value* cntxt,
                                              llvm::Value* native,
                                              llvm::Value* rh) {

        std::vector<llvm::Value*> args_;
        args_.push_back(cntxt);
        args_.push_back(native);
        args_.push_back(rh);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<ClosureNativeCallTrampoline>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::ClosureNativeCallTrampoline);
        return ins;
    }

    static char const* intrinsicName() { return "closureNativeCallTrampoline"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::cntxt, t::SEXP, t::SEXP},
                                       false);
    }
};

// Replacement for GETSTACK_LOGICAL_NO_NA_PTR The call is used only for
// error reporting.
class ConvertToLogicalNoNA : public Intrinsic {
  public:
    llvm::Value* what() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int call() { return getValueInt(2); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    ConvertToLogicalNoNA(llvm::Instruction* ins) : Intrinsic(ins) {}

    static ConvertToLogicalNoNA create(Builder& b, llvm::Value* what,
                                       SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(what);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<ConvertToLogicalNoNA>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::ConvertToLogicalNoNA);
        return ins;
    }

    static char const* intrinsicName() { return "convertToLogicalNoNA"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Int, {t::SEXP, t::SEXP, t::Int},
                                       false);
    }
};

class PrintValue : public Intrinsic {
  public:
    llvm::Value* value() { return getValue(0); }

    PrintValue(llvm::Instruction* ins) : Intrinsic(ins) {}

    static PrintValue create(Builder& b, llvm::Value* value) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<PrintValue>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::PrintValue);
        return ins;
    }

    static char const* intrinsicName() { return "printValue"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {t::SEXP}, false);
    }
};

// startFor returns the sequence over which the loop will iterate. No
// need for all the other things here because we do not support other
// than generic variable loads and stores.
class StartFor : public Intrinsic {
  public:
    llvm::Value* seq() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }

    StartFor(llvm::Instruction* ins) : Intrinsic(ins) {}

    static StartFor create(Builder& b, llvm::Value* seq, llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(seq);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<StartFor>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::StartFor);
        return ins;
    }

    static char const* intrinsicName() { return "startFor"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }
};

// Loop sequence length returns the length of the sequence the loop will
// iterate over and errors if the sequence is of wrong type.
class LoopSequenceLength : public Intrinsic {
  public:
    llvm::Value* seq() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int call() { return getValueInt(2); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    LoopSequenceLength(llvm::Instruction* ins) : Intrinsic(ins) {}

    static LoopSequenceLength create(Builder& b, llvm::Value* seq, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(seq);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<LoopSequenceLength>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::LoopSequenceLength);
        return ins;
    }

    static char const* intrinsicName() { return "loopSequenceLength"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Int, {t::SEXP, t::SEXP, t::Int},
                                       false);
    }
};

// Given the for loop sequence, and index, returns the index-th value of
// the sequence. TODO Note that this always allocates for vectors.
class GetForLoopValue : public Intrinsic {
  public:
    llvm::Value* seq() { return getValue(0); }
    llvm::Value* index() { return getValue(1); }

    GetForLoopValue(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GetForLoopValue create(Builder& b, llvm::Value* seq,
                                  llvm::Value* index) {

        std::vector<llvm::Value*> args_;
        args_.push_back(seq);
        args_.push_back(index);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetForLoopValue>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GetForLoopValue);
        return ins;
    }

    static char const* intrinsicName() { return "getForLoopValue"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }
};

class MarkVisible : public Intrinsic {
  public:
    MarkVisible(llvm::Instruction* ins) : Intrinsic(ins) {}

    static MarkVisible create(Builder& b) {

        std::vector<llvm::Value*> args_;

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<MarkVisible>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::MarkVisible);
        return ins;
    }

    static char const* intrinsicName() { return "markVisible"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {

                                                },
                                       false);
    }
};

class MarkInvisible : public Intrinsic {
  public:
    MarkInvisible(llvm::Instruction* ins) : Intrinsic(ins) {}

    static MarkInvisible create(Builder& b) {

        std::vector<llvm::Value*> args_;

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<MarkInvisible>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::MarkInvisible);
        return ins;
    }

    static char const* intrinsicName() { return "markInvisible"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {

                                                },
                                       false);
    }
};

// When LLVM IR creates user visible constant, this function contains all
// the code required to make the constant. Currently this means taking
// the value from the constant pool and marking it as not mutable.
class UserLiteral : public Intrinsic {
  public:
    llvm::Value* constantPool() { return getValue(0); }

    int index() { return getValueInt(1); }
    SEXP index(SEXP constantPool) { return VECTOR_ELT(constantPool, index()); }
    SEXP index(Builder const& b) { return b.constantPool(index()); }

    UserLiteral(llvm::Instruction* ins) : Intrinsic(ins) {}

    static UserLiteral create(Builder& b, SEXP index) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(index)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<UserLiteral>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::UserLiteral);
        return ins;
    }

    static char const* intrinsicName() { return "userLiteral"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }
};

// Just returns the index-th constant from the constant pool.
class Constant : public Intrinsic {
  public:
    llvm::Value* constantPool() { return getValue(0); }

    int index() { return getValueInt(1); }
    SEXP index(SEXP constantPool) { return VECTOR_ELT(constantPool, index()); }
    SEXP index(Builder const& b) { return b.constantPool(index()); }

    Constant(llvm::Instruction* ins) : Intrinsic(ins) {}

    static Constant create(Builder& b, SEXP index) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(index)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<Constant>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::Constant);
        return ins;
    }

    static char const* intrinsicName() { return "constant"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }
};

// Generic getvar does not use any caches whatsoever. TODO this means we
// can get rid of the checks in getvar(), and reduce its code to this. We
// definitely want faster versions.
class GenericGetVar : public Intrinsic {
  public:
    llvm::Value* rho() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int symbol() { return getValueInt(2); }
    SEXP symbol(SEXP constantPool) {
        return VECTOR_ELT(constantPool, symbol());
    }
    SEXP symbol(Builder const& b) { return b.constantPool(symbol()); }

    GenericGetVar(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericGetVar create(Builder& b, llvm::Value* rho, SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericGetVar>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericGetVar);
        return ins;
    }

    static char const* intrinsicName() { return "genericGetVar"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP, t::Int},
                                       false);
    }
};

class GenericGetEllipsisArg : public Intrinsic {
  public:
    llvm::Value* rho() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int symbol() { return getValueInt(2); }
    SEXP symbol(SEXP constantPool) {
        return VECTOR_ELT(constantPool, symbol());
    }
    SEXP symbol(Builder const& b) { return b.constantPool(symbol()); }

    GenericGetEllipsisArg(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericGetEllipsisArg create(Builder& b, llvm::Value* rho,
                                        SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericGetEllipsisArg>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericGetEllipsisArg);
        return ins;
    }

    static char const* intrinsicName() { return "genericGetEllipsisArg"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP, t::Int},
                                       false);
    }
};

class GenericSetVar : public Intrinsic {
  public:
    llvm::Value* value() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int symbol() { return getValueInt(3); }
    SEXP symbol(SEXP constantPool) {
        return VECTOR_ELT(constantPool, symbol());
    }
    SEXP symbol(Builder const& b) { return b.constantPool(symbol()); }

    GenericSetVar(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericSetVar create(Builder& b, llvm::Value* value,
                                llvm::Value* rho, SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericSetVar>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericSetVar);
        return ins;
    }

    static char const* intrinsicName() { return "genericSetVar"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::Void, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericSetVarParent : public Intrinsic {
  public:
    llvm::Value* value() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int symbol() { return getValueInt(3); }
    SEXP symbol(SEXP constantPool) {
        return VECTOR_ELT(constantPool, symbol());
    }
    SEXP symbol(Builder const& b) { return b.constantPool(symbol()); }

    GenericSetVarParent(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericSetVarParent create(Builder& b, llvm::Value* value,
                                      llvm::Value* rho, SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericSetVarParent>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericSetVarParent);
        return ins;
    }

    static char const* intrinsicName() { return "genericSetVarParent"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::Void, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GetFunction : public Intrinsic {
  public:
    llvm::Value* rho() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int symbol() { return getValueInt(2); }
    SEXP symbol(SEXP constantPool) {
        return VECTOR_ELT(constantPool, symbol());
    }
    SEXP symbol(Builder const& b) { return b.constantPool(symbol()); }

    GetFunction(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GetFunction create(Builder& b, llvm::Value* rho, SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GetFunction>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GetFunction);
        return ins;
    }

    static char const* intrinsicName() { return "getFunction"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP, t::Int},
                                       false);
    }
};

class GetGlobalFunction : public Intrinsic {
  public:
    llvm::Value* constantPool() { return getValue(0); }

    int symbol() { return getValueInt(1); }
    SEXP symbol(SEXP constantPool) {
        return VECTOR_ELT(constantPool, symbol());
    }
    SEXP symbol(Builder const& b) { return b.constantPool(symbol()); }

    GetGlobalFunction(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GetGlobalFunction create(Builder& b, SEXP symbol) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(symbol)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetGlobalFunction>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GetGlobalFunction);
        return ins;
    }

    static char const* intrinsicName() { return "getGlobalFunction"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }
};

class GetSymFunction : public Intrinsic {
  public:
    llvm::Value* constantPool() { return getValue(0); }

    int name() { return getValueInt(1); }
    SEXP name(SEXP constantPool) { return VECTOR_ELT(constantPool, name()); }
    SEXP name(Builder const& b) { return b.constantPool(name()); }

    GetSymFunction(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GetSymFunction create(Builder& b, SEXP name) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(name)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GetSymFunction>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GetSymFunction);
        return ins;
    }

    static char const* intrinsicName() { return "getSymFunction"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }
};

class GetBuiltinFunction : public Intrinsic {
  public:
    llvm::Value* constantPool() { return getValue(0); }

    int name() { return getValueInt(1); }
    SEXP name(SEXP constantPool) { return VECTOR_ELT(constantPool, name()); }
    SEXP name(Builder const& b) { return b.constantPool(name()); }

    GetBuiltinFunction(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GetBuiltinFunction create(Builder& b, SEXP name) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(name)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetBuiltinFunction>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GetBuiltinFunction);
        return ins;
    }

    static char const* intrinsicName() { return "getBuiltinFunction"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }
};

class GetInternalBuiltinFunction : public Intrinsic {
  public:
    llvm::Value* constantPool() { return getValue(0); }

    int name() { return getValueInt(1); }
    SEXP name(SEXP constantPool) { return VECTOR_ELT(constantPool, name()); }
    SEXP name(Builder const& b) { return b.constantPool(name()); }

    GetInternalBuiltinFunction(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GetInternalBuiltinFunction create(Builder& b, SEXP name) {

        std::vector<llvm::Value*> args_;
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(name)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GetInternalBuiltinFunction>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GetInternalBuiltinFunction);
        return ins;
    }

    static char const* intrinsicName() { return "getInternalBuiltinFunction"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::Int}, false);
    }
};

class CheckFunction : public Intrinsic {
  public:
    llvm::Value* f() { return getValue(0); }

    CheckFunction(llvm::Instruction* ins) : Intrinsic(ins) {}

    static CheckFunction create(Builder& b, llvm::Value* f) {

        std::vector<llvm::Value*> args_;
        args_.push_back(f);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CheckFunction>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::CheckFunction);
        return ins;
    }

    static char const* intrinsicName() { return "checkFunction"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {t::SEXP}, false);
    }
};

// Creates a promise out of the given code and environment and returns
// its value.
class CreatePromise : public Intrinsic {
  public:
    llvm::Value* fun() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }

    CreatePromise(llvm::Instruction* ins) : Intrinsic(ins) {}

    static CreatePromise create(Builder& b, llvm::Value* fun,
                                llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(fun);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CreatePromise>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::CreatePromise);
        return ins;
    }

    static char const* intrinsicName() { return "createPromise"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }
};

// Given a SEXP, returns its type. We can perfectly do this in LLVM, but
// having an function for it simplifies the analysis on our end.
class SexpType : public Intrinsic {
  public:
    llvm::Value* value() { return getValue(0); }

    SexpType(llvm::Instruction* ins) : Intrinsic(ins) {}

    static SexpType create(Builder& b, llvm::Value* value) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<SexpType>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::SexpType);
        return ins;
    }

    static char const* intrinsicName() { return "sexpType"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Int, {t::SEXP}, false);
    }
};

class AddArgument : public Intrinsic {
  public:
    llvm::Value* args() { return getValue(0); }
    llvm::Value* arg() { return getValue(1); }

    AddArgument(llvm::Instruction* ins) : Intrinsic(ins) {}

    static AddArgument create(Builder& b, llvm::Value* args, llvm::Value* arg) {

        std::vector<llvm::Value*> args_;
        args_.push_back(args);
        args_.push_back(arg);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<AddArgument>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::AddArgument);
        return ins;
    }

    static char const* intrinsicName() { return "addArgument"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }
};

class AddKeywordArgument : public Intrinsic {
  public:
    llvm::Value* args() { return getValue(0); }
    llvm::Value* arg() { return getValue(1); }
    llvm::Value* name() { return getValue(2); }

    AddKeywordArgument(llvm::Instruction* ins) : Intrinsic(ins) {}

    static AddKeywordArgument create(Builder& b, llvm::Value* args,
                                     llvm::Value* arg, llvm::Value* name) {

        std::vector<llvm::Value*> args_;
        args_.push_back(args);
        args_.push_back(arg);
        args_.push_back(name);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<AddKeywordArgument>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::AddKeywordArgument);
        return ins;
    }

    static char const* intrinsicName() { return "addKeywordArgument"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP, t::SEXP},
                                       false);
    }
};

class AddEllipsisArgumentHead : public Intrinsic {
  public:
    llvm::Value* args() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* eager() { return getValue(2); }

    AddEllipsisArgumentHead(llvm::Instruction* ins) : Intrinsic(ins) {}

    static AddEllipsisArgumentHead create(Builder& b, llvm::Value* args,
                                          llvm::Value* rho,
                                          llvm::Value* eager) {

        std::vector<llvm::Value*> args_;
        args_.push_back(args);
        args_.push_back(rho);
        args_.push_back(eager);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<AddEllipsisArgumentHead>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::AddEllipsisArgumentHead);
        return ins;
    }

    static char const* intrinsicName() { return "addEllipsisArgumentHead"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP, t::Bool},
                                       false);
    }
};

class AddEllipsisArgumentTail : public Intrinsic {
  public:
    llvm::Value* args() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* eager() { return getValue(2); }

    AddEllipsisArgumentTail(llvm::Instruction* ins) : Intrinsic(ins) {}

    static AddEllipsisArgumentTail create(Builder& b, llvm::Value* args,
                                          llvm::Value* rho,
                                          llvm::Value* eager) {

        std::vector<llvm::Value*> args_;
        args_.push_back(args);
        args_.push_back(rho);
        args_.push_back(eager);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<AddEllipsisArgumentTail>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::AddEllipsisArgumentTail);
        return ins;
    }

    static char const* intrinsicName() { return "addEllipsisArgumentTail"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP, t::Bool},
                                       false);
    }
};

class CallBuiltin : public Intrinsic {
  public:
    llvm::Value* call() { return getValue(0); }
    llvm::Value* closure() { return getValue(1); }
    llvm::Value* arguments() { return getValue(2); }
    llvm::Value* rho() { return getValue(3); }

    CallBuiltin(llvm::Instruction* ins) : Intrinsic(ins) {}

    static CallBuiltin create(Builder& b, llvm::Value* call,
                              llvm::Value* closure, llvm::Value* arguments,
                              llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(call);
        args_.push_back(closure);
        args_.push_back(arguments);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CallBuiltin>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::CallBuiltin);
        return ins;
    }

    static char const* intrinsicName() { return "callBuiltin"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP}, false);
    }
};

class CallSpecial : public Intrinsic {
  public:
    llvm::Value* call() { return getValue(0); }
    llvm::Value* closure() { return getValue(1); }
    llvm::Value* arguments() { return getValue(2); }
    llvm::Value* rho() { return getValue(3); }

    CallSpecial(llvm::Instruction* ins) : Intrinsic(ins) {}

    static CallSpecial create(Builder& b, llvm::Value* call,
                              llvm::Value* closure, llvm::Value* arguments,
                              llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(call);
        args_.push_back(closure);
        args_.push_back(arguments);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CallSpecial>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::CallSpecial);
        return ins;
    }

    static char const* intrinsicName() { return "callSpecial"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP}, false);
    }
};

class CallClosure : public Intrinsic {
  public:
    llvm::Value* call() { return getValue(0); }
    llvm::Value* closure() { return getValue(1); }
    llvm::Value* arguments() { return getValue(2); }
    llvm::Value* rho() { return getValue(3); }

    CallClosure(llvm::Instruction* ins) : Intrinsic(ins) {}

    static CallClosure create(Builder& b, llvm::Value* call,
                              llvm::Value* closure, llvm::Value* arguments,
                              llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(call);
        args_.push_back(closure);
        args_.push_back(arguments);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CallClosure>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::CallClosure);
        return ins;
    }

    static char const* intrinsicName() { return "callClosure"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP}, false);
    }
};

class CreateClosure : public Intrinsic {
  public:
    llvm::Value* rho() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int forms() { return getValueInt(2); }
    SEXP forms(SEXP constantPool) { return VECTOR_ELT(constantPool, forms()); }
    SEXP forms(Builder const& b) { return b.constantPool(forms()); }

    int body() { return getValueInt(3); }
    SEXP body(SEXP constantPool) { return VECTOR_ELT(constantPool, body()); }
    SEXP body(Builder const& b) { return b.constantPool(body()); }

    CreateClosure(llvm::Instruction* ins) : Intrinsic(ins) {}

    static CreateClosure create(Builder& b, llvm::Value* rho, SEXP forms,
                                SEXP body) {

        std::vector<llvm::Value*> args_;
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(forms)));
        args_.push_back(Builder::integer(b.constantPoolIndex(body)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<CreateClosure>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::CreateClosure);
        return ins;
    }

    static char const* intrinsicName() { return "createClosure"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::Int, t::Int}, false);
    }
};

class GenericUnaryMinus : public Intrinsic {
  public:
    llvm::Value* op() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int call() { return getValueInt(3); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericUnaryMinus(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericUnaryMinus create(Builder& b, llvm::Value* op,
                                    llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericUnaryMinus>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericUnaryMinus);
        return ins;
    }

    static char const* intrinsicName() { return "genericUnaryMinus"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericUnaryPlus : public Intrinsic {
  public:
    llvm::Value* op() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int call() { return getValueInt(3); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericUnaryPlus(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericUnaryPlus create(Builder& b, llvm::Value* op,
                                   llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericUnaryPlus>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericUnaryPlus);
        return ins;
    }

    static char const* intrinsicName() { return "genericUnaryPlus"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericAdd : public Intrinsic {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericAdd(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericAdd create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                             llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericAdd>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericAdd);
        return ins;
    }

    static char const* intrinsicName() { return "genericAdd"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericSub : public Intrinsic {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericSub(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericSub create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                             llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericSub>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericSub);
        return ins;
    }

    static char const* intrinsicName() { return "genericSub"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericMul : public Intrinsic {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericMul(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericMul create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                             llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericMul>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericMul);
        return ins;
    }

    static char const* intrinsicName() { return "genericMul"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericDiv : public Intrinsic {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericDiv(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericDiv create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                             llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericDiv>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericDiv);
        return ins;
    }

    static char const* intrinsicName() { return "genericDiv"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericPow : public Intrinsic {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericPow(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericPow create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                             llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericPow>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericPow);
        return ins;
    }

    static char const* intrinsicName() { return "genericPow"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericSqrt : public Intrinsic {
  public:
    llvm::Value* op() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int call() { return getValueInt(3); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericSqrt(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericSqrt create(Builder& b, llvm::Value* op, llvm::Value* rho,
                              SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericSqrt>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericSqrt);
        return ins;
    }

    static char const* intrinsicName() { return "genericSqrt"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericExp : public Intrinsic {
  public:
    llvm::Value* op() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int call() { return getValueInt(3); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericExp(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericExp create(Builder& b, llvm::Value* op, llvm::Value* rho,
                             SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericExp>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericExp);
        return ins;
    }

    static char const* intrinsicName() { return "genericExp"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericEq : public Intrinsic {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericEq(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericEq create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                            llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericEq>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericEq);
        return ins;
    }

    static char const* intrinsicName() { return "genericEq"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericNe : public Intrinsic {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericNe(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericNe create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                            llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericNe>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericNe);
        return ins;
    }

    static char const* intrinsicName() { return "genericNe"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericLt : public Intrinsic {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericLt(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericLt create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                            llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericLt>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericLt);
        return ins;
    }

    static char const* intrinsicName() { return "genericLt"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericLe : public Intrinsic {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericLe(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericLe create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                            llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericLe>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericLe);
        return ins;
    }

    static char const* intrinsicName() { return "genericLe"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericGe : public Intrinsic {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericGe(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericGe create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                            llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericGe>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericGe);
        return ins;
    }

    static char const* intrinsicName() { return "genericGe"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericGt : public Intrinsic {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericGt(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericGt create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                            llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericGt>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericGt);
        return ins;
    }

    static char const* intrinsicName() { return "genericGt"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericBitAnd : public Intrinsic {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericBitAnd(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericBitAnd create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                                llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericBitAnd>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericBitAnd);
        return ins;
    }

    static char const* intrinsicName() { return "genericBitAnd"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericBitOr : public Intrinsic {
  public:
    llvm::Value* lhs() { return getValue(0); }
    llvm::Value* rhs() { return getValue(1); }
    llvm::Value* rho() { return getValue(2); }
    llvm::Value* constantPool() { return getValue(3); }

    int call() { return getValueInt(4); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericBitOr(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericBitOr create(Builder& b, llvm::Value* lhs, llvm::Value* rhs,
                               llvm::Value* rho, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(lhs);
        args_.push_back(rhs);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericBitOr>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericBitOr);
        return ins;
    }

    static char const* intrinsicName() { return "genericBitOr"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericNot : public Intrinsic {
  public:
    llvm::Value* op() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }
    llvm::Value* constantPool() { return getValue(2); }

    int call() { return getValueInt(3); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    GenericNot(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericNot create(Builder& b, llvm::Value* op, llvm::Value* rho,
                             SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(op);
        args_.push_back(rho);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<GenericNot>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericNot);
        return ins;
    }

    static char const* intrinsicName() { return "genericNot"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::SEXP, {t::SEXP, t::SEXP, t::SEXP, t::Int}, false);
    }
};

class GenericGetVarMissOK : public Intrinsic {
  public:
    llvm::Value* symbol() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }

    GenericGetVarMissOK(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericGetVarMissOK create(Builder& b, llvm::Value* symbol,
                                      llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(symbol);
        args_.push_back(rho);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericGetVarMissOK>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericGetVarMissOK);
        return ins;
    }

    static char const* intrinsicName() { return "genericGetVarMissOK"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }
};

class GenericGetEllipsisValueMissOK : public Intrinsic {
  public:
    llvm::Value* symbol() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }

    GenericGetEllipsisValueMissOK(llvm::Instruction* ins) : Intrinsic(ins) {}

    static GenericGetEllipsisValueMissOK create(Builder& b, llvm::Value* symbol,
                                                llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(symbol);
        args_.push_back(rho);

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<GenericGetEllipsisValueMissOK>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::GenericGetEllipsisValueMissOK);
        return ins;
    }

    static char const* intrinsicName() {
        return "genericGetEllipsisValueMissOK";
    }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::SEXP, {t::SEXP, t::SEXP}, false);
    }
};

class CheckSwitchControl : public Intrinsic {
  public:
    llvm::Value* ctrl() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int call() { return getValueInt(2); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    CheckSwitchControl(llvm::Instruction* ins) : Intrinsic(ins) {}

    static CheckSwitchControl create(Builder& b, llvm::Value* ctrl, SEXP call) {

        std::vector<llvm::Value*> args_;
        args_.push_back(ctrl);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<CheckSwitchControl>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::CheckSwitchControl);
        return ins;
    }

    static char const* intrinsicName() { return "checkSwitchControl"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {t::SEXP, t::SEXP, t::Int},
                                       false);
    }
};

class SwitchControlCharacter : public Intrinsic {
  public:
    llvm::Value* ctrl() { return getValue(0); }
    llvm::Value* constantPool() { return getValue(1); }

    int call() { return getValueInt(2); }
    SEXP call(SEXP constantPool) { return VECTOR_ELT(constantPool, call()); }
    SEXP call(Builder const& b) { return b.constantPool(call()); }

    int cases() { return getValueInt(3); }
    SEXP cases(SEXP constantPool) { return VECTOR_ELT(constantPool, cases()); }
    SEXP cases(Builder const& b) { return b.constantPool(cases()); }

    SwitchControlCharacter(llvm::Instruction* ins) : Intrinsic(ins) {}

    static SwitchControlCharacter create(Builder& b, llvm::Value* ctrl,
                                         SEXP call, SEXP cases) {

        std::vector<llvm::Value*> args_;
        args_.push_back(ctrl);
        args_.push_back(b.consts());
        args_.push_back(Builder::integer(b.constantPoolIndex(call)));
        args_.push_back(Builder::integer(b.constantPoolIndex(cases)));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<SwitchControlCharacter>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::SwitchControlCharacter);
        return ins;
    }

    static char const* intrinsicName() { return "switchControlCharacter"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(
            t::Int, {t::SEXP, t::SEXP, t::Int, t::Int}, false);
    }
};

class SwitchControlInteger : public Intrinsic {
  public:
    llvm::Value* ctrl() { return getValue(0); }
    int numCases() { return getValueInt(1); }

    SwitchControlInteger(llvm::Instruction* ins) : Intrinsic(ins) {}

    static SwitchControlInteger create(Builder& b, llvm::Value* ctrl,
                                       int numCases) {

        std::vector<llvm::Value*> args_;
        args_.push_back(ctrl);
        args_.push_back(Builder::integer(numCases));

        llvm::CallInst* ins = llvm::CallInst::Create(
            b.intrinsic<SwitchControlInteger>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::SwitchControlInteger);
        return ins;
    }

    static char const* intrinsicName() { return "switchControlInteger"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Int, {t::SEXP, t::Int}, false);
    }
};

class ReturnJump : public Intrinsic {
  public:
    llvm::Value* value() { return getValue(0); }
    llvm::Value* rho() { return getValue(1); }

    ReturnJump(llvm::Instruction* ins) : Intrinsic(ins) {}

    static ReturnJump create(Builder& b, llvm::Value* value, llvm::Value* rho) {

        std::vector<llvm::Value*> args_;
        args_.push_back(value);
        args_.push_back(rho);

        llvm::CallInst* ins =
            llvm::CallInst::Create(b.intrinsic<ReturnJump>(), args_, "", b);

        b.insertCall(ins);
        setIRType(ins, ::rjit::ir::Type::ReturnJump);
        return ins;
    }

    static char const* intrinsicName() { return "returnJump"; }

    static llvm::FunctionType* intrinsicType() {
        return llvm::FunctionType::get(t::Void, {t::SEXP, t::SEXP}, false);
    }
};

} // namespace ir
} // namespace rjit
#endif // INTRINSICS_H_
