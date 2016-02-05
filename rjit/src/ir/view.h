#ifndef IR_VIEW_H
#define IR_VIEW_H

#include "llvm.h"
#include "ir.h"


namespace rjit {
namespace ir {


/** Class for llvm bitcode & patterns manipulation.

  TODO it might be the name is stupid.
 */
class View {
public:

    View(llvm::Function * f):
        f(f) {
    }

    LLVMContext & getContext() const {
        return f->getContext();
    }

    llvm::Module * getModule() {
        return f->getParent();
    }

    /** Detaches given pattern from its instructions and deletes it. Does not delete the instructions themselves.
     */
    void detach(Pattern * p) {

    }



    /** The function associated with the view.
     */
    llvm::Function * const f;






};




} // namespace ir
} // namespace rjit



#endif // IR_VIEW_H
