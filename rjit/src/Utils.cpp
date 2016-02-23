#include <llvm/IR/Module.h>
#include "Compiler.h"

#include "api.h"

#include "RIntlns.h"

#include "ir/Ir.h"
#include "ir/Builder.h"
#include "ir/primitive_calls.h"

#include "Utils.h"

#include <llvm/IR/BasicBlock.h>
#include "llvm/Analysis/TargetTransformInfo.h"

#include "R.h"

#include "FunctionCall.h"

#include "OSRHandler.h"
#include "OSRInliner.h"
#include "StateMap.hpp"
#include <llvm/IR/Verifier.h>

using namespace rjit;
using namespace llvm;
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

REXPORT SEXP getFresh(SEXP expr) {
    Compiler c("module");
    SEXP result = OSRHandler::getFreshIR(expr, &c, false);
    /*SEXP control = c.compile("rfunction", BODY(expr), FORMALS(expr));
    Function* f = GET_LLVM(control);
    assert(f);*/
    OSRHandler::addSexpToModule(result, c.getBuilder()->module());
    c.getBuilder()->module()->fixRelocations(FORMALS(expr), result,
                                             GET_LLVM(result));
    OSRHandler::resetSafepoints(result, &c);
    // printf("THE MODULE ADDRESS %p \n", c.getBuilder()->module());
    // llvm::verifyModule(*(c.getBuilder()->module()), &llvm::outs());
    // llvm::verifyFunction(*(GET_LLVM(result)), &llvm::outs());
    c.jitAll();
    return result;
}
REXPORT SEXP clearHandler() {
    OSRHandler::clear();
    return R_NilValue;
}
}
