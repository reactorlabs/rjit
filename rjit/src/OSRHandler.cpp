#include "OSRHandler.h"
#include "OSRLibrary.hpp"
#include <llvm/IR/InstIterator.h>
#include "JITCompileLayer.h"
#include "api.h"
#include <llvm/Transforms/Utils/Cloning.h>

using namespace llvm;

namespace osr {

/*      Initialize static variables     */
OSRHandler OSRHandler::instance;
std::map<SEXP, SEXP> OSRHandler::baseVersions;
std::map<std::pair<Function*, Function*>, StateMap*> OSRHandler::transitiveMaps;
/******************************************************************************/
/*                        Public functions                                    */
/******************************************************************************/
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
        std::reverse(compensation->begin(), compensation->end());
        for (auto it = compensation->begin(); it != compensation->end(); ++it)
            (*it)->insertBefore(&(res.second->getEntryBlock().back()));
    }
    return res;
}

void OSRHandler::removeEntry(Function* opt, Function* instrument, Value* val) {
    auto key = std::pair<Function*, Function*>(opt, instrument);
    assert(transitiveMaps.find(key) != transitiveMaps.end() &&
           "This key has no statemap registered.");
    auto map = transitiveMaps[key];
    auto bidirect = map->getCorrespondingOneToOneValue(val);
    map->unregisterOneToOneValue(val);
    map->unregisterOneToOneValue(bidirect);
}

void OSRHandler::updateEntry(Func_Pair key, Value* phi, Value* stub) {
    assert(transitiveMaps.find(key) != transitiveMaps.end() &&
           "This key has no statemap registered.");
    auto map = transitiveMaps[key];
    auto stubInstr = map->getCorrespondingOneToOneValue(stub);
    map->unregisterOneToOneValue(stub);
    map->registerOneToOneValue(phi, stubInstr, true);
}

SEXP OSRHandler::getFreshIR(SEXP closure, rjit::Compiler* c) {
    assert(TYPEOF(closure) == CLOSXP && "getFreshIR requires a closure.");

    SEXP body = BODY(closure);
    SEXP func = R_NilValue;

    if (baseVersions.find(closure) == baseVersions.end()) {
        if (TYPEOF(body) == NATIVESXP &&
            GET_LLVM(body)->getParent() == c->getBuilder()->module()) {
            func = body;
        } else {
            func = c->compile("rfunction", body, FORMALS(closure));
            SETCDR(closure, func);
        }

        ValueToValueMapTy VMap;
        Function* clone = CloneFunction(GET_LLVM(func), VMap, false);
        baseVersions[closure] = cloneSEXP(func, clone);
        assert(baseVersions.find(closure) != baseVersions.end());
    }

    SEXP entry = baseVersions[closure];
    ValueToValueMapTy VMap;
    Function* workingCopy = CloneFunction(GET_LLVM(entry), VMap, false);

    func = cloneSEXP(entry, workingCopy);

    return func;
}
SEXP OSRHandler::cloneSEXP(SEXP func, Function* llvm) {
    SEXP result = CONS(nullptr, CDR(func));
    SET_TAG(result, reinterpret_cast<SEXP>(llvm));
    SET_TYPEOF(result, NATIVESXP);
    return result;
}

void OSRHandler::addSexpToModule(SEXP f, Module* m) {
    assert(GET_LLVM(f) && "Trying to add a null function to the module.");
    m->getFunctionList().push_back(GET_LLVM(f));
}

SEXP OSRHandler::resetSafepoints(SEXP func, rjit::Compiler* c) {
    assert(TYPEOF(func) == NATIVESXP && GET_LLVM(func) && "Invalid function.");
    Function* f = GET_LLVM(func);
    f->setGC("rjit");
    Module* m = c->getBuilder()->module();
    for (inst_iterator it = inst_begin(f), e = inst_end(f); it != e; ++it) {
        CallInst* call = dynamic_cast<CallInst*>(&(*it));
        if (call) {
            if (call->getCalledFunction()->getParent() != m) {
                Function* target = call->getCalledFunction();
                Function* resolve = m->getFunction(target->getName());
                if (!resolve)
                    resolve = Function::Create(
                        target->getFunctionType(), // This might be a problem
                        Function::ExternalLinkage, target->getName(), m);
                call->setCalledFunction(resolve);
            }

            // Fix the ic stub
            if (IS_STUB(call)) {
                unsigned index = call->getNumArgOperands() - 2;
                if (call->getArgOperand(index) != f) {
                    call->setArgOperand(index, f);
                }
            }
        }
    }
    return func;
}

void OSRHandler::clear() {
    baseVersions.clear();
    transitiveMaps.clear();
}

} // namespace osr