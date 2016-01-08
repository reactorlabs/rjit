#include "Utils.h"
#include "OSRInliner.h"
#include "OSRHandler.h"
#include <llvm/IR/BasicBlock.h>

using namespace llvm;

namespace osr {

/******************************************************************************/
/*					Public functions */
/******************************************************************************/

SEXP OSRInliner::inlineCalls(SEXP f, SEXP env) {
    /*Get the compiled version*/
    rjit::Compiler c("module");
    SEXP fSexp = c.compile("outer", f);
    Function* fLLVM = GET_LLVM(fSexp);
    assert(fLLVM && "Could not extract the LLVM function.");

    /*Set up the working copies*/
    auto pair = OSRHandler::setup(fLLVM);
    Function* toOpt = pair.first, *toInstrument = pair.second;
    SET_TAG(fSexp, reinterpret_cast<SEXP>(toOpt));

    /*Get the function calls inside f*/
    FunctionCalls* calls = FunctionCall::getFunctionCalls(toOpt);

    for (auto it = calls->begin(); it != calls->end(); ++it) {
        // Get the callee
        SEXP constantPool = CDR(fSexp);
        SEXP toInlineSexp =
            getFunction(&c, constantPool, (*it)->getFunctionSymbol(), env);
        if (!toInlineSexp)
            continue;
        Function* toInline = Utils::cloneFunction(GET_LLVM(toInlineSexp));

        // Replace constant pool accesses and argument uses.
        ReturnInst* ret = nullptr;
        prepareCodeToInline(toInline, *it, LENGTH(CDR(fSexp)), &ret);
        insertBody(toOpt, toInline, toInstrument, *it, ret);

        // Set the constant pool.
        setCP(fSexp, toInlineSexp);
    }
    // Finish the compilation.
    c.jitAll(); // TODO maybe need to remove what we want to keep
                // uninstrumented.
    return fSexp;
}

/******************************************************************************/
/*					Private functions */
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

SEXP OSRInliner::getFunction(rjit::Compiler* c, SEXP cp, int symbol, SEXP env) {
    SEXP symb = VECTOR_ELT(cp, symbol);
    SEXP fSexp = findFun(symb, env);
    if (TYPEOF(fSexp) == NATIVESXP) {
        return fSexp;
    } else if (TYPEOF(fSexp) == CLOSXP) {
        return c->compile("inner", BODY(fSexp));
    }

    return nullptr;
}

void OSRInliner::prepareCodeToInline(Function* toInline, FunctionCall* fc,
                                     int cpOffset, ReturnInst** ret) {
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
        if (!call && ret)
            *ret = dynamic_cast<ReturnInst*>(&(*it));
    }

    replaceArgs(fc->getArgs(), &vars, fc->getNumbArguments());
    vars.clear();
}

void OSRInliner::insertBody(Function* toOpt, Function* toInline,
                            Function* toInstrument, FunctionCall* fc,
                            ReturnInst* ret) {
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

    // Replace the return value.
    fc->getIcStub()->replaceAllUsesWith(ret->getReturnValue());

    // Fix the successors and remove the return from the inlined blocks.
    if (!blocks->empty()) {
        callBlock->getTerminator()->setSuccessor(0, *(blocks->begin()));
        BasicBlock* parentReturn = dynamic_cast<BasicBlock*>(ret->getParent());
        assert(parentReturn &&
               "The basic block containing the return is null.");
        BasicBlock* split = parentReturn->splitBasicBlock(ret, "DEADRETURN");
        parentReturn->getTerminator()->setSuccessor(0, continuation);
        split->removeFromParent();
        delete split;
    }

    // OSR Instrumentation.
    OSRHandler::insertOSR(toOpt, toInstrument, fc->getGetFunc());

    // Clean up.
    blocks->clear();
    fc->getGetFunc()->removeFromParent();
    deadBlock->removeFromParent();
    toInline->removeFromParent();
    delete deadBlock;
    delete toInline;

    toOpt->dump();
    toInstrument->dump();
}

} // namespace osr
