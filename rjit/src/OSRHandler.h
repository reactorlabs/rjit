#ifndef OSR_HANDLER_H
#define OSR_HANDLER_H

#include "llvm.h"
#include <list>
#include "OSRLibrary.hpp"
#include "Compiler.h"
#include <Rinternals.h>

using namespace llvm;
namespace osr {
#define ICSTUB_NAME "icStub"
#define NAME_CONTAINS(x, y)                                                    \
    ((((x)->getName().str()).find((y))) != std::string::npos)
#define IS_STUB(x) NAME_CONTAINS((x)->getCalledFunction(), ICSTUB_NAME)

#define GET_LLVM(sexp) (reinterpret_cast<llvm::Function*>(TAG(sexp)))
typedef std::vector<Instruction*> Inst_Vector;
typedef std::pair<Function*, Function*> OSRPair;
typedef std::pair<SEXP, Function*> SEXPFunc;

class OSRHandler : public OSRLibrary {
  public:
    static std::map<SEXP, SEXP> baseVersions;
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

    static std::pair<Function*, Function*>
    insertOSRExit(Function* opt, Function* instrument, Instruction* src,
                  Inst_Vector* cond, Inst_Vector* compensation = nullptr);

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

    static SEXP cloneSEXP(SEXP func, Function* llvm);

    static void addSexpToModule(SEXP f, Module* m);
    static SEXP resetSafepoints(SEXP func, rjit::Compiler* c);

  private:
    static OSRHandler instance;

    OSRHandler() {}
    // static bool existInstrument(Function* f);
    static bool transContains(std::pair<Function*, Function*> key);
    static bool baseVersionContains(SEXP key);
    static void setAttributes(CallInst* call, uint64_t smid, bool stub);
};

} // namespace osr

#endif