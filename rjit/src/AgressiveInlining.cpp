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

#define PRINT_DEBUG_TRUE
#ifdef PRINT_DEBUG_TRUE
#define DEBB(x) x;
#else
#define DEBB(x)
#endif

using namespace rjit;
namespace osr {

llvm::CallInst* getCall(llvm::Function* f) {
    llvm::CallInst* call = NULL;
    for (inst_iterator I = inst_begin(f), E = inst_end(f); I != E; ++I) {
        call = dynamic_cast<llvm::CallInst*>(&(*I));
        if (call != NULL) {
            // Big hack
            std::string name = call->getCalledFunction()->getName().str();
            if (name.find("icStub") != std::string::npos) {
                // get the first argument to check if we can get the called
                // function
                // according to Oli this is the first argument
                // for the moment only return the icStub
                return call;
            }
        }
    }
    return NULL;
}

llvm::Instruction* getConstant(llvm::Function* f) {
    llvm::CallInst* call = NULL;
    for (inst_iterator I = inst_begin(f), E = inst_end(f); I != E; ++I) {
        call = dynamic_cast<llvm::CallInst*>(&(*I));
        if (call != NULL) {
            std::string name = call->getCalledFunction()->getName().str();
            if (name.find("userLiteral") != std::string::npos) {
                return &(*I);
            }
        }
    }
    return NULL;
}

int getNumberOfBasicBlocks(llvm::Function* f) {
    int bb = 0;
    for (llvm::Function::iterator i = f->begin(), e = f->end(); i != e; ++i) {
        printf("WE HAVE A BLOCK:\n");
        i->dump();
        printf("END OF BLOCK\n");
        ++bb;
    }
    return bb;
}

// TODO aghosn this is what we gonna do for the moment.
REXPORT SEXP inlineFunctions(SEXP outter, SEXP inner) {
    Compiler c("module");
    SEXP f = c.compile("f", outter);
    SEXP g = c.compile("g", inner);
    llvm::Function* llvmF = reinterpret_cast<llvm::Function*>(TAG(f));
    llvm::Function* llvmG = reinterpret_cast<llvm::Function*>(TAG(g));

    DEBB(printf("The outter function before \n"); llvmF->dump();
         printf("Has #blocks: %d\n", getNumberOfBasicBlocks(llvmF));
         printf("\n\n"); printf("The inner function before \n"); llvmG->dump();
         printf("Has #blocks: %d\n", getNumberOfBasicBlocks(llvmG));
         printf("\n\n");)
    // Get all instructions in the inner function.
    std::vector<llvm::Instruction*> v;
    for (inst_iterator it = inst_begin(llvmG), e = inst_end(llvmG); it != e;
         ++it) {
        v.push_back(&(*it));
    }

    // find the call inside the outter function
    llvm::CallInst* call = getCall(llvmF);
    llvm::Instruction* arg = NULL;

    if (call != NULL) {
        // insert the instructions
        for (std::vector<llvm::Instruction*>::iterator it = v.begin();
             it != v.end(); ++it) {
            llvm::CallInst* inarg = dynamic_cast<llvm::CallInst*>(*it);
            if (inarg != NULL &&
                (inarg->getCalledFunction()->getName().str().find(
                     "genericGetVar") != std::string::npos)) {

                arg = *it;
            }
            (*it)->removeFromParent();
            (*it)->insertBefore(call);
        }
        if (arg != NULL) {
            llvm::Instruction* value = getConstant(llvmF);
            arg->replaceAllUsesWith(value);
            arg->removeFromParent();
        }

        call->removeFromParent();
        // MAGIC
        inst_begin(llvmF)->removeFromParent();
        (--inst_end(llvmF))->removeFromParent();
        (--(--inst_end(llvmF)))->removeFromParent();
        // MAGIC END
        printf("after removing the dead code\n");
        llvmF->dump();
        printf("Has #blocks: %d\n", getNumberOfBasicBlocks(llvmF));
        llvmG->removeFromParent();
        c.removeFromRelocations(g);
        // TODO instrument
        c.jitAll();
        return f;
    }

    DEBB(printf("Failure of inlining, the call found was NULL\n"));

    // TODO create a valid closure to return
    return R_NilValue;
}
}