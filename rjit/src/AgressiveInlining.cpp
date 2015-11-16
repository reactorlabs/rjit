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

#include "R.h"

using namespace rjit;
namespace osr {

/**
 * @brief      We generate the llvm IR for the function without the
 *			   instrumentation (safepoints)
 *
 * @param[in]  expression
 *
 * @return     NATIVESXP
 */
REXPORT SEXP prototypeInlining(SEXP expression) {
    Compiler c("module");
    SEXP result = c.compile("name_here", expression);

    // TODO the bookkeeping by registering the function with a correct name.
    std::string name = DeparseUtils::getName(expression);
    printf("The name when adding it %s.\n", name.c_str());
    llvm::Function* f = reinterpret_cast<llvm::Function*>(TAG(result));
    osr::InliningEnv::getInstance().store[name] = f;
    return result;
}

REXPORT SEXP containsPrototype(SEXP expression) {
    std::string name = DeparseUtils::getName(expression);

    printf("The name when looking for it %s.\n", name.c_str());
    if (InliningEnv::getInstance().storeContains(name))
        printf("Yeah, we found it in the store.\n");
    else
        printf("Neeh, never seen it.\n");
    return R_NilValue;
}

// TODO aghosn this is what we gonna do for the moment.
REXPORT SEXP inlineFunctions(SEXP outter, SEXP inner) {
    Compiler c("module");
    SEXP f = c.compile("f", outter);
    SEXP g = c.compile("g", inner);
    llvm::Function* llvmF = reinterpret_cast<llvm::Function*>(TAG(f));
    llvm::Function* llvmG = reinterpret_cast<llvm::Function*>(TAG(g));
    printf("The outter function before \n");
    llvmF->dump();
    printf("\n\n");
    printf("The inner function before \n");
    llvmG->dump();
    printf("\n\n");
    // TODO do the inlining.
    std::vector<llvm::Instruction*> v;
    for (inst_iterator it = inst_begin(llvmG), e = inst_end(llvmG); it != e;
         ++it) {
        v.push_back(&(*it));
    }

    printf("The outter function before \n");
    llvmF->dump();
    printf("\n\n");
    printf("The inner function before \n");
    llvmG->dump();
    printf("\n\n");

    // find the call inside the outter function
    llvm::CallInst* call = NULL;
    for (inst_iterator I = inst_begin(llvmF), E = inst_end(llvmF); I != E;
         ++I) {
        call = dynamic_cast<llvm::CallInst*>(&(*I));
        if (call != NULL) {
            // TODO try to id the function, this is not the correct one.
            for (std::vector<llvm::Instruction*>::iterator it = v.begin();
                 it != v.end(); ++it) {
                (*it)->removeFromParent();
                (*it)->insertBefore(call);
            }

            printf("What f looks like now\n");
            llvmF->dump();

            // llvm::BasicBlock* parent = call->getParent();
            // creates a problem
            call->removeFromParent();
            printf("After removing the parent");

            return R_NilValue;
        }
    }

    // TODO do the instrumentation

    // TODO create a valid closure to return
    return R_NilValue;
}
}