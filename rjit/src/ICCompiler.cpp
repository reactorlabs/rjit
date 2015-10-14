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

#include "JITCompileLayer.h"
#include "api.h"

#include "RIntlns.h"

#include <sstream>

using namespace llvm;

namespace rjit {

Value* loadConstant(SEXP value, Module* m, BasicBlock* b);

Value* insertCall(Value* fun, std::vector<Value*> args, Function* f,
                  BasicBlock* b, rjit::JITModule& m, bool safepoint);

void setupFunction(Function& f);

std::vector<bool> ICCompiler::hasStub;

ICCompiler::ICCompiler(unsigned size, JITModule& m)
    : ICCompiler(size, m, stubName(size)) {}

ICCompiler::ICCompiler(unsigned size, JITModule& m, std::string name)
    : name(name), m(m), size(size) {
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

void ICCompiler::initFunction() {
    assert(!f);

    f = Function::Create(ic_t, Function::ExternalLinkage, name, m);
    setupFunction(*f);
    b = BasicBlock::Create(getGlobalContext(), "start", f, nullptr);

    // Load the args in the same order as the stub
    Function::arg_iterator argI = f->arg_begin();
    for (unsigned i = 0; i < size; i++) {
        icArgs.push_back(argI++);
    }

    call = argI++;
    call->setName("call");
    fun = argI++;
    fun->setName("op");
    rho = argI++;
    rho->setName("rho");
    caller = argI++;
    caller->setName("caller");
    stackmapId = argI++;
    stackmapId->setName("stackmapId");
}

std::string ICCompiler::stubName(unsigned size) {
    std::ostringstream os;
    os << "icStub_" << size;
    return os.str();
}

Function* ICCompiler::getStub(unsigned size, JITModule& m) {

    return CodeCache::get(stubName(size),
                          [size, &m]() {
                              ICCompiler stubCompiler(size, m);
                              return stubCompiler.compileCallStub();
                          },
                          m);
}

void* ICCompiler::compile(SEXP inCall, SEXP inFun, SEXP inRho) {
    initFunction();

    if (RJIT_DEBUG)
        std::cout << " Compiling IC " << f->getName().str() << " @ " << (void*)f
                  << "\n";

    if (!compileIc(inCall, inFun))
        compileGenericIc(inCall, inFun);

    return finalize();
}

void* ICCompiler::finalize() {
    // FIXME: Allocate a NATIVESXP, or link it to the caller??

    // m.dump();
    auto engine = JITCompileLayer::singleton.getEngine(m.getM());
    auto ic = engine->getPointerToFunction(f);

    return ic;
}

Function* ICCompiler::compileCallStub() {
    initFunction();

    Value* icAddr = INTRINSIC(
        m.compileIC, ConstantInt::get(getGlobalContext(), APInt(64, size)),
        call, fun, rho, stackmapId);

    INTRINSIC(m.patchIC, icAddr, stackmapId, caller);

    Value* ic = new BitCastInst(icAddr, PointerType::get(ic_t, 0), "", b);

    std::vector<Value*> allArgs;
    allArgs.insert(allArgs.end(), icArgs.begin(), icArgs.end());
    allArgs.push_back(call);
    allArgs.push_back(fun);
    allArgs.push_back(rho);
    allArgs.push_back(caller);
    allArgs.push_back(stackmapId);

    auto res = INTRINSIC_NO_SAFEPOINT(ic, allArgs);
    ReturnInst::Create(getGlobalContext(), res, b);

    return f;
}

bool ICCompiler::compileIc(SEXP inCall, SEXP inFun) {
    if (TYPEOF(inFun) == CLOSXP) {
        std::vector<bool> promarg(icArgs.size(), false);

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
            if (CAR(arg) == R_DotsSymbol || TAG(form) == R_DotsSymbol)
                return false;

            // TODO: figure out how to handle those
            if (CAR(arg) == R_MissingArg)
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
        if (form != R_NilValue || i != icArgs.size())
            return false;

        SEXP inBody = CDR(inFun);
        if (TYPEOF(inBody) == NATIVESXP) {

            BasicBlock* icMatch =
                BasicBlock::Create(getGlobalContext(), "icMatch", f, nullptr);
            BasicBlock* icMiss =
                BasicBlock::Create(getGlobalContext(), "icMiss", f, nullptr);
            BasicBlock* end =
                BasicBlock::Create(getGlobalContext(), "end", f, nullptr);

            // Insert a guard to check if the incomming function matches
            // the one we got this time
            ICmpInst* test = new ICmpInst(*b, ICmpInst::ICMP_EQ, fun,
                                          constant(inFun), "guard");
            BranchInst::Create(icMatch, icMiss, test, b);

            b = icMatch;

            // This is an inlined version of applyNativeClosure
            Value* arglist = constant(R_NilValue);

            // This reverses the arglist, but quickArgumentAdapter
            // reverses again
            // TODO: construct the environment in one go,
            // without using quickArgumentAdapter
            for (unsigned i = 0; i < icArgs.size(); ++i) {
                Value* arg = icArgs[i];
                if (promarg[i])
                    arg = INTRINSIC(m.createPromise, arg, rho);
                arglist = INTRINSIC(m.CONS_NR, arg, arglist);
            }

            Value* newrho =
                INTRINSIC(m.closureQuickArgumentAdaptor, fun, arglist);

            Value* cntxt = new AllocaInst(t::cntxt, "", b);

            INTRINSIC(m.initClosureContext, cntxt, call, newrho, rho, arglist,
                      fun);

            Value* res = INTRINSIC_NO_SAFEPOINT(
                m.closureNativeCallTrampoline, cntxt, constant(inBody), newrho);

            INTRINSIC(m.endClosureContext, cntxt, res);

            BranchInst::Create(end, b);
            b = icMiss;

            Value* missRes = callMyStub();

            BranchInst::Create(end, b);
            b = end;

            PHINode* phi = PHINode::Create(t::SEXP, 2, "", b);
            phi->addIncoming(res, icMatch);
            phi->addIncoming(missRes, icMiss);
            ReturnInst::Create(getGlobalContext(), phi, b);

            return true;
        }
    }
    return false;
}

Value* ICCompiler::callMyStub() {
    auto stub = getStub(size, m);

    std::vector<Value*> ic_args;
    // Closure arguments
    for (auto arg : icArgs) {
        ic_args.push_back(arg);
    }
    // Additional IC arguments
    ic_args.push_back(call);
    ic_args.push_back(fun);
    ic_args.push_back(rho);
    ic_args.push_back(caller);
    ic_args.push_back(stackmapId);

    return INTRINSIC_NO_SAFEPOINT(stub, ic_args);
}

bool ICCompiler::compileGenericIc(SEXP inCall, SEXP inFun) {
    Value* call = compileCall(inCall, inFun);
    ReturnInst::Create(getGlobalContext(), call, b);

    return true;
}

Value* ICCompiler::compileCall(SEXP call, SEXP op) {
    BasicBlock* icMatch =
        BasicBlock::Create(getGlobalContext(), "icMatch", f, nullptr);
    BasicBlock* icMiss =
        BasicBlock::Create(getGlobalContext(), "icMiss", f, nullptr);
    BasicBlock* end = BasicBlock::Create(getGlobalContext(), "end", f, nullptr);

    Value* test;
    switch (TYPEOF(op)) {
    case SPECIALSXP: {
        // Specials only care about the ast, so we can call any special through
        // this ic
        Value* ftype = INTRINSIC(m.sexpType, fun);
        test = new ICmpInst(*b, ICmpInst::ICMP_EQ, ftype, constant(SPECIALSXP),
                            "guard");
        break;
    }
    case BUILTINSXP:
    case CLOSXP: {
        test = new ICmpInst(*b, ICmpInst::ICMP_EQ, fun, constant(op), "guard");
        break;
    }
    default:
        assert(false);
    }

    BranchInst::Create(icMatch, icMiss, test, b);

    b = icMatch;

    Value* res;
    switch (TYPEOF(op)) {
    case SPECIALSXP:
        res = INTRINSIC(m.callSpecial, constant(call), fun,
                        constant(R_NilValue), rho);
        break;
    case BUILTINSXP: {
        Value* args = compileArguments(CDR(call), /*eager=*/true);
        res = INTRINSIC(m.callBuiltin, constant(call), fun, args, rho);
        break;
    }
    case CLOSXP: {
        Value* args = compileArguments(CDR(call), /*eager=*/false);
        res = INTRINSIC(m.callClosure, constant(call), fun, args, rho);
        break;
    }
    default:
        assert(false);
    }
    BranchInst::Create(end, b);

    b = icMiss;

    Value* missRes = callMyStub();

    BranchInst::Create(end, b);
    b = end;

    PHINode* phi = PHINode::Create(t::SEXP, 2, "", b);
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
    Value* arglist = constant(R_NilValue);

    // if there are no arguments
    int argnum = 0;
    bool seendots = false;
    while (argAsts != R_NilValue) {
        if (CAR(argAsts) == R_DotsSymbol) {
            assert(!seendots);
            seendots = true;

            // first only get the first dots arg to get the top of the list
            arglist = INTRINSIC(m.addEllipsisArgumentHead, arglist, rho,
                                eager ? constant(TRUE) : constant(FALSE));
            if (!arglistHead)
                arglistHead = arglist;

            // then add the rest
            arglist = INTRINSIC(m.addEllipsisArgumentTail, arglist, rho,
                                eager ? constant(TRUE) : constant(FALSE));
            argnum++;
        } else {
            arglist = compileArgument(arglist, argAsts, argnum++, eager);
            if (!arglistHead)
                arglistHead = arglist;
        }
        argAsts = CDR(argAsts);
    }
    if (arglistHead)
        return arglistHead;
    return constant(R_NilValue);
}

/** Compiles a single argument.

  Self evaluating literals are always returned as SEXP constants, anything else
  is either evaluated directly if eager is true, or they are compiled as new
  promises.
 */
Value* ICCompiler::compileArgument(Value* arglist, SEXP argAst, int argnum,
                                   bool eager) {
    SEXP arg = CAR(argAst);
    SEXP name = TAG(argAst);
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
        result = icArgs[argnum];
        break;
    case SYMSXP:
        assert(arg != R_DotsSymbol);
        if (arg == R_MissingArg) {
            return INTRINSIC(m.addKeywordArgument, arglist,
                             constant(R_MissingArg), constant(name));
        }
    // Fall through:
    default:
        if (eager) {
            // TODO make this more efficient?
            result = INTRINSIC(m.callNative, icArgs[argnum], rho);
        } else {
            // we must create a promise out of the argument
            result = INTRINSIC(m.createPromise, icArgs[argnum], rho);
        }
        break;
    }
    if (name != R_NilValue)
        return INTRINSIC(m.addKeywordArgument, arglist, result, constant(name));

    return INTRINSIC(m.addArgument, arglist, result);
}

Value* ICCompiler::constant(SEXP value) {
    return loadConstant(value, m.getM(), b);
}

Value* ICCompiler::INTRINSIC_NO_SAFEPOINT(llvm::Value* fun,
                                          std::vector<Value*> args) {
    return insertCall(fun, args, f, b, m, false);
}

Value* ICCompiler::INTRINSIC(llvm::Value* fun, std::vector<Value*> args) {
    return insertCall(fun, args, f, b, m, true);
}

} // namespace rjit
