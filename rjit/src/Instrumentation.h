#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

#include "RDefs.h"
#include "TypeInfo.h"

namespace rjit {

class TypeFeedback {
  public:
    TypeFeedback(SEXP store);
    void record(SEXP value, int idx);

  private:
    SEXP store;
};

class Instrumentation {
  public:
    static int getInvocationCount(SEXP closure);
    static void clearInvocationCount(SEXP closure);
    static TypeInfo getTypefeedback(SEXP sym);
    static bool hasTypeInfo() {
        return __cp__;
    }
    static SEXP __cp__;
};
}

extern "C" void recordType(SEXP value, SEXP store, int idx);

#endif
