#include <assert.h>
#include <alloca.h>

#include "interp.h"
#include "interp_context.h"
#include "runtime.h"



#define NOT_IMPLEMENTED assert(false)

// TODO we are using the RInternals, but soud not when the code moves to GNU-R
// #include "RIntlns.h"

#undef eval

extern SEXP R_TrueValue;
extern SEXP R_FalseValue;
extern SEXP Rf_NewEnvironment(SEXP, SEXP, SEXP);
extern Rboolean R_Visible;

typedef SEXP (*CCODE)(SEXP, SEXP, SEXP, SEXP);

/* Information for Deparsing Expressions */
typedef enum {
    PP_INVALID = 0,
    PP_ASSIGN = 1,
    PP_ASSIGN2 = 2,
    PP_BINARY = 3,
    PP_BINARY2 = 4,
    PP_BREAK = 5,
    PP_CURLY = 6,
    PP_FOR = 7,
    PP_FUNCALL = 8,
    PP_FUNCTION = 9,
    PP_IF = 10,
    PP_NEXT = 11,
    PP_PAREN = 12,
    PP_RETURN = 13,
    PP_SUBASS = 14,
    PP_SUBSET = 15,
    PP_WHILE = 16,
    PP_UNARY = 17,
    PP_DOLLAR = 18,
    PP_FOREIGN = 19,
    PP_REPEAT = 20
} PPkind;

typedef enum {
    PREC_FN = 0,
    PREC_LEFT = 1,
    PREC_EQ = 2,
    PREC_RIGHT = 3,
    PREC_TILDE = 4,
    PREC_OR = 5,
    PREC_AND = 6,
    PREC_NOT = 7,
    PREC_COMPARE = 8,
    PREC_SUM = 9,
    PREC_PROD = 10,
    PREC_PERCENT = 11,
    PREC_COLON = 12,
    PREC_SIGN = 13,
    PREC_POWER = 14,
    PREC_DOLLAR = 15,
    PREC_NS = 16,
    PREC_SUBSET = 17
} PPprec;

typedef struct {
    PPkind kind;             /* deparse kind */
    PPprec precedence;       /* operator precedence */
    unsigned int rightassoc; /* right associative? */
} PPinfo;

/* The type definitions for the table of built-in functions. */
/* This table can be found in ../main/names.c */
typedef struct {
    char* name;  /* print name */
    CCODE cfun;  /* c-code address */
    int code;    /* offset within c-code */
    int eval;    /* evaluate args? */
    int arity;   /* function arity */
    PPinfo gram; /* pretty-print info */
} FUNTAB;

extern FUNTAB R_FunTab[];

typedef struct sxpinfo_struct_rjit {
    unsigned int type : 5; /* ==> (FUNSXP == 99) %% 2^5 == 3 == CLOSXP
                        * -> warning: `type' is narrower than values
                        *              of its type
                        * when SEXPTYPE was an enum */
    unsigned int obj : 1;
    unsigned int named : 2;
    unsigned int gp : 16;
    unsigned int mark : 1;
    unsigned int debug : 1;
    unsigned int trace : 1; /* functions and memory tracing */
    unsigned int spare : 1; /* currently unused */
    unsigned int gcgen : 1; /* old generation number */
    unsigned int gccls : 3; /* node class */
} sxpifo_struct_rjit;       /*		    Tot: 32 */

typedef struct cons_rjit {
    SEXP car;
    SEXP cdr;
    SEXP tag;
} cons_rjit;

typedef struct sexprec_rjit {
    struct sxpinfo_struct_rjit sxpinfo;
    SEXP attrib;
    SEXP gengc_next_node, gengc_prev_node;
    union {
        struct cons_rjit cons;
        int i;
    } u;
} sexprec_rjit;

// helpers

/** Moves the pc to next instruction, based on the current instruction length
 */
