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

typedef struct CW {
    std::vector<SEXP> cp;
    llvm::Function* f;
    CW(std::vector<SEXP> CP, llvm::Function* F) {
        cp = CP;
        f = F;
    }
} ContWrapper;

class Utils {
  public:
    static Utils& getInstance() {
        static Utils instance;
        return instance;
    }

    static std::string getIcStubName(unsigned int i) {
        std::ostringstream oss;
        oss << "icStub_" << (i);
        return oss.str();
    }

    inline static llvm::inst_iterator advance(llvm::inst_iterator I,
                                              unsigned int pos) {
        for (unsigned int i = 0; i < pos; ++i, ++(I)) {
        }
        return I;
    }

    void activate() { this->active = true; }

    void deactivate() {
        this->active = false;
        contexts.clear();
    }

    bool isActive() { return this->active; }

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

    std::vector<ContWrapper*> contexts;

  private:
    bool active;
    Utils() : active(false) {}
};
} // namespace osr

#endif // AGRESSIVE_INLINING_H