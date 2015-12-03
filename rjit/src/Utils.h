#ifndef AGRESSIVE_INLINING_H
#define AGRESSIVE_INLINING_H

#include "llvm.h"
#include <Rinternals.h>
#include <algorithm>
#include <llvm/IR/InstIterator.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <string>
#include <sstream>

namespace osr {
typedef std::vector<llvm::BasicBlock*> BB_Vector;

class Utils {
  public:
    static std::string getIcStubName(unsigned int i) {
        std::ostringstream oss;
        oss << "icStub_" << (i);
        return oss.str();
    }

    /**
     * @brief      { function_description }
     *
     * @param      f     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    static BB_Vector* getBBs(llvm::Function* f) {
        BB_Vector* res = new BB_Vector();
        for (auto it = f->begin(); it != f->end(); ++it) {
            res->push_back(it);
        }
        return res;
    }

    static llvm::Function* cloneFunction(llvm::Function* f) {
        llvm::ValueToValueMapTy VMap;
        llvm::Function* duplicateFunction = llvm::CloneFunction(f, VMap, false);
        f->getParent()->getFunctionList().push_back(duplicateFunction);
        return duplicateFunction;
    }

  private:
};
} // namespace osr

#endif // AGRESSIVE_INLINING_H