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
typedef std::vector<llvm::BasicBlock*> BB_Vector;

class Utils {
  public:
    static std::string getIcStubName(unsigned int i) {
        std::ostringstream oss;
        oss << "icStub_" << (i);
        return oss.str();
    }

    /*For measurements*/
    static high_resolution_clock::time_point start;
    static high_resolution_clock::time_point end;

    /**
     * @brief      Put the f's basic blocks into a vector
     *
     * @param      f     llvm::Function*
     *
     * @return     an std::vector of f's basic blocks
     */
    /*static BB_Vector* getBBs(llvm::Function* f) {
        BB_Vector* res = new BB_Vector();
        for (auto it = f->begin(); it != f->end(); ++it) {
            res->push_back(it);
        }
        return res;
    }*/

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