#ifndef OSR_HANDLER_H
#define OSR_HANDLER_H

#include "llvm.h"
#include <list>
#include "OSRLibrary.hpp"

using namespace llvm;
namespace osr {

#define GET_LLVM(sexp) (reinterpret_cast<llvm::Function*>(TAG(sexp)))
typedef std::vector<Instruction*> Inst_Vector;

class OSRHandler : public OSRLibrary {
  public:
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

    /**
     * @brief      Inserts a bidirectional (lie, later) osr relation.
     *
     * @param      opt         optimized function
     * @param      instrument  continuation function
     * @param      src         OSRExit
     * @param      lPad        entry in instrument
     */
    static void insertOSR(Function* opt, Function* instrument, Instruction* src,
                          Inst_Vector* cond);

  private:
    static OSRHandler instance;

    OSRHandler() {}
    // static bool existInstrument(Function* f);
};

} // namespace osr

#endif