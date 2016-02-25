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

TypeInfo Instrumentation::getTypefeedback(SEXP sym) {
    if (!__cp__) return TypeInfo();

    SEXP store = VECTOR_ELT(__cp__, 1);
    SEXP names = VECTOR_ELT(__cp__, 2);
    assert(TYPEOF(store) == INTSXP);
    assert(TYPEOF(names) == VECSXP);
    assert(XLENGTH(store) == XLENGTH(names));
    for (unsigned i = 0; i < XLENGTH(store); ++i) {
        if (VECTOR_ELT(names, i) == sym) {
            return TypeInfo(INTEGER(store)[i]);
        }
    }
    return TypeInfo();
}

SEXP Instrumentation::__cp__ = nullptr;

}

extern "C" void recordType(SEXP value, SEXP store, int idx) {
    rjit::TypeFeedback record(store);
    record.record(value, idx);
}
