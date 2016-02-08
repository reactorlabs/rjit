#ifndef OSR_HANDLER_H
#define OSR_HANDLER_H

#include "llvm.h"
#include <list>
#include "OSRLibrary.hpp"
#include "Compiler.h"
#include <Rinternals.h>

using namespace llvm;
namespace osr {

#define GET_LLVM(sexp) (reinterpret_cast<llvm::Function*>(TAG(sexp)))
typedef std::vector<Instruction*> Inst_Vector;
typedef std::pair<Function*, Function*> OSRPair;
typedef std::pair<SEXP, Function*> SEXPFunc;

class OSRHandler : public OSRLibrary {
  public:
    static std::map<SEXP, SEXP> baseVersions;
    /**
     * Map from base function to instrumented
     */
    static std::map<Function*, std::list<Function*>> instruments;

    /**
     * Map from version to base.
     */
    static std::map<Function*, Function*> toBase;

    /**
     * Map from a function pair <toOpt, toInstrument> to their statemaps.
     */
    static std::map<std::pair<Function*, Function*>, StateMap*> transitiveMaps;

    /**
     * Map that keeps a reference to the exit functions.TODO
     */
    static std::map<uint64_t, Function*> exits;

    /**
     * @brief      Returns the singleton of the OSRHandler.
     *
     * @return     A pointer to the singleton instance.
     */
    static OSRHandler* getInstance() { return &instance; }

    /**
     * @brief      { function_description }
     *
     * @param      base  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    static Function* setupOpt(Function* base);

    /**
     * @brief      Returns a copy of base and registers a new mapping.
     *
     * @param      base  The base function.
     *
     * @return     Function* clone of the base function.
     */
    static Function* getToInstrument(Function* base);

    static Function* getExit(uint64_t id);

    /**
     * @brief      Inserts an osr exit in opt to instrument.
     *
     * @param      opt         Optimized function (e.g., source).
     * @param      instrument  Continuation function.
     * @param      src         Where to put the exit condition.
     * @param      pad         Point in opt that corresponds to landing pad in
     * instrument.
     * @param      cond        OSR condition.
     */
    static std::pair<Function*, Function*>
    insertOSR(Function* opt, Function* instrument, Instruction* src,
              Instruction* pad, Inst_Vector* cond);

    static uint64_t getId();

    /**
     * @brief      Removes a bidirectional mapping in the stateMap registered
     * under the key <otp, instrument>.
     *
     * @param      opt         The optimized version.
     * @param      instrument  The instrumented version.
     * @param      val         The value to remove.
     *
     * @note       This is not required for the code to perform well, it is just
     * needed to have a clean StateMap.
     */
    static void removeEntry(Function* opt, Function* instrument, Value* val);

    static SEXP getFreshIR(SEXP closure, rjit::Compiler* c,
                           bool compile = true);

  private:
    static OSRHandler instance;
    static uint64_t id;

    OSRHandler() {}
    // static bool existInstrument(Function* f);
    static bool transContains(std::pair<Function*, Function*> key);
    static bool baseVersionContains(SEXP key);
    static SEXP cloneSEXP(SEXP func, Function* llvm);
};

} // namespace osr

#endif