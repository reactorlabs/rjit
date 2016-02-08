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
#include "StateMap.hpp"

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

REXPORT SEXP testme(SEXP expr) {
    Compiler c("module");
    /*SEXP result = c.compile("rfunction", BODY(expr), FORMALS(expr));
    auto test = StateMap::generateIdentityMapping(GET_LLVM(result));
    //test.first->removeFromParent();
    (GET_LLVM(result))->getParent()->getFunctionList().push_back(test.first);
    FunctionCall::fixIcStubs(test.first);
    c.jitAll();
    test.first->dump();*/
    SEXP result = OSRHandler::getFreshIR(expr, &c, true);
    FunctionCall::fixIcStubs(GET_LLVM(result));
    c.jitAll();
    return result;
}
}
