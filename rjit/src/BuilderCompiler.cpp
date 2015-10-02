#include <llvm/IR/Verifier.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/raw_ostream.h>
#include "llvm/Analysis/Passes.h"

#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Host.h"

#include "llvm/CodeGen/GCStrategy.h"
#include "llvm/CodeGen/GCs.h"

#include "Compiler.h"
#include "JITCompileLayer.h"
#include "StackMap.h"
#include "StackMapParser.h"
#include "ICCompiler.h"
#include "Symbols.h"
#include "Runtime.h"
#include "ir/Builder.h"
#include "ir/intrinsics.h"
#include "ir/ir.h"

#include "RIntlns.h"

#include <memory>

using namespace llvm;

namespace {

void emitStackmap(uint64_t id, std::vector<Value*> values, rjit::JITModule& m,
                  BasicBlock* b) {
    ConstantInt* const_0 =
        ConstantInt::get(m.getContext(), APInt(32, StringRef("0"), 10));
    Constant* const_null =
        ConstantExpr::getCast(Instruction::IntToPtr, const_0, rjit::t::i8ptr);
    ConstantInt* const_num_bytes = ConstantInt::get(
        m.getContext(), APInt(32, rjit::patchpointSize, false));
    ConstantInt* const_id =
        ConstantInt::get(m.getContext(), APInt(64, id, false));

    std::vector<Value*> sm_args;

    // Args to the stackmap
    sm_args.push_back(const_id);
    sm_args.push_back(const_num_bytes);
    sm_args.push_back(const_null);
    sm_args.push_back(const_0);

    // Values to record
    for (auto arg : values) {
        sm_args.push_back(arg);
    }

    CallInst::Create(m.patchpoint, sm_args, "", b);
}


} // namespace

namespace rjit {

/**

SEXP createNativeSXP(RFunctionPtr fptr, SEXP ast,
                     std::vector<SEXP> const& objects, Function* f) {
    SEXP objs = allocVector(VECSXP, objects.size() + 1);
    PROTECT(objs);
    SET_VECTOR_ELT(objs, 0, ast);
    for (size_t i = 0; i < objects.size(); ++i)
        SET_VECTOR_ELT(objs, i + 1, objects[i]);
    SEXP result = CONS(reinterpret_cast<SEXP>(fptr), objs);
    UNPROTECT(
        objects.size() +
        1); // all objects in objects + objs itself which is now part of result
    SET_TAG(result, reinterpret_cast<SEXP>(f));
    SET_TYPEOF(result, NATIVESXP);
    return result;
}
*/

/** Converts given SEXP to a bitcode constant.
 * The SEXP address is taken as an integer constant into LLVM which is then
 * converted to SEXP.
 * NOTE that this approach assumes that any GC used is non-moving.
 * We are using it because it removes one level of indirection when reading
 * it from the constants vector as R bytecode compiler does.
 */

/**
Value* loadConstant(SEXP value, Module* m, BasicBlock* b) {
    return ConstantExpr::getCast(
        Instruction::IntToPtr,
        ConstantInt::get(getGlobalContext(), APInt(64, (std::uint64_t)value)),
        rjit::t::SEXP);
}
*/

/**

Value* insertCall(Value* fun, std::vector<Value*> args, BasicBlock* b,
                  rjit::JITModule& m, uint64_t function_id) {

    auto res = CallInst::Create(fun, args, "", b);

    if (function_id != (uint64_t)-1) {
        assert(function_id > 1);
        assert(function_id < StackMap::nextStackmapId);

        AttributeSet PAL;
        {
            SmallVector<AttributeSet, 4> Attrs;
            AttributeSet PAS;
            {
                AttrBuilder B;
                B.addAttribute("statepoint-id", std::to_string(function_id));
                PAS = AttributeSet::get(m.getContext(), ~0U, B);
            }
            Attrs.push_back(PAS);
            PAL = AttributeSet::get(m.getContext(), Attrs);
        }
        res->setAttributes(PAL);
    }

    return res;
}

void setupFunction(Function& f, uint64_t functionId) {
    f.setGC("statepoint-example");
    auto attrs = f.getAttributes();
    attrs = attrs.addAttribute(f.getContext(), AttributeSet::FunctionIndex,
                               "no-frame-pointer-elim", "true");
    attrs = attrs.addAttribute(f.getContext(), AttributeSet::FunctionIndex,
                               "statepoint-id", std::to_string(functionId));

    f.setAttributes(attrs);
}
*/

