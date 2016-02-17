#include "Instrumentation.h"

#include "RIntlns.h"
#include "TypeInfo.h"

#include <iostream>
#include <cassert>

namespace rjit {

TypeFeedback::TypeFeedback(SEXP store) : store(store) {
    assert(TYPEOF(store) == INTSXP);
}

void TypeFeedback::record(SEXP value, int idx) {
    assert(idx < XLENGTH(store));

    TypeInfo old_info(INTEGER(store)[idx]);
    TypeInfo info(INTEGER(store)[idx]);

    info.mergeAll(value);

    if (old_info != info) {
        INTEGER(store)[idx] = info;
    }
}

int Instrumentation::getInvocationCount(SEXP closure) {
    assert(TYPEOF(closure) == CLOSXP);
    SEXP body = BODY(closure);
    SEXP consts = CDR(body);
    SEXP invocationCount = VECTOR_ELT(consts, 3);
    return INTEGER(invocationCount)[0];
}

void Instrumentation::clearInvocationCount(SEXP closure) {
    assert(TYPEOF(closure) == CLOSXP);
    SEXP body = BODY(closure);
    SEXP consts = CDR(body);
    SEXP invocationCount = VECTOR_ELT(consts, 3);
    INTEGER(invocationCount)[0] = 0;
}
}

extern "C" void recordType(SEXP value, SEXP store, int idx) {
    rjit::TypeFeedback record(store);
    record.record(value, idx);
}
