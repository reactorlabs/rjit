#include "ir/Pass.h"
#include "llvm.h"
#include "RIntlns.h"
#include "ir/Intrinsics.h"

#pragma GCC diagnostic ignored "-Wswitch"
bool rjit::ir::DummyPass::dispatch(llvm::BasicBlock::iterator& i) {
    bool success = true;

    llvm::BasicBlock::iterator ii = i;
    if (!rjit::ir::Pattern::isInstruction(ii))
        return false;
    Pattern* pattern = rjit::ir::Pattern::match(ii);
    switch (pattern->getKind()) {
    case Pattern::PatternKind::GenericGetVar: {
        gv(static_cast<rjit::ir::GenericGetVar*>(Pattern::getIR(i)));
        i = ii;
        return true;
    }
    }
    if (rjit::ir::Pass::dispatch(i))
        goto DONE;

    success = false;
DONE:
    i = ii;
    return success;
}