OpcodeT* advancePc(OpcodeT* pc) {
    switch (*pc++) {
#define DEF_INSTR(name, imm, ...)                                              \
    case name:                                                                 \
        pc += sizeof(ArgT) * imm;                                              \
        break;
#include "ir/insns.h"
    default:
        assert(false && "Unknown instruction");
    }
    return pc;
}

// bytecode accesses

INLINE Opcode readOpcode(OpcodeT** pc) {
    Opcode result = *(OpcodeT*)(*pc);
    *pc += sizeof(OpcodeT);
    return result;
}

INLINE unsigned readImmediate(OpcodeT** pc) {
    unsigned result = *(Immediate*)*pc;
    *pc += sizeof(Immediate);
    return result;
}

INLINE int readSignedImmediate(OpcodeT** pc) {
    int result = *(SignedImmediate*)*pc;
    *pc += sizeof(SignedImmediate);
    return result;
}

INLINE SEXP readConst(Context* ctx, OpcodeT** pc) {
    return cp_pool_at(ctx, readImmediate(pc));
}

INLINE int readJumpOffset(OpcodeT** pc) {
    int result = *(JumpOffset*)(*pc);
    *pc += sizeof(JumpOffset);
    return result;
}

INLINE SEXP getSrcAt(Code* c, OpcodeT* pc, Context* ctx) {
    // we need to determine index of the current instruction
    OpcodeT* x = code(c);
    // find the pc of the current instructions, it is ok to be slow
    unsigned insIdx = 0;
    while ((x = advancePc(x)) != pc)
        ++insIdx;
    unsigned sidx = src(c)[insIdx];
    // return the ast for the instruction, or if not defined, the ast of the
    // function
    return src_pool_at(ctx, sidx == 0 ? c->src : sidx);
}

INLINE SEXP getSrcForCall(Code* c, OpcodeT* pc, Context* ctx) {
    // we need to determine index of the current instruction
    OpcodeT* x = code(c);
    // find the pc of the current instructions, it is ok to be slow
    unsigned insIdx = 0;
    while ((x = advancePc(x)) != pc)
        ++insIdx;
    unsigned sidx = src(c)[insIdx];
    // return the ast for the instruction, or if not defined, the ast of the
    // function
    assert(sidx);
    return src_pool_at(ctx, sidx);
}


/** Creates a promise from given code object and environment.

 */
INLINE SEXP createPromise(Code* code, SEXP env) {
#if RIR_AS_PACKAGE == 1
    return mkPROMISE(rir_createWrapperPromise(code), env);
#else
    return mkPROMISE((SEXP)code, env);
#endif
}

INLINE SEXP promiseValue(SEXP promise, Context * ctx) {
    // if already evaluated, return the value
    if (PRVALUE(promise) && PRVALUE(promise) != R_UnboundValue) {
        promise = PRVALUE(promise);
        SET_NAMED(promise, 2);
        return promise;
    } else {
        return forcePromise(promise);
    }
}

// TODO remove numArgs and bp -- this is only needed for the on stack argument
// handling
#define INSTRUCTION(name)                                                      \
    INLINE void ins_##name(Code* c, SEXP env, OpcodeT** pc, Context* ctx,      \
                           unsigned numArgs, unsigned bp)

INSTRUCTION(push_) {
    SEXP x = readConst(ctx, pc);
    ostack_push(ctx, x);
}

INSTRUCTION(ldfun_) {
    SEXP sym = readConst(ctx, pc);
    SEXP val = findFun(sym, env);

    // TODO something should happen here
    if (val == R_UnboundValue)
        assert(false && "Unbound var");
    else if (val == R_MissingArg)
        assert(false && "Missing argument");

    switch (TYPEOF(val)) {
    case CLOSXP:
        /** If compile on demand is active, check that the function to be called
         * is compiled already, and compile if not.
         */
        if (COMPILE_ON_DEMAND) {
            SEXP body = BODY(val);
            if (TYPEOF(body) == BCODESXP)
                body = VECTOR_ELT(CDR(body), 0);
            if (TYPEOF(body) != INTSXP) {
                SEXP env = CLOENV(val);
                SEXP b = ctx->compiler(body);
                SET_BODY(val, b);
                //SET_BODY(val, ctx->compiler(body, CLOENV(val)));

            }
        }
        break;
    case SPECIALSXP:
    case BUILTINSXP:
        // special and builtin functions are ok
        break;
    default:
        assert(false);
    }
    ostack_push(ctx, val);
}

