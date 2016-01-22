#include <llvm/IR/InstIterator.h>

#include "FunctionCall.h"
#include "Utils.h"
#include "ir/intrinsics.h"

using namespace llvm;
namespace osr {

Inst_Vector* FunctionCall::extractArguments(Function* f, inst_iterator it,
                                            Instruction* ic) {
    Inst_Vector* args = new Inst_Vector();
    inst_iterator end = inst_end(f);
    ++it; // skip the getFunction
    for (; it != end && (&(*it) != ic); ++it) {
        if (it->user_back() == ic) // TODO not sure that's correct
            args->push_back(&(*it));
    }
    return args;
}

FunctionCalls* FunctionCall::getFunctionCalls(Function* f) {
    FunctionCalls* result = new FunctionCalls();
    CallInst* gf = nullptr;
    CallInst* ics = nullptr;

    for (inst_iterator it = inst_begin(f), e = inst_end(f); it != e; ++it) {
        gf = dynamic_cast<CallInst*>(&(*it));
        // TODO we assume there is only one use for the getFunc
        if (gf && IS_GET_FUNCTION(gf) && gf->getNumUses() == 1) {
            ics = dynamic_cast<CallInst*>(gf->user_back());
            if (ics && IS_STUB(ics)) {
                inst_iterator argsIt = it;
                Inst_Vector* args =
                    extractArguments(f, argsIt, gf->user_back());
                result->push_back(new FunctionCall(gf, *args, ics));
            } else {
                // TODO error malformed IR or wrong assumptions on my side
                printf("Malformed call in getFunctionCalls\n");
                return result;
            }
        }
    }

    return result;
}

void FunctionCall::printFunctionCall() {
    printf("------------------------------------------------\n");
    printf("The getFunction:\n");
    getFunc->dump();
    printf("The intermediary args:\n");
    for (Inst_Vector::iterator it = args.begin(); it != args.end(); ++it) {
        (*it)->dump();
    }
    printf("The icStub:\n");
    icStub->dump();
    printf("------------------------------------------------\n");
}

int FunctionCall::getNumbArguments() {
    std::string name = icStub->getCalledFunction()->getName().str();
    return ((int)name.back() - '0');
}

int FunctionCall::getFunctionSymbol() {
    ConstantInt* cst =
        dynamic_cast<ConstantInt*>(this->getFunc->getArgOperand(2));
    return cst->getSExtValue();
}

void FunctionCall::getNatives(SEXP cp) {
    for (auto it = args.begin(); it != args.end(); ++it) {
        CallInst* call = dynamic_cast<CallInst*>(*it);
        if (call && IS_USERLIT(call)) {
            // Get the last argument
            int end = call->getNumArgOperands() - 1;
            ConstantInt* index =
                dynamic_cast<ConstantInt*>(call->getArgOperand(end));
            assert(index && "Could not access index");
            int64_t value = index->getSExtValue();
            SEXP access = VECTOR_ELT(cp, value);
            if (TYPEOF(access) == NATIVESXP) {
                printf("Found a problem!!\n");
                (*it)->dump();
            }
        }
    }
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