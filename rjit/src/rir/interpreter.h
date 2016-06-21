#ifndef RIR_INTERPRETER_H
#define RIR_INTERPRETER_H

#include <R.h>
#include <Rinternals.h>

#include <stdint.h>

/** TODO BC_t cannot be enum class perhaps, have to be in proper header and all that.

    const pool, interpreter stack and debugInfo must be recreated in C

 */


#ifdef __cplusplus
extern "C" {
#endif



typedef uint8_t BC_t;

typedef uint32_t PoolIndex;
typedef uint16_t ArgCount;
typedef int16_t JumpTarget;

typedef enum {
    // This is only here to trap accidentally calling zero initialized memory
    invalid,

    // Push a constant to the stack
    // I: constant (via Pool)
    // S: +1
    push,

    // Function lookup
    // Pushes a closure (or primitive) to the stack
    // I: symbol (via Pool)
    // S: +1
    getfun,

    // Variable lookup
    // I: symbol (via Pool)
    // S: +1
    getvar,

    // Call function
    // Immediate arguments are the arguments to the call (given as a list of
    //  code object indices) and a list of name tags of the arguments.
    // I: {arguments, names}
    // S: -N
    BC_call,

    // Create a promise
    // I: promise code object index
    // S: +1
    mkprom,

    // Create a closure
    // I: closure code object index
    // S: +1
    mkclosure,

    // Return
    // return value is tos
    ret,

    // Force the promise on tos
    // Leaves promise on tos
    force,

    // Pop one value from stack
    pop,

    // Load a specific function argument to the stack
    // argument# is immediate
    // Only valid for CallingConventions CC::*Stack
    load_arg,

    // Expects a promise tos, replaces it by its ast
    get_ast,

    // name and value from stack
    // value left on stack
    setvar,

    // push the number of arguments given to a CC::*Stack function
    numargi,

    // converts tos to a bool scalar
    to_bool,

    // pc += offset iff tos == true
    jmp_true,

    // pc += offset iff tos == false
    jmp_false,

    // unconditional jump
    jmp,

    // less than on unboxed integers
    lti,

    // equality on unboxed integers
    eqi,

    // force all promise arguments to this function passed on the stack
    // (currently unused)
    force_all,

    // push unboxed integer
    pushi,

    // duplicate unboxed integer
    dupi,

    // Load a specific function argument to the stack
    // unboxed integer argument# expected
    // Only valid for CallingConventions CC::*Stack
    load_argi,

    // Increment tos unboxed integer
    inci,

    // duplicate tos
    dup,

    // +
    add,

    // -
    sub,

    // <
    lt,

    // Immediate symbol of a special as argument. Checks whether special is
    // overwritten. Currently asserts(). TODO: osr
    check_special,

    // Checks that the top of the stack is a function (closure, builtin or
    // special)
    check_function,

    num_of
} BC;


typedef struct Function Function;  // Forward declaration

  // all sizes in bytes,
  // length in element sizes

/**
 * How many bytes do we need to align on 4 byte boundary?
 */
unsigned pad4(unsigned sizeInBytes) {
    unsigned x = sizeInBytes % 4;
    return (x != 0) ? (sizeInBytes + 4 - x)  :  sizeInBytes;
}

/**
 * Code holds a sequence of instructions; for each instruction
 * it records the index of the source AST. Code is part of a
 * Function.
 *
 * Code objects are allocated contiguously within the data
 * section of a Function. The Function header can be found,
 * at an offset from the start of each Code object
 *
 * Instructions are variable size; Code knows how many bytes
 * are required for instructions.
 *
 * The number of indices of source ASTs stored in Code equals
 * the number of instructions.
 *
 * Instructions and AST indices are allocated one after the
 * other in the Code's data section with padding to ensure
 * alignment of indices.
 */
typedef struct Code {
    unsigned header; /// offset to Function object

    unsigned codeSize;  /// bytes of code (not padded)
    unsigned srcLength;  /// number of instructions

    uint8_t data[]; /// the instructions
} Code;

/** Returns a pointer to the instructions in c.  */
BC_t * code(Code * c) {
    return (BC_t)c->data;
}

/** Returns a pointer to the source AST indices in c.  */
unsigned * src(Code * c) {
    return (unsigned *)(c->data + pad4(c->codeSize));
}

/** Returns a pointer to the Function to which c belongs. */
Function * function(Code * c) {
    return (Function*)(c - c->header);
}


/** Returns the next Code in the current function. */
Code * next(Code * c) {
    return (Code *)(c->data + pad4(c->codeSize) + c->srcLength);
}


// This will create  Code from
Code * serialize();




/** A Function holds the RIR code for some GNU R function.
 *  Each function start with a header and a sequence of
 *  Code objects for the body and all of the promises
 *  in the code.
 *
 *  The header start with a magic constant. This is a
 *  temporary hack so that it is possible to differentiate
 *  an R int vector from a Function. Eventually, we will
 *  add a new SEXP type for this purpose.
 *
 *  The size of the function, in bytes, includes the size
 *  of all of its Code objects and is padded to a word
 *  boundary.
 *
 *  A Function may be the result of optimizing another
 *  Function, in which case the origin field stores that
 *  Function as a SEXP pointer.
 *
 *  A Function has a source AST, stored in src.
 *
 *  A Function has a number of Code objects, codeLen, stored
 *  inline in data.
 */
typedef struct Function {
    unsigned magic = 0xCAFEBABE; /// used to detect Functions

    unsigned size; /// Size, in bytes, of the function and its data

    SEXP origin; /// Same Function with fewer optimizations, NULL if this is the original

    unsigned src; /// index of the AST of the whole function

    unsigned codeLen; /// number of Code objects in the Function

    Code data[];  // Code objects stored inline

} Function;

//
bool isFunction(SEXP s) {
    if (TYPEOF(s) != INTSXP)
      return false;
    // TODO check magicVersion
}

Function * origin(Function * f) {
    // TODO
}

/** Returns the first code object associated with the function.
 */
Code * begin(Function * f) {
    return f->data;
}

/** Returns the end of the function as code object, for interation purposes.
 */
Code * end(Function * f) {
  return (Code*)((uint8_t*)f->data + f->size);
}

/** Returns an AST located at index in the AST_Pool */
SEXP source(unsigned index);




SEXP rirEval_c(Code * cure, SEXP env, unsigned numArgs);


// Cleaner runtime


// ------------------------------------------------------------------------------------------------------------------------------------------------------
// main stack


/***/


typedef struct {
    SEXP * stack;
    size_t length;
    size_t capacity;
} Stack;

Stack stack_;

INLINE bool stackEmpty() {
    return stack.length == 0;
}

INLINE SEXP pop() {
    return stack_.stack[--stack_.length];
}

INLINE SEXP top() {
    return stack_.stack[stack_.length];
}

INLINE void push(SEXP val) {
    if (stack_.length == stack_.capacity) {
        size_t newCap = stack_.capacity * 2;
        SEXP * newStack = malloc(newCap * sizeof(SEXP*));
        memcpy(newStack, stack_.stack, stack_.capacity * sizeof(SEXP*));
        free(stack_.stack);
        stack_.stack = newStack;
        stack_.capacity = newCap;
    }
    stack_.stack[stack_.length++] = val;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// int stack - I would rather have the main stack typed, but this is exact copy of the existing code, for now




typedef struct {
    int * stack;
    size_t length;
    size_t capacity;

} iStack;

iStack istack_;

INLINE bool iStackEmpty() {
    return istack_.length == 0;
}

INLINE SEXP iPop() {
    return istack_.stack[--istack_.length];
}
INLINE SEXP iTop() {
    return istack_.stack[istack_.length];
}
INLINE void iPush(int val) {
    if (istack_.length == istack.capacity) {
        size_t newCap = istack.capacity * 2;
        SEXP * newStack = malloc(newCap * sizeof(int*));
        memcpy(newStack, istack_.stack, istack_.capacity * sizeof(int*));
        free(istack_.stack);
        istack_.stack = newStack;
        istack_.capacity = newCap;
    }
    istack_.stack[stack.length++] = val;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// constant pool

SEXP cp_; // VECSXP

INLINE SEXP constant(size_t index) {
    return VECTOR_ELT(cp_);
}

INLINE size_t addConstant(SEXP value) {
  // TODO
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// src pool

SEXP src_; // VECSXP

INLINE SEXP source(size_t index) {
    return VECTOR_ELT(src_);
}

INLINE size_t addSource(SEXP value) {
  // TODO
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// bytecode basics

    //                 immediate pop   push popi pushi

    def_instr(invalid_,   0,    0,    0,   0,   0) // invalid operation
    def_instr(push_,      1,    0,    1,   0,   0) // push a constant to the stack
    def_instr(ldfun_,     1,    0,    1,   0,   0) // load function from env
    def_instr(ldvar_,     1,    0,    1,   0,   0) // load variable from env
    def_instr(call_,      ?,    0,   0,   0) // call fun
    def_instr(promise_,   1,    0,   1,   0) // create promise
    def_instr(close_,     1,    0,   0,   0) // create closure
    def_instr(ret_,       0,    1,   0,   0) // return
    def_instr(force_,     0,    1,   1,   0) // eval promise
    def_instr(pop_,       0,    1,   0,   0) // pop tos
    def_instr(pusharg_,   1,    0,   0,   0) // push argument to stack
    def_instr(asast_,     0,    1,   1,   0) // pop promise, push ast
    def_instr(stvar_,     0,    2,   1,   0) // ??
    def_instr(numargi_,   0,    0,   1,   0) // ??
    def_instr(asbool_,    0,    0,   0,   0) // convert ?? on tos to bool
    def_instr(condjmp_,   0,    0,   0,   0) // jump … if true
    def_instr(jmp_,       0,    0,   0,   0) // unconditional jump
    def_instr(lti_,       0,    0,   0,   0) // less than on ints
    def_instr(eqi_,       0,    0,   0,   0) // equality on ints
    def_instr(pushi_,     0,    0,   0,   0) // push int
    def_instr(dupi_,      0,    0,   0,   0) // dup int
    def_instr(dup_,       0,    0,   0,   0) // dup tos
    def_instr(add_,       0,    0,   0,   0) // +
    def_instr(sub_,       0,    0,   0,   0) // -
    def_instr(lt_,        0,    0,   0,   0) // <
    def_instr(isspecial_, 0,    0,   0,   0) // ??
    def_instr(isfun_,     0,    0,   0,   0) // check tos is a function
    def_instr(end_,       0,    0,   0,   0)



  // enums in C are not namespaces so I am using OP_ to disambiguate
typedef enum {
    invalid_, // -- --> --
    push_,  //
    ldfun_,
    ldvar_,
    call_,
    promise_,
    close_,
    ret_,
    force_,
    pop_,
    rdarg_,
    ldast_,
    stvar_,
    OP_NumArg_I,
    OP_ToBool,
    OP_JmpTrue,
    OP_JmpFalse,
    OP_Jmp,
    OP_Lt_I,
    OP_Eq_I,
    OP_Push_I,
    OP_Dup_I,
    OP_LoadArg_I,
    OP_Inc_I,
    OP_Dup,
    OP_Add,
    OP_Sub,
    OP_Lt,
    OP_CheckSpecial,
    OP_CheckFunction,
    OP_end
} Opcode;

// we cannot use specific sizes for enums in C
typedef uint8_t OpcodeSize;

// type  for constant & ast pool indices
typedef uint32_t PoolIndex;

// type for code offsets (from Function header)
typedef uint16_t CodeOffset;

// type of relative jump offset (all jumps are relative)
typedef int16_t JumpOffset;

INLINE Opcode readOpcode(uint8_t ** pc) {
    Opcode result = (Opcode)(**pc);
    *pc += sizeof(OpcodeSize);
    return result;
}

INLINE unsigned readPoolIndex(uint8_t ** pc) {
    unsigned result = (PoolIndex)(**pc);
    *pc += sizeof(PoolIndex);
    return result;
}

INLINE unsigned readCodeOffset(uint8_t ** pc) {
    unsigned result = (CodeOffset)(**pc);
    *pc += sizeof(CodeOffset);
    return result;
}

INLINE int readJumpOffset(uint8_t ** pc) {
    int result = (JumpOffset)(**pc);
    *pc += sizeof(JumpOffset);
    return result;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// actual execution

SEXP exec_rir(Code * code, SEXP rho, unsigned numArgs) {
    Function * f = function(code);

}



#ifdef __cplusplus
}
#endif

#endif


// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
/*/ ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================

#include <assert.h>

#include "interpreter.h"

#define bool int
#define true 1
#define false 0

#define INLINE __attribute__((always_inline)) inline

// Stuff from R
extern SEXP R_TrueValue;
extern SEXP R_FalseValue;
extern SEXP Rf_NewEnvironment(SEXP, SEXP, SEXP);
extern Rboolean R_Visible;
extern SEXP forcePromise(SEXP);

Code * begin(Function * f) {
    return f->codeObjects;
}

Code * end(Function * f) {
    return f->codeObjects + f->numObjects;
}

SEXP getAST(unsigned index) {
    return NULL;
}




// -----------------------------------------------------------------------------
// Interpreter op  & support
// -----------------------------------------------------------------------------

INLINE SEXP getCurrentCall(Code * cur, BC_t * pc, SEXP call) {
    // TODO find proper ast and return it
    return call;
}

INLINE SEXP getArg(unsigned idx, unsigned numArgs, size_t bp) {
    return Stack_at(&stack, bp - numArgs + idx);
}

INLINE SEXP loadConst(BC_t ** pc) {
    // TODO load the constant
    return NULL;
}


f
a
b
c
----- 9bp)
x
y



void eval() {
    uint8_t data[];
}




SEXP rirEval_c(Code * cur, SEXP env, unsigned numArgs) {
    SEXP call = getCodeAST(cur);
    // cannot be register because of the read functions taking address of it
    BC_t * pc = code(cur);
    size_t bp = stack.size;



    while (true) {
        switch (readBC(&pc)) {
        case to_bool: {
            SEXP t = Stack_top(&stack);
            int cond = NA_LOGICAL;
            if (Rf_length(t) > 1)
                warningcall(getCurrentCall(cur, pc, call), "the condition has length > 1 and only the first element will be used");
            Stack_pop(&stack);
            if (Rf_length(t) > 0) {
                /* inline common cases for efficiency */
                switch (TYPEOF(t)) {
                case LGLSXP:
                    cond = LOGICAL(t)[0];
                    break;
                case INTSXP:
                    cond =
                        INTEGER(t)[0]; /* relies on NA_INTEGER == NA_LOGICAL */
                    break;
                default:
                    cond = asLogical(t);
                }
            }
            if (cond == NA_LOGICAL) {
                const char* msg =
                    Rf_length(t)
                        ? (isLogical(t)
                               ? ("missing value where TRUE/FALSE needed")
                               : ("argument is not interpretable as logical"))
                        : ("argument is of length zero");
                errorcall(getCurrentCall(cur, pc, call), msg);
            }
            if (cond)
                Stack_push(&stack, R_TrueValue);
            else
                Stack_push(&stack, R_FalseValue);
            break;
        }
        case jmp: {
            pc = pc + readJumpTarget(&pc);
            break;
        }
        case jmp_true: {
            JumpTarget j = readJumpTarget(&pc);
            if (Stack_pop(&stack) == R_TrueValue)
                pc = pc + j;
            break;
        }
        case jmp_false: {
            JumpTarget j = readJumpTarget(&pc);
            if (Stack_pop(&stack) == R_FalseValue)
                pc = pc + j;
            break;
        }
        case push: {
            SEXP c = loadConst(&pc);
            Stack_push(&stack, c);
            break;
        }

        case lti: {
            int b = StackI_pop(&stacki);
            int a = StackI_pop(&stacki);
            Stack_push(&stack, a < b ? R_TrueValue : R_FalseValue);
            break;
        }

        case eqi: {
            int b = StackI_pop(&stacki);
            int a = StackI_pop(&stacki);
            Stack_push(&stack, a == b ? R_TrueValue : R_FalseValue);
            break;
        }

        case check_special: {
            SEXP sym = loadConst(&pc);
            SEXP val = findVar(sym, env);
            assert(TYPEOF(val) == SPECIALSXP);
            break;
        }

        case check_function: {
            SEXP f = Stack_top(&stack);
            assert(TYPEOF(f) == CLOSXP || TYPEOF(f) == BUILTINSXP ||
                   TYPEOF(f) == SPECIALSXP);
        }
        case getfun: {
            SEXP sym = loadConst(&pc);
            SEXP val = findVar(sym, env);
            R_Visible = TRUE;

            if (val == R_UnboundValue)
                assert(false && "Unbound var");
            else if (val == R_MissingArg)
                assert(false && "Missing argument");

            if (TYPEOF(val) == PROMSXP) {
                assert(false && "IMPLEMENT ME!!");
                /*
                BCProm* prom = getBCProm(val);

                if (prom->val(val)) {
                    val = prom->val(val);
                } else {
                    val = forcePromise(prom, val);
                }
                */
            }

            // WTF? is this just defensive programming or what?
            if (NAMED(val) == 0 && val != R_NilValue)
                SET_NAMED(val, 1);

            switch (TYPEOF(val)) {
            case INTSXP:
                // TODO check that it is proper stuff
                break;
            case CLOSXP:
                // TODO jitting all functions on demand, probably not what we want
                //val = (SEXP)jit(val);
                break;
            case SPECIALSXP:
            case BUILTINSXP: {
                /*SEXP prim = Primitives::compilePrimitive(val);
                if (prim)
                    val = prim;
                break; */
            }

            default:
                // TODO!
                assert(false);
            }
            Stack_push(&stack, val);
            break;
        }
        case getvar: {
            SEXP sym = loadConst(&pc);
            SEXP val = findVar(sym, env);
            R_Visible = TRUE;

            if (val == R_UnboundValue)
                assert(false && "Unbound var");
            else if (val == R_MissingArg)
                assert(false && "Missing argument");

            if (TYPEOF(val) == PROMSXP) {
/*                BCProm* prom = getBCProm(val);

                if (prom->val(val)) {
                    val = prom->val(val);
                } else {
                    val = forcePromise(prom, val);
                } */
            }

            // WTF? is this just defensive programming or what?
            if (NAMED(val) == 0 && val != R_NilValue)
                SET_NAMED(val, 1);
            Stack_push(&stack, val);
            break;
        }
        case mkprom: {
            assert(false);
            /*fun_idx_t idx = readFunctionIndex(&pc);
            SEXP prom = mkBCProm(cur->children[idx], env);
            assert(cur->children.size() > idx);
            Stack_push(&stack, prom); */
            break;
        }

        case BC_call: {
            assert(false);
        }

        case load_arg: {
            unsigned a = readArgIndex(&pc);
            assert(a < numArgs);
            Stack_push(&stack, getArg(a, numArgs, bp));
            break;
        }

        case numargi: {
            StackI_push(&stacki, numArgs);
            break;
        }

        case get_ast: {
            SEXP t = Stack_pop(&stack);
            /*assert(isBCProm(t));
            BCProm* p = getBCProm(t);
            stack.push(p->ast()); */
            assert(false);
            break;
        }

        case setvar: {
            SEXP val = Stack_pop(&stack);
            SEXP sym = Stack_pop(&stack);
            // TODO: complex assign
            assert(TYPEOF(sym) == SYMSXP);
            defineVar(sym, val, env);
            Stack_push(&stack, val);
            break;
        }

        case force_all: {
            assert(false);
/*            for (size_t a = 0; a < numArgs; ++a) {
                SEXP arg = getArg(a);
                if (isBCProm(arg)) {
                    BCProm* prom = getBCProm(arg);
                    SEXP val = forcePromise(prom, arg);
                    stack.push(val);
                } else {
                    stack.push(arg);
                }
            } */
            break;
        }

        case force: {
            assert(false);
/*            SEXP tos = stack.pop();
            assert(isBCProm(tos));
            BCProm* prom = getBCProm(tos);
            SEXP val = forcePromise(prom, tos);
            stack.push(val); */
            break;
        }

        case pop:
            Stak_pop(&stack);
            break;

        case ret: {
            return Stack_pop(&pc);
        }
/*
        case BC_t::pushi: {
            stacki.push(BC::readImmediate<int>(&pc));
            break;
        }

        case BC_t::dup:
            stack.push(stack.top());
            break;

        case BC_t::dupi:
            stacki.push(stacki.top());
            break;

        case BC_t::load_argi: {
            int pos = stacki.pop();
            stack.push(getArg(pos));
            break;
        }

        case BC_t::inci: {
            stacki.top()++;
            break;
        }

        case BC_t::mkclosure: {
            SEXP body = stack.pop();
            SEXP arglist = stack.pop();

            SEXP bcls = jit(body, arglist, env);
            stack.push(bcls);
            break;
        }

        case BC_t::add: {
            SEXP rhs = stack.pop();
            SEXP lhs = stack.pop();
            if (Rinternals::typeof(lhs) == REALSXP &&
                Rinternals::typeof(lhs) == REALSXP && Rf_length(lhs) == 1 &&
                Rf_length(rhs) == 1) {
                SEXP res = Rf_allocVector(REALSXP, 1);
                SET_NAMED(res, 1);
                REAL(res)[0] = REAL(lhs)[0] + REAL(rhs)[0];
                stack.push(res);
            } else {
                static SEXP op = getPrimitive("+");
                static CCODE primfun = getPrimfun("+");
                SEXP res = callPrimitive(primfun, getCurrentCall(), op, env,
                                         {lhs, rhs});
                stack.push(res);
            }
            break;
        }

        case BC_t::sub: {
            SEXP rhs = stack.pop();
            SEXP lhs = stack.pop();
            if (Rinternals::typeof(lhs) == REALSXP &&
                Rinternals::typeof(lhs) == REALSXP && Rf_length(lhs) == 1 &&
                Rf_length(rhs) == 1) {
                SEXP res = Rf_allocVector(REALSXP, 1);
                SET_NAMED(res, 1);
                REAL(res)[0] = REAL(lhs)[0] - REAL(rhs)[0];
                stack.push(res);
            } else {
                static SEXP op = getPrimitive("-");
                static CCODE primfun = getPrimfun("-");
                SEXP res = callPrimitive(primfun, getCurrentCall(), op, env,
                                      git    {lhs, rhs});
                stack.push(res);
            }
            break;
        }

        case BC_t::lt: {
            SEXP rhs = stack.pop();
            SEXP lhs = stack.pop();
            if (Rinternals::typeof(lhs) == REALSXP &&
                Rinternals::typeof(lhs) == REALSXP && Rf_length(lhs) == 1 &&
                Rf_length(rhs) == 1) {
                stack.push(REAL(lhs)[0] < REAL(rhs)[0] ? R_TrueValue
                                                       : R_FalseValue);
            } else {
                static SEXP op = getPrimitive("<");
                static CCODE primfun = getPrimfun("<");
                SEXP res = callPrimitive(primfun, getCurrentCall(), op, env,
                                         {lhs, rhs});
                stack.push(res);
            }
            break;
        } */

        case num_of:
        case invalid:
            assert(false);
        default:
            assert(false && "Not implemented (yet)");
        }








    }
    return NULL;
}

/*
types for first class macros
ryan,
dave forman: ?verman
paul stens...: Typing for macros, or alpha relation for macros
scala and rust: process ASTs , are macros-ish


*/
