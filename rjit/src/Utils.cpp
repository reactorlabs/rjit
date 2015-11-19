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

using namespace rjit;
namespace osr {

REXPORT SEXP printWithoutSP(SEXP expr) {
    Compiler c("module");
    SEXP result = c.compile("rfunction", expr);
    llvm::Function* rfunction = reinterpret_cast<llvm::Function*>(TAG(result));
    rfunction->dump();
    return result;
}

REXPORT SEXP extractFunctionCalls(SEXP expr) {
    Compiler c("module");
    SEXP result = c.compile("rfunction", expr);
    llvm::Function* rfunction = reinterpret_cast<llvm::Function*>(TAG(result));
    rfunction->dump();
    FunctionCalls* calls = FunctionCall::getFunctionCalls(rfunction);
    for (FunctionCalls::iterator it = calls->begin(); it != calls->end();
         ++it) {
        (*it)->printFunctionCall();
    }
    return R_NilValue;
}
}
