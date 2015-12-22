#ifndef IR_H
#define IR_H

#include "llvm.h"

#include <cstdint>

#include "RIntlns.h"

#include "Builder.h"

/** \file "rjit/src/ir/Ir.h"
 */

namespace rjit {
namespace ir {

/** LLVM RTTI kinds of recognized patterns. Also used for fast matching.
 */
enum class PatternKind {
    ExtractConstantPool,
    UserLiteral,
    Constant,
    ConvertToLogicalNoNA,
    PrintValue,
    StartFor,
    LoopSequenceLength,
    GetForLoopValue,
    MarkVisible,
    MarkInvisible,
    UserConstant,
    GenericGetVar,
    GenericGetEllipsisArg,
    GenericSetVar,
    GenericSetVarParent,
    GetFunction,
    GetGlobalFunction,
    GetSymFunction,
    GetBuiltinFunction,
    GetInternalBuiltinFunction,
    CheckFunction,
    CreatePromise,
    SexpType,
    AddArgument,
    AddKeywordArgument,
    AddEllipsisArgument,
    AddEllipsisArgumentHead,
    AddEllipsisArgumentTail,
    CallBuiltin,
    CallSpecial,
    CallClosure,
    CreateClosure,
    GenericUnaryMinus,
    GenericUnaryPlus,
    GenericAdd,
    GenericSub,
    GenericMul,
    GenericDiv,
    GenericPow,
    GenericSqrt,
    GenericExp,
    GenericEq,
    GenericNe,
    GenericLt,
    GenericLe,
    GenericGe,
    GenericGt,
    GenericBitAnd,
    GenericBitOr,
    GenericNot,
    GenericGetVarMissOK,
    GenericGetEllipsisValueMissOK,
    CheckSwitchControl,
    SwitchControlCharacter,
    SwitchControlInteger,
    ReturnJump,
    InitClosureContext,
    EndClosureContext,
    ClosureQuickArgumentAdaptor,
    ClosureNativeCallTrampoline,
    CallNative,
};

/** Pattern of llvm instructions that is recognized as high level function and can be matched on.
 */
class Pattern {
public:
    static char const * const MD_NAME;


    /** Pattern kind used for RTTI and matching.
     */
    PatternKind const kind;

    /** Result instruction for the pattern. This value should be used as the value for the whole pattern.
     */
    llvm::Instruction * const result;

    /** A pattern can always typecast to its result instruction.
     */
    operator llvm::Instruction * () {
        return result;
    }

protected:

    /** Creates a pattern with given kind and result instruction.
     */
    Pattern(PatternKind kind, llvm::Instruction * result):
        kind(kind),
        result(result) {
    }

    /** When a pattern is destroyed, it detaches itself from all its instructions.
     */
    virtual ~Pattern() {
        detachAll();
    }

    /** Returns pattern the given llvm instruction belongs to. Returns nullptr if the instruction is not part of any pattern. */
    static Pattern * getPattern(llvm::Instruction * ins) {
        llvm::MDNode* m = ins->getMetadata(MD_NAME);
        if (m == nullptr)
            return nullptr;
        llvm::Metadata* mx = m->getOperand(0);
        llvm::APInt const& ap =
            llvm::cast<llvm::ConstantInt>(
                llvm::cast<llvm::ValueAsMetadata>(mx)->getValue())
                ->getUniqueInteger();
        assert(ap.isIntN(64) and "Expected 64bit integer");
        Pattern * res = reinterpret_cast<Pattern *>(ap.getZExtValue());
        assert(res);
        return res;
    }

    /** Attaches the pattern to specified llvm instruction. All llvm instructions that are part of a pattern must be attached to it.
     */
    void attachTo(llvm::Instruction * ins) {
        std::vector<llvm::Metadata*> v = {
            llvm::ValueAsMetadata::get(llvm::ConstantInt::get(
                ins->getContext(),
                llvm::APInt(64, reinterpret_cast<std::uintptr_t>(this))
            ))
        };
        llvm::MDNode* m = llvm::MDNode::get(ins->getContext(), v);
        ins->setMetadata(MD_NAME, m);
    }

