#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

#include "RDefs.h"

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
};
}

extern "C" void recordType(SEXP value, SEXP store, int idx);

#endif
