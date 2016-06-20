#ifndef RIR_INTERPRETER_H
#define RIR_INTERPRETER_H

#include <R.h>
#include <Rinternals.h>

#include <stdint.h>

/** TODO BC_t cannot be enum class perhaps, have to be in proper header and all that.

    const pool, interpreter stack and debugInfo must be recreated in C

 */

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
    call,

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


#ifdef __cplusplus
extern "C" {
#endif

typedef struct CodeObject {
    /** offset to the beginning of the FunctioObject from this */
    unsigned offset;
    /** Code size, in bytes */
    unsigned size;
    /** Number of instructions, and by extension size of the debug section in ints */
    unsigned numInsns;
    /** Index to the debug info to retrive the complete AST of the function */
    unsigned ast;
} CodeObject;

/** Container for function code representation.

  Each function consists of a header, followed by code objects. Function objects provide support for iteration
 */
typedef struct FunctionObject {
    /** Number of code objects stored in the function. */
    unsigned numObjects;
    /** Total length of the function object.

      This information is also in the actual SEXP, but having it here feels nice. The size reported is in bytes, including the header.
     */
    unsigned size;
    /** Less optimized version. NULL if the current code is unoptimized. */
    struct FunctionObject * parent;
} FunctionObject;

/** Returns the first code object associated with the function.
 */
CodeObject * begin(FunctionObject * f);

/** Returns the end of the function as code object, for interation purposes.
 */
CodeObject * end(FunctionObject * f);

/** Returns the function object parent of the given code.
 */
FunctionObject * function(CodeObject * c);

/** Returns pointer to the code stream (immediately after header).
 */
BC_t * code(CodeObject * c);

/** Returns pointer to the debug information (after header and code, aligned to ints).
 */
unsigned * debugInfo(CodeObject *c);

/** Returns the next code object, for iteration purposes.
 */
CodeObject * next(CodeObject * c);


/** Takes size in bytes and returns the number of integers required to store the bytes.
 */
unsigned sizeInInts(unsigned sizeInBytes);





SEXP getCodeAST(CodeObject * c);

SEXP getAST(unsigned index);






SEXP rirEval_c(CodeObject * cure, SEXP env, unsigned numArgs);



#ifdef __cplusplus
}
#endif

#endif