    /** Detaches the pattern from the specified llvm instruction.
     */
    void detachFrom(llvm::Instruction * ins) {
        assert(getPattern(ins) == this and "Cannot detach from instruction that is not a member");
        // TODO remove the metadata here
    }


private:

    /** Detaches the pattern from all its instructions calling detachFrom on them.

      This method is called by the destructor to detach the pattern from its instructions. The general way of doing this is to scan instructions before and after if they are part of the pattern and detach from them. Subclasses of Pattern may choose to override this to deal with the specific cases faster.
      */
    virtual void detachAll() {
        // TODO implement
    }

    /** Advances given iterator past the pattern.

      This generic method searches the instructions to find first one that does not belong to the pattern. Subclasses may override this to deal with the specific cases faster.
     */
    virtual void advance(llvm::BasicBlock::iterator & i) {
        while (true) {
            llvm::Instruction * ii = i;
            // advance the iterator
            ++i;
            // if the last instruction was terminating instruction, return - we are at the end of bb
            if (llvm::isa<llvm::TerminatorInst>(ii))
                return;
            // check if the current instruction still belongs to the pattern and terminate if not
            ii = i;
            if (getPattern(ii) != this)
                return;
        }
    }
};

/** A pattern that consists of a single llvm instruction.

  Provides fast detachAll and advance methods utilizing the single instruction length.
 */
class SingleInstructionPattern : public Pattern {
protected:
    SingleInstructionPattern(PatternKind kind, llvm::Instruction * ins):
        Pattern(kind, ins) {
    }
private:
    /** Single instruction just detaches from its result.
     */
    void detachAll() override {
        detachFrom(result);
    }

    /** Advances by single instruction.
     */
    void advance(llvm::BasicBlock::iterator & i) override {
        ++i;
    }
};

/** Primitive call pattern base class.
 */
class PrimitiveCall : public SingleInstructionPattern {
public:

    /** Returns the call instruction of the primitive call.
     */
    llvm::CallInst * callInst() const {
        // reinterpret is faster and we know it must be call inst so all is fine
        assert(llvm::isa<llvm::CallInst>(result) and "PrimitiveCall result must be llvm::CallInst");
        return reinterpret_cast<llvm::CallInst *>(result);
    }

    /** Primitive calls can typecast to llvm::CallInst where appropriate.
     */
    operator llvm::CallInst * () {
        return callInst();
    }

protected:
    PrimitiveCall(PatternKind kind, llvm::CallInst * result):
        SingleInstructionPattern(kind, result) {
    }

    llvm::Value* getArgument(unsigned argIndex) {
        return callInst()->getArgOperand(argIndex);
    }

    SEXP getArgumentSEXP(unsigned argIndex) {
        assert(false and "NOT IMPLEMENTED");
    }

    int getArgumentInt(unsigned argIndex) {
        return Builder::integer(callInst()->getArgOperand(argIndex));
    }

};

















class Matcher {};

/** Type of the IR.
 */
/** Generic class for all IR objects.

  They all must point to an existing llvm value.
 */
class Instruction : public Matcher {
  public:
    static char const* const MD_NAME;