INSTRUCTION(ldddvar_) {
    SEXP sym = readConst(ctx, pc);
    SEXP val = Rf_ddfindVar(sym, env);

    // TODO better errors
    if (val == R_UnboundValue) {
        Rf_error("object not found");
    } else if (val == R_MissingArg) {
        error("argument is missing, with no default");
    }

    // if promise, evaluate & return
    if (TYPEOF(val) == PROMSXP)
        val = promiseValue(val, ctx);

    // WTF? is this just defensive programming or what?
    if (NAMED(val) == 0 && val != R_NilValue)
        SET_NAMED(val, 1);

    ostack_push(ctx, val);
}


INSTRUCTION(ldvar_) {
    SEXP sym = readConst(ctx, pc);
    SEXP val = findVar(sym, env);

    // TODO better errors
    if (val == R_UnboundValue) {
        Rf_error("object not found");
    } else if (val == R_MissingArg) {
        error("argument is missing, with no default");
    }

    // if promise, evaluate & return
    if (TYPEOF(val) == PROMSXP)
        val = promiseValue(val, ctx);

    // WTF? is this just defensive programming or what?
    if (NAMED(val) == 0 && val != R_NilValue)
        SET_NAMED(val, 1);

    ostack_push(ctx, val);
}

/** Given argument code offsets, creates the argslist from their promises.
 */
// TODO unnamed only at this point
int __listAppend(SEXP* front, SEXP* last, SEXP value, SEXP name) {
    int protected = 0;

    assert(TYPEOF(*front) == LISTSXP || TYPEOF(*front) == NILSXP);
    assert(TYPEOF(*last) == LISTSXP || TYPEOF(*last) == NILSXP);

    SEXP app = CONS_NR(value, R_NilValue);
    
    SET_TAG(app, name);

    if (*front == R_NilValue) {
        *front = app;
        PROTECT(*front);
        protected++;
    }

    if (*last != R_NilValue)
        SETCDR(*last, app);
    *last = app;

    return protected;
}

SEXP createArgsList(Code * c, FunctionIndex * args, size_t nargs, SEXP names, SEXP env) {
    SEXP result = R_NilValue;
    SEXP pos = result;
    int protected = 0;

    // loop through the arguments and create a promise, unless it is a missing argument
    for (size_t i = 0; i < nargs; ++i) {
        SEXP name = VECTOR_ELT(names, i);

        // if the argument is an ellipsis, then retrieve it from the environment and 
        // flatten the ellipsis
        if (name == R_DotsSymbol) {
            SEXP ellipsis = findVar(name, env);
            if (TYPEOF(ellipsis) == DOTSXP) {
                while (ellipsis != R_NilValue) {
                    name = TAG(ellipsis);
                    SEXP promise = mkPROMISE(CAR(ellipsis), env);
                    protected += __listAppend(&result, &pos, promise, name);
                    ellipsis = CDR(ellipsis);
                }
            } else if (args[i] == MISSING_ARG_OFFSET) {
                assert(false);
            }
        } else if (args[i] == MISSING_ARG_OFFSET) {
            protected += __listAppend(&result, &pos, R_MissingArg, R_NilValue);
        } else {
            unsigned offset = args[i];
            Code* arg = codeAt(function(c), offset);
            SEXP promise = createPromise(arg, env);
            protected += __listAppend(&result, &pos, promise, name);
        }
    }

    UNPROTECT(protected);
    return result;
}

