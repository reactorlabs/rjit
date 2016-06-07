#ifndef RIR_OPEN_FUNCTION_H
#define RIR_OPEN_FUNCTION_H

#include "../RDefs.h"
#include "../Precious.h"
#include "CodeStream.h"
#include "Function.h"

namespace rjit {
namespace rir {

// Function is an array of code objects. Usually contained in a BCClosure
class OpenFunction {
  public:
    std::vector<CodeStream*> code;

    OpenFunction() {}

    CodeStream& addCode(SEXP ast) {
        assert(code.size() < MAX_FUN_IDX);
        CodeStream* s = new CodeStream(ast, code.size());
        code.push_back(s);
        return *s;
    }

    Function* finalize() {
        Function* fun = new Function();
        for (auto cs : code) {
            fun->code.push_back(cs->toCode());
            delete cs;
        }
        code.clear();
        return fun;
    }
};
}
}

#endif