    /** Depending on how we want the RTTI to behave, either put only leaves
     * (that is actual instructions the user might see) in here, or put them
     * all. I would be in favour of the first option. THe thing we want is not a
     * real RTTI in the end.
     */
    enum InstructionKind {
        ExtractConstantPool,
        UserLiteral,
        Constant,
        ConvertToLogicalNoNA,
        PrintValue,
        StartFor,
        LoopSequenceLength,
        GetForLoopValue,
        MarkVisible,
        MarkInvisible,
        UserConstant,
        GenericGetVar,
        GenericGetEllipsisArg,
        GenericSetVar,
        GenericSetVarParent,
        GetFunction,
        GetGlobalFunction,
        GetSymFunction,
        GetBuiltinFunction,
        GetInternalBuiltinFunction,
        CheckFunction,
        CreatePromise,
        SexpType,
        AddArgument,
        AddKeywordArgument,
        AddEllipsisArgument,
        AddEllipsisArgumentHead,
        AddEllipsisArgumentTail,
        CallBuiltin,
        CallSpecial,
        CallClosure,
        CreateClosure,
        GenericUnaryMinus,
        GenericUnaryPlus,
        GenericAdd,
        GenericSub,
        GenericMul,
        GenericDiv,
        GenericPow,
        GenericSqrt,
        GenericExp,
        GenericEq,
        GenericNe,
        GenericLt,
        GenericLe,
        GenericGe,
        GenericGt,
        GenericBitAnd,
        GenericBitOr,
        GenericNot,
        GenericGetVarMissOK,
        GenericGetEllipsisValueMissOK,
        CheckSwitchControl,
        SwitchControlCharacter,
        SwitchControlInteger,
        ReturnJump,
        Return,
        Branch,
        Cbr,
        IntegerLessThan,
        IntegerEquals,
        UnsignedIntegerLessThan,
        InitClosureContext,
        Switch,
        IntegerAdd,
        EndClosureContext,
        ClosureQuickArgumentAdaptor,
        ClosureNativeCallTrampoline,
        CallNative,
        unknown,
    };

    /** Returns the IR type of the intrinsic call for faster matching.
     */
    static Instruction* getIR(llvm::Instruction* ins);

    /** Returns the IR type of the instruction sequence starting at i and
     * advances i past it. Returns Type::unknown if the sequence start cannot be
     * matched and advances one instruction further.
     */
    static Instruction* match(llvm::BasicBlock::iterator& i);

    static bool isInstruction(llvm::Instruction* i);

    /** Each ir instruction can typecast to the underlying llvm bitcode.
     */
    operator llvm::Instruction*() { return ins_; }

    Instruction(llvm::Instruction* ins, InstructionKind kind)
        : ins_(ins), kind_(kind) {}

    InstructionKind getKind() const { return kind_; }

  protected:
    template <typename T>
    T* ins() {
        return llvm::cast<T>(ins_);
    }

    /** Sets the ir kind.
     */
    static void setIR(llvm::Instruction* llvmIns,
                      rjit::ir::Instruction* rjitIns) {
        assert(rjitIns);
        std::vector<llvm::Metadata*> v = {
            llvm::ValueAsMetadata::get(llvm::ConstantInt::get(
                llvmIns->getContext(),
                llvm::APInt(64, reinterpret_cast<std::uintptr_t>(rjitIns))))};
        llvm::MDNode* m = llvm::MDNode::get(llvmIns->getContext(), v);
        llvmIns->setMetadata(MD_NAME, m);
        assert(getIR(llvmIns));
    }

  private:
    llvm::Instruction* ins_;
    InstructionKind kind_;
};

class Return : public Instruction {
  public:
    Return(llvm::Instruction* ins) : Instruction(ins, InstructionKind::Return) {
        assert(llvm::isa<llvm::ReturnInst>(ins) and "Return expected");
    }

    llvm::Value* result() { return ins<llvm::ReturnInst>()->getOperand(0); }

    static Return create(Builder& b, llvm::Value* value) {
        return llvm::ReturnInst::Create(llvm::getGlobalContext(), value, b);
    }

    static bool classof(Instruction const* s) {
        return s->getKind() == InstructionKind::Return;
    }
};

class Branch : public Instruction {
  public:
    Branch(llvm::Instruction* ins) : Instruction(ins, InstructionKind::Branch) {
        assert(llvm::isa<llvm::BranchInst>(ins) and
               "Branch instruction expected");
        assert(not llvm::cast<llvm::BranchInst>(ins)->isConditional() and
               "Branch must be unconditional");
    }

