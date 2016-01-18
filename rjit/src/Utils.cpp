#include <llvm/IR/Module.h>
#include "Compiler.h"

#include "api.h"

#include "RIntlns.h"

#include "ir/ir.h"
#include "ir/Builder.h"
#include "ir/intrinsics.h"
#include "ir/Handler.h"

#include "Utils.h"

#include <llvm/IR/BasicBlock.h>
#include "llvm/Analysis/TargetTransformInfo.h"

#include "R.h"

#include "FunctionCall.h"

#include "OSRHandler.h"
#include "OSRInliner.h"

using namespace rjit;
namespace osr {

REXPORT SEXP printSplitBlock(SEXP expr) {
    Compiler c("module");
    SEXP result = c.compile("rfunction", expr);
    llvm::Function* rfunction = reinterpret_cast<llvm::Function*>(TAG(result));
    printf("Before the split.\n");
    rfunction->dump();
    for (inst_iterator it = inst_begin(rfunction), e = inst_end(rfunction);
         it != e; ++it) {
        llvm::CallInst* call = dynamic_cast<llvm::CallInst*>(&(*it));
        if (call != NULL && IS_GET_FUNCTION(call)) {
            llvm::BasicBlock* bb =
                dynamic_cast<llvm::BasicBlock*>(call->getParent());
            if (bb != NULL) {
                printf("We're in the correct spot!\n");
                llvm::BasicBlock* res =
                    bb->splitBasicBlock(call, "OSRinlining");
                printf("The BB returned\n");
                res->dump();
                printf("The full function now\n");
                rfunction->dump();
                return result;
            }
        }
    }
    return result;
}

REXPORT SEXP printWithoutSP(SEXP expr) {
    Compiler c("module");
    SEXP result = c.compile("rfunction", expr);
    llvm::Function* rfunction = reinterpret_cast<llvm::Function*>(TAG(result));
    rfunction->dump();
    return result;
}

REXPORT SEXP testOSR(SEXP outer, SEXP env) {
    SEXP res = OSRInliner::inlineCalls(outer, env);
    return res;
}

REXPORT SEXP printFormals(SEXP f) {
    SEXP res = FORMALS(f);
    return res;
}
}