 /**

void Compiler::Context::addObject(SEXP object) {
    PROTECT(object);
    objects.push_back(object);
}

*/ 

/**
Compiler::Context::Context(std::string const& name, llvm::Module* m) {
    f = llvm::Function::Create(t::sexp_sexpsexpint,
                               llvm::Function::ExternalLinkage, name, m);
    functionId = StackMap::nextStackmapId++;
    setupFunction(*f, functionId);
    llvm::Function::arg_iterator args = f->arg_begin();
    llvm::Value* body = args++;
    body->setName("body");
    rho = args++;
    rho->setName("rho");
    llvm::Value* useCache = args++;
    useCache->setName("useCache");
    b = llvm::BasicBlock::Create(llvm::getGlobalContext(), "start", f, nullptr);
    returnJump = false;
}
*/

/***************  The new compiler with builder will start here. Everything above ***********
  *                         should be in the builder.cpp or builder.
  */

SEXP Compiler::compileFunction(std::string const& name, SEXP ast,
                               bool isPromise) {

    
    b.openFunction(name, ast, isPromise);

    //Context * old = b.getContext();

    Value* last = compileExpression(ast);

    // since we are going to insert implicit return, which is a simple return
    // even from a promise
    b.setJump(false);
    if (last != nullptr)
        compileReturn(last, /*tail=*/true);
    // now we create the NATIVESXP
    // NATIVESXP should be a static builder, but this is not how it works 
    // at the moment
    SEXP result = b.closeFunction();
    // add the non-jitted SEXP to relocations
    // dump the function IR before wwe add statepoints
    b.f().dump();
    return result;
}

/** Compiles an expression.

  The expression as a result is always visible by default, which can be changed
  in the respective compiling functions.

  An expression is either a constant, or symbol (variable read), or a function
  call.
  */
Value* Compiler::compileExpression(SEXP value) {
    b.setResultVisible(true);
    switch (TYPEOF(value)) {
    case SYMSXP:
        return compileSymbol(value);
    case LANGSXP:
        return compileCall(value);
    case LGLSXP:
    case INTSXP:
    case REALSXP:
    case CPLXSXP:
    case STRSXP:
    case NILSXP:
    case CLOSXP:
        return compileConstant(value);
    case BCODESXP:
    // TODO: reuse the compiled fun
    case NATIVESXP:
        return compileExpression(VECTOR_ELT(CDR(value), 0));
    default:
        assert(false && "Unknown SEXP type in compiled ast.");
    }
}

/** Compiles user constant, which constant marked with userConstant intrinsic.
  */
Value* Compiler::compileConstant(SEXP value) {
    Value* result = b.constantPoolSexp(value);
    userConstant::create(b, result);
    return result;
}

/** Compiles a symbol, which reads as variable read using genericGetVar
 * intrinsic.
  */
Value* Compiler::compileSymbol(SEXP value) {
    return genericGetVar::create(b, value, rho);
}

/** Inline caching for a function (call) with operator (op)
 *  that have arguments (callArgs).
 */

Value* Compiler::compileICCallStub(Value* call, Value* op,
                                   std::vector<Value*>& callArgs) {
    uint64_t smid = StackMap::nextStackmapId++;

    auto ic_stub = ICCompiler::getStub(callArgs.size(), m);

    std::vector<Value*> ic_args;
    // Closure arguments
    for (auto arg : callArgs) {
        ic_args.push_back(arg);
    }

    // Additional IC arguments
    ic_args.push_back(call);
    ic_args.push_back(op);
    ic_args.push_back(b.rho());
    ic_args.push_back(b.f());
    ic_args.push_back(ConstantInt::get(getGlobalContext(), APInt(64, smid)));

    // Record a patch point
    emitStackmap(smid, {{ic_stub}}, m, b.block());

    auto res = CallInst::Create(ic_stub, ic_args, "", b.block());
    return b.insertCall(res);
}

Value* Compiler::compileCall(SEXP call) {
    Value* f;

    if (TYPEOF(CAR(call)) != SYMSXP) {
        // it is a complex function, first get the value of the function and
        // then check it
        f = compileExpression(CAR(call));
        checkFunction::create(b, f);
    } else {
        // it is simple function - try compiling it with intrinsics
        f = compileIntrinsic(call);
        if (f != nullptr)
            return f;
        // otherwise just do get function
        f = getFunction::create(b, CAR(call), rho);
    }

    std::vector<Value*> args;
    compileArguments(CDR(call), args);

    return compileICCallStub(b.constantPoolSexp(call), f, args);
}

void Compiler::compileArguments(SEXP argAsts, std::vector<Value*>& res) {
    while (argAsts != R_NilValue) {
        res.push_back(compileArgument(CAR(argAsts), TAG(argAsts)));
        argAsts = CDR(argAsts);
    }
}

Value* Compiler::compileArgument(SEXP arg, SEXP name) {
    switch (TYPEOF(arg)) {
    case LGLSXP:
    case INTSXP:
    case REALSXP:
    case CPLXSXP:
    case STRSXP:
    case NILSXP:
        // literals are self-evaluating
        return b.constantPoolSexp(arg);
        break;
    case SYMSXP:
        if (arg == R_DotsSymbol) {
            return b.constantPoolSexp(arg);
        }
    default: {
        SEXP code = compileFunction("promise", arg, /*isPromise=*/true);
        //Should the objects be inside the builder?
        b.addConstantPoolObject(code);
        return b.constantPoolSexp(code);
    }
    }
}

/** Many function calls may be compiled using intrinsics directly and not the R
  calling mechanism itself.

  This function determines based on the function symbol whether a compilation
  using intrinsics is possible and attempts it. It returns the result value of
  the compilation if successful, or nullptr if the function cannot be compiled
  using intrinsics.

  TODO this now uses even simpler approach than R bytecode compiler, I am simply
  assuming that these will never be overloaded. But we can change this when we
  want to.
  */
Value* Compiler::compileIntrinsic(SEXP call) {
#define CASE(sym) if (CAR(call) == sym)
    CASE(symbol::Block)
    return compileBlock(CDR(call));
    CASE(symbol::Parenthesis)
    return compileParenthesis(CDR(call));
    CASE(symbol::Function)
    return compileFunctionDefinition(CDR(call));
    CASE(symbol::Return) {
        return (CDR(call) == R_NilValue)
                   ? compileReturn(b.constantPoolSexp(R_NilValue))
                   : compileReturn(compileExpression(CAR(CDR(call))));
    }
    CASE(symbol::Assign)
    return compileAssignment(call);
    CASE(symbol::Assign2)
    return compileAssignment(call);
    CASE(symbol::SuperAssign)
    return compileSuperAssignment(call);
    CASE(symbol::If)
    return compileCondition(call);
    CASE(symbol::Break)
    return compileBreak(call);
    CASE(symbol::Next)
    return compileNext(call);
    CASE(symbol::Repeat)
    return compileRepeatLoop(call);
    CASE(symbol::While)
    return compileWhileLoop(call);
    CASE(symbol::For)
    return compileForLoop(call);
    CASE(symbol::Switch)
    return compileSwitch(call);
    CASE(symbol::Add)
    return compileBinaryOrUnary<GenericAdd, GenericUnaryPlus>(call);
    CASE(symbol::Sub)
    return compileBinaryOrUnary<GenericSub, GenericUnaryMinus>(call);
    CASE(symbol::Mul)
    return compileBinary<GenericMul>(call);
    CASE(symbol::Div)
    return compileBinary<GenericDiv>(call);
    CASE(symbol::Pow)
    return compileBinary<GenericPow>(call);
    CASE(symbol::Sqrt)
    return compileUnary<GenericSqrt>(call);
    CASE(symbol::Exp)
    return compileUnary<GenericExp>(call);
    CASE(symbol::Eq)
    return compileBinary<GenericEq>(call);
    CASE(symbol::Ne)
    return compileBinary<GenericNe>(call);
    CASE(symbol::Lt)
    return compileBinary<GenericLt>(call);
    CASE(symbol::Le)
    return compileBinary<GenericLe>(call);
    CASE(symbol::Ge)
    return compileBinary<GenericGe>(call);
    CASE(symbol::Gt)
    return compileBinary<GenericGt>(call);
    CASE(symbol::BitAnd)
    return compileBinary<GenericBitAnd>(call);
    CASE(symbol::BitOr)
    return compileBinary<GenericBitOr>(call);
    CASE(symbol::Not)
    return compileUnary<GenericNot>(call);

    return nullptr;
#undef CASE
}

/** Block (a call to {) is compiled as a simple sequence of its statements with
 * its return value being the result of the last statement. If a block is empty,
 * a visible R_NilValue is returned.
  */
Value* Compiler::compileBlock(SEXP block) {
    Value* result = nullptr;
    while (block != R_NilValue) {
        result = compileExpression(CAR(block));
        block = CDR(block);
    }
    if (result == nullptr)
        result = b.constantPoolSexp(R_NilValue);
    return result;
}

/** Parenthesis expects a single argument only. Ellipsis is allowed, but not
  supported with the intrinsics at the moment so we default to R call.

  Otherwise markVisible intrinsic is applied to the result in accordance to the
  manual.
  */
Value* Compiler::compileParenthesis(SEXP arg) {
    arg = CAR(arg);
    if (arg == symbol::Ellipsis)
        return nullptr; // we can't yet do this
    Value* result = compileExpression(arg);
    b.setResultVisible(true);
    return result;
}

/** Similar to R bytecode compiler, only the body of the created function is
  compiled, the default arguments are left in their ast forms for now.

  TODO this should change.
 */
Value* Compiler::compileFunctionDefinition(SEXP fdef) {
    SEXP forms = CAR(fdef);
    SEXP body = compileFunction("function", CAR(CDR(fdef)));
    b.addConstantPoolObject(body);
    return createClosure::create(b, forms, body, b.rho());
}

/** Simple assignments (that is to a symbol) are compiled using the
 * genericSetVar intrinsic.
  */
Value* Compiler::compileAssignment(SEXP e) {
    e = CDR(e);
    // intrinsic only handles simple assignments
    if (TYPEOF(CAR(e)) != SYMSXP)
        return nullptr;
    Value* v = compileExpression(CAR(CDR(e)));
    genericSetVar::create(b, CAR(e), v, b.rho());
    b.isResultVisible(false);
    return v;
}

/**
Value* Compiler::compileAssignment(SEXP e) {
    e = CDR(e);
    // intrinsic only handles simple assignments
    if (TYPEOF(CAR(e)) != SYMSXP)
        return nullptr;
    Value* v = compileExpression(CAR(CDR(e)));
    genericSetVar::create(b, constant(CAR(e)), v, rho);
    isResultVisible = false;
    return v;
}
*/

/** Super assignment is compiled as genericSetVarParentIntrinsic
 */
Value* Compiler::compileSuperAssignment(SEXP e) {
    e = CDR(e);
    // intrinsic only handles simple assignments
    if (TYPEOF(CAR(e)) != SYMSXP)
        return nullptr;
    Value* v = compileExpression(CAR(CDR(e)));
    genericSetVarParent::create(b, CAR(e), v, b.rho());
    b.isResultVisible(false);
    return v;
}

/** Return calls or returns in general are compiled depending on the context.
 * Usually a simple return instruction in bitcode is enough, but while in
 * promises, we must use longjmp, which is done by calling returnJump intrinsic.
  */
Value* Compiler::compileReturn(Value* value, bool tail) {
    if (not b.getResultVisible())
        markInvisible::create(b);
    if (b.getReturnJump())) {
        returnJump::create(b, value, b.rho());
        // we need to have a return instruction as well to fool LLVM into
        // believing the basic block has a terminating instruction
        Return::create(R_NilValue);
    } else {
        Return::create(value);
    }
    // this is here to allow compilation of wrong code where statements are even
    // after return
    if (not tail)
       b.setBlock(b.createBasicBlock("deadcode"));
    return nullptr;
}