    llvm::BasicBlock* target() {
        return ins<llvm::BranchInst>()->getSuccessor(0);
    }

    static Branch create(Builder& b, llvm::BasicBlock* target) {
        return llvm::BranchInst::Create(target, b);
    }

    static bool classof(Instruction const* s) {
        return s->getKind() == InstructionKind::Branch;
    }
};

class IntegerComparison : public Instruction {
  public:
    typedef llvm::ICmpInst::Predicate Predicate;

    IntegerComparison(llvm::Instruction* ins, InstructionKind kind)
        : Instruction(ins, kind) {
        assert(llvm::isa<llvm::ICmpInst>(ins) and "ICmpInst expected");
    }

    Predicate predicate() {
        return ins<llvm::ICmpInst>()->getSignedPredicate();
    }

    llvm::Value* lhs() { return ins<llvm::ICmpInst>()->getOperand(0); }

    llvm::Value* rhs() { return ins<llvm::ICmpInst>()->getOperand(1); }
};

class IntegerLessThan : public IntegerComparison {
  public:
    IntegerLessThan(llvm::Instruction* ins)
        : IntegerComparison(ins, InstructionKind::IntegerLessThan) {
        assert(llvm::cast<llvm::ICmpInst>(ins)->getSignedPredicate() ==
                   Predicate::ICMP_SLT and
               "Less than comparison expected");
    }

    static IntegerLessThan create(Builder& b, llvm::Value* lhs,
                                  llvm::Value* rhs) {
        return new llvm::ICmpInst(*b.block(), Predicate::ICMP_SLT, lhs, rhs);
    }

    static bool classof(Instruction const* s) {
        return s->getKind() == InstructionKind::IntegerLessThan;
    }
};

class IntegerEquals : public IntegerComparison {
  public:
    IntegerEquals(llvm::Instruction* ins)
        : IntegerComparison(ins, InstructionKind::IntegerEquals) {
        assert(llvm::cast<llvm::ICmpInst>(ins)->getSignedPredicate() ==
                   Predicate::ICMP_EQ and
               "Equality comparison expected");
    }

    static llvm::ICmpInst* create(Builder& b, llvm::Value* lhs,
                                  llvm::Value* rhs) {
        return new llvm::ICmpInst(*b.block(), Predicate::ICMP_EQ, lhs, rhs);
    }

    static bool classof(Instruction const* s) {
        return s->getKind() == InstructionKind::IntegerEquals;
    }
};

class UnsignedIntegerLessThan : public IntegerComparison {
  public:
    UnsignedIntegerLessThan(llvm::Instruction* ins)
        : IntegerComparison(ins, InstructionKind::UnsignedIntegerLessThan) {
        assert(llvm::cast<llvm::ICmpInst>(ins)->getSignedPredicate() ==
                   Predicate::ICMP_ULT and
               "Unsigned less than comparison expected");
    }

    static llvm::ICmpInst* create(Builder& b, llvm::Value* lhs,
                                  llvm::Value* rhs) {
        return new llvm::ICmpInst(*b.block(), Predicate::ICMP_ULT, lhs, rhs);
    }

    static bool classof(Instruction const* s) {
        return s->getKind() == InstructionKind::UnsignedIntegerLessThan;
    }
};

// TODO the hierarchy of this is wrong, but actual thought is required to fix it
class BinaryOperator : public Instruction {
  public:
    BinaryOperator(llvm::Instruction* ins, InstructionKind kind)
        : Instruction(ins, kind) {}
};

// TODO the hierarchy here should be better as well
class IntegerAdd : public BinaryOperator {
  public:
    IntegerAdd(llvm::Instruction* ins)
        : BinaryOperator(ins, InstructionKind::IntegerAdd) {
        assert(llvm::isa<llvm::BinaryOperator>(ins) and
               "Binary operator expected");
        assert(llvm::cast<llvm::BinaryOperator>(ins)->getOpcode() ==
                   llvm::Instruction::Add and
               "Add opcode expected");
    }

