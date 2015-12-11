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
#include "ABInliner.h"

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

REXPORT SEXP testOSR(SEXP outer, SEXP inner, SEXP env) {
    Compiler c("module");
    SEXP rO = c.compile("outer", outer);
    SEXP rI = c.compile("inner", inner);
    SEXP poolO = CDR(rO);
    SEXP poolI = CDR(rI);
    int sizeO = LENGTH(poolO);
    int sizeI = LENGTH(poolI);
    SEXP objs = allocVector(VECSXP, sizeO + sizeI);
    for (int i = 0; i < sizeO; ++i)
        SET_VECTOR_ELT(objs, i, VECTOR_ELT(poolO, i));
    for (int i = 0; i < sizeI; ++i)
        SET_VECTOR_ELT(objs, i + sizeO, VECTOR_ELT(poolI, i));

    SETCDR(rO, objs);
    c.jitAll();
    return rO;
}

REXPORT SEXP testInline(SEXP outer, SEXP inner, SEXP env) {
    /* Compiler c("module");
     SEXP rO = c.compile("outer", outer);
     SEXP rI = c.compile("inner", inner);*/
    SEXP res = ABInliner::inlineThisInThat(outer, inner, env);
    // c.jitAll();
    return res;
}
}
