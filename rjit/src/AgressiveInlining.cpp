#include <llvm/IR/Module.h>
#include "Compiler.h"

#include "api.h"

#include "RIntlns.h"

#include "ir/ir.h"
#include "ir/Builder.h"
#include "ir/intrinsics.h"
#include "ir/Handler.h"

#include "AgressiveInlining.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/InstIterator.h>
#include "llvm/Analysis/TargetTransformInfo.h"

using namespace rjit;
namespace osr {

BookKeeping BookKeeping::singleton;

void iterateInstructionsDirectly(llvm::Function* f) {
    assert(f != NULL and "TODO");
    llvm::CallInst* call = NULL;
    for (inst_iterator I = inst_begin(f), E = inst_end(f); I != E; ++I) {
        // I->dump();
        // TODO check if is a callInst
        call = dynamic_cast<llvm::CallInst*>(&(*I));
        if (call != NULL) {
            warning("We found a call!");
            llvm::Function* called = call->getCalledFunction();
            std::string name = called->getName().str();
            warning("Hey I just found you %s", name.c_str());
            // best way to proceed is to check if it is an icstub
            if (!called->isDeclaration())
                warning("The function %s is not a declaration", name.c_str());
            if (name.find("icStub") != std::string::npos) {
                // TODO didn't work that way.
            }
        }
    }
}

// TODO This one produces a segfault. I should try and see why
void getAllFunctionCalls(llvm::BasicBlock* b) {
    assert(b != NULL and "TODO aghosn");
    for (llvm::BasicBlock::iterator it = b->begin(), e = b->end(); it != e;
         ++it) {
        e->dump();
    }
}
/**
 * @brief      printing all the blocks one by one
 *
 * @param      f     llvm::Function*
 */
void getAllBlocksForFunction(llvm::Function* f) {
    assert(f != NULL and "TODO aghosn");
    for (llvm::Function::iterator it = f->begin(), e = f->end(); it != e;
         ++it) {
        (*it).dump();
        // getAllFunctionCalls(it);
    }
}

/**
 * @brief      playing around. Next step is to get all basic blocks printed one
 *by one
 *
 * @param[in]  expression  the function we need to look at
 *
 * @return     R_NilValue2
 */
REXPORT SEXP jitGetIR(SEXP expression) {
    assert(TYPEOF(expression) == NATIVESXP and
           "LLVM code can only be extracted from a NATIVESXP argument");
    llvm::Function* f = reinterpret_cast<llvm::Function*>(TAG(expression));

    warning("Trying to find the function's name %s",
            f->getName().str().c_str());
    if (BookKeeping::singleton.contains2(expression))
        warning("Found it!");
    return R_NilValue;
}

/**
 * @brief      We generate the llvm IR for the function without the
 *instrumentation (safepoints)
 *
 * @param[in]  expression
 *
 * @return     NATIVESXP
 */
REXPORT SEXP prototypeInlining(SEXP expression) {
    Compiler c("module");
    SEXP result = c.compile("name_here", expression);
    // TODO the bookkeeping by registering the function with a correct name.
    return result;
}
}