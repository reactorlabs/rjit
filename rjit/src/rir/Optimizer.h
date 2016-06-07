#ifndef RIR_OPTIMIZER_H
#define RIR_OPTIMIZER_H

#include "BC.h"
#include "OpenFunction.h"
#include "../Symbols.h"
#include "CodeStream.h"
#include <cassert>

namespace rjit {
namespace rir {

namespace {

// ============================================================================
// ==== Thats just a toy example of how to compile away a call to some specials
//
// void optimize_(OpenFunction* fun, fun_idx_t idx);
// void optimize(CodeStream& cs, OpenFunction* fun, CodeStream* cur);
//
// BC_t* doInlineIf(CodeStream& cs, OpenFunction* fun, CodeStream* cur, BC_t*
// pc,
//                  BC_t* end) {
//
//     cs << BC::check_primitive(symbol::If);
//
//     BC bc = BC::advance(&pc);
//
//     assert(bc.bc == BC_t::call);
//     fun_idx_t* args = bc.immediateCallArgs();
//     num_args_t nargs = bc.immediateCallNargs();
//
//     Label trueBranch = cs.mkLabel();
//     Label nextBranch = cs.mkLabel();
//
//     optimize(cs, fun, fun->code[args[0]]);
//     cs << BC::to_bool() << BC::jmp_true(trueBranch);
//
//     if (nargs < 3) {
//         cs << BC::push(R_NilValue);
//     } else {
//         optimize(cs, fun, fun->code[args[2]]);
//     }
//     cs << BC::jmp(nextBranch);
//
//     cs << trueBranch;
//     optimize(cs, fun, fun->code[args[1]]);
//
//     cs << nextBranch;
//
//     return pc;
// }
//
// BC_t* doInlinePar(CodeStream& cs, OpenFunction* fun, CodeStream* cur, BC_t*
// pc,
//                   BC_t* end) {
//
//     cs << BC::check_primitive(symbol::Parenthesis);
//
//     BC bc = BC::advance(&pc);
//
//     assert(bc.bc == BC_t::call);
//     fun_idx_t* args = bc.immediateCallArgs();
//     num_args_t nargs = bc.immediateCallNargs();
//
//     assert(nargs == 1);
//
//     optimize(cs, fun, fun->code[args[0]]);
//
//     return pc;
// }
//
// BC_t* doInlineBlock(CodeStream& cs, OpenFunction* fun, CodeStream* cur, BC_t*
// pc,
//                     BC_t* end) {
//
//     cs << BC::check_primitive(symbol::Block);
//
//     BC bc = BC::advance(&pc);
//
//     assert(bc.bc == BC_t::call);
//     fun_idx_t* args = bc.immediateCallArgs();
//     num_args_t nargs = bc.immediateCallNargs();
//
//     for (int i = 0; i < nargs; ++i) {
//         optimize(cs, fun, fun->code[args[i]]);
//     }
//
//     return pc;
// }
//
// BC_t* doInlineSub(CodeStream& cs, OpenFunction* fun, CodeStream* cur, BC_t*
// pc,
//                   BC_t* end) {
//
//     cs << BC::check_primitive(symbol::Sub);
//
//     BC bc = BC::advance(&pc);
//
//     assert(bc.bc == BC_t::call);
//     fun_idx_t* args = bc.immediateCallArgs();
//     num_args_t nargs = bc.immediateCallNargs();
//
//     if (nargs == 1) {
//         cs << BC::push(0.0);
//         optimize(cs, fun, fun->code[args[0]]);
//         cs << BC::sub();
//         return pc;
//     }
//     assert(nargs == 2);
//     optimize(cs, fun, fun->code[args[0]]);
//     optimize(cs, fun, fun->code[args[1]]);
//
//     cs << BC::sub();
//
//     return pc;
// }
//
// BC_t* doInlineLt(CodeStream& cs, OpenFunction* fun, CodeStream* cur, BC_t*
// pc,
//                  BC_t* end) {
//
//     cs << BC::check_primitive(symbol::Lt);
//
//     BC bc = BC::advance(&pc);
//
//     assert(bc.bc == BC_t::call);
//     fun_idx_t* args = bc.immediateCallArgs();
//     num_args_t nargs = bc.immediateCallNargs();
//     assert(nargs == 2);
//
//     optimize(cs, fun, fun->code[args[0]]);
//     optimize(cs, fun, fun->code[args[1]]);
//
//     cs << BC::lt();
//
//     return pc;
// }
//
// BC_t* doInlineAdd(CodeStream& cs, OpenFunction* fun, CodeStream* cur, BC_t*
// pc,
//                   BC_t* end) {
//
//     cs << BC::check_primitive(symbol::Add);
//
//     BC bc = BC::advance(&pc);
//
//     assert(bc.bc == BC_t::call);
//     fun_idx_t* args = bc.immediateCallArgs();
//     num_args_t nargs = bc.immediateCallNargs();
//
//     assert(nargs == 2);
//     optimize(cs, fun, fun->code[args[0]]);
//     optimize(cs, fun, fun->code[args[1]]);
//
//     cs << BC::add();
//
//     return pc;
// }
//
// void optimize(CodeStream& cs, OpenFunction* fun, CodeStream* cur) {
//     BC_t* pc = cur->bc;
//     BC_t* end = (BC_t*)(uintptr_t)pc + cur->size;
//
//     while (pc != end) {
//         BC bc = BC::advance(&pc);
//
//         switch (bc.bc) {
//         case BC_t::getfun:
//             if (bc.immediateConst() == symbol::If) {
//                 pc = doInlineIf(cs, fun, cur, pc, end);
//                 continue;
//             }
//             if (bc.immediateConst() == symbol::Block) {
//                 pc = doInlineBlock(cs, fun, cur, pc, end);
//                 continue;
//             }
//             if (bc.immediateConst() == symbol::Lt) {
//                 pc = doInlineLt(cs, fun, cur, pc, end);
//                 continue;
//             }
//             if (bc.immediateConst() == symbol::Add) {
//                 pc = doInlineAdd(cs, fun, cur, pc, end);
//                 continue;
//             }
//             if (bc.immediateConst() == symbol::Sub) {
//                 pc = doInlineSub(cs, fun, cur, pc, end);
//                 continue;
//             }
//             if (bc.immediateConst() == symbol::Parenthesis) {
//                 pc = doInlinePar(cs, fun, cur, pc, end);
//                 continue;
//             }
//
//             break;
//
//         case BC_t::ret:
//             continue;
//
//         case BC_t::call: {
//             fun_idx_t* args = bc.immediateCallArgs();
//             num_args_t nargs = bc.immediateCallNargs();
//             for (int i = 0; i < nargs; ++i) {
//                 optimize_(fun, args[i]);
//             }
//             break;
//         }
//
//         default:
//             break;
//         }
//         cs << bc;
//     }
// }
//
// void optimize_(OpenFunction* fun, fun_idx_t idx) {
//     CodeStream* c = fun->code[idx];
//
//     CodeStreamStream opt(c->ast, idx);
//     optimize(opt, fun, c);
//     opt << BC::ret();
//
//     CodeStream* optCode = opt.toCode();
//     delete fun->code[idx];
//     fun->code[idx] = optCode;
// }
}

class Optimizer {
  public:
    static void optimize(OpenFunction* fun) {
        // optimize_(fun, 0);
    }
};

} // rir
} // rjit

#endif