SEXP createNoNameArgsList(Code * c, FunctionIndex * args, size_t nargs, SEXP env) {
    SEXP result = R_NilValue;
    SEXP pos = result;
    int protected = 0;

    for (size_t i = 0; i < nargs; ++i) {
        unsigned offset = args[i];
        if (args[i] == MISSING_ARG_OFFSET) {
            protected += __listAppend(&result, &pos, R_MissingArg, R_NilValue);
        } else {
            SEXP arg = createPromise(codeAt(function(c), offset), env);
            protected += __listAppend(&result, &pos, arg, R_NilValue);
        }
    }

    UNPROTECT(protected);
    return result;
}

SEXP createEagerArgsList(Code* c, FunctionIndex* args, size_t nargs, SEXP names,
                         SEXP env, Context* ctx) {
    SEXP result = R_NilValue;
    SEXP pos = result;
    int protected = 0;

    for (size_t i = 0; i < nargs; ++i) {
        unsigned offset = args[i];
        SEXP name = names != R_NilValue ? VECTOR_ELT(names, i) : R_NilValue;

        // if the argument is an ellipsis, then retrieve it from the environment and 
        // flatten the ellipsis
        if (name == R_DotsSymbol) {
            SEXP ellipsis = findVar(name, env);
            if (TYPEOF(ellipsis) == DOTSXP) {
                while (ellipsis != R_NilValue) {
                    SEXP arg = Rf_eval(CAR(ellipsis), env);
                    name = TAG(ellipsis);
                    protected += __listAppend(&result, &pos, arg, name);
                    ellipsis = CDR(ellipsis);
                }
            } else if (args[i] == MISSING_ARG_OFFSET) {
                assert(false);
            }
        } else if (args[i] == MISSING_ARG_OFFSET) {
            // TODO error
            assert(false);
        } else {
            SEXP arg = evalRirCode(codeAt(function(c), offset), ctx, env, 0);
            protected += __listAppend(&result, &pos, arg, name);
        }
    }

    UNPROTECT(protected);
    return result;
}

/** Returns us the CCODE object from R_FunTab based on name.

  TODO This exists in gnu-r (names.c), when integrated inside, we want to make
  use of it.
 */
CCODE getBuiltin(SEXP f) {
    int i = ((sexprec_rjit*)f)->u.i;
    return R_FunTab[i].cfun;
}
int getFlag(SEXP f) {
    int i = ((sexprec_rjit*)f)->u.i;
    return (((R_FunTab[i].eval)/100)%10);
}

// hooks for the call
SEXP closureArgumentAdaptor(SEXP call, SEXP op, SEXP arglist, SEXP rho, SEXP suppliedvars);
typedef struct {
    Code* code;
    Context* ctx;
    SEXP env;
    size_t nargs;
} EvalCbArg;
SEXP hook_rirCallTrampoline(void* cntxt, SEXP (*evalCb)(EvalCbArg*), EvalCbArg* arg);
SEXP evalCbFunction(EvalCbArg* arg_) {
    EvalCbArg* arg = (EvalCbArg*)arg_;
    return evalRirCode(arg->code, arg->ctx, arg->env, arg->nargs);
}

/** Performs the call.

  TODO this is currently super simple.

 */
