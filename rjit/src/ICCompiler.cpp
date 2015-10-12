#include <llvm/IR/Verifier.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/raw_ostream.h>
#include "llvm/Analysis/Passes.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/CodeGen/GCStrategy.h"
#include "llvm/CodeGen/GCs.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#include "ICCompiler.h"

#include "Compiler.h"
#include "JITMemoryManager.h"
#include "JITCompileLayer.h"
#include "StackMap.h"
#include "StackMapParser.h"
#include "CodeCache.h"
#include "ir/Builder.h"
#include "ir/intrinsics.h"
#include "ir/ir.h"

#include "JITCompileLayer.h"

#include "RIntlns.h"

#include <sstream>

using namespace llvm;

namespace rjit {

ICCompiler::ICCompiler(unsigned size, ir::Builder& b) : b(b), size(size) {
#define DECLARE(name, type)                                                    \
    name = llvm::Function::Create(                             \
        t::type, llvm::Function::ExternalLinkage, #name, b.module())

    DECLARE(CONS_NR, sexp_sexpsexp);
    DECLARE(closureQuickArgumentAdaptor, sexp_sexpsexp);
    DECLARE(initClosureContext, void_cntxtsexpsexpsexpsexpsexp);
    DECLARE(endClosureContext, void_cntxtsexp);
    DECLARE(closureNativeCallTrampoline, sexp_contxtsexpsexp);
    DECLARE(compileIC, compileIC_t);
    DECLARE(patchIC, patchIC_t);
    DECLARE(callNative, sexp_sexpsexp);
#undef DECLARE

    // Set up a function type which corresponds to the ICStub signature
    std::vector<Type*> argT;
    for (unsigned i = 0; i < size + 3; i++) {
        argT.push_back(t::SEXP);
    }
    argT.push_back(t::nativeFunctionPtr_t);
    argT.push_back(t::t_i64);

    auto funT = FunctionType::get(t::SEXP, argT, false);
    ic_t = funT;
}

void ICCompiler::initFunction(std::string name) {
    b.openIC(name, ic_t);
}

std::string ICCompiler::stubName(unsigned size) {
    std::ostringstream os;
    os << "icStub_" << size;
    return os.str();
}

Function* ICCompiler::getStub(unsigned size, ir::Builder& b) {

    return CodeCache::get(stubName(size),
                          [size, &b]() {
                              ICCompiler stubCompiler(size, b);
                              return stubCompiler.compileCallStub();
                          },
                          b.module());
}

void* ICCompiler::compile(SEXP inCall, SEXP inFun, SEXP inRho) {
    initFunction("callIC");

    if (!compileIc(inCall, inFun))
        compileGenericIc(inCall, inFun);

    return finalize();
}

void* ICCompiler::finalize() {
    // FIXME: Allocate a NATIVESXP, or link it to the caller??

    // m.dump();
    auto handle = JITCompileLayer::getHandle(b.module());
    auto ic = JITCompileLayer::getFunctionPointer(handle, b.f()->getName());

    return ic;
}

Function* ICCompiler::compileCallStub() {
    initFunction(stubName(size));

    Value* icAddr = INTRINSIC(
        compileIC, ConstantInt::get(getGlobalContext(), APInt(64, size)),
        call(), fun(), rho(), stackmapId());

    INTRINSIC(patchIC, icAddr, stackmapId(), caller());
    //create new intrinics function for patchIC (maybe?)

    Value* ic = new BitCastInst(icAddr, PointerType::get(ic_t, 0), "", b);

    auto res = INTRINSIC_NO_SAFEPOINT(ic, b.args());
    ReturnInst::Create(getGlobalContext(), res, b);

    return b.f();
}

bool ICCompiler::compileIc(SEXP inCall, SEXP inFun) {
    auto f = b.f();

    if (TYPEOF(inFun) == CLOSXP) {
        std::vector<bool> promarg(size, false);

        // Check for named args or ...
        SEXP arg = CDR(inCall);
        SEXP form = FORMALS(inFun);
        unsigned i = 0;
        while (arg != R_NilValue && form != R_NilValue) {
            // We do not yet do the static version of match.c, thus cannot
            // support named args
            if (TAG(arg) != R_NilValue)
                return false;

            // We cannot inline ellipsis
            if (CAR(arg) == R_DotsSymbol || CAR(form) == R_DotsSymbol)
                return false;

            switch (TYPEOF(CAR(arg))) {
            case LGLSXP:
            case INTSXP:
            case REALSXP:
            case CPLXSXP:
            case STRSXP:
                break;
            default:
                promarg[i] = true;
            }
            i++;
            arg = CDR(arg);
            form = CDR(form);
        }

        // number of args != number of formal args, fallback to generic
        if (form != R_NilValue || i != size)
            return false;

        SEXP inBody = CDR(inFun);
        // TODO: If the body is not native we could jit it here
        if (TYPEOF(inBody) == NATIVESXP) {

            BasicBlock* icMatch =
                BasicBlock::Create(getGlobalContext(), "icMatch", f, nullptr);
            BasicBlock* icMiss =
                BasicBlock::Create(getGlobalContext(), "icMiss", f, nullptr);
            BasicBlock* end =
                BasicBlock::Create(getGlobalContext(), "end", f, nullptr);

            // Insert a guard to check if the incomming function matches
            // the one we got this time
            ICmpInst* test = new ICmpInst(*b.block(), ICmpInst::ICMP_EQ, fun(),
                                          b.constantPoolSexp(inFun), "guard");
            BranchInst::Create(icMatch, icMiss, test, b.block());
            b.setBlock(icMatch);

            // This is an inlined version of applyNativeClosure
            Value* arglist = b.constantPoolSexp(R_NilValue);

            // This reverses the arglist, but quickArgumentAdapter
            // reverses again
            // TODO: construct the environment in one go,
            // without using quickArgumentAdapter
            for (unsigned i = 0; i < size; ++i) {
                Value* arg = b.args()[i];
                if (promarg[i])
                    arg = INTRINSIC(b.intrinsic<ir::CreatePromise>(), arg, rho());
                arglist = INTRINSIC(CONS_NR, arg, arglist);
            }

            Value* newrho = 
                INTRINSIC(closureQuickArgumentAdaptor, fun(), arglist);

            Value* cntxt = new AllocaInst(t::cntxt, "", b.block());

            INTRINSIC(initClosureContext, cntxt, call(), newrho, rho(), arglist,
                      fun());

            Value* res = INTRINSIC_NO_SAFEPOINT(
                closureNativeCallTrampoline, cntxt, b.constantPoolSexp(inBody), newrho);


            INTRINSIC(endClosureContext, cntxt, res);

            ir::Branch::create(b, end);
            b.setBlock(icMiss);

            Value* missRes = callMyStub();
            ir::Branch::create(b, end);
            b.setBlock(end);

            PHINode* phi = PHINode::Create(t::SEXP, 2, "", b);
            phi->addIncoming(res, icMatch);
            phi->addIncoming(missRes, icMiss);
            ir::Return::create(b, phi);

            return true;
        }
    }
    return false;
}

Value* ICCompiler::callMyStub() {
    auto stub = getStub(size, b);
    return INTRINSIC_NO_SAFEPOINT(stub, b.args());
}

bool ICCompiler::compileGenericIc(SEXP inCall, SEXP inFun) {
    Value* call = compileCall(inCall, inFun);
    ir::Return::create(b, call);

    return true;
}

Value* ICCompiler::compileCall(SEXP call, SEXP op) {
    // TODO: only emit one branch depending on the type we currently see
    BasicBlock* icTestType = b.createBasicBlock("icTypeTest");
    BasicBlock* icMatch = b.createBasicBlock("icMatch");
    BasicBlock* icMiss = b.createBasicBlock("icMiss");
    BasicBlock* end = b.createBasicBlock("end");

    ICmpInst * test = new ICmpInst(*b.block(), ICmpInst::ICMP_EQ, b.constantPoolSexp(op), 
                                        fun(), "guard");
    BranchInst::Create(icMatch, icTestType, test, b.block());

    b.setBlock(icTestType);
    Value* ftype = ir::SexpType::create(b, fun());
    switch (TYPEOF(op)) {
    case SPECIALSXP:
        test = new ICmpInst(*b.block(), ICmpInst::ICMP_EQ, ftype, b.integer(SPECIALSXP),
                            "guard");
        break;     
    case BUILTINSXP:
        test = new ICmpInst(*b.block(), ICmpInst::ICMP_EQ, ftype, b.integer(BUILTINSXP),
                            "guard");
        break;
    case CLOSXP:
        test = new ICmpInst(*b.block(), ICmpInst::ICMP_EQ, ftype, b.integer(CLOSXP),
                            "guard");
        break;
    default:
        assert(false);
    }

    BranchInst::Create(icMatch, icMiss, test, b.block());
    b.setBlock(icMatch);

    Value* res;
    switch (TYPEOF(op)) {
    case SPECIALSXP:
        res = ir::CallSpecial::create(b, b.constantPoolSexp(call), fun(), b.constantPoolSexp(R_NilValue), b.rho());
        break;
    case BUILTINSXP: {
        Value* args = compileArguments(CDR(call), /*eager=*/true);
        res = ir::CallBuiltin::create(b, b.constantPoolSexp(call), fun(), args, rho());
        break;
    }
    case CLOSXP: {
        Value* args = compileArguments(CDR(call), /*eager=*/false);
        res = ir::CallClosure::create(b, b.constantPoolSexp(call), fun(), args, rho());
        break;
    }
    default:
        assert(false);
    }
    ir::Branch::create(b, end);
    b.setBlock(icMiss);

    Value* missRes = callMyStub();

    ir::Branch::create(b, end);
    b.setBlock(end);

    PHINode* phi = PHINode::Create(t::SEXP, 2, "", b.block());
    phi->addIncoming(res, icMatch);
    phi->addIncoming(missRes, icMiss);

    return phi;
}

/** Compiles arguments for given function.

  Creates the pairlist of arguments used in R from the arguments and their
  names.
  */
Value* ICCompiler::compileArguments(SEXP argAsts, bool eager) {
    Value* arglistHead = nullptr;
    Value* arglist = b.constantPoolSexp(R_NilValue);

    // if there are no arguments
    int argnum = 0;
    while (argAsts != R_NilValue) {
        arglist = compileArgument(arglist, argAsts, argnum++, eager);
        if (!arglistHead)
            arglistHead = arglist;
        argAsts = CDR(argAsts);
    }
    if (arglistHead)
        return arglistHead;
    return b.constantPoolSexp(R_NilValue);
}

/** Compiles a single argument.

  Self evaluating literals are always returned as SEXP constants, anything else
  is either evaluated directly if eager is true, or they are compiled as new
  promises.
 */
Value* ICCompiler::compileArgument(Value* arglist, SEXP argAst, int argnum,
                                   bool eager) {
    SEXP arg = CAR(argAst);
    Value* result;
    // This list has to stay in sync with Compiler::compileArgument
    // note: typeof(arg) does not correspond to the runtime type of the ic
    // arg, since the caller already converts non self evaluating arguments
    // to promises or native code.
    switch (TYPEOF(arg)) {
    case LGLSXP:
    case INTSXP:
    case REALSXP:
    case CPLXSXP:
    case STRSXP:
    case NILSXP:
        // literals are self-evaluating
        result = b.args()[argnum];
        break;
    case SYMSXP:
        if (arg == R_DotsSymbol) {
            return ir::AddEllipsisArgument::create(b, arglist, rho(),
                             eager ? b.integer(TRUE) : b.integer(FALSE));
        }
    // Fall through:
    default:
        if (eager) {
            // TODO make this more efficient?
            result = INTRINSIC(callNative, b.args()[argnum], rho());
        } else {
            // we must create a promise out of the argument
            result = INTRINSIC(b.intrinsic<ir::CreatePromise>(),  b.args()[argnum], rho());
        }
        break;
    }
    SEXP name = TAG(argAst);
    if (name != R_NilValue)
        return ir::AddKeywordArgument::create(b, arglist, result, b.constantPoolSexp(name));

    return ir::AddArgument::create(b, arglist, result);
}

Value* ICCompiler::INTRINSIC_NO_SAFEPOINT(llvm::Value* fun,
                                          std::vector<Value*> args) {
    return llvm::CallInst::Create(fun, args, fun->getName(), b.block());
}

Value* ICCompiler::INTRINSIC(llvm::Value* fun, std::vector<Value*> args) {
    llvm::CallInst * ins = llvm::CallInst::Create(fun, args, "", b.block());
    return b.insertCall(ins);
}

} // namespace rjit
