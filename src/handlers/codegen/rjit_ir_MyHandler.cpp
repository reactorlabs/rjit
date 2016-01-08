#include "ir/Handler.h"
#include "llvm.h"
#include "RIntlns.h"
#include "ir/intrinsics.h"

#pragma GCC diagnostic ignored "-Wswitch"
bool rjit::ir::MyHandler::dispatch(llvm::BasicBlock::iterator& i) {
    llvm::BasicBlock::iterator ii = i;
    rjit::ir::Type t = rjit::ir::Instruction::match(ii);
    switch (t) {
    case rjit::ir::Type::Return: {
        ret(&*i);
        i = ii;
        return true;
    }
    case rjit::ir::Type::GenericGetVar: {
        if (not i->isTerminator()) {
            llvm::BasicBlock::iterator iii = ii;
            rjit::ir::Type t = rjit::ir::Instruction::match(iii);
            switch (t) {
            case rjit::ir::Type::GenericGetVar: {
                genericGetVar2x(&*i, &*ii);
                i = iii;
                return true;
            }
            }
        }
        {
            rjit::ir::MockupPredicateA p;
            if (p.match(*this, &*i)) {
                genericGetVar(&*i);
                return true;
            }
        }
        {
            rjit::ir::MockupPredicateB p;
            if (p.match(*this, &*i)) {
                genericGetVar(&*i);
                return true;
            }
        }
        genericGetVar(&*i);
        i = ii;
        return true;
    }
    case rjit::ir::Type::GenericAdd: {
        genericAdd(&*i);
        i = ii;
        return true;
    }
    }
    if (rjit::ir::Handler::dispatch(i))
        return true;

    return false;
}
