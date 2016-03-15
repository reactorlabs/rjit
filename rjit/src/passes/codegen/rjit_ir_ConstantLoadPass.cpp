
/* This file has been automatically generated. Do not hand-edit. */
#include <llvm.h>
#include "RIntlns.h"
#include "ir/Ir.h"
#include "ir/primitive_calls.h"
#include "ir/Optimization/ConstantLoad.h"

#pragma GCC diagnostic ignored "-Wswitch"
bool rjit::ir::ConstantLoadPass::dispatch(llvm::BasicBlock::iterator& it0) {

    llvm::BasicBlock::iterator it1 = it0;

    Pattern* p0 = Pattern::get(it0);
    if (p0 != nullptr)
        switch (p0->kind) {

        case Pattern::Kind::UserLiteral: {
            p0->advance(it1);

            u(static_cast<rjit::ir::UserLiteral*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::Constant: {
            p0->advance(it1);

            c(static_cast<rjit::ir::Constant*>(p0));
            it0 = it1;
            return true;

            break;
        }
        }
    return rjit::ir::Pass::dispatch(it0);
}
