#ifndef AGRESSIVE_INLINING_H
#define AGRESSIVE_INLINING_H

#include "llvm.h"
#include <Rinternals.h>
#include <algorithm>
#include <llvm/IR/InstIterator.h>

#include <string>
#include <sstream>

namespace osr {

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

  private:
    Utils() {}
};

class inst_iterator_wrap {
  public:
    inst_iterator_wrap(llvm::inst_iterator it) : it(it), pos(0) {}

    inline inst_iterator_wrap& operator++() {
        ++it;
        ++pos;
        return *this;
    }

    inline llvm::Instruction* operator->() { return &(*it); }

    inline inst_iterator_wrap& operator+(unsigned int n) {
        for (unsigned int i = 0; i < n; ++i, ++it) {
        };
        return *this;
    }

    inline bool operator!=(inst_iterator_wrap e) {
        return (e.getIt() != this->getIt());
    }

    inline llvm::inst_iterator getIt() { return it; }

    inline inst_iterator_wrap& operator-(unsigned int val) {
        for (unsigned int i = 0; i < val; --it, ++i, --pos) {
        }
        return *this;
    }

    inline llvm::Instruction* get() { return &(*it); }

    inline unsigned int getPos() { return pos; }
    // inline inst_iterator
  private:
    llvm::inst_iterator it;
    unsigned int pos;
};
} // namespace osr

#endif // AGRESSIVE_INLINING_H