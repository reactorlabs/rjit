#include "ir/Pass.h"
#include "llvm.h"
#include "RIntlns.h"
#include "ir/Intrinsics.h"

#pragma GCC diagnostic ignored "-Wswitch"
bool rjit::ir::MyPass::dispatch(llvm::BasicBlock::iterator& i) {
    bool success = true;

    llvm::BasicBlock::iterator ii = i;
    if (!rjit::ir::Pattern::isInstruction(ii))
        return false;
    Pattern* pattern = rjit::ir::Pattern::match(ii);
    switch (pattern->getKind()) {
    case Pattern::PatternKind::GenericAdd: {
        genericAdd(static_cast<rjit::ir::GenericAdd*>(Pattern::getIR(i)));
        i = ii;
        return true;
    }
    case Pattern::PatternKind::Return: {
        ret(static_cast<rjit::ir::Return*>(Pattern::getIR(i)));
        i = ii;
        return true;
    }
    case Pattern::PatternKind::GenericGetVar: {
        if (not i->isTerminator()) {
            llvm::BasicBlock::iterator iii = ii;
            if (!rjit::ir::Pattern::isInstruction(iii))
                return false;
            Pattern* pattern = rjit::ir::Pattern::match(iii);
            switch (pattern->getKind()) {
            case Pattern::PatternKind::GenericGetVar: {
                genericGetVar2x(
                    static_cast<rjit::ir::GenericGetVar*>(Pattern::getIR(i)),
                    static_cast<rjit::ir::GenericGetVar*>(Pattern::getIR(ii)));
                i = iii;
                return true;
            }
            }
        }
        {
            rjit::ir::MockupPredicateA p;
            if (p.match(*this, static_cast<rjit::ir::GenericGetVar*>(
                                   Pattern::getIR(ii)))) {
                genericGetVar(
                    static_cast<rjit::ir::GenericGetVar*>(Pattern::getIR(i)));
                goto DONE;
            }
        }
        {
            rjit::ir::MockupPredicateB p;
            if (p.match(*this, static_cast<rjit::ir::GenericGetVar*>(
                                   Pattern::getIR(ii)))) {
                genericGetVar(
                    static_cast<rjit::ir::GenericGetVar*>(Pattern::getIR(i)));
                goto DONE;
            }
        }
        genericGetVar(static_cast<rjit::ir::GenericGetVar*>(Pattern::getIR(i)));
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