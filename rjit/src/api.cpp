#include <llvm/IR/Module.h>

#include "Compiler.h"

#include "api.h"

#include "RIntlns.h"

#include "ir/Ir.h"
#include "ir/Builder.h"
#include "ir/Intrinsics.h"
#include "OSRInliner.h"
#include "OSRHandler.h"

using namespace rjit;

/** Compiles given ast and returns the NATIVESXP for it.
 */
REXPORT SEXP jitAst(SEXP ast, SEXP formals, SEXP rho) {
    Compiler c("module");
    SEXP result = c.compile("rfunction", ast, formals);
    c.jitAll();
    return result;
}

// TODO aghosn
REXPORT SEXP testOSR(SEXP outer, SEXP env) {
    Compiler c("module");
    OSR_INLINE = 1;
    // return jitAst(BODY(outer), FORMALS(outer), env);
    osr::OSRInliner inliner(&c);
    SEXP res = inliner.inlineCalls(outer);
    c.jitAll();
    return res;
}

REXPORT SEXP osrInline(SEXP f) {
    assert(TYPEOF(f) == CLOSXP);
    if (OSR_INLINE) {
        Compiler c("module");
        osr::OSRInliner inliner(&c);
        SEXP result = inliner.inlineCalls(f);
        c.jitAll();
        return result;
    } else {
        return jitAst(BODY(f), FORMALS(f), TAG(f));
    }
}

/** More complex compilation method that compiles multiple functions into a
  specified module name.

  The module name is expected to be a STRSXP and the functions is expected to be
  a pairlist. If pairlist has tags associated with the elements, they will be
  used as function names.
 */
REXPORT SEXP jitFunctions(SEXP moduleName, SEXP functions) {
    char const* mName = CHAR(STRING_ELT(moduleName, 0));
    Compiler c(mName);
    while (functions != R_NilValue) {
        SEXP f = CAR(functions);
        // get the function ast
        SEXP body = BODY(f);
        SEXP formals = FORMALS(f);
        SEXP name = TAG(functions);
        char const* fName =
            (name == R_NilValue) ? "unnamed function" : CHAR(PRINTNAME(name));
        if (TYPEOF(body) == NATIVESXP)
            warning("Ignoring %s because it is already compiled", fName);
        else
            SET_BODY(f, c.compileFunction(fName, body, formals));
        // move to next function
        functions = CDR(functions);
    }
    c.jitAll();
    return moduleName;
}

/** Returns the constant pool associated with the given NATIVESXP.
 */
REXPORT SEXP jitConstants(SEXP expression) {
    assert(TYPEOF(expression) == NATIVESXP and
           "JIT constants can only be extracted from a NATIVESXP argument");
    return CDR(expression);
}

/** Displays the LLVM IR for given NATIVESXP.
 */
REXPORT SEXP jitLLVM(SEXP expression) {
    assert(TYPEOF(expression) == NATIVESXP and
           "LLVM code can only be extracted from a NATIVESXP argument");
    llvm::Function* f = reinterpret_cast<llvm::Function*>(TAG(expression));
    f->dump();
    return R_NilValue;
}

// Should rjit code recompile uncompiled functions before calling them
int RJIT_COMPILE = getenv("RJIT_COMPILE") ? atoi(getenv("RJIT_COMPILE")) : 0;
// The status of R_ENABLE_JIT variable used by gnur
int R_ENABLE_JIT = getenv("R_ENABLE_JIT") ? atoi(getenv("R_ENABLE_JIT")) : 0;

int RJIT_DEBUG = getenv("RJIT_DEBUG") ? atoi(getenv("RJIT_DEBUG")) : 0;

// TODO aghosn
int OSR_INLINE = getenv("OSR_INLINE") ? atoi(getenv("OSR_INLINE")) : 0;
int INLINE_ALL = getenv("INLINE_ALL") ? atoi(getenv("INLINE_ALL")) : 0;
int ONLY_GLOBAL = getenv("ONLY_GLOBAL") ? atoi(getenv("ONLY_GLOBAL")) : 0;

REXPORT SEXP jitDisable(SEXP expression) {
    RJIT_COMPILE = false;
    return R_NilValue;
}

REXPORT SEXP jitEnable(SEXP expression) {
    RJIT_COMPILE = true;
    return R_NilValue;
}

// TODO aghosn
REXPORT void fixClosure(uint64_t bim) {
    printf("I'am here %d\n", (int)bim);
    auto f = osr::OSRHandler::getExit(bim);
    f->dump();
}
