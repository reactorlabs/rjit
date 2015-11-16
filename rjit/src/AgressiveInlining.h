#ifndef AGRESSIVE_INLINING_H
#define AGRESSIVE_INLINING_H

#include "llvm.h"
#include <Rinternals.h>
#include <algorithm>

#include <string>
#include <sstream>

using namespace rjit;

namespace osr {

class InliningEnv {

  public:
    typedef std::map<std::string, llvm::Function*> FunctionStore;
    FunctionStore store;

    static InliningEnv& getInstance() {
        static InliningEnv instance;
        return instance;
    }

    bool storeContains(std::string key) {
        FunctionStore::iterator it = store.find(key);
        return (it != store.end());
    }

  private:
    InliningEnv() {}
};

// extern SEXP deparse1(SEXP call, Rboolean abbrev, int opts);

class DeparseUtils {
  public:
    static DeparseUtils& getInstance() {
        static DeparseUtils instance;
        return instance;
    }

    static std::string getName(SEXP expr) {
        const void* address = static_cast<const void*>(expr);
        std::stringstream ss;
        ss << address;
        std::string name = ss.str();
        return name;
    }

    /*SEXP deparse4print(SEXP call, int cutoff) {
            int opts = 1 | 2 | 4 | 64;
            return deparse1(call, FALSE, opts);
    }

    std::string getName(SEXP call) {

            SEXP deparsed_sexp = deparse4print(call, 80);
            for (int i = 0; i < LENGTH(deparsed_sexp); i++) {
                    printf("deparsed: %s\n", CHAR(STRING_ELT(deparsed_sexp,
    i)));
            }

            return "blob";
    }*/

  private:
    DeparseUtils() {}
};

} // namespace osr

#endif // AGRESSIVE_INLINING_H