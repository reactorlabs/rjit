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
#include "ir/primitive_calls.h"
#include "ir/Ir.h"

#include "Instrumentation.h"
#include "api.h"

#include "Flags.h"

#include "RIntlns.h"

using namespace llvm;

namespace rjit {

SEXP Compiler::compilePromise(std::string const& name, SEXP ast) {
    b.openPromise(name, ast);
    finalizeCompile(ast);
    return b.closePromise();
}

SEXP Compiler::compileFunction(std::string const& name, SEXP ast, SEXP formals,
                               bool optimize) {

    if (TYPEOF(ast) == NATIVESXP) {
        SEXP native = ast;
        ast = VECTOR_ELT(CDR(ast), 0);
        if (optimize) {
            TypeFeedback* tf = new TypeFeedback(native);
            tf->clearInvocationCount();
            b.openFunction(name, ast, formals, tf);
        } else {
            b.openFunction(name, ast, formals);
        }
    } else {
        if (TYPEOF(ast) == BCODESXP) {
            ast = VECTOR_ELT(CDR(ast), 0);
        }
        b.openFunction(name, ast, formals);
    }

    if (!optimize && Flag::singleton().recompileHot) {
        // Check the invocation count and recompile the function if it is hot.
        auto limit =
            ConstantInt::get(b.getContext(), APInt(32, StringRef("500"), 10));
        auto invocations = ir::InvocationCount::create(b);
        auto condition = ir::IntegerLessThan::create(b, invocations, limit);
        BasicBlock* bRecompile = b.createBasicBlock("recompile");
        BasicBlock* next = b.createBasicBlock("functionBegin");

        // TODO set weight to mark that the branch is unlikely
        ir::Cbr::create(b, condition, next, bRecompile);

        b.setBlock(bRecompile);
        auto res =
            ir::Recompile::create(b, b.closure(), b.f(), b.consts(), b.rho());
        ir::Return::create(b, res);

        b.setBlock(next);
    }

    finalizeCompile(ast);

    return b.closeFunction();
}

void Compiler::finalizeCompile(SEXP ast) {
    Value* last = compileExpression(ast);

    // since we are going to insert implicit return, which is a simple return
    // even from a promise
    b.setResultJump(false);
    if (last != nullptr)
        compileReturn(last, /*tail=*/true);
}

void Compiler::finalize() {
    assert(!finalized);

    auto engine = JITCompileLayer::singleton.finalize(b);

    if (!RJIT_DEBUG) {
        // Keep the llvm ir around
        engine->removeModule(b.module());
        delete engine;
    }

    finalized = true;
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
    case REALSXP:
    case CPLXSXP:
    case STRSXP:
    case NILSXP:
    case CLOSXP:
    case INTSXP: {
        return ir::UserLiteral::create(b, value)->result();
    }
    case BCODESXP:
        return compileExpression(VECTOR_ELT(CDR(value), 0));
    default:
        assert(false && "Unknown SEXP type in compiled ast.");
    }
    return nullptr;
}

/** Compiles a symbol, which reads as variable read using genericGetVar
    intrinsic.
  */
Value* Compiler::compileSymbol(SEXP value) {
    assert(TYPEOF(value) == SYMSXP);
    auto name = CHAR(PRINTNAME(value));
    assert(strlen(name));

    Value* res = ir::GenericGetVar::create(b, b.rho(), value)->result();
    if (Flag::singleton().recordTypes && b.isFunction()) {
        auto tf = TypeFeedback::get(b.f());
        if (!tf) {
            ir::RecordType::create(b, value, res);
        } else {
            TypeInfo inf = tf->get(value);
            if (!Flag::singleton().unsafeOpt && !inf.isAny() &&
                !inf.isBottom()) {
                ir::CheckType::create(b, res,
                                      TypeFeedback::get(b.f())->get(value));
            }
        }
    }
    res->setName(name);
    return res;
}

/** Inline caching for a function (call) with operator (op)
 *  that have arguments (callArgs).
 */

Value* Compiler::compileICCallStub(Value* call, Value* op,
                                   std::vector<Value*>& callArgs) {
    auto size = callArgs.size();

    Function* ic_stub;
    {
        ir::Builder b_(b.module());
        ic_stub = ICCompiler::getStub(callArgs.size(), b_);
    }

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
    ic_args.push_back(ConstantInt::get(getGlobalContext(), APInt(64, 0)));

    return ir::ICStub::create(b, ic_stub, ic_args, size)->result();
}

Value* Compiler::compileCall(SEXP call) {
    Value* f;

    if (TYPEOF(CAR(call)) != SYMSXP) {
        // it is a complex function, first get the value of the function and
        // then check it
        f = compileExpression(CAR(call));
        ir::CheckFunction::create(b, f);
    } else {
        // it is simple function - try compiling it with intrinsics
        f = compileIntrinsic(call);
        if (f != nullptr)
            return f;
        // otherwise just do get function
        f = ir::GetFunction::create(b, b.rho(), CAR(call))->result();
        f->setName(CHAR(PRINTNAME(CAR(call))));
    }

    std::vector<Value*> args;
    compileArguments(CDR(call), args);

    return compileICCallStub(ir::Constant::create(b, call)->result(), f, args);
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
        return ir::UserLiteral::create(b, arg)->result();
    case SYMSXP:
        if (arg == R_DotsSymbol) {
            return ir::Constant::create(b, arg)->result();
        }
        if (arg == R_MissingArg) {
            return ir::Constant::create(b, arg)->result();
        }
    default: {
        SEXP code = compilePromise("promise", arg);
        // Should the objects be inside the builder?
        // not needed with new API, compile constant adds automaatically if not
        // present yet
        // b.addConstantPoolObject(code);
        return ir::UserLiteral::create(b, code)->result();
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
    CASE(symbol::Bracket)
    return compileBracket(call);
    CASE(symbol::DoubleBracket)
    return compileDoubleBracket(call);
    CASE(symbol::Colon)
    return compileColon(call);
    CASE(symbol::DoubleAnd)
    return compileDoubleAnd(call);
    CASE(symbol::DoubleOr)
    return compileDoubleOr(call);
    CASE(symbol::Parenthesis)
    return compileParenthesis(CDR(call));
    CASE(symbol::Function)
    return compileFunctionDefinition(CDR(call));
    CASE(symbol::Return) {
        return (CDR(call) == R_NilValue)
                   ? compileReturn(
                         ir::Constant::create(b, R_NilValue)->result())
                   : compileReturn(compileExpression(CAR(CDR(call))));
    }
    CASE(symbol::Assign) { return compileAssignment(call); }
    CASE(symbol::Assign2) { return compileAssignment(call); }
    CASE(symbol::SuperAssign) { return compileSuperAssignment(call); }
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
    return compileBinaryOrUnary<ir::GenericAdd, ir::GenericUnaryPlus>(call);
    CASE(symbol::Sub)
    return compileBinaryOrUnary<ir::GenericSub, ir::GenericUnaryMinus>(call);
    CASE(symbol::Mul)
    return compileBinary<ir::GenericMul>(call);
    CASE(symbol::Div)
    return compileBinary<ir::GenericDiv>(call);
    CASE(symbol::Pow)
    return compileBinary<ir::GenericPow>(call);
    CASE(symbol::Sqrt)
    return compileUnary<ir::GenericSqrt>(call);
    CASE(symbol::Exp)
    return compileUnary<ir::GenericExp>(call);
    CASE(symbol::Eq)
    return compileBinary<ir::GenericEq>(call);
    CASE(symbol::Ne)
    return compileBinary<ir::GenericNe>(call);
    CASE(symbol::Lt)
    return compileBinary<ir::GenericLt>(call);
    CASE(symbol::Le)
    return compileBinary<ir::GenericLe>(call);
    CASE(symbol::Ge)
    return compileBinary<ir::GenericGe>(call);
    CASE(symbol::Gt)
    return compileBinary<ir::GenericGt>(call);
    CASE(symbol::BitAnd)
    return compileBinary<ir::GenericBitAnd>(call);
    CASE(symbol::BitOr)
    return compileBinary<ir::GenericBitOr>(call);
    CASE(symbol::Not)
    return compileUnary<ir::GenericNot>(call);

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
        result = ir::Constant::create(b, R_NilValue)->result();
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

/** Compiling colons.
  */
Value* Compiler::compileColon(SEXP call) {

    SEXP expression = CDR(call);
    SEXP lhs = CAR(expression);
    SEXP rhs = CAR(CDR(expression));

    if (colonCasesNotHandled(lhs) || colonCasesNotHandled(rhs)) {
        return nullptr;
    }

    Value* resultLHS = compileExpression(lhs);
    Value* resultRHS = compileExpression(rhs);
    assert(resultLHS);
    assert(resultRHS);

    b.setResultVisible(true);
    return ir::ColonValue::create(b, resultLHS, resultRHS, b.rho(), call)
        ->result();
}

/** Compiling double and. The semantics for a && b, if a is false
    then don't execute b - return a if a is false else return b.
    The branching is generated by the phi notes.
    If b is executed then the semantics changes.
  */
Value* Compiler::compileDoubleAnd(SEXP call) {

    SEXP expression = CDR(call);
    SEXP lhs = CAR(expression);
    SEXP rhs = CAR(CDR(expression));

    if (logicCasesNotHandled(lhs) || logicCasesNotHandled(rhs)) {
        return nullptr;
    }

    Value* resultLHS = compileExpression(lhs);
    Value* lhsValue =
        ir::ConvertToLogicalNoNA::create(b, resultLHS, lhs)->result();

    BasicBlock* lhsTrue = b.createBasicBlock("lhsTrue");
    BasicBlock* lhsFalse = b.createBasicBlock("lhsFalse");
    BasicBlock* next = b.createBasicBlock("next");
    ir::CbrZero::create(b, lhsValue, lhsTrue, lhsFalse);

    // // When lhs is false, return lhs
    b.setBlock(lhsFalse);
    lhsFalse = b.block();
    ir::Branch::create(b, next);

    // When lhs is true, return rhs
    b.setBlock(lhsTrue);
    Value* resultRHS = compileExpression(rhs);
    lhsTrue = b.block();
    ir::Branch::create(b, next);

    // Setting the phi node
    b.setBlock(next);
    PHINode* phi = PHINode::Create(t::SEXP, 2, "doubleAndBranch", b);
    phi->addIncoming(resultLHS, lhsFalse);
    phi->addIncoming(resultRHS, lhsTrue);

    b.setResultVisible(true);
    return phi;
}

/** Compiling double or.
  */
Value* Compiler::compileDoubleOr(SEXP call) {

    SEXP expression = CDR(call);
    SEXP lhs = CAR(expression);
    SEXP rhs = CAR(CDR(expression));

    if (logicCasesNotHandled(lhs) || logicCasesNotHandled(rhs)) {
        return nullptr;
    }

    Value* resultLHS = compileExpression(lhs);
    Value* lhsValue =
        ir::ConvertToLogicalNoNA::create(b, resultLHS, lhs)->result();

    BasicBlock* lhsTrue = b.createBasicBlock("lhsTrue");
    BasicBlock* lhsFalse = b.createBasicBlock("lhsFalse");
    BasicBlock* next = b.createBasicBlock("next");
    ir::CbrZero::create(b, lhsValue, lhsTrue, lhsFalse);

    // // When lhs is false, return rhs
    b.setBlock(lhsFalse);
    Value* resultRHS = compileExpression(rhs);
    lhsFalse = b.block();
    ir::Branch::create(b, next);

    // When lhs is true, return lhs
    b.setBlock(lhsTrue);
    lhsTrue = b.block();
    ir::Branch::create(b, next);

    // Setting the phi node
    b.setBlock(next);
    PHINode* phi = PHINode::Create(t::SEXP, 2, "doubleOrBranch", b);
    phi->addIncoming(resultRHS, lhsFalse);
    phi->addIncoming(resultLHS, lhsTrue);

    b.setResultVisible(true);
    return phi;
}

/** Compile vector access (single bracket).
    The cases that we are not currently handling for vector access is when the
    index being accessed is empty.
    We do not handle array access, i.e. x[a,b,c,...,n]
    To compile matrices the compileMatrixRead flag in Flag.h need to be set to
   true.
  */
Value* Compiler::compileBracket(SEXP call) {

    // This flag is used to determine if compileBracket was called
    // after a failed attempt at compiling assignment.
    // If the compilation for assignment to a vector failed then
    // we can not use this primitive function, otherwise the assignment
    // will be performed on the value inside the vector instead of on
    // the vector.
    if (b.getAssignmentLHS()) {
        return nullptr;
    }

    // Retrieving the vector and index from the AST.
    SEXP expression = CDR(call);
    SEXP vector = CAR(expression);
    SEXP index = CAR(CDR(expression));

    Value* resultVector = compileExpression(vector);
    assert(resultVector);

    Value* resultIndex = nullptr;
    if (emptyIndex(index)) {
        resultIndex = ir::Constant::create(b, index)->result();
    } else {
        resultIndex = compileExpression(index);
    }

    // Checks the AST is matrix access (and not array access).
    if (CDDR(expression) != R_NilValue && CDDDR(expression) == R_NilValue &&
        Flag::singleton().compileMatrixRead) {

        SEXP col = CAR(CDDR(expression));
        Value* resultCol = nullptr;

        // Ensuring we handle the column value of the matrix.
        if (emptyIndex(col)) {
            resultCol = ir::Constant::create(b, col)->result();
        } else {
            resultCol = compileExpression(col);
        }

        b.setResultVisible(true);
        return ir::GetMatrixValue::create(b, resultVector, resultIndex,
                                          resultCol, b.rho(), call)
            ->result();

        // Vector access for single bracket.
    } else if (CDDR(expression) == R_NilValue) {

        b.setResultVisible(true);
        return ir::GetDispatchValue::create(b, resultVector, resultIndex,
                                            b.rho(), call)
            ->result();
        // TODO need to handle n-dimen array access.
    } else {
        return nullptr;
    }
}

/** Compile vector access (double bracket).
    In compilation there is no difference between the semantics for double
    bracket vector access and single backet vector access.
*/
Value* Compiler::compileDoubleBracket(SEXP call) {

    // This flag is used to determine if compileBracket was called
    // after a failed attempt at compiling assignment.
    // If the compilation for assignment to a vector failed then
    // we can not use this primitive function, otherwise the assignment
    // will be performed on the value inside the vector instead of on
    // the vector.
    if (b.getAssignmentLHS()) {
        return nullptr;
    }

    // Retrieving the vector and index from the AST.
    SEXP expression = CDR(call);
    SEXP vector = CAR(expression);
    SEXP index = CAR(CDR(expression));

    Value* resultVector = compileExpression(vector);
    assert(resultVector);

    Value* resultIndex = nullptr;
    if (emptyIndex(index)) {
        resultIndex = ir::Constant::create(b, index)->result();
    } else {
        resultIndex = compileExpression(index);
    }

    // Checks the AST that the expression is matrix access (and not array
    // access).
    if (CDDR(expression) != R_NilValue && CDDDR(expression) == R_NilValue &&
        Flag::singleton().compileMatrixRead) {

        SEXP col = CAR(CDDR(expression));
        Value* resultCol = nullptr;

        // Ensuring we handle the column value of the matrix.
        if (emptyIndex(col)) {
            resultCol = ir::Constant::create(b, col)->result();
        } else {
            resultCol = compileExpression(col);
        }

        b.setResultVisible(true);
        return ir::GetMatrixValue2::create(b, resultVector, resultIndex,
                                           resultCol, b.rho(), call)
            ->result();

        // Vector access for double bracket.
    } else if (CDDR(expression) == R_NilValue) {

        b.setResultVisible(true);
        return ir::GetDispatchValue2::create(b, resultVector, resultIndex,
                                             b.rho(), call)
            ->result();

        // TODO need to handle n-dimen array access.
    } else {
        return nullptr;
    }
}

/** Compiling single bracket vector assignment ( and super assignment).
*/

Value* Compiler::compileAssignBracket(SEXP call, SEXP vector, SEXP index,
                                      SEXP value, bool super) {

    Value* resultVector = compileExpression(vector);
    assert(resultVector);
    Value* resultVal = compileExpression(value);

    Value* resultIndex = nullptr;
    if (emptyIndex(index)) {
        resultIndex = ir::Constant::create(b, index)->result();
    } else {
        resultIndex = compileExpression(index);
    }

    // Create the primitive function for super assignment.
    if (super) {
        ir::SuperAssignDispatch::create(b, resultVector, resultIndex, resultVal,
                                        b.rho(), call)
            ->result();
        b.setResultVisible(false);
        return resultVal;
    }

    ir::AssignDispatchValue::create(b, resultVector, resultIndex, resultVal,
                                    b.rho(), call)
        ->result();
    b.setResultVisible(false);
    return resultVal;
}

/** Compiling matrix over single bracket for normal and super assignment.
    Matrices assignment is not being handled in this release.
*/
Value* Compiler::compileAssignMatrix(SEXP call, SEXP vector, SEXP row, SEXP col,
                                     SEXP value, bool super) {

    Value* resultVector = compileExpression(vector);
    assert(resultVector);
    Value* resultVal = compileExpression(value);

    Value* resultRow = nullptr;
    if (emptyIndex(row)) {
        resultRow = ir::Constant::create(b, row)->result();
    } else {
        resultRow = compileExpression(row);
    }

    Value* resultCol = nullptr;
    if (emptyIndex(col)) {
        resultCol = ir::Constant::create(b, col)->result();
    } else {
        resultCol = compileExpression(col);
    }

    // Super assignment for matrices
    if (super) {
        ir::SuperAssignMatrix::create(b, resultVector, resultRow, resultCol,
                                      resultVal, b.rho(), call)
            ->result();

        b.setResultVisible(false);
        return resultVal;
    }

    ir::AssignMatrixValue::create(b, resultVector, resultRow, resultCol,
                                  resultVal, b.rho(), call)
        ->result();
    b.setResultVisible(false);
    return resultVal;
}

/** Compiling double bracket for normal and super assignment.
*/

Value* Compiler::compileAssignDoubleBracket(SEXP call, SEXP vector, SEXP index,
                                            SEXP value, bool super) {

    Value* resultVector = compileExpression(vector);
    assert(resultVector);
    Value* resultVal = compileExpression(value);

    Value* resultIndex = nullptr;
    if (emptyIndex(index)) {
        resultIndex = ir::Constant::create(b, index)->result();
    } else {
        resultIndex = compileExpression(index);
    }

    // Create the primitive function for super assignment.
    if (super) {
        ir::SuperAssignDispatch2::create(b, resultVector, resultIndex,
                                         resultVal, b.rho(), call)
            ->result();

        b.setResultVisible(false);
        return resultVal;
    }

    ir::AssignDispatchValue2::create(b, resultVector, resultIndex, resultVal,
                                     b.rho(), call)
        ->result();
    b.setResultVisible(false);
    return resultVal;
}

/** Compiling matrix over single bracket for normal and super assignment.
    Matrices assignment over double brackets is not being handled in this
   release.
*/
Value* Compiler::compileAssignDoubleMatrix(SEXP call, SEXP vector, SEXP row,
                                           SEXP col, SEXP value, bool super) {

    Value* resultVector = compileExpression(vector);
    assert(resultVector);
    Value* resultVal = compileExpression(value);

    Value* resultRow = nullptr;
    if (emptyIndex(row)) {
        resultRow = ir::Constant::create(b, row)->result();
    } else {
        resultRow = compileExpression(row);
    }

    Value* resultCol = nullptr;
    if (emptyIndex(col)) {
        resultCol = ir::Constant::create(b, col)->result();
    } else {
        resultCol = compileExpression(col);
    }

    // Super assignment for double bracket matrices
    if (super) {
        ir::SuperAssignMatrix2::create(b, resultVector, resultRow, resultCol,
                                       resultVal, b.rho(), call)
            ->result();

        b.setResultVisible(false);
        return resultVal;
    }

    ir::AssignMatrixValue2::create(b, resultVector, resultRow, resultCol,
                                   resultVal, b.rho(), call)
        ->result();
    b.setResultVisible(false);
    return resultVal;
}

/** The index of a vector can not be empty.
*/
bool Compiler::emptyIndex(SEXP index) {

    // TODO handle the case when the index is empty.
    // In this case only the "relevant" attributes of vector are retained.
    if (TYPEOF(index) == SYMSXP && !strlen(CHAR(PRINTNAME(index)))) {
        return true;
    }

    return false;
}

/** We only handle vectors that are symbols for vector assignment.
*/
bool Compiler::caseHandledVector(SEXP vector) {

    // For vector assignment only vectors that are symbols are handled.
    if (TYPEOF(vector) != SYMSXP) {
        return false;
    }

    return true;
}

/** Returns true for all the cases currently not being handled for
    && and ||
*/
bool Compiler::logicCasesNotHandled(SEXP logic) {

    // For vector assignment only vectors that are symbols are handled.
    if (logic == R_NilValue) {
        return true;
    } else if (TYPEOF(logic) == SPECIALSXP) {
        return true;
    } else if (TYPEOF(logic) == BUILTINSXP) {
        return true;
    } else if (TYPEOF(logic) == LANGSXP) {
        // TODO I really need to handle this case!
        return true;
    }

    return false;
}

/** Returns true for all the cases currently not being handled for :
*/
bool Compiler::colonCasesNotHandled(SEXP logic) {

    // For vector assignment only vectors that are symbols are handled.
    if (logic == R_NilValue) {
        return true;
    } else if (TYPEOF(logic) == SPECIALSXP) {
        return true;
    } else if (TYPEOF(logic) == BUILTINSXP) {
        return true;
    } else if (TYPEOF(logic) == LANGSXP && TYPEOF(CAR(logic)) != SYMSXP) {
        // TODO I really need to handle this case!
        return true;
    }

    return false;
}

/** Compiling assignments (<-).
    The genericSetVar primitive function is created when the LHS is a symbol.
    Otherwise primitive functions for vector and matrix assignment is created
    if the LHS is a vector or matrix.
  */
Value* Compiler::compileAssignment(SEXP e) {

    // return nullptr;

    if (b.getAssignmentLHS()) {
        return nullptr;
    }

    SEXP expr = CDR(e);

    if (TYPEOF(CAR(expr)) == SYMSXP) {
        Value* v = compileExpression(CAR(CDR(expr)));
        ir::GenericSetVar::create(b, v, b.rho(), CAR(expr));
        b.setResultVisible(false);
        return v;
    }

    // Retrieve the LHS and RHS of the assignment from the AST.
    SEXP lhs = CAR(expr);
    SEXP rhs = CAR(CDR(expr));

    // Check if the LHS is a language object
    if (TYPEOF(lhs) == LANGSXP && CDR(lhs) && CDDR(lhs)) {
        SEXP vector = CAR(CDR(lhs));
        SEXP index = CAR(CDDR(lhs));

        // Check we handle the index and vector.
        if (caseHandledVector(vector)) {

            // Matrix assignment
            if (CDDDR(lhs) != R_NilValue && CDR(CDDDR(lhs)) == R_NilValue &&
                Flag::singleton().compileMatrixWrite) {

                SEXP col = CAR(CDDDR(lhs));
                // Single bracket matrix assignment
                if (CAR(lhs) == symbol::Bracket) {
                    return compileAssignMatrix(lhs, vector, index, col, rhs,
                                               false);
                }

                // Double bracket matrix assignment
                if (CAR(lhs) == symbol::DoubleBracket) {
                    return compileAssignDoubleMatrix(lhs, vector, index, col,
                                                     rhs, false);
                }
            }

            // Vector assignmnt
            if (CDDDR(lhs) == R_NilValue) {

                if (CAR(lhs) == symbol::Bracket) {
                    return compileAssignBracket(lhs, vector, index, rhs, false);
                }

                if (CAR(lhs) == symbol::DoubleBracket) {
                    return compileAssignDoubleBracket(lhs, vector, index, rhs,
                                                      false);
                }
            }
        }
    }

    b.setAssignmentLHS(true);
    Value* result = compileExpression(e);
    b.setAssignmentLHS(false);
    return result;
}

/** Compiling super assignments (<<-).
    The genericSetVar primitive function is created when the LHS is a symbol.
    Otherwise primitive functions for vector and matrix assignment is created
    if the LHS is a vector or matrix.
  */
Value* Compiler::compileSuperAssignment(SEXP e) {

    if (b.getAssignmentLHS()) {
        return nullptr;
    }

    SEXP expr = CDR(e);

    if (TYPEOF(CAR(expr)) == SYMSXP) {
        Value* v = compileExpression(CAR(CDR(expr)));
        ir::GenericSetVarParent::create(b, v, b.rho(), CAR(expr));
        b.setResultVisible(false);
        return v;
    }

    // Retrieve the LHS and RHS of the assignment from the AST.
    SEXP lhs = CAR(expr);
    SEXP rhs = CAR(CDR(expr));

    // Check if the LHS is a language object
    if (TYPEOF(lhs) == LANGSXP && CDR(lhs) && CDDR(lhs)) {
        SEXP vector = CAR(CDR(lhs));
        SEXP index = CAR(CDDR(lhs));

        // Check we handle the index and vector.
        if (caseHandledVector(vector)) {

            // Matrix assignment (NOT being handled in this release).
            if (CDDDR(lhs) != R_NilValue && CDR(CDDDR(lhs)) == R_NilValue &&
                Flag::singleton().compileSuperMatrixWrite) {

                SEXP col = CAR(CDDDR(lhs));

                // Single bracket matrix assignment
                if (CAR(lhs) == symbol::Bracket) {
                    return compileAssignMatrix(lhs, vector, index, col, rhs,
                                               true);
                }

                // Double bracket matrix assignment
                if (CAR(lhs) == symbol::DoubleBracket) {
                    return compileAssignDoubleMatrix(lhs, vector, index, col,
                                                     rhs, true);
                }
            }

            // Vector assignmnt
            if (CDDDR(lhs) == R_NilValue) {

                if (CAR(lhs) == symbol::Bracket) {
                    return compileAssignBracket(lhs, vector, index, rhs, true);
                }

                if (CAR(lhs) == symbol::DoubleBracket) {
                    return compileAssignDoubleBracket(lhs, vector, index, rhs,
                                                      true);
                }
            }
        }
    }

    // When our assignment primitive function is not created, we must ensure
    // any vector read on the LHS is also not handled by our primitive function.
    b.setAssignmentLHS(true);
    Value* result = compileExpression(e);
    b.setAssignmentLHS(false);
    return result;
}

/** Similar to R bytecode compiler, only the body of the created function is
  compiled, the default arguments are left in their ast forms for now.

  TODO this should change.
 */
Value* Compiler::compileFunctionDefinition(SEXP fdef) {
    SEXP forms = CAR(fdef);
    SEXP body = compileFunction("function", CAR(CDR(fdef)), forms);
    return ir::CreateClosure::create(b, b.rho(), forms, body)->result();
}

/** Return calls or returns in general are compiled depending on the context.
 * Usually a simple return instruction in bitcode is enough, but while in
 * promises, we must use longjmp, which is done by calling returnJump intrinsic.
  */
Value* Compiler::compileReturn(Value* value, bool tail) {
    if (not b.getResultVisible())
        ir::MarkInvisible::create(b);
    if (b.getResultJump()) {
        ir::ReturnJump::create(b, value, b.rho());
        // we need to have a return instruction as well to fool LLVM into
        // believing the basic block has a terminating instruction
        ir::Return::create(b, ir::Constant::create(b, R_NilValue)->result());
    } else {
        ir::Return::create(b, value);
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
    Value* cond = ir::ConvertToLogicalNoNA::create(b, cond2, condAst)->result();
    BasicBlock* ifTrue = b.createBasicBlock("ifTrue");
    BasicBlock* ifFalse = b.createBasicBlock("ifFalse");
    BasicBlock* next = b.createBasicBlock("next");
    ir::CbrZero::create(b, cond, ifTrue, ifFalse);

    // true case has to be always present
    b.setBlock(ifTrue);
    Value* trueResult = compileExpression(trueAst);
    ir::Branch::create(b, next);
    ifTrue = b.block();

    // false case may not be present in which case invisible R_NilValue should
    // be returned
    b.setBlock(ifFalse);
    Value* falseResult;
    if (falseAst == nullptr) {
        falseResult = ir::Constant::create(b, R_NilValue)->result();
        b.setResultVisible(false);
    } else {
        falseResult = compileExpression(falseAst);
    }
    ifFalse = b.block();
    ir::Branch::create(b, next);

    // add a phi node for the result
    b.setBlock(next);
    PHINode* phi = PHINode::Create(t::SEXP, 2, "", b);
    phi->addIncoming(trueResult, ifTrue);
    phi->addIncoming(falseResult, ifFalse);

    b.setResultVisible(true);
    return phi;
}

/** Compiles break. Whenever we see break in the compiler, we know it is for a
  loop where context was skipped and therefore it must always be translated as
  direct jump in bitcode.

  TODO The error is probably not right.
   */
Value* Compiler::compileBreak(SEXP ast) {
    llvm::BasicBlock* bb = b.breakTarget();
    ir::Branch::create(b, bb);
    // TODO this is really simple, but fine for us - dead code elimination will
    // remove the block if required
    b.setBlock(b.createBasicBlock("deadcode"));
    return ir::Constant::create(b, R_NilValue)->result();
}

/** Compiles next. Whenever we see next in the compiler, we know it is for a
  loop where context was skipped and therefore it must always be translated as
  direct jump in bitcode.

  TODO The error is probably not right.
   */
Value* Compiler::compileNext(SEXP ast) {
    llvm::BasicBlock* bb = b.nextTarget();
    ir::Branch::create(b, bb);
    // TODO this is really simple, but fine for us - dead code elimination will
    // remove the block if required
    b.setBlock(b.createBasicBlock("deadcode"));
    return ir::Constant::create(b, R_NilValue)->result();
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

    ir::Branch::create(b, b.nextTarget());
    b.setBlock(b.nextTarget());

    compileExpression(bodyAst);
    ir::Branch::create(b, b.nextTarget());
    b.setBlock(b.breakTarget());
    // restore the old loop pointers in the context
    b.closeLoop();
    // return R_NilValue
    b.setResultVisible(false);
    return ir::Constant::create(b, R_NilValue)->result();
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

    ir::Branch::create(b, b.nextTarget());
    b.setBlock(b.nextTarget());
    // compile the condition
    Value* cond2 = compileExpression(condAst);
    Value* cond = ir::ConvertToLogicalNoNA::create(b, cond2, condAst)->result();
    BasicBlock* whileBody = b.createBasicBlock("whileBody");
    ir::CbrZero::create(b, cond, whileBody, b.breakTarget());
    // compile the body
    b.setBlock(whileBody);
    compileExpression(bodyAst);
    ir::Branch::create(b, b.nextTarget());
    b.setBlock(b.breakTarget());
    // restore the old loop pointers in the context
    b.closeLoop();
    // return R_NilValue
    b.setResultVisible(false);
    return ir::Constant::create(b, R_NilValue)->result();
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
    BasicBlock* forBody = b.createBasicBlock("forBody");
    // now initialize the loop control structures
    Value* seq2 = compileExpression(seqAst);
    Value* seq = ir::StartFor::create(b, seq2, b.rho())->result();
    Value* seqLength = ir::LoopSequenceLength::create(b, seq, ast)->result();
    Value* cInit = ir::Constant::create(b, R_NilValue)->result();

    BasicBlock* forStart = b.block();
    ir::Branch::create(b, forCond);
    b.setBlock(forCond);
    PHINode* control = PHINode::Create(t::Int, 2, "loopControl", b);
    PHINode* index = PHINode::Create(t::SEXP, 2, "index", b.block());

    control->addIncoming(b.integer(0), forStart);
    index->addIncoming(cInit, forStart);
    // now check if control is smaller than length
    auto test = ir::UnsignedIntegerLessThan::create(b, control, seqLength);
    BranchInst::Create(forBody, b.breakTarget(), test->result(), b);

    // move to the for loop body, where we have to set the control variable
    // properly
    b.setBlock(forBody);
    Value* controlValue =
        ir::GetForLoopValue::create(b, seq, control, index)->result();
    ir::GenericSetVar::create(b, controlValue, b.rho(), controlAst);
    // now compile the body of the loop
    compileExpression(bodyAst);
    ir::Branch::create(b, b.nextTarget());

    // in the next block, increment the internal control variable and jump to
    // forCond
    b.setBlock(b.nextTarget());
    // TODO: Need an intrinsic function for BinaryOperator
    Value* control1 =
        ir::IntegerAdd::create(b, control, b.integer(1))->result();
    control->addIncoming(control1, b.nextTarget());
    index->addIncoming(controlValue, b.nextTarget());
    ir::Branch::create(b, forCond);

    b.setBlock(b.breakTarget());
    // restore the old loop pointers in the context
    b.closeLoop();
    // return R_NilValue
    b.setResultVisible(false);
    return ir::Constant::create(b, R_NilValue)->result();
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

    if (condAst == R_NilValue) {
        return nullptr;
    }

    // actual switch compilation - get the control value and check it
    Value* control = compileExpression(condAst);

    if (caseAsts.size() == 0) {
        return compileExpression(R_NilValue);
    }

    ir::CheckSwitchControl::create(b, control, call);
    Value* ctype = ir::SexpType::create(b, control)->result();
    auto cond = ir::IntegerEquals::create(b, ctype, b.integer(STRSXP));
    BasicBlock* switchIntegral = b.createBasicBlock("switchIntegral");
    BasicBlock* switchCharacter = b.createBasicBlock("switchCharacter");
    BasicBlock* switchNext = b.createBasicBlock("switchNext");

    BranchInst::Create(switchCharacter, switchIntegral, cond->result(), b);

    // integral switch is simple
    b.setBlock(switchIntegral);

    Value* caseIntegral =
        ir::SwitchControlInteger::create(b, control, caseAsts.size())->result();
    auto swInt =
        ir::Switch::create(b, caseIntegral, switchNext, caseAsts.size());
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
    Value* caseCharacter =
        ir::SwitchControlCharacter::create(b, control, call, cases)->result();

    auto swChar =
        ir::Switch::create(b, caseCharacter, switchNext, caseAsts.size());
    // create the phi node at the end
    b.setBlock(switchNext);
    PHINode* result = PHINode::Create(t::SEXP, caseAsts.size(), "", b);
    // walk the cases and create their blocks, add them to switches and their
    // results to the phi node
    // TODO: fix empty switch
    BasicBlock* last = nullptr;
    BasicBlock* fallThrough = nullptr;
    for (unsigned i = 0; i < caseAsts.size(); ++i) {
        last = b.createBasicBlock("switchCase");
        if (fallThrough != nullptr) {
            ir::Branch::create(b, last);
            fallThrough = nullptr;
        }
        b.setBlock(last);
        swInt->addCase(i, last);
        if (defaultIdx == -1 or defaultIdx > static_cast<int>(i)) {
            swChar->addCase(i, last);
        } else if (defaultIdx < static_cast<int>(i)) {
            swChar->addCase(i - 1, last);
        } else {
            swChar->addCase(caseAsts.size() - 1, last);
            swChar->setDefaultDest(last);
        }
        SEXP value = caseAsts[i];
        if (TYPEOF(value) == SYMSXP && !strlen(CHAR(PRINTNAME(value)))) {
            fallThrough = b.block();
        } else {
            Value* caseResult = compileExpression(caseAsts[i]);
            ir::Branch::create(b, switchNext);
            result->addIncoming(caseResult, b.block());
        }
    }
    if (swChar->getDefaultDest() == switchNext)
        swChar->setDefaultDest(last);
    swInt->setDefaultDest(last);
    if (fallThrough != nullptr) {
        result->addIncoming(ir::Constant::create(b, R_NilValue)->result(),
                            b.block());
        ir::Branch::create(b, switchNext);
    }
    b.setBlock(switchNext);
    return result;
}

std::set<Compiler*> Compiler::_instances;

void Compiler::gcCallback(void (*forward_node)(SEXP)) {
    for (Compiler* c : _instances) {
        // if this happens gc triggered after finalize() but before dtr
        // please restrict lifetime of Compiler such that dtr is called right
        // after finalize
        assert(!c->finalized);
        if (!c->finalized)
            c->doGcCallback(forward_node);
    }
}

void Compiler::doGcCallback(void (*forward_node)(SEXP)) {
    b.doGcCallback(forward_node);
}

/** The fast functions for vector (store) retrieval (x[a]).
    If we know statically the type of x and a, and if a has
    the correct type, then generate the LLVM native that
    access the memory location of x at a.
*/

Value* vectorRetr(SEXP vector, SEXP index) { return nullptr; }
}
