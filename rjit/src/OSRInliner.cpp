#include "Utils.h"
#include "OSRInliner.h"
#include "OSRHandler.h"
#include <llvm/IR/BasicBlock.h>

using namespace llvm;

namespace osr {

/******************************************************************************/
/*                  Public functions */
/******************************************************************************/

SEXP OSRInliner::inlineCalls(SEXP f, SEXP env) {
    /*Get the compiled version*/
    rjit::Compiler c("module");
    SEXP fSexp = c.compile("outer", f);
    Function* fLLVM = GET_LLVM(fSexp);
    assert(fLLVM && "Could not extract the LLVM function.");

    /*Set up the working copy*/
    Function* toOpt = OSRHandler::setupOpt(fLLVM);
    SET_TAG(fSexp, reinterpret_cast<SEXP>(toOpt));

    /*Get the function calls inside f*/
    FunctionCalls* calls = FunctionCall::getFunctionCalls(toOpt);

    for (auto it = calls->begin(); it != calls->end(); ++it) {
        // Get the callee
        SEXP constantPool = CDR(fSexp);
        (*it)->fixNatives(constantPool, &c);
        SEXP toInlineSexp =
            getFunction(constantPool, (*it)->getFunctionSymbol(), env);
        if (!toInlineSexp)
            continue;

        // For the OSR condition.
        (*it)->setInPtr(&c, toInlineSexp);

        // Get the LLVM IR for the function to Inline.
        toInlineSexp = c.compile("inner", BODY(toInlineSexp));

        Function* toInline = Utils::cloneFunction(GET_LLVM(toInlineSexp));

        // Replace constant pool accesses and argument uses.
        Return_List ret;
        prepareCodeToInline(toInline, *it, LENGTH(CDR(fSexp)), &ret);
        Function* toInstrument = OSRHandler::getToInstrument(toOpt);
        insertBody(toOpt, toInline, toInstrument, *it, &ret);

        // clean up
        ret.clear();
        // Set the constant pool.
        setCP(fSexp, toInlineSexp);
    }
    // Finish the compilation.
    c.jitAll(); // TODO maybe need to remove what we want to keep
                // uninstrumented.
    return fSexp;
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
                i, ConstantInt::get(getGlobalContext(), APInt(32, newValue)));
        }
    }
}

void OSRInliner::replaceArgs(Inst_Vector* args, Inst_Vector* vars, int n) {
    int counter = 0;
    for (auto it = vars->begin(), ait = args->begin();
         (it != vars->end()) && (ait != args->end()) && (counter < n);
         ++it, ++ait, ++counter) {
        (*it)->replaceAllUsesWith(*ait);
        (*it)->removeFromParent();
    }
}

SEXP OSRInliner::getFunction(SEXP cp, int symbol, SEXP env) {
    SEXP symb = VECTOR_ELT(cp, symbol);
    SEXP fSexp = findFun(symb, env);
    if (TYPEOF(fSexp) == CLOSXP) {
        return fSexp;
    }
    return nullptr;
}

void OSRInliner::prepareCodeToInline(Function* toInline, FunctionCall* fc,
                                     int cpOffset, Return_List* ret) {
    Function* caller = fc->getFunction();
    assert((caller && toInline) && "Null pointer.");

    Inst_Vector vars;

    for (auto RI = caller->arg_begin(), EI = toInline->arg_begin();
         (RI != caller->arg_end()) && (EI != toInline->arg_end()); ++RI, ++EI)
        (*EI).replaceAllUsesWith(&(*RI));

    for (auto it = inst_begin(toInline), e = inst_end(toInline); it != e;
         ++it) {
        CallInst* call = dynamic_cast<CallInst*>(&(*it));
        if (call)
            updateCPAccess(call, cpOffset);
        if (call && IS_GET_VAR(call))
            vars.push_back(&(*it));

        ReturnInst* ri = dynamic_cast<ReturnInst*>(&(*it));
        if (!call && ri)
            ret->push_back(ri);
    }

    replaceArgs(fc->getArgs(), &vars, fc->getNumbArguments());
    vars.clear();
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
    toInline->removeFromParent();
    delete deadBlock;
    delete toInline;

    // OSR Instrumentation.
    OSRHandler::insertOSR(toOpt, toInstrument, fc->getArg_back(),
                          fc->getGetFunc(), getOSRCondition(fc));
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

} // namespace osr