/** Condition is compiled using the convertToLogicalNoNA intrinsic. True block
 * has to be always present, but false block does not have to be present in
 * which case an invisible R_NilValue should be returned.
  */
Value* Compiler::compileCondition(SEXP e) {
    e = CDR(e);
    SEXP condAst = CAR(e);
    e = CDR(e);
    SEXP trueAst = CAR(e);
    e = CDR(e);
    SEXP falseAst = (e != R_NilValue) ? CAR(e) : nullptr;
    Value* cond2 = compileExpression(condAst);
    Value* cond = convertToLogicalNoNA::create(b, cond2, condAst);
    BasicBlock* ifTrue = b.createBasicBlock("ifTrue");
    BasicBlock* ifFalse = b.createBasicBlock("ifFalse");
    BasicBlock* next = b.createBasicBlock("next");
    Cbr::create(b, cond, ifTrue, ifFalse);
    //    *(context->b), ICmpInst::ICMP_EQ, cond, constant(TRUE), "condition");
    // BranchInst::Create(ifTrue, ifFalse, test, context->b);

    // true case has to be always present
    b.setBlock(ifTrue);
    Value* trueResult = compileExpression(trueAst);
    Branch::create(b, next);
    ifTrue = b.block();

    // false case may not be present in which case invisible R_NilValue should
    // be returned
    b.setBlock(ifFalse);
    Value* falseResult;
    if (falseAst == nullptr) {
        falseResult = b.constantPoolSexp(R_NilValue);
        b.setResultVisible(false);
    } else {
        falseResult = compileExpression(falseAst);
        ifFalse = b.block();
    }
    Branch::create(b, next);

    // add a phi node for the result
    b.setBlock(next);
    PHINode* phi = PHINode::Create(t::SEXP, 2, "", b);
    phi->addIncoming(trueResult, ifTrue);
    phi->addIncoming(falseResult, ifFalse);
    return phi;
}

