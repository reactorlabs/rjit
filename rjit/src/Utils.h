#ifndef AGRESSIVE_INLINING_H
#define AGRESSIVE_INLINING_H

#include "llvm.h"
#include <Rinternals.h>
#include <algorithm>
#include <llvm/IR/InstIterator.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <string>
#include <sstream>

#include <chrono>
#include <iostream>
#include <fstream>

using namespace std;
using namespace chrono;

namespace osr {

class Utils {
  public:
    /*For measurements*/
    static high_resolution_clock::time_point start;
    static high_resolution_clock::time_point end;

    /**
     * @brief      Creates a deep clone of the function f
     *
     * @param      f     llvm::Function*
     *
     * @return     A new llvm::Function* cloned from f
     */
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