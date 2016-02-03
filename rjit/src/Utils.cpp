#include <llvm/IR/Module.h>
#include "Compiler.h"

#include "api.h"

#include "RIntlns.h"

#include "ir/Ir.h"
#include "ir/Builder.h"
#include "ir/Intrinsics.h"

#include "Utils.h"

#include <llvm/IR/BasicBlock.h>
#include "llvm/Analysis/TargetTransformInfo.h"

#include "R.h"

#include "FunctionCall.h"

#include "OSRHandler.h"
#include "OSRInliner.h"

using namespace rjit;
namespace osr {

REXPORT SEXP printWithoutSP(SEXP expr) {
    Compiler c("module");
    SEXP result = c.compile("rfunction", BODY(expr), FORMALS(expr));
    llvm::Function* rfunction = reinterpret_cast<llvm::Function*>(TAG(result));
    rfunction->dump();
    return result;
}

REXPORT SEXP printFormals(SEXP f) {
    SEXP res = FORMALS(f);
    return res;
}
}
