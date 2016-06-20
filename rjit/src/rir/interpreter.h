#ifndef RIR_INTERPRETER_H
#define RIR_INTERPRETER_H

/** TODO BC_t cannot be enum class perhaps, have to be in proper header and all that.
 */



#ifdef __cplusplus

#include "BC_inc.h"
extern "C" {

#else

typedef char BC_t;

#endif


typedef struct CodeObject {
    /** offset to the beginning of the FunctioObject from this */
    unsigned offset;
    /** Code size, in bytes */
    unsigned size;
    /** Number of instructions, and by extension size of the debug section in ints */
    unsigned numInsns;
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



#ifdef __cplusplus
}
#endif

#endif