SEXP doCall(Code * caller, SEXP call, SEXP callee, unsigned * args, size_t nargs, SEXP names, SEXP env, Context * ctx) {

    size_t oldbp = ostack_length(ctx);
    size_t oldbpi = ctx->istack.length;
    SEXP result = R_NilValue;
    switch (TYPEOF(callee)) {
    case SPECIALSXP: {
        // get the ccode
        CCODE f = getBuiltin(callee);
        int flag = getFlag(callee);
        R_Visible = flag != 1;
        // call it with the AST only
        result = f(call, callee, CDR(call), env);
        if (flag < 2) R_Visible = flag != 1;
        break;
    }
    case BUILTINSXP: {
        // get the ccode
        CCODE f = getBuiltin(callee);
        int flag = getFlag(callee);
        // create the argslist
        SEXP argslist = createEagerArgsList(caller, args, nargs, names, env, ctx);
        // callit
        PROTECT(argslist);
        R_Visible = flag != 1;
        result = f(call, callee, argslist, env);
        if (flag < 2) R_Visible = flag != 1;
        UNPROTECT(1);
        break;
    }
    case CLOSXP: {
        SEXP actuals;
        if (names != R_NilValue) {
            actuals = createArgsList(caller, args, nargs, names, env);
        } else {
            actuals = createNoNameArgsList(caller, args, nargs, env);
        }
        PROTECT(actuals);
#if RIR_AS_PACKAGE == 0
        // if body is INTSXP, it is rir serialized code, execute it directly
        SEXP body = BODY(callee);
        if (TYPEOF(body) == INTSXP) {
            SEXP newEnv = closureArgumentAdaptor(call, callee, actuals, env, R_NilValue);

            // TODO since we do not have access to the context definition we
            // just create a buffer big enough and let gnur do the rest.
            // Once we integrate we should really setup the context ourselves!
            char cntxt[400];
            initClosureContext(&cntxt, call, newEnv, env, actuals, callee);
            EvalCbArg arg = {functionCode((Function*)INTEGER(body)), ctx, newEnv, nargs};
            result = hook_rirCallTrampoline(&cntxt, evalCbFunction, &arg);
            endClosureContext(&cntxt, result);
            UNPROTECT(1);
            break;
        }
#endif
        Function * f = isValidClosureSEXP(callee);
        if (f != NULL) {
            // TODO we do not have to go to gnu-r here, but setting up the context w/o gnu-r will be tricky
            result = applyClosure(call, callee, actuals, env, R_NilValue);
        } else {
            // otherwise use R's own call mechanism
            result = applyClosure(call, callee, actuals, env, R_NilValue);
        }
        UNPROTECT(1); // argslist
        break;
    }
    default:
        assert(false && "Don't know how to run other stuff");
    }
    assert(oldbp == ostack_length(ctx) && oldbpi == ctx->istack.length &&
           "Corrupted stacks");
    return result;
}


INSTRUCTION(call_) {
    // get the indices of argument promises
    SEXP args_ = readConst(ctx, pc);
    assert(TYPEOF(args_) == INTSXP &&
           "TODO change to INTSXP, not RAWSXP it used to be");
    unsigned nargs = Rf_length(args_) / sizeof(unsigned);
    unsigned* args = (unsigned*)INTEGER(args_);
    // get the names of the arguments (or R_NilValue) if none
    SEXP names = readConst(ctx, pc);
    // get the closure itself
    SEXP cls = ostack_pop(ctx);
    // do the call
    SEXP call = getSrcForCall(c, *pc, ctx);

    ostack_push(ctx, doCall(c, call, cls, args, nargs, names, env, ctx));
}

INSTRUCTION(promise_) {
    // get the Code * pointer we need
    unsigned codeOffset = readImmediate(pc);
    Code* promiseCode = codeAt(function(c), codeOffset);
    // create the promise and push it on stack
    ostack_push(ctx, createPromise(promiseCode, env));
}

INSTRUCTION(close_) {
    SEXP body = ostack_pop(ctx);
    SEXP formals = ostack_pop(ctx);
    PROTECT(body);
    PROTECT(formals);
    SEXP result = allocSExp(CLOSXP);
    SET_FORMALS(result, formals);
    SET_BODY(result, body);
    SET_CLOENV(result, env);
    UNPROTECT(2);
    ostack_push(ctx, result);
}

