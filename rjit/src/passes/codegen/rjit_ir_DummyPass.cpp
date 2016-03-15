
/* This file has been automatically generated. Do not hand-edit. */
#include <llvm.h>
#include "RIntlns.h"
#include "ir/Ir.h"
#include "ir/primitive_calls.h"
#include "ir/Pass.h"

#pragma GCC diagnostic ignored "-Wswitch"
bool rjit::ir::DummyPass::dispatch(llvm::BasicBlock::iterator& it0) {

    llvm::BasicBlock::iterator it1 = it0;

    Pattern* p0 = Pattern::get(it0);
    if (p0 != nullptr)
        switch (p0->kind) {

        case Pattern::Kind::GenericGetVar: {
            p0->advance(it1);

            gv(static_cast<rjit::ir::GenericGetVar*>(p0));
            it0 = it1;
            return true;

            break;
        }
        }
    return rjit::ir::Pass::dispatch(it0);
}
