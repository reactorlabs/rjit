#ifndef RIR_OPTIMIZER_H
#define RIR_OPTIMIZER_H

#include "BC.h"
#include "Function.h"
#include "../Symbols.h"
#include "CodeStream.h"
#include <cassert>

#include "CodeEditor.h"

namespace rjit {
namespace rir {

namespace {

// ============================================================================
// ==== Thats just a toy example of how to compile away a call to some specials
//
void optimize_(Function* fun, fun_idx_t idx);
void optimize(CodeEditor& e, Function* fun);

void inlProm(CodeEditor::Cursor cur, Function* fun, fun_idx_t idx) {
    CodeEditor ce(fun->code[idx], idx);
    ce.normalizeReturn();
    cur << ce;
}

void doInlineBlock(CodeEditor::Cursor& cur, Function* fun) {
    assert((*cur).bc == BC_t::getfun);
    cur.remove();
    BC bc = *cur;
    assert(bc.bc == BC_t::call);
    cur.remove();

    cur << BC::check_primitive(symbol::Parenthesis);

    fun_idx_t* args = bc.immediateCallArgs();
    num_args_t nargs = bc.immediateCallNargs();

    for (int i = 0; i < nargs; ++i) {
        inlProm(cur, fun, args[i]);
        if (i != nargs - 1)
            cur << BC::pop();
    }
}

void doInlineIf(CodeEditor::Cursor& cur, Function* fun) {
    assert((*cur).bc == BC_t::getfun);
    cur.remove();
    BC bc = *cur;
    assert(bc.bc == BC_t::call);
    cur.remove();

    cur << BC::check_primitive(symbol::If);

    fun_idx_t* args = bc.immediateCallArgs();
    num_args_t nargs = bc.immediateCallNargs();

    Label trueBranch = cur.mkLabel();
    Label nextBranch = cur.mkLabel();

    inlProm(cur, fun, args[0]);
    cur << BC::to_bool() << BC::jmp_true(trueBranch);

    if (nargs < 3) {
        cur << BC::push(R_NilValue);
    } else {
        inlProm(cur, fun, args[2]);
    }
    cur << BC::jmp(nextBranch);

    cur << BC::label(trueBranch);
    inlProm(cur, fun, args[1]);

    cur << BC::label(nextBranch);
}

void doInlinePar(CodeEditor::Cursor& cur, Function* fun) {
    assert((*cur).bc == BC_t::getfun);
    cur.remove();
    BC bc = *cur;
    assert(bc.bc == BC_t::call);
    cur.remove();

    cur << BC::check_primitive(symbol::Parenthesis);

    fun_idx_t* args = bc.immediateCallArgs();
    num_args_t nargs = bc.immediateCallNargs();

    assert(nargs == 1);

    inlProm(cur, fun, args[0]);
}

void doInlineAdd(CodeEditor::Cursor& cur, Function* fun) {
    assert((*cur).bc == BC_t::getfun);
    cur.remove();
    BC bc = *cur;
    assert(bc.bc == BC_t::call);
    cur.remove();

    cur << BC::check_primitive(symbol::Add);

    fun_idx_t* args = bc.immediateCallArgs();
    num_args_t nargs = bc.immediateCallNargs();

    assert(nargs == 2);
    inlProm(cur, fun, args[0]);
    inlProm(cur, fun, args[1]);
    cur << BC::add();
}

void doInlineSub(CodeEditor::Cursor& cur, Function* fun) {
    assert((*cur).bc == BC_t::getfun);
    cur.remove();
    BC bc = *cur;
    assert(bc.bc == BC_t::call);
    cur.remove();

    cur << BC::check_primitive(symbol::Sub);

    fun_idx_t* args = bc.immediateCallArgs();
    num_args_t nargs = bc.immediateCallNargs();

    assert(nargs == 2);
    inlProm(cur, fun, args[0]);
    inlProm(cur, fun, args[1]);
    cur << BC::sub();
}

void doInlineLt(CodeEditor::Cursor& cur, Function* fun) {
    assert((*cur).bc == BC_t::getfun);
    cur.remove();
    BC bc = *cur;
    assert(bc.bc == BC_t::call);
    cur.remove();

    cur << BC::check_primitive(symbol::Lt);

    fun_idx_t* args = bc.immediateCallArgs();
    num_args_t nargs = bc.immediateCallNargs();

    assert(nargs == 2);
    inlProm(cur, fun, args[0]);
    inlProm(cur, fun, args[1]);
    cur << BC::lt();
}

void optimize(CodeEditor& e, Function* fun) {
    for (auto cur = e.getCursor(); !cur.atEnd(); ++cur) {
        BC bc = *cur;

        switch (bc.bc) {
        case BC_t::getfun:
            if (bc.immediateConst() == symbol::If) {
                doInlineIf(cur, fun);
                continue;
            }
            if (bc.immediateConst() == symbol::Block) {
                doInlineBlock(cur, fun);
                continue;
            }
            if (bc.immediateConst() == symbol::Lt) {
                doInlineLt(cur, fun);
                continue;
            }
            if (bc.immediateConst() == symbol::Add) {
                doInlineAdd(cur, fun);
                continue;
            }
            if (bc.immediateConst() == symbol::Sub) {
                doInlineSub(cur, fun);
                continue;
            }
            if (bc.immediateConst() == symbol::Parenthesis) {
                doInlinePar(cur, fun);
                continue;
            }

            break;

        case BC_t::call: {
            fun_idx_t* args = bc.immediateCallArgs();
            num_args_t nargs = bc.immediateCallNargs();
            for (int i = 0; i < nargs; ++i) {
                optimize_(fun, args[i]);
            }
            break;
        }

        default:
            break;
        }
    }
}

void optimize_(Function* fun, fun_idx_t idx) {
    Code* c = fun->code[idx];
    c->print();
    CodeEditor edit(c, idx);
    edit.print();
    optimize(edit, fun);
    edit.print();
    Code* opt = edit.toCode();
    opt->print();
    fun->code[idx] = opt;
    delete c;
}
}

class Optimizer {
  public:
    static void optimize(Function* fun) {
        for (int i = 0; i < 5; ++i)
            optimize_(fun, 0);
    }
};

} // rir
} // rjit

#endif