INSTRUCTION(force_) {
    SEXP p = ostack_pop(ctx);
    assert(TYPEOF(p) == PROMSXP);
    // If the promise is already evaluated then push the value inside the
    // promise
    // onto the stack, otherwise push the value from forcing the promise
    ostack_push(ctx, promiseValue(p, ctx));
}

INSTRUCTION(pop_) { ostack_pop(ctx); }

INSTRUCTION(pusharg_) {
    unsigned n = readImmediate(pc);
    assert(n < numArgs);
    ostack_push(ctx, ostack_at(ctx, bp - numArgs + n));
}

INSTRUCTION(asast_) {
    SEXP p = ostack_pop(ctx);
    assert(TYPEOF(p) == PROMSXP);
    SEXP ast = PRCODE(p);
    // if the code is NILSXP then it is rir Code object, get its ast
    if (TYPEOF(ast) == NILSXP)
        ast = cp_pool_at(ctx, ((Code*)ast)->src);
    // otherwise return whatever we had, make sure we do not see bytecode
    assert(TYPEOF(ast) != BCODESXP);
    ostack_push(ctx, ast);
}

INSTRUCTION(stvar_) {
    SEXP sym = ostack_pop(ctx);
    assert(TYPEOF(sym) == SYMSXP);
    SEXP val = ostack_pop(ctx);
    defineVar(sym, val, env);
    ostack_push(ctx, val);
}

INSTRUCTION(asbool_) {
    SEXP t = ostack_pop(ctx);
    int cond = NA_LOGICAL;
    if (Rf_length(t) > 1)
        warningcall(getSrcAt(c, *pc, ctx),
                    ("the condition has length > 1 and only the first "
                     "element will be used"));

    if (Rf_length(t) > 0) {
        switch (TYPEOF(t)) {
        case LGLSXP:
            cond = LOGICAL(t)[0];
            break;
        case INTSXP:
            cond = INTEGER(t)[0]; // relies on NA_INTEGER == NA_LOGICAL
        default:
            cond = asLogical(t);
        }
    }

    if (cond == NA_LOGICAL) {
        const char* msg =
            Rf_length(t)
                ? (isLogical(t) ? ("missing value where TRUE/FALSE needed")
                                : ("argument is not interpretable as logical"))
                : ("argument is of length zero");
        errorcall(getSrcAt(c, *pc, ctx), msg);
    }

    ostack_push(ctx, cond ? R_TrueValue : R_FalseValue);
}

INSTRUCTION(brtrue_) {
    int offset = readJumpOffset(pc);
    if (ostack_pop(ctx) == R_TrueValue)
        *pc = *pc + offset;
}

INSTRUCTION(brfalse_) {
    int offset = readJumpOffset(pc);
    if (ostack_pop(ctx) == R_FalseValue)
        *pc = *pc + offset;
}

INSTRUCTION(br_) {
    int offset = readJumpOffset(pc);
    *pc = *pc + offset;
}

INSTRUCTION(lti_) {
    int rhs = istack_pop(ctx);
    int lhs = istack_pop(ctx);
    ostack_push(ctx, lhs < rhs ? R_TrueValue : R_FalseValue);
}

INSTRUCTION(eqi_) {
    int rhs = istack_pop(ctx);
    int lhs = istack_pop(ctx);
    ostack_push(ctx, lhs == rhs ? R_TrueValue : R_FalseValue);
}

INSTRUCTION(pushi_) { istack_push(ctx, readSignedImmediate(pc)); }

INSTRUCTION(dupi_) { istack_push(ctx, istack_top(ctx)); }

INSTRUCTION(dup_) { ostack_push(ctx, ostack_top(ctx)); }

