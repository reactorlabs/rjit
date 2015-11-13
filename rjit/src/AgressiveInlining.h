#ifndef AGRESSIVE_INLINING_H
#define AGRESSIVE_INLINING_H

#include "llvm.h"
#include <Rinternals.h>
#include <algorithm>

using namespace rjit;
namespace osr {
class BookKeeping {
    typedef std::map<std::string, llvm::Value*> ContextMap;
    typedef std::vector<SEXP> SEXPMap;

  public:
    static BookKeeping singleton;
    ContextMap functions;
    SEXPMap sfunctions;

    bool empty() { return singleton.functions.empty(); }
    void add(std::string name, llvm::Value* val) {
        singleton.functions[name] = val;
    }

    bool contains(std::string name) {
        ContextMap::iterator it = functions.find(name);
        return (it != functions.end());
    }

    bool contains2(SEXP exp) {
        return (std::find(sfunctions.begin(), sfunctions.end(), exp) !=
                sfunctions.end());
    }
};
}

// SAVING
/*llvm::Function* f = reinterpret_cast<llvm::Function*>(TAG(result));
        ValueToValueMapTy VMap;
        llvm::Function* fprime = llvm::CloneFunction(f,VMap, false);
        osr::BookKeeping::singleton.add(fprime->getName().str(), fprime);
        f->dump();*/

#endif // AGRESSIVE_INLINING_H