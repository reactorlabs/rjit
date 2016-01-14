#include "OSRHandler.h"
#include "OSRLibrary.hpp"

using namespace llvm;

namespace osr {

/*      Initialize static variables     */
OSRHandler OSRHandler::instance;
std::map<Function*, std::list<Function*>> OSRHandler::instruments;
std::map<Function*, Function*> OSRHandler::toBase;
std::map<std::pair<Function*, Function*>, StateMap*> OSRHandler::transitiveMaps;

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

void OSRHandler::insertOSR(Function* opt, Function* instrument,
                           Instruction* src, Instruction* pad,
                           Inst_Vector* cond) {
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

    OSRLibrary::insertResolvedOSR(getGlobalContext(), *opt, *src, *instrument,
                                  *lPad, *cond, *transitive, configuration);
}

/******************************************************************************/
/*                        Private functions                                   */
/******************************************************************************/

} // namespace osr