INSTRUCTION(add_) {
    assert(false);
    SEXP rhs = ostack_pop(ctx);
    SEXP lhs = ostack_pop(ctx);
    if (TYPEOF(lhs) == REALSXP && TYPEOF(rhs) == REALSXP &&
        Rf_length(lhs) == 1 && Rf_length(rhs) == 1) {
        SEXP res = Rf_allocVector(REALSXP, 1);
        SET_NAMED(res, 1);
        REAL(res)[0] = REAL(lhs)[0] + REAL(rhs)[0];
        ostack_push(ctx, res);
    } else {
        NOT_IMPLEMENTED;
        // SEXP op = getPrimitive("+");
        // SEXP primfun = getPrimfun("+");
        // TODO we should change how primitives are called now that we can
        // integrate
        // SEXP res = callPrimitive(primfun, getCurrentCall(c, pc), op, { lhs,
        // rhs });
        // TODO push
    }
}

INSTRUCTION(sub_) {
    assert(false);
    SEXP rhs = ostack_pop(ctx);
    SEXP lhs = ostack_pop(ctx);
    if (TYPEOF(lhs) == REALSXP && TYPEOF(rhs) == REALSXP &&
        Rf_length(lhs) == 1 && Rf_length(rhs) == 1) {
        SEXP res = Rf_allocVector(REALSXP, 1);
        SET_NAMED(res, 1);
        REAL(res)[0] = REAL(lhs)[0] - REAL(rhs)[0];
        ostack_push(ctx, res);
    } else {
        NOT_IMPLEMENTED;
        // SEXP op = getPrimitive("-");
        // SEXP primfun = getPrimfun("-");
        // TODO we should change how primitives are called now that we can
        // integrate
        // SEXP res = callPrimitive(primfun, getCurrentCall(c, pc), op, { lhs,
        // rhs });
        // TODO push
    }
}

INSTRUCTION(lt_) {
    SEXP rhs = ostack_pop(ctx);
    SEXP lhs = ostack_pop(ctx);
    if (TYPEOF(lhs) == REALSXP && TYPEOF(rhs) == REALSXP &&
        Rf_length(lhs) == 1 && Rf_length(rhs) == 1) {
        SEXP res = Rf_allocVector(REALSXP, 1);
        SET_NAMED(res, 1);
        ostack_push(ctx,
                    REAL(lhs)[0] < REAL(rhs)[0] ? R_TrueValue : R_FalseValue);
    } else {
        NOT_IMPLEMENTED;
        // SEXP op = getPrimitive("<");
        // SEXP primfun = getPrimfun("<");
        // TODO we should change how primitives are called now that we can
        // integrate
        // SEXP res = callPrimitive(primfun, getCurrentCall(c, pc), op, { lhs,
        // rhs });
        // TODO push
    }
}

INSTRUCTION(isspecial_) {
    // TODO I do not think this is a proper way - we must check all the way
    // down, not just findVar (vars do not shadow closures)
    SEXP sym = readConst(ctx, pc);
    SEXP val = findVar(sym, env);
    // TODO better check
    assert(TYPEOF(val) == SPECIALSXP || TYPEOF(val) == BUILTINSXP);
}

INSTRUCTION(isfun_) {
    SEXP val = ostack_top(ctx);

    switch (TYPEOF(val)) {
    case CLOSXP:
        /** No need to compile, we can handle functions */
        // val = (SEXP)jit(val);
        break;
    case SPECIALSXP:
    case BUILTINSXP:
        // builtins and specials are fine
        // TODO for now - we might be fancier here later
        break;
    /*

        {
            // TODO do we need to compile primitives? not really I think
            SEXP prim = Primitives::compilePrimitive(val);
            if (prim)
                val = prim;
            break;
        } */

    default:
        // TODO: error not a function!
        // TODO: check our functions and how they are created
        assert(false);
    }
}

INSTRUCTION(inci_) { istack_push(ctx, istack_pop(ctx) + 1); }

INSTRUCTION(push_argi_) {
    int pos = istack_pop(ctx);
    ostack_push(ctx, cp_pool_at(ctx, bp - numArgs + pos));
}

INSTRUCTION(invisible_) {
    R_Visible = 0;
}

extern void printCode(Code* c);
extern void printFunction(Function* f);