/** Compiles break. Whenever we see break in the compiler, we know it is for a
  loop where context was skipped and therefore it must always be translated as
  direct jump in bitcode.

  TODO The error is probably not right.
   */
Value* Compiler::compileBreak(SEXP ast) {
    llvm::BasicBlock * bb = b.breakTarget();
    Branch::create(b, bb);
    // TODO this is really simple, but fine for us - dead code elimination will
    // remove the block if required
    b.setBlock(b.createBasicBlock("deadcode"));
    return b.constantPoolSexp(R_NilValue);
}

/** Compiles next. Whenever we see next in the compiler, we know it is for a
  loop where context was skipped and therefore it must always be translated as
  direct jump in bitcode.

  TODO The error is probably not right.
   */
Value* Compiler::compileNext(SEXP ast) {
    llvm::BasicBlock * bb = b.nextTarget();
    Branch::create(bb);
    // TODO this is really simple, but fine for us - dead code elimination will
    // remove the block if required
    b.setBlock(b.createBasicBlock("deadcode"));
    return b.constantPoolSexp(R_NilValue);
}

/** Compiles repeat loop. This is simple infinite loop. Only break can exit it.

  Return value of break loop is invisible R_NilValue.
 */
Value* Compiler::compileRepeatLoop(SEXP ast) {
    SEXP bodyAst = CAR(CDR(ast));
    if (not canSkipLoopContext(bodyAst))
        return nullptr;
    // save old loop pointers from the context
    // create the body and next basic blocks
    b.openLoop();

    Branch::create(b, b.nextBlock());
    b.setBlock(b.nextTarget());
    
    compileExpression(bodyAst);
    Branch::create(b, b.nextTarget());
    b.setBlock(b.breakTarget());
    // restore the old loop pointers in the context
    b.closeLoop();
    // return R_NilValue
    b.setResultVisible(false);
    return b.constantPoolSexp(R_NilValue);
}

