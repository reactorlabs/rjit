#ifndef FunctionCloner_H
#define FunctionCloner_H

#include "llvm.h"
#include "FunctionCall.h"

namespace osr {

typedef std::vector<llvm::Instruction*> Inst_Vector;
typedef std::vector<llvm::BasicBlock*> BB_Vector;
typedef std::vector<llvm::ReturnInst*> RInst_Vector;
/**
 * @brief      Enables to make a copy of a function's body in order to inline
 * 						 it directly inside another
 * function.
 * TODO: 			Implement a function that takes a FunctionCall
 * to
 * f
 * and
 * inlines it.
 */
class FunctionCloner {
  public:
    FunctionCloner(llvm::Function* f) : f(f) {}

    llvm::Function* cloneF();
    llvm::Function* insertValues(FunctionCall* fc);

    // TODO move that in another class
    static BB_Vector* getBBs(llvm::Function* f) {
        BB_Vector* res = new BB_Vector();
        for (auto it = f->begin(); it != f->end(); ++it) {
            res->push_back(it);
        }
        return res;
    }

    RInst_Vector* getReturnInsts();

  private:
    llvm::Function* f;
};

} // namespace osr
#endif
