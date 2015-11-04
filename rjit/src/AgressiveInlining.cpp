#include <llvm/IR/Module.h>
#include "Compiler.h"

#include "api.h"

#include "RIntlns.h"

#include "ir/ir.h"
#include "ir/Builder.h"
#include "ir/intrinsics.h"
#include "ir/Handler.h"

#include "AgressiveInlining.h"

#include <llvm/IR/BasicBlock.h>
#include "llvm/Analysis/TargetTransformInfo.h"

using namespace rjit;
namespace osr {

/**
 * @brief      printing all the blocks one by one
 *
 * @param      f     llvm::Function*
 */
void getAllBlocksForFunction(llvm::Function* f) {
    assert(f != NULL and "TODO aghosn");
    llvm::Function::iterator it = f->begin();
    for (it = f->begin(); it != f->end(); it++) {
        (*it).dump();
    }
}

/**
 * @brief      playing around. Next step is to get all basic blocks printed one
 *by one
 *
 * @param[in]  expression  the function we need to look at
 *
 * @return     R_NilValue2
 */
REXPORT SEXP jitGetIR(SEXP expression) {
    assert(TYPEOF(expression) == NATIVESXP and
           "LLVM code can only be extracted from a NATIVESXP argument");
    llvm::Function* f = reinterpret_cast<llvm::Function*>(TAG(expression));
    f->dump();
    warning("Printing the basic blocks one by one");
    getAllBlocksForFunction(f);
    return R_NilValue;
}
}