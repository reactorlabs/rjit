#include "OSRHandler.h"
#include "OSRLibrary.hpp"

using namespace llvm;

namespace osr {

/*      Initialize static variables     */
OSRHandler OSRHandler::instance;
Module* OSRHandler::lastMod = nullptr;
GlobalVariable* OSRHandler::osrValue = nullptr;
std::map<Function*, std::list<Function*>> OSRHandler::instruments;
std::map<Function*, StateMap*> OSRHandler::statemaps;
std::map<Function*, Function*> OSRHandler::toBase;
std::map<std::pair<Function*, Function*>, StateMap*> OSRHandler::transitiveMaps;

/******************************************************************************/
/*                        Public functions                                    */
/******************************************************************************/

std::pair<Function*, Function*> OSRHandler::setupOptAndInstr(Function* base) {
    assert(base && "OSRHandler: setup with nullptr.");

    // Generate a new clone of the function that we can modify.
    Function* toOpt = setupOpt(base);

    // Generate the version to instrument for the continuation.
    Function* toInstrument = getFreshInstrument(base, toOpt);

    return std::pair<Function*, Function*>(toOpt, toInstrument);
}

Function* OSRHandler::setupOpt(Function* base) {
    assert(base && "OSRHandler: setup with nullptr");

    // Generate a new clone of the function that we can modify.
    auto toOpt = StateMap::generateIdentityMapping(base);
    toBase[toOpt.first] = base;
    statemaps[toOpt.first] = toOpt.second;

    // Add the function to the module.
    base->getParent()->getFunctionList().push_back(toOpt.first);

    return toOpt.first;
}

Function* OSRHandler::getFreshInstrument(Function* base, Function* toOpt) {
    auto toInstrument = StateMap::generateIdentityMapping(base);
    (instruments[base]).push_back(toInstrument.first);
    statemaps[toInstrument.first] = toInstrument.second;

    auto toOptMap = statemaps[toOpt];

    // Generate & Register the transitive statemap.
    StateMap* transitive =
        new StateMap(toOpt, toOptMap, toInstrument.first, toInstrument.second);
    auto transitiveKey =
        std::pair<Function*, Function*>(toOpt, toInstrument.first);
    transitiveMaps[transitiveKey] = transitive;

    // Add the function to the module.
    base->getParent()->getFunctionList().push_back(toInstrument.first);

    return toInstrument.first;
}

void OSRHandler::insertOSR(Function* opt, Function* instrument,
                           Instruction* src, Inst_Vector* cond) {
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

    OSRLibrary::insertResolvedOSR(getGlobalContext(), *opt, *src, *instrument,
                                  *lPad, *cond, *transitive, configuration);
}

/******************************************************************************/
/*                        Private functions                                   */
/******************************************************************************/

/*bool OSRHandler::existInstrument(Function* f) {
  std::map<Function*, Function*>::iterator it = instruments.find(f);
  return (it != instruments.end());
}*/

} // namespace osr