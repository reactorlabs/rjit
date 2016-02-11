#include "Utils.h"
#include "OSRInliner.h"
#include "OSRHandler.h"
#include <llvm/IR/BasicBlock.h>
#include <string>
#include "api.h"
#include "ir/Builder.h"
#include <llvm/IR/Verifier.h>

using namespace llvm;

namespace osr {

/*      Initialize static variables     */
uint64_t OSRInliner::id = 0;
std::map<uint64_t, ExitEntry> OSRInliner::exits;

#define VERIFYFUN2(F)                                                          \
    assert((!verifyFunction(*F, &outs()) ? true : (F->dump(), false)) &&       \
           "ill-formed function")
/******************************************************************************/
/*                  Public functions */
/******************************************************************************/

OSRInliner::OSRInliner(rjit::Compiler* c) : c(c) {

    closureQuickArgumentAdaptor = Function::Create(
        rjit::t::sexp_sexpsexp, Function::ExternalLinkage,
        "closureQuickArgumentAdaptor", c->getBuilder()->module());

    CONS_NR =
        Function::Create(rjit::t::sexp_sexpsexp, Function::ExternalLinkage,
                         "CONS_NR", c->getBuilder()->module());

    fixClosure =
        Function::Create(rjit::t::fixclosure_t, Function::ExternalLinkage,
                         "fixClosure", c->getBuilder()->module());
}

SEXP OSRInliner::inlineCalls(SEXP f) {
    /*Get the elements out of the closure*/
    assert(TYPEOF(f) == CLOSXP &&
           "Call for OSR Inliner on a non-closure sexp.");
    SEXP formals = FORMALS(f);
    SEXP env = TAG(f);
    assert(TYPEOF(env) == ENVSXP && "Cannot extract environment.");

    SEXP fSexp = OSRHandler::getFreshIR(f, c, false);
    OSRHandler::addSexpToModule(fSexp, c->getBuilder()->module());

    Function* toOpt = GET_LLVM(fSexp);
    c->getBuilder()->module()->fixRelocations(formals, fSexp, toOpt);

    /*Get the function calls inside f*/
    FunctionCalls* calls = FunctionCall::getFunctionCalls(toOpt);

    for (auto it = calls->begin(); it != calls->end(); ++it) {
        // Get the callee
        SEXP constantPool = CDR(fSexp);
        SEXP toInlineClosure =
            getFunction(constantPool, (*it)->getFunctionSymbol(), env);

        // Function not found or arguments are missing, or recursive call.
        if (!toInlineClosure ||
            isMissingArgs(FORMALS(toInlineClosure), (*it)) ||
            f == toInlineClosure)
            continue;

        // For the OSR condition.
        (*it)->setInPtr(c, toInlineClosure);

        // For the promises.
        (*it)->fixPromises(constantPool, toInlineClosure, c);

        // Get the LLVM IR for the function to Inline.
        SEXP toInlineFunc = R_NilValue;
        if (!INLINE_ALL) {
            toInlineFunc = OSRHandler::getFreshIR(toInlineClosure, c, false);
        } else {
            toInlineClosure = inlineCalls(toInlineClosure);
            toInlineFunc = OSRHandler::getFreshIR(toInlineClosure, c, false);
        }
        // TODO because of verifier
        /*OSRHandler::addSexpToModule(toInlineFunc, c->getBuilder()->module());
        OSRHandler::resetSafepoints(toInlineFunc, c);*/

        Function* toInline = GET_LLVM(toInlineFunc);

        Function* toInstrument = OSRHandler::getToInstrument(toOpt);
        SEXP toInstrSexp = OSRHandler::cloneSEXP(fSexp, toInstrument);
        OSRHandler::resetSafepoints(toInstrSexp, c);
        c->getBuilder()->module()->fixRelocations(formals, toInstrSexp,
                                                  toInstrument);
        Inst_Vector* compensation = createCompensation(toInstrSexp, f);

        auto newrho = createNewRho((*it));

        // Replace constant pool accesses and argument uses.
        Return_List ret;
        prepareCodeToInline(toInline, *it, newrho, LENGTH(CDR(fSexp)), &ret);
        // TODO because of verifier
        // OSRHandler::resetSafepoints(fSexp, c);
        insertBody(toOpt, toInline, toInstrument, *it, &ret);
        auto res =
            OSRHandler::insertOSRExit(toOpt, toInstrument, (*it)->getConsts(),
                                      getOSRCondition(*it), compensation);
        // clean up
        VERIFYFUN2(res.second);
        res.second->setGC("rjit");

        ret.clear();
        // Set the constant pool.
        setCP(fSexp, toInlineFunc);
    }
    OSRHandler::resetSafepoints(fSexp, c);
    VERIFYFUN2(GET_LLVM(fSexp));
    SETCDR(f, fSexp);
    return f;
}

/******************************************************************************/
/*                  Private functions */
/******************************************************************************/

void OSRInliner::setCP(SEXP first, SEXP second) {
    int sizeO = LENGTH(CDR(first));
    int sizeI = LENGTH(CDR(second));
    SEXP objs = allocVector(VECSXP, sizeO + sizeI);
    for (int i = 0; i < sizeO; ++i)
        SET_VECTOR_ELT(objs, i, VECTOR_ELT(CDR(first), i));
    for (int i = 0; i < sizeI; ++i)
        SET_VECTOR_ELT(objs, i + sizeO, VECTOR_ELT(CDR(second), i));
    SETCDR(first, objs);
}

void OSRInliner::updateCPAccess(CallInst* call, int offset) {
    for (uint32_t i = 0; i < call->getNumArgOperands(); ++i) {
        ConstantInt* index = dynamic_cast<ConstantInt*>(call->getArgOperand(i));
        if (index) {
            int64_t newValue = index->getSExtValue() + offset;
            call->setArgOperand(
                i, ConstantInt::get(getGlobalContext(),
                                    APInt(index->getBitWidth(), newValue)));
        }
    }
}

// TODO aghosn I am not inlining the print function because it generates a type
// Problem. FIXME
SEXP OSRInliner::getFunction(SEXP cp, int symbol, SEXP env) {
    SEXP symb = VECTOR_ELT(cp, symbol);
    SEXP fSexp = findFun(symb, env);
    std::string name = CHAR(PRINTNAME(symb));
    if (TYPEOF(fSexp) != CLOSXP || TYPEOF(TAG(fSexp)) != ENVSXP ||
        (TAG(fSexp) != R_GlobalEnv && ONLY_GLOBAL))
        return nullptr;
    SEXP formals = FORMALS(fSexp);
    while (formals != R_NilValue) {
        if (TAG(formals) == R_DotsSymbol)
            return nullptr;
        formals = CDR(formals);
    }
    // printf("\n\n\nWE INLINE %s\n\n\n", name.c_str());
    return fSexp;
}

bool OSRInliner::isMissingArgs(SEXP formals, FunctionCall* fc) {
    SEXP form = formals;
    unsigned i = 0;
    while (form != R_NilValue) {
        ++i;
        form = CDR(form);
    }
    return (i != fc->getArgs()->size());
}

void OSRInliner::prepareCodeToInline(Function* toInline, FunctionCall* fc,
                                     CallInst* newrho, int cpOffset,
                                     Return_List* ret) {
    Function* caller = fc->getFunction();
    assert((caller && toInline) && "Null pointer.");

    assert(toInline->arg_size() == 3 && "Wrong function signature.");
    auto EI = toInline->arg_begin();
    auto RI = caller->arg_begin();
    // Replace the consts.
    (*EI).replaceAllUsesWith(&(*RI));
    ++EI;
    // Replace the rho with the newrho
    (*EI).replaceAllUsesWith(newrho);

    for (auto it = inst_begin(toInline), e = inst_end(toInline); it != e;
         ++it) {
        CallInst* call = dynamic_cast<CallInst*>(&(*it));
        if (call)
            updateCPAccess(call, cpOffset);

        ReturnInst* ri = dynamic_cast<ReturnInst*>(&(*it));
        if (!call && ri)
            ret->push_back(ri);
    }
}

void OSRInliner::insertBody(Function* toOpt, Function* toInline,
                            Function* toInstrument, FunctionCall* fc,
                            Return_List* ret) {
    BasicBlock* callBlock =
        dynamic_cast<BasicBlock*>(fc->getGetFunc()->getParent());
    assert(callBlock && "Call block is null.");

    // Isolate the function call.
    BasicBlock* deadBlock = callBlock->splitBasicBlock(fc->getIcStub(), "DEAD");
    BasicBlock::iterator it(fc->getIcStub());
    ++it;
    BasicBlock* continuation = deadBlock->splitBasicBlock(*it, "CONTINUATION");

    // Insert the blocks from toInline in toOpt.
    std::vector<BasicBlock*>* blocks = Utils::getBBs(toInline);
    for (auto it = blocks->begin(); it != blocks->end(); ++it) {
        (*it)->removeFromParent();
        (*it)->insertInto(toOpt, deadBlock);
    }

    // Fix the successors and remove the return from the inlined blocks.
    if (!blocks->empty() && !ret->empty()) {
        // Create the phi node to merge all the return instructions.
        PHINode* node = nullptr;

        // Set the correct successor for the callBlock.
        callBlock->getTerminator()->setSuccessor(0, *(blocks->begin()));

        for (auto r = ret->begin(); r != ret->end(); ++r) {
            BasicBlock* parentReturn =
                dynamic_cast<BasicBlock*>((*r)->getParent());
            assert(parentReturn &&
                   "The basic block containing the return is null.");

            // Add the value to the phinode.
            if (!node && fc->getIcStub()->getNumUses())
                node = PHINode::Create((*r)->getReturnValue()->getType(),
                                       ret->size(), "OSRInlineMerge",
                                       continuation->getFirstNonPHI());

            if (node && fc->getIcStub()->getNumUses())
                node->addIncoming((*r)->getReturnValue(), parentReturn);

            // Remove the return instruction.
            BasicBlock* split = parentReturn->splitBasicBlock(*r, "DEADRETURN");
            parentReturn->getTerminator()->setSuccessor(0, continuation);
            split->removeFromParent();
            delete split;
        }

        if (node)
            fc->getIcStub()->replaceAllUsesWith(node);
    }

    // Clean up.
    blocks->clear();
    // Remove the icStub from the StateMap.
    OSRHandler::removeEntry(toOpt, toInstrument, fc->getIcStub());
    deadBlock->removeFromParent();
    // toInline->removeFromParent();
    delete deadBlock;
    delete toInline;

    /*OSRHandler::insertOSRExit(toOpt, toInstrument, fc->getConsts(),
                              getOSRCondition(fc), compensation);*/
}

Inst_Vector* OSRInliner::getTrueCondition() {
    Inst_Vector* res = new Inst_Vector();
    ConstantInt* one = ConstantInt::get(getGlobalContext(), APInt(32, 1));
    // ConstantInt* zero = ConstantInt::get(getGlobalContext(), APInt(32, 0));
    ICmpInst* cond = new ICmpInst(ICmpInst::ICMP_EQ, one, one /*zero*/);
    res->push_back(cond);
    return res;
}

// TODO do the sexp
Inst_Vector* OSRInliner::getOSRCondition(FunctionCall* fc) {
    Inst_Vector* res = new Inst_Vector();
    assert(fc->getInPtr() && "The function address has not been set.");
    ICmpInst* test =
        new ICmpInst(ICmpInst::ICMP_NE, fc->getGetFunc(), fc->getInPtr());
    res->push_back(test);
    return res;
}

CallInst* OSRInliner::createNewRho(FunctionCall* fc) {
    Value* arglist = rjit::ir::Builder::convertToPointer(R_NilValue);

    auto args = fc->getArgs();
    // TODO aghosn clean that
    for (auto it = args->begin(); it != (args->end()); ++it) {
        Value* arg = (*it);
        std::vector<Value*> f_arg;
        f_arg.push_back(arg);
        f_arg.push_back(arglist);
        CallInst* inter =
            CallInst::Create(CONS_NR, f_arg,
                             ""); // TODO aghosn add somewhere in function.
        arglist = inter;
        inter->insertBefore(fc->getIcStub());
    }

    std::vector<Value*> f_args;
    f_args.push_back(fc->getGetFunc());
    f_args.push_back(arglist);
    auto res = CallInst::Create(closureQuickArgumentAdaptor, f_args, "");
    res->insertBefore(fc->getIcStub());
    return res;
}

Inst_Vector* OSRInliner::createCompensation(SEXP fun, SEXP closure) {
    c->getBuilder()->module()->fixRelocations(FORMALS(closure), fun,
                                              GET_LLVM(fun));
    std::vector<Value*> args;
    args.push_back(ConstantInt::get(getGlobalContext(), APInt(64, ++id)));
    Inst_Vector* compensation = new Inst_Vector();
    compensation->push_back(CallInst::Create(fixClosure, args));
    exits[id] = ExitEntry(closure, fun);
    return compensation;
}

} // namespace osr