    llvm::Value* lhs() { return ins<llvm::ICmpInst>()->getOperand(0); }

    llvm::Value* rhs() { return ins<llvm::ICmpInst>()->getOperand(1); }

    static IntegerAdd create(Builder& b, llvm::Value* lhs, llvm::Value* rhs) {
        return llvm::BinaryOperator::Create(llvm::Instruction::Add, lhs, rhs,
                                            "", b);
    }

    static bool classof(Instruction const* s) {
        return s->getKind() == InstructionKind::IntegerAdd;
    }
};

/** Conditional branch.

  Takes three arguments, the condition on which it jumps
  (this can be any integer) and true and false blocks.

  Conditional branch consists of ICmpInst followed by BranchInst internally.

  TODO We might want to change this in the future and get
  the comparison out of the branch, but for now, this is the
  only branch we have and it is a showcase for matching multiple
  llvm bitcodes to single ir.
 */
class Cbr : public Instruction {
  public:
    llvm::Value* cond() { return ins<llvm::ICmpInst>()->getOperand(0); }

    llvm::BasicBlock* trueCase() {
        llvm::BranchInst* b =
            llvm::cast<llvm::BranchInst>(ins<llvm::ICmpInst>()->getNextNode());
        return b->getSuccessor(0);
    }

    llvm::BasicBlock* falseCase() {
        llvm::BranchInst* b =
            llvm::cast<llvm::BranchInst>(ins<llvm::ICmpInst>()->getNextNode());
        return b->getSuccessor(1);
    }

    Cbr(llvm::Instruction* ins) : Instruction(ins, InstructionKind::Cbr) {}

    static void create(Builder& b, llvm::Value* cond,
                       llvm::BasicBlock* trueCase, llvm::BasicBlock* falseCase);

    static bool classof(Instruction const* s) {
        return s->getKind() == InstructionKind::Cbr;
    }
};

/** Interface to llvm's switch instruction
  */
class Switch : public Instruction {
  public:
    Switch(llvm::Instruction* ins) : Instruction(ins, InstructionKind::Switch) {
        assert(llvm::isa<llvm::SwitchInst>(ins) and
               "Expecting llvm's switch instruction");
    }

    void addCase(int i, llvm::BasicBlock* target) {
        ins<llvm::SwitchInst>()->addCase(Builder::integer(i), target);
    }

    // TODO add meaningful accessors

    static Switch create(Builder& b, llvm::Value* cond,
                         llvm::BasicBlock* defaultTarget, int numCases) {
        return llvm::SwitchInst::Create(cond, defaultTarget, numCases, b);
    }

    void setDefaultDest(llvm::BasicBlock* target) {
        ins<llvm::SwitchInst>()->setDefaultDest(target);
    }

    llvm::BasicBlock* getDefaultDest() {
        return ins<llvm::SwitchInst>()->getDefaultDest();
    }

    static bool classof(Instruction const* s) {
        return s->getKind() == InstructionKind::Switch;
    }
};

/** Base class for all intrinsics.

 */
class Intrinsic : public Instruction {
  public:
    /** Returns the CallInst associated with the intrinsic.
     */
    llvm::CallInst* ins() { return Instruction::ins<llvm::CallInst>(); }

  protected:
    Intrinsic(llvm::Instruction* ins, InstructionKind kind)
        : Instruction(ins, kind) {
        assert(llvm::isa<llvm::CallInst>(ins) and
               "Intrinsics must be llvm calls");
    }

    llvm::Value* getValue(unsigned argIndex) {
        return ins()->getArgOperand(argIndex);
    }

    SEXP getValueSEXP(unsigned argIndex) {
        assert(false and "NOT IMPLEMENTED");
    }

    int getValueInt(unsigned argIndex) {
        return Builder::integer(ins()->getArgOperand(argIndex));
    }
};

} // namespace ir

} // namespace rjit

#endif // IR_H
