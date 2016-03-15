
/* This file has been automatically generated. Do not hand-edit. */
#include <llvm.h>
#include "RIntlns.h"
#include "ir/Ir.h"
#include "ir/primitive_calls.h"
#include "ir/Pass.h"

#pragma GCC diagnostic ignored "-Wswitch"
bool rjit::ir::MyPass::dispatch(llvm::BasicBlock::iterator& it0) {

    llvm::BasicBlock::iterator it1 = it0;

    Pattern* p0 = Pattern::get(it0);
    if (p0 != nullptr)
        switch (p0->kind) {

        case Pattern::Kind::MarkNotMutable: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericSetVarParent: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::CreateClosure: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericNe: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::PatchIC: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::NewEnv: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::UnsignedIntegerLessThan: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericGetVar: {
            p0->advance(it1);
            llvm::BasicBlock::iterator it2 = it1;

            Pattern* p1 = Pattern::get(it1);
            if (p1 != nullptr)
                switch (p1->kind) {

                case Pattern::Kind::GenericGetVar: {
                    p1->advance(it2);

                    genericGetVar2x(static_cast<rjit::ir::GenericGetVar*>(p0),
                                    static_cast<rjit::ir::GenericGetVar*>(p1));
                    it0 = it2;
                    return true;

                    break;
                }
                }
            {
                rjit::ir::MockupPredicateA p = rjit::ir::MockupPredicateA();
                if (p.match(*this, static_cast<rjit::ir::GenericGetVar*>(p0))) {
                    genericGetVar(static_cast<rjit::ir::GenericGetVar*>(p0), p);
                    it0 = it1;
                    return true;
                }
            }
            {
                rjit::ir::MockupPredicateB p = rjit::ir::MockupPredicateB();
                if (p.match(*this, static_cast<rjit::ir::GenericGetVar*>(p0))) {
                    genericGetVar(static_cast<rjit::ir::GenericGetVar*>(p0), p);
                    it0 = it1;
                    return true;
                }
            }

            genericGetVar(static_cast<rjit::ir::GenericGetVar*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::Nop: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::CompileIC: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::AddEllipsisArgumentHead: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericUnaryMinus: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericLe: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::Branch: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::VectorGetElement: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::Return: {
            p0->advance(it1);

            ret(static_cast<rjit::ir::Return*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GetSymFunction: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GetFunction: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::CheckFunction: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericBitAnd: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::ClosureQuickArgumentAdaptor: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::IntegerEquals: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::MarkVisible: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::CallNative: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::LoopSequenceLength: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericGetVarMissOK: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::Car: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::CallBuiltin: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::ReturnJump: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::Cdr: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::CheckSwitchControl: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericGetEllipsisArg: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericPow: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::UserLiteral: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GetInternalBuiltinFunction: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::Cbr: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::AddArgument: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::AddKeywordArgument: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::Constant: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::Switch: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericSetVar: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericEq: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GetGlobalFunction: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::InitClosureContext: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::Tag: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::StartFor: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::ConvertToLogicalNoNA: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::IntegerLessThan: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::SexpType: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericBitOr: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericAdd: {
            p0->advance(it1);

            genericAdd(static_cast<rjit::ir::GenericAdd*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::AddEllipsisArgumentTail: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::ClosureNativeCallTrampoline: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericDiv: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericGetEllipsisValueMissOK: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::SwitchControlCharacter: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericGt: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericNot: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::MarkInvisible: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GetBuiltinFunction: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::SwitchControlInteger: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericGe: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::EndClosureContext: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::PrintValue: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GetForLoopValue: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericSub: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::IntegerAdd: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::CallClosure: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::ConsNr: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericSqrt: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::CreatePromise: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericUnaryPlus: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::CallToAddress: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericLt: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericMul: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::CallSpecial: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }

        case Pattern::Kind::GenericExp: {
            p0->advance(it1);

            defaultMatch(static_cast<rjit::ir::Pattern*>(p0));
            it0 = it1;
            return true;

            break;
        }
        }
    return rjit::ir::Pass::dispatch(it0);
}
