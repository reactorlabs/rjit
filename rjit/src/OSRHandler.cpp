#include "OSRHandler.h"
#include "OSRLibrary.hpp"
#include <llvm/IR/InstIterator.h>
#include "JITCompileLayer.h"

using namespace llvm;

namespace osr {

/*      Initialize static variables     */
OSRHandler OSRHandler::instance;
std::map<SEXP, SEXP> OSRHandler::baseVersions;
std::map<std::pair<Function*, Function*>, StateMap*> OSRHandler::transitiveMaps;
/******************************************************************************/
/*                        Public functions                                    */
/******************************************************************************/
Function* OSRHandler::setupOpt(Function* base) {
    assert(base && "OSRHandler: setup with nullptr");

    // Generate a new clone of the function that we can modify.
    auto toOpt = StateMap::generateIdentityMapping(base);

    // Add the function to the module.
    base->getParent()->getFunctionList().push_back(toOpt.first);

    return toOpt.first;
}

Function* OSRHandler::getToInstrument(Function* base) {
    auto toInstrument = StateMap::generateIdentityMapping(base);

    auto transitiveKey =
        std::pair<Function*, Function*>(base, toInstrument.first);
    transitiveMaps[transitiveKey] = toInstrument.second;

    // add the function to the module.
    base->getParent()->getFunctionList().push_back(toInstrument.first);

    return toInstrument.first;
}

std::pair<Function*, Function*>
OSRHandler::insertOSRExit(Function* opt, Function* instrument, Instruction* src,
                          Inst_Vector* cond, Inst_Vector* compensation) {
    StateMap* F2NewToF2Map = nullptr;
    OSRLibrary::OSRPointConfig configuration(
        false /*verbose*/, true /*updateF1*/, -1 /*branch taken prob*/,
        nullptr /*keep F1 name*/, opt->getParent() /*keep mod for F1*/,
        nullptr /*keep stateMap*/, nullptr /*default name generation*/,
        opt->getParent() /*mod for F2*/,
        &F2NewToF2Map /*statemap cont to target*/);

    // Get the landing pad.
    auto transitive =
        transitiveMaps[std::pair<Function*, Function*>(opt, instrument)];
    assert(transitive && "No transitive map registered for this pair.");
    Instruction* lPad = dynamic_cast<Instruction*>(
        transitive->getCorrespondingOneToOneValue(src));
    assert(lPad && "The landing pad could not be found.");

    auto res = OSRLibrary::insertResolvedOSR(getGlobalContext(), *opt, *src,
                                             *instrument, *lPad, *cond,
                                             *transitive, configuration);
    if (compensation) {
        for (auto it = compensation->begin(); it != compensation->end(); ++it)
            (*it)->insertBefore(&(res.second->getEntryBlock().back()));
    }

    // Printing
    /*res.first->dump();
    res.second->dump();*/
    return res;
}

void OSRHandler::removeEntry(Function* opt, Function* instrument, Value* val) {
    auto key = std::pair<Function*, Function*>(opt, instrument);
    assert(transContains(key) && "This key has no statemap registered.");
    auto map = transitiveMaps[key];
    auto bidirect = map->getCorrespondingOneToOneValue(val);
    map->unregisterOneToOneValue(val);
    map->unregisterOneToOneValue(bidirect);
}

SEXP OSRHandler::getFreshIR(SEXP closure, rjit::Compiler* c, bool compile) {
    assert(TYPEOF(closure) == CLOSXP && "getFreshIR requires a closure.");

    SEXP body = BODY(closure);
    SEXP func = R_NilValue;

    if (TYPEOF(body) == NATIVESXP &&
        GET_LLVM(body)->getParent() == c->getBuilder()->module())
        func = body; // TODO that's wrong
    else {
        if (!baseVersionContains(closure)) {
            // Setup the copy.
            func = c->compile("rfunction", body, FORMALS(closure));

            // TODO remove
            printf("Original compile result\n");
            GET_LLVM(func)->dump();

            Function* clone =
                StateMap::generateIdentityMapping(GET_LLVM(func)).first;
            baseVersions[closure] = cloneSEXP(func, clone);
            assert(baseVersionContains(closure));
        }

        SEXP entry = baseVersions[closure];
        Function* workingCopy =
            StateMap::generateIdentityMapping(GET_LLVM(entry)).first;

        func = cloneSEXP(entry, workingCopy);
        // Adding the result to the relocations.
        if (compile) {
            c->getBuilder()->module()->getFunctionList().push_back(workingCopy);
            c->getBuilder()->module()->fixRelocations(FORMALS(closure), func,
                                                      workingCopy);
            // TODO remove
            printf("The working copy\n");
            workingCopy->dump();
            addIRToModule(func, c);
        }
    }
    return func;
}
SEXP OSRHandler::cloneSEXP(SEXP func, Function* llvm) {
    SEXP result = CONS(nullptr, CDR(func));
    SET_TAG(result, reinterpret_cast<SEXP>(llvm));
    SET_TYPEOF(result, NATIVESXP);
    return result;
}

SEXP OSRHandler::addIRToModule(SEXP func, rjit::Compiler* c) {
    assert(TYPEOF(func) == NATIVESXP && GET_LLVM(func) && "Invalid function.");
    Function* f = GET_LLVM(func);
    Module* m = c->getBuilder()->module();
    for (inst_iterator it = inst_begin(f), e = inst_end(f); it != e; ++it) {
        CallInst* call = dynamic_cast<CallInst*>(&(*it));
        if (call && call->getCalledFunction()->getParent() != m) {
            Function* target = call->getCalledFunction();
            Function* resolve = m->getFunction(target->getName());
            if (!resolve)
                resolve = Function::Create(target->getFunctionType(),
                                           Function::ExternalLinkage,
                                           target->getName(), m);
            call->setCalledFunction(resolve);
            auto id = rjit::JITCompileLayer::singleton.getSafepointId(f);
            setAttributes(call, id);
        }
    }
    return func;
}
/******************************************************************************/
/*                        Private functions                                   */
/******************************************************************************/

bool OSRHandler::transContains(std::pair<Function*, Function*> key) {
    return transitiveMaps.find(key) != transitiveMaps.end();
}

bool OSRHandler::baseVersionContains(SEXP key) {
    return baseVersions.find(key) != baseVersions.end();
}

void OSRHandler::setAttributes(CallInst* call, uint64_t smid) {
    Module* m = call->getParent()->getParent()->getParent();
    assert(m && "Module not set for call instruction.");
    llvm::AttributeSet PAL;
    {
        llvm::SmallVector<llvm::AttributeSet, 4> Attrs;
        llvm::AttributeSet PAS;
        {
            llvm::AttrBuilder B;
            B.addAttribute("statepoint-id", std::to_string(smid));
            PAS = llvm::AttributeSet::get(m->getContext(), ~0U, B);
        }
        Attrs.push_back(PAS);
        PAL = llvm::AttributeSet::get(m->getContext(), Attrs);
    }
    call->setAttributes(PAL);
}

} // namespace osr