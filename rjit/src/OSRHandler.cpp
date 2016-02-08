#include "OSRHandler.h"
#include "OSRLibrary.hpp"

using namespace llvm;

namespace osr {

/*      Initialize static variables     */
OSRHandler OSRHandler::instance;
uint64_t OSRHandler::id = 0;
std::map<SEXP, SEXP> OSRHandler::baseVersions;
std::map<Function*, std::list<Function*>> OSRHandler::instruments;
std::map<Function*, Function*> OSRHandler::toBase;
std::map<std::pair<Function*, Function*>, StateMap*> OSRHandler::transitiveMaps;
std::map<uint64_t, Function*> OSRHandler::exits;

/******************************************************************************/
/*                        Public functions                                    */
/******************************************************************************/
Function* OSRHandler::setupOpt(Function* base) {
    assert(base && "OSRHandler: setup with nullptr");

    // Generate a new clone of the function that we can modify.
    auto toOpt = StateMap::generateIdentityMapping(base);
    toBase[toOpt.first] = base;

    // Add the function to the module.
    base->getParent()->getFunctionList().push_back(toOpt.first);

    return toOpt.first;
}

Function* OSRHandler::getToInstrument(Function* base) {
    auto toInstrument = StateMap::generateIdentityMapping(base);
    (instruments[base]).push_back(toInstrument.first);

    auto transitiveKey =
        std::pair<Function*, Function*>(base, toInstrument.first);
    transitiveMaps[transitiveKey] = toInstrument.second;

    // add the function to the module.
    base->getParent()->getFunctionList().push_back(toInstrument.first);

    return toInstrument.first;
}

std::pair<Function*, Function*>
OSRHandler::insertOSR(Function* opt, Function* instrument, Instruction* src,
                      Instruction* pad, Inst_Vector* cond) {
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
        transitive->getCorrespondingOneToOneValue(pad));
    assert(lPad && "The landing pad could not be found.");

    auto res = OSRLibrary::insertResolvedOSR(getGlobalContext(), *opt, *src,
                                             *instrument, *lPad, *cond,
                                             *transitive, configuration);
    exits[++id] = instrument;
    // Printing
    res.first->dump();
    res.second->dump();
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
        func = body;
    else {
        if (!baseVersionContains(closure)) {
            // Setup the copy.
            func = c->compile("rfunction", body, FORMALS(closure));
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

uint64_t OSRHandler::getId() { return id; }

Function* OSRHandler::getExit(uint64_t idx) { return exits[idx]; }

SEXP OSRHandler::cloneSEXP(SEXP func, Function* llvm) {
    SEXP result = CONS(nullptr, CDR(func));
    SET_TAG(result, reinterpret_cast<SEXP>(llvm));
    SET_TYPEOF(result, NATIVESXP);
    // TODO it is not in the relocations for the moment !!!
    return result;
}

} // namespace osr