/** Compiles while loop.

  Return value of a while loop is invisible R_NilValue.
 */
Value* Compiler::compileWhileLoop(SEXP ast) {
    SEXP condAst = CAR(CDR(ast));
    SEXP bodyAst = CAR(CDR(CDR(ast)));
    if (not canSkipLoopContext(bodyAst))
        return nullptr;
    // save old loop pointers from the context
    // create the body and next basic blocks
    b.openLoop();

    Branch::create(b, b.nextTarget());
    b.setBlock(b.nextTarget());
    // compile the condition
    Value* cond2 = compileExpression(condAst);
    Value* cond = ConvertToLogicalNoNA::create(b , cond2, condAst);
    BasicBlock* whileBody = b.createBasicBlock("whileBody");
    Cbr::create(b, cond, whileBody, b.breakTarget());
    // compile the body
    b.setBlock(whileBody);
    compileExpression(bodyAst);
    Branch::create(b, b.breakTarget());
    b.setBlock(b.breakTarget());
    // restore the old loop pointers in the context
    b.closeLoop();
    // return R_NilValue
    b.setResultVisible(false);
    return b.constantPoolSexp(R_NilValue);
}

/** For loop is compiled into the following structure:

      get the sequence
      length = sequence length
      index = 0
      goto forCond
  forCond:
      goto (index < length) ? forBody : forBreak
  forBody:
      setVar(controlVar, getForLoopValue(seq, index)
      body of the loop
      goto forNext
  forNext:
      index += 1
      goto forCond
  forBreak:

  This uses a jump too many, but it simplifies the SSA considerations and will
  be optimized by LLVM anyhow when we go for LLVM optimizations.
  */
Value* Compiler::compileForLoop(SEXP ast) {
    SEXP controlAst = CAR(CDR(ast));
    assert(TYPEOF(controlAst) == SYMSXP and
           "Only symbols allowed as loop control variables");
    SEXP seqAst = CAR(CDR(CDR(ast)));
    SEXP bodyAst = CAR(CDR(CDR(CDR(ast))));
    if (not canSkipLoopContext(bodyAst))
        return nullptr;
    // save old loop pointers from the context
    b.openLoop();
    // create the body and next basic blocks
    // This is a simple basic block to which all next's jump and which then
    // jumps to forCond so that there is a simpler phi node at forCond.
    BasicBlock* forCond = b.createBasicBlock("forCond");
    BasicBlock* forBody =b.createBasicBlock("forBody");
    // now initialize the loop control structures
    Value* seq2 = compileExpression(seqAst);
    Value* seq = StartFor::create(b, seq2, b.rho());
    Value* seqLength = LoopSequenceLength::create(b, seq, ast);
    BasicBlock* forStart = b.block();
    Branch::create(b, forCond);
    b.setBlock(forCond);
    PHINode* control = PHINode::Create(t::Int, 2, "loopControl", context->b);
    control->addIncoming(constant(0), forStart);
    // now check if control is smaller than length

    //TODO: Need to add the correct ICmpInst for this call
    Cbr test = Cbr::create(b, ICmpInst::ICMP_ULT, control,
                                  seqLength, "condition");
    BranchInst::Create(forBody, context->breakBlock, test, context->b);

    // move to the for loop body, where we have to set the control variable
    // properly
    b.setBlock(forBody);
    Value* controlValue = GetForLoopValue::create(b, seq, control);
    GenericSetVar::create(b, controlAst, controlValue, b.rho());
    // now compile the body of the loop
    compileExpression(bodyAst);
    Brnach::create(b,b.nextTarget());
    // in the next block, increment the internal control variable and jump to
    // forCond
    b.setBlock(b.nextTarget);

    //TODO: Need an intrinsic function for BinaryOperator
    Value* control1 = BinaryOperator::Create(Instruction::Add, control,
                                             constant(1), "", b.block());
    control->addIncoming(control1, b.nextTarget());

    Branch::create(b, forCond);
    b.setBlock(b.breakTarget());
    // restore the old loop pointers in the context
    b.closeLoop();
    // return R_NilValue
    b.setResultVisible(false);
    return b.constantPoolSexp(R_NilValue);
}

/** Determines whether we can skip creation of the loop context or not. The code
 * is taken from Luke's bytecode compiler.
 */
bool Compiler::canSkipLoopContext(SEXP ast, bool breakOK) {
    if (TYPEOF(ast) == LANGSXP) {
        SEXP cs = CAR(ast);
        if (TYPEOF(cs) == SYMSXP) {
            if (not breakOK and (cs == symbol::Break or cs == symbol::Next)) {
                return false;
            } else if (cs == symbol::Function or cs == symbol::For or
                       cs == symbol::While or cs == symbol::Repeat) {
                return true;
            } else if (cs == symbol::Parenthesis or cs == symbol::Block or
                       cs == symbol::If) {
                return canSkipLoopContextList(CDR(ast), breakOK);
            } else {
                return canSkipLoopContextList(CDR(ast), false);
            }
        }
        // this is change to Luke's code - I believe that whatever will return
        // us the function to call might be compiled using intrinsics and
        // therefore should follow the breakOK rules, not the rules for promises
        // the arguments of user functions do
        return canSkipLoopContext(CAR(ast), breakOK) and
               canSkipLoopContextList(CDR(ast), false);
    } else {
        return true;
    }
}

bool Compiler::canSkipLoopContextList(SEXP ast, bool breakOK) {
    while (ast != R_NilValue) {
        if (not canSkipLoopContext(CAR(ast), breakOK))
            return false;
        ast = CDR(ast);
    }
    return true;
}

/** Compiles the switch statement.

  There are two kinds of switch - integral and character one and they differ in
  what they are doing. The integral switch can be used always, and in its case
  the control variable is simple index to the cases. Contrary to the

      ctrl = evaluate switch control
      checkSwitchControl()
      goto sexptype() == STRSXP ? switchCharacter : switchIntegral
  switchIntegral:
      t = switchControlInteger(ctrl, numCases)
      switch (t):
  switchCharacter:
      t = switchControlCharacter(ctrl, call, caseStrings)
      switch (t):
  switchCase1:
      ...
      goto switchNext
  switchCaseN:
      ...
      goto switchNext
  switchNext:
      phi for the cases


 */
