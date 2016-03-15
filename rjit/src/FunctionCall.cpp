#include <llvm/IR/InstIterator.h>

#include "FunctionCall.h"
#include "Utils.h"
#include "ir/primitive_calls.h"
#include "JITCompileLayer.h"
#include "Runtime.h"

using namespace llvm;
using namespace rjit;
namespace osr {

FunctionCall::FunctionCall(CallInst* icStub) : icStub(icStub) {
    unsigned size_arg = getNumbArguments();
    assert((size_arg + 5) == icStub->getNumArgOperands() && "Bad signature.");
    unsigned i = 0;
    for (; i < size_arg; ++i) {
        Instruction* inst =
            dynamic_cast<Instruction*>(icStub->getArgOperand(i));
        assert(inst && "Trying to add a value arg that is not an instruction.");
        args.push_back(inst);
    }
    assert(i == size_arg);
    consts = dynamic_cast<CallInst*>(icStub->getArgOperand(i));
    assert(consts && "Cannot convert consts to Instruction.");
    ++i;
    getFunc = dynamic_cast<CallInst*>(icStub->getArgOperand(i));
    assert(getFunc && "Could not find the getFunc.");
    inPtr = nullptr;
}

FunctionCalls* FunctionCall::getFunctionCalls(Function* f) {
    FunctionCalls* result = new FunctionCalls();
    for (inst_iterator it = inst_begin(f), e = inst_end(f); it != e; ++it) {
        CallInst* ic = dynamic_cast<CallInst*>(&(*it));
        if (ic && IS_STUB(ic))
            result->push_back(new FunctionCall(ic));
    }
    return result;
}

unsigned int FunctionCall::getNumbArguments() {
    std::string name = icStub->getCalledFunction()->getName().str();
    return (atoi(name.substr(name.find_last_of("_") + 1).c_str()));
    // return ((unsigned int)name.back() - '0');
}

Function* FunctionCall::getFunction() {
    return dynamic_cast<Function*>(getFunc->getParent()->getParent());
}

int FunctionCall::getFunctionSymbol() {
    ConstantInt* cst =
        dynamic_cast<ConstantInt*>(this->getFunc->getArgOperand(2));
    return cst->getSExtValue();
}

bool FunctionCall::tryFix(SEXP cp, SEXP inFun, rjit::Compiler* c) {
    // Obtain the call.
    ConstantInt* callIdx = dynamic_cast<ConstantInt*>(consts->getArgOperand(1));
    assert(callIdx);
    assert(TYPEOF(inFun) == CLOSXP);
    SEXP call = VECTOR_ELT(cp, callIdx->getSExtValue());
    unsigned size = args.size();

    // Check for named arguments and promises.
    std::vector<bool> promarg(size, false);
    std::vector<long> positionalArg;
    std::unordered_map<long, SEXP> namedArg;
    std::unordered_map<SEXP, long> formals;

    SEXP arg = CDR(call);
    SEXP form = FORMALS(inFun);
    unsigned i = 0;

    while (arg != R_NilValue && form != R_NilValue) {
        if (TAG(arg) != R_NilValue)
            namedArg[i] = TAG(arg);
        else
            positionalArg.push_back(i);
        formals[TAG(form)] = i;

        // Ellipsis.
        if (CAR(arg) == R_DotsSymbol || TAG(form) == R_DotsSymbol)
            return false;

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

    // number of args != number of formal args
    if (form != R_NilValue || i != size)
        return false;

    std::vector<long> argOrder(size, -1);
    for (auto p : namedArg) {
        long argnum = std::get<0>(p);
        SEXP name = std::get<1>(p);
        if (!formals.count(name)) {
            return false; // named argument does not match formal.
        }
        long pos = formals[name];
        argOrder[pos] = argnum;
    }

    unsigned position = 0;
    for (long argnum : positionalArg) {
        while (position < size && argOrder[position] != -1)
            ++position;
        assert(position < size);
        argOrder[position] = argnum;
    }

    // Copy of args.
    std::vector<Instruction*> args_ = args;

    // Sorting the args and creating promises.
    for (unsigned i = 0; i < size; ++i) {
        long pos = argOrder[i];
        Instruction* arg = args_[pos];
        if (promarg[pos]) {
            Instruction* promiseInst = arg;
            std::vector<Value*> _args_;
            _args_.push_back(promiseInst);
            _args_.push_back(getRho());
            CallInst* promise = CallInst::Create(
                c->getBuilder()->intrinsic<rjit::ir::CreatePromise>(), _args_,
                "");
            promise->insertAfter(arg);
            arg->replaceUsesOutsideBlock(promise, arg->getParent());
            args.at(i) = promise;
        } else {
            args.at(i) = arg;
        }
    }
    return true;
}

Value* FunctionCall::getRho() {
    Function* fun = this->getFunction();
    assert(fun && "The function for this fc is null.");
    // TODO CHANGE THIS ?
    for (auto RI = fun->arg_begin(), EI = fun->arg_end(); RI != EI; ++RI) {
        if (NAME_CONTAINS(&(*RI), "rho"))
            return &(*RI);
    }
    return nullptr;
}

void FunctionCall::setInPtr(rjit::Compiler* c, SEXP addr) {
    inPtr = c->getBuilder()->convertToPointer(addr);
}

Value* FunctionCall::getInPtr() { return inPtr; }

Instruction* FunctionCall::getArg_back() { return args.back(); }

} // namespace osr