extern SEXP Rf_deparse1(SEXP call, Rboolean abbrev, int opts);
extern void R_SetErrorHook(void (*hook)(SEXP, char *));

extern void rirBacktrace(Context* ctx) {
    if (fstack_empty(ctx))
        return;

    FStack* f = ctx->fstack;
    while(f) {
        for (int i = f->length - 1; i >= 0; i--) {
            Frame* frame = &f->data[i];
            Code* code = frame->code;
            SEXP call = src_pool_at(ctx, code->src);

            Rprintf("%d : %s\n", i, CHAR(STRING_ELT(Rf_deparse1(call, 0, 0), 0)));
            Rprintf(" env: ");
            SEXP names = R_lsInternal3(frame->env, TRUE, FALSE);
            PROTECT(names);
            for (int i = 0; i < Rf_length(names); ++i)
                Rprintf("%s ", CHAR(STRING_ELT(R_lsInternal3(frame->env, TRUE, FALSE), i)));
            Rprintf("\n");
        }
        f = f->prev;
    }
}

SEXP evalRirCode(Code* c, Context* ctx, SEXP env, unsigned numArgs) {

    // printCode(c);

    // make sure there is enough room on the stack
    ostack_ensureSize(ctx, c->stackLength);
    istack_ensureSize(ctx, c->iStackLength);
    Frame* frame = fstack_push(ctx, c, env);

    // get pc and bp regs, we do not need istack bp
    frame->pc = code(c);
    OpcodeT** pc = &frame->pc;
    size_t bp = ostack_length(ctx);

    // main loop
    while (true) {
        // printf("%p : %p, %p\n", c, *pc, *pc - c->data);
        switch (readOpcode(pc)) {

#define INS(name)                                                              \
    case name:                                                                 \
        ins_##name(c, env, pc, ctx, numArgs, bp);                              \
        break

            INS(push_);
            INS(ldfun_);
            INS(ldvar_);
            INS(ldddvar_);
            INS(call_);
            INS(promise_);
            INS(close_);
            INS(force_);
            INS(pop_);
            INS(pusharg_);
            INS(asast_);
            INS(stvar_);
            INS(asbool_);
            INS(brtrue_);
            INS(brfalse_);
            INS(br_);
            INS(lti_);
            INS(eqi_);
            INS(pushi_);
            INS(dupi_);
            INS(dup_);
            INS(add_);
            INS(sub_);
            INS(lt_);
            INS(isspecial_);
            INS(isfun_);
            INS(inci_);
            INS(push_argi_);
            INS(invisible_);
        case ret_: {
            // not in its own function so that we can avoid nonlocal returns
            goto __eval_done;
        }
        default:
            assert(false && "wrong or unimplemented opcode");
        }
    }
__eval_done:
    fstack_pop(ctx, frame);
    return ostack_pop(ctx);
}


SEXP rirExpr(SEXP f) {
    if (isValidCodeObject(f)) {
        Code* c = (Code*)f;
        return src_pool_at(globalContext(), c->src);
    }
    if (isValidFunctionObject(f)) {
        Function* ff = (Function*)(INTEGER(f));
        return src_pool_at(globalContext(), functionCode(ff)->src);
    }
    return f;
}

SEXP rirEval_f(SEXP f, SEXP env) {
    // TODO we do not really need the arg counts now
    if (isValidCodeObject(f)) {
        //        Rprintf("Evaluating promise:\n");
        Code* c = (Code*)f;
        SEXP x = evalRirCode(c, globalContext(), env, 0);
      //        Rprintf("Promise evaluated, length %u, value %d",
        //        Rf_length(x), REAL(x)[0]);
        return x;
    } else {
        //        Rprintf("=====================================================\n");
        //        Rprintf("Evaluating function\n");
        Function* ff = (Function*)(INTEGER(f));
        return evalRirCode(functionCode(ff), globalContext(), env, 0);
    }
}