Value* Compiler::compileSwitch(SEXP call) {
    SEXP x = CDR(call);
    SEXP condAst = CAR(x);
    x = CDR(x);
    std::vector<SEXP> caseAsts;
    std::vector<SEXP> caseNames;
    int defaultIdx = -1;
    int i = 0;
    while (x != R_NilValue) {
        caseAsts.push_back(CAR(x));
        SEXP name = TAG(x);
        if (name == R_NilValue)
            if (defaultIdx == -1)
                defaultIdx = i;
            else
                defaultIdx = -2;
        else
            caseNames.push_back(name);
        x = CDR(x);
        ++i;
    }
    // actual switch compilation - get the control value and check it
    Value* control = compileExpression(condAst);
   
    CheckSwitchControl::create(b, control, call);
    Value* ctype = SexpType::create(b, control);
    //
    Cbr::create(b, ctype, switchCharacter, switchIntegral); 
    //
    BasicBlock* switchIntegral = b.createBasicBlock("switchIntegral");
    BasicBlock* switchCharacter = b.createBasicBlock("switchCharacter");
    BasicBlock* switchNext = b.createBasicBlock("switchNext");

    // integral switch is simple
    b.setBlock(switchIntegral);
    
    Value* caseIntegral = SwitchControlInteger::create(b, control, caseAsts.size());
    SwitchInst* swInt = SwitchInst::create(b, caseIntegral, switchNext,
                                           caseAsts.size(), b.block());
    // for character switch we need to construct the vector,
    b.setBlock(switchCharacter);
    SEXP cases;
    if (defaultIdx != -2) {
        cases = allocVector(STRSXP, caseNames.size());
        for (size_t i = 0; i < caseNames.size(); ++i)
            SET_STRING_ELT(cases, i, PRINTNAME(caseNames[i]));
    } else {
        cases = R_NilValue;
    }
    //
    b.addConstantPoolObject(cases);
    Value* caseCharacter = SwitchControlCharacter::create(b, control, call, cases);
    SwitchInst* swChar = SwitchInst::create(b, caseCharacter, switchNext,
                                            caseAsts.size(), context->b);
    // create the phi node at the end
    b.setBlock(switchNext);
    PHINode* result = PHINode::Create(t::SEXP, caseAsts.size(), "", b.block());
    // walk the cases and create their blocks, add them to switches and their
    // results to the phi node
    BasicBlock* last;
    for (unsigned i = 0; i < caseAsts.size(); ++i) {
        b.block() = last = b.createBasicBlock("switchCase");
        swInt->addCase(b.constantPoolSexp(i), last);
        if (defaultIdx == -1 or defaultIdx > static_cast<int>(i)) {
            swChar->addCase(b.constantPoolSexp(i), last);
        } else if (defaultIdx < static_cast<int>(i)) {
            swChar->addCase(b.constantPoolSexp(i - 1), last);
        } else {
            swChar->addCase(b.constantPoolSexp(caseAsts.size() - 1), last);
            swChar->setDefaultDest(last);
        }
        Value* caseResult = compileExpression(caseAsts[i]);
        Branch::create(b, switchNext);
        result->addIncoming(caseResult, b.block());
    }
    if (swChar->getDefaultDest() == switchNext)
        swChar->setDefaultDest(last);
    swInt->setDefaultDest(last);
    b.setBlock(switchNext);
    return result;
}

/** Compiles operators that can be either binary, or unary, based on the number
 * of call arguments. Takes the binary and unary intrinsics to be used and the
 * full call ast.
  */

/** Compiles binary operator using the given intrinsic and full call ast.
  
Value* Compiler::compileBinary(Function* f, SEXP call) {
    Value* lhs = compileExpression(CAR(CDR(call)));
    Value* rhs = compileExpression(CAR(CDR(CDR(call))));
    return INTRINSIC(f, lhs, rhs, constant(call), context->rho);
}


// Compiles unary operator using the given intrinsic and full call ast.
  
Value* Compiler::compileUnary(Function* f, SEXP call) {
    Value* op = compileExpression(CAR(CDR(call)));
    return INTRINSIC(f, op, constant(call), context->rho);
}

Value* Compiler::constant(SEXP value) {
    return loadConstant(value, m.getM(), b);
}

Value* Compiler::INTRINSIC(llvm::Value* fun, std::vector<Value*> args) {
    return insertCall(fun, args, context->b, m, context->functionId);
}
*/

}
