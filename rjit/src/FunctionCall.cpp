#include <llvm/IR/InstIterator.h>

#include "FunctionCall.h"
#include "Utils.h"
#include "ir/intrinsics.h"

using namespace llvm;
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
    consts = dynamic_cast<Instruction*>(icStub->getArgOperand(i));
    assert(consts && "Cannot convert consts to Instruction.");
    ++i;
    getFunc = dynamic_cast<CallInst*>(icStub->getArgOperand(i));
    assert(getFunc && IS_GET_FUNCTION(getFunc) &&
           "Could not find the getFunc.");
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
    return ((unsigned int)name.back() - '0');
}

Function* FunctionCall::getFunction() {
    return dynamic_cast<Function*>(getFunc->getParent()->getParent());
}

int FunctionCall::getFunctionSymbol() {
    ConstantInt* cst =
        dynamic_cast<ConstantInt*>(this->getFunc->getArgOperand(2));
    return cst->getSExtValue();
}

void FunctionCall::fixNatives(SEXP cp, rjit::Compiler* c) {
    for (unsigned int i = 0; i < args.size(); ++i) {
        CallInst* call = dynamic_cast<CallInst*>(args.at(i));
        if (call && IS_USERLIT(call)) {
            ConstantInt* idx = dynamic_cast<ConstantInt*>(
                call->getArgOperand(call->getNumArgOperands() - 1));
            assert(idx && "Could not access the index in userLiteral");
            int64_t value = idx->getSExtValue();
            SEXP access = VECTOR_ELT(cp, value);
            if (TYPEOF(access) == NATIVESXP) {
                std::vector<Value*> args_;
                args_.push_back(call);
                args_.push_back(getRho());

                /*CallInst* promise = CallInst::Create(
                    c->getBuilder()->intrinsic<rjit::ir::CreatePromise>(),
                    args_, "");*/
                CallInst* promise = CallInst::Create(
                    c->getBuilder()->intrinsic<rjit::ir::CallNative>(), args_,
                    "");

                promise->insertAfter(args.at(i));
                AttributeSet PAL;
                {
                    SmallVector<AttributeSet, 4> Attrs;
                    AttributeSet PAS;
                    {
                        AttrBuilder B;
                        auto id =
                            rjit::JITCompileLayer::singleton.getSafepointId(
                                getFunction());
                        B.addAttribute("statepoint-id", std::to_string(id));
                        PAS = AttributeSet::get(getGlobalContext(), ~0U, B);
                    }
                    Attrs.push_back(PAS);
                    PAL = AttributeSet::get(getGlobalContext(), Attrs);
                }
                promise->setAttributes(PAL);

                args.at(i)
                    ->replaceUsesOutsideBlock(promise, args.at(i)->getParent());
                // promise->insertAfter(args.at(i));
                args.at(i) = promise;
            }
        }
    }
}

Value* FunctionCall::getRho() {
    Function* fun = this->getFunction();
    assert(fun && "The function for this fc is null.");
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