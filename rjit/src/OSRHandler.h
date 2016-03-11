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
typedef std::pair<Function*, Function*> Func_Pair;

class OSRHandler : public OSRLibrary {
  public:
    static void clear();
    static std::map<SEXP, SEXP> baseVersions;
    /**
     * Map from a function pair <toOpt, toInstrument> to their statemaps.
     */
    static std::map<Func_Pair, StateMap*> transitiveMaps;

    /**
     * @brief      Returns the singleton of the OSRHandler.
     *
     * @return     A pointer to the singleton instance.
     */
    static OSRHandler* getInstance() { return &instance; }

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

    /**
     * @brief      Registers a new mapping between map(stub) and phi.
     * USE IT FOR OSR INLINER, NEED A MORE GENERAL IMPL FOR OTHER CASES.
     *
     * @param[in]  key   Function pair key for transitive maps.
     * @param      phi   The phi replacing the function call.
     * @param      stub  The stub call that is being removed.
     */
    static void updateEntry(Func_Pair key, Value* phi, Value* stub);

    /**
     * @brief      Returns a fresh, non-instrumented IR for the input closure.
     * If the function
     *
     * @param[in]  closure  The closure containing the function SEXP we want to
     * copy.
     * @param      c        The compiler instance.
     *
     * @return     { description_of_the_return_value }
     */
    static SEXP getFreshIR(SEXP closure, rjit::Compiler* c);

    static SEXP cloneSEXP(SEXP func, Function* llvm);

    static void addSexpToModule(SEXP f, Module* m);
    static SEXP resetSafepoints(SEXP func, rjit::Compiler* c);

  private:
    static OSRHandler instance;

    OSRHandler() {}
};

} // namespace osr

#endif