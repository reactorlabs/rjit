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

using namespace std;
using namespace chrono;

using namespace rjit;
using namespace llvm;
namespace osr {

high_resolution_clock::time_point Utils::start;
high_resolution_clock::time_point Utils::end;

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
    SEXP result = OSRHandler::getFreshIR(expr, &c);
    OSRHandler::addSexpToModule(result, c.getBuilder()->module());
    c.getBuilder()->module()->fixRelocations(FORMALS(expr), result,
                                             GET_LLVM(result));
    OSRHandler::resetSafepoints(result, &c);
    c.jitAll();
    return result;
}
REXPORT SEXP clearHandler() {
    OSRHandler::clear();
    return R_NilValue;
}

REXPORT SEXP enableOSR() {
    OSR_INLINE = 1;
    return R_NilValue;
}
REXPORT SEXP disableOSR() {
    OSR_INLINE = 0;
    return R_NilValue;
}

REXPORT SEXP startChrono() {
    Utils::start = high_resolution_clock::now();
    return R_NilValue;
}

REXPORT SEXP endChrono() {
    Utils::end = high_resolution_clock::now();
    auto duration =
        duration_cast<milliseconds>(Utils::end - Utils::start).count();
    ofstream file;
    file.open("benchResults/result.out", ios::out | ios::app);
    file << duration << "\n";
    file.close();
    return R_NilValue;
}

REXPORT SEXP endRecord() {
    ofstream file;
    file.open("benchResults/result.out", ios::out | ios::app);
    file << "\n";
    file.close();
    return R_NilValue;
}
}
