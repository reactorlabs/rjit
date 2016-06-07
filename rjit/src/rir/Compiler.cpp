#include "Compiler.h"

#include "BC.h"
#include "OpenFunction.h"
#include "CodeStream.h"

#include "../Sexp.h"
#include "../RIntlns.h"
#include "../RList.h"
#include "../Symbols.h"

#include "Pool.h"
#include "Optimizer.h"

namespace rjit {
namespace rir {

namespace {

fun_idx_t compilePromise(OpenFunction* f, SEXP exp);
void compileExpression(OpenFunction* f, CodeStream& cs, SEXP exp);

// function application
void compileCall(OpenFunction* f, CodeStream& cs, SEXP ast, SEXP fun,
                 SEXP args) {
    // application has the form:
    // LHS ( ARGS )

    // LHS can either be an identifier or an expression
    Match(fun) {
        Case(SYMSXP) { cs << BC::getfun(fun); }
        Else({
            compileExpression(f, cs, fun);
            cs << BC::check_function();
        });
    }

    // Process arguments:
    // Arguments can be optionally named
    std::vector<fun_idx_t> callArgs;
    std::vector<SEXP> names;

    for (auto arg = RList(args).begin(); arg != RList::end(); ++arg) {
        // (1) Arguments are wrapped as Promises:
        //     create a new Code object for the promise
        size_t prom = compilePromise(f, *arg);
        callArgs.push_back(prom);

        // (2) remember if the argument had a name associated
        names.push_back(arg.hasTag() ? arg.tag() : R_NilValue);
    }
    assert(callArgs.size() < MAX_NUM_ARGS);

    cs << BC::call(callArgs, names);

    cs.addAst(ast);
}

// Lookup
void compileGetvar(CodeStream& cs, SEXP name) { cs << BC::getvar(name); }

// Constant
void compileConst(CodeStream& cs, SEXP constant) {
    SET_NAMED(constant, 2);
    cs << BC::push(constant);
}

void compileExpression(OpenFunction* f, CodeStream& cs, SEXP exp) {
    // Dispatch on the current type of AST node
    Match(exp) {
        // OpenFunction application
        Case(LANGSXP, fun, args) { compileCall(f, cs, exp, fun, args); }
        // Variable lookup
        Case(SYMSXP) { compileGetvar(cs, exp); }
        // Constant
        Else(compileConst(cs, exp));
    }
}

void compileFormals(CodeStream& cs, SEXP formals) {
    size_t narg = 0;
    for (auto arg = RList(formals).begin(); arg != RList::end(); ++arg) {
        // TODO support default args
        assert(*arg == R_MissingArg);

        SEXP name = arg.tag();
        assert(name && name != R_NilValue);

        // TODO
        assert(name != symbol::Ellipsis);

        narg++;
    }
}

fun_idx_t compileFunction(OpenFunction* f, SEXP exp, SEXP formals) {
    CodeStream& cs = f->addCode(exp);
    if (formals)
        compileFormals(cs, formals);
    compileExpression(f, cs, exp);
    cs << BC::ret();
    return cs.getIdx();
}

fun_idx_t compilePromise(OpenFunction* f, SEXP exp) {
    CodeStream& cs = f->addCode(exp);
    compileExpression(f, cs, exp);
    cs << BC::ret();
    return cs.getIdx();
}
}

Function* Compiler::finalize() {
    OpenFunction* f = new OpenFunction;

    compileFunction(f, exp, formals);
    Optimizer::optimize(f);

    Function* res = f->finalize();
    delete f;
    return res;
}
}
}
