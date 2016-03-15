#ifndef OSR_INLINER_H
#define OSR_INLINER_H

#include "llvm.h"
#include "Compiler.h"
#include "FunctionCall.h"
#include "RDefs.h"
#include "Types.h"
#include <Rinternals.h>
#include <list>
using namespace llvm;

namespace osr {
typedef std::vector<llvm::Instruction*> Inst_Vector;
typedef std::list<llvm::ReturnInst*> Return_List;
typedef std::pair<SEXP, SEXP> ExitEntry;
typedef std::list<FunctionCall*> Call_List;
typedef std::map<SEXP, Call_List> Call_Map;
typedef std::vector<BasicBlock*> BB_Vector;

class OSRInliner {
  public:
    OSRInliner(rjit::Compiler* c);

    /**
     * @brief      Inlines calls inside f, whenever possible.
     *
     * @param[in]  f     A closure SEXP.
     *
     * @return     SEXP  A closure SEXP containing a function with inlined
     * calls.
     */
    SEXP inlineCalls(SEXP f);

    /**
     * Enables to register function SEXP to be used when the OSR exit is taken.
     */
    static std::map<uint64_t, ExitEntry> exits;

  private:
    rjit::Compiler* c;

    /*Intrinsics used to create a new rho*/
    Function* closureQuickArgumentAdaptor;
    Function* CONS_NR;

    /**
     * LLVM function corresponding to the function called in OSR ENTRY to
     * replace
     * the incorrect optimized version with the toInstrument version.
     */
    Function* fixClosure;

    /* Unique id counter. Used for registering exits.*/
    static uint64_t id;

    /**
     * @brief      Concatenates two constant pools and sets the result in the
     * first SEXP.
     *
     * @param[in]  firstP   A function SEXP.
     * @param[in]  secondP  A function SEXP.
     */
    static void setCP(SEXP firstP, SEXP secondP);

    /**
     * @brief      Increases the indexes for constant pool access in a callInst.
     *
     * @param      call    The call instruction.
     * @param[in]  offset  The offset to add to the constant pool accesses.
     */
    static void updateCPAccess(CallInst* call, int offset);

    /**
     * @brief      Get a closure corresponding to a symbol, in a given
     * environment.
     *
     * @param[in]  cp      The constant pool of the caller trying to access the
     * definition.
     * @param[in]  symbol  The index of the symbol in the constant pool.
     * @param[in]  env     The caller's environment.
     *
     * @return     The closure SEXP if found, nullptr otherwise.
     */
    static SEXP getFunction(SEXP cp, int symbol, SEXP env);

    /**
     * @brief      Groups function calls by callee.
     *
     * @param      calls  A vector of function call pointers.
     * @param[in]  outer  The caller closure.
     *
     * @return     A map from callee SEXP to a list of function calls.
     */
    Call_Map sortCalls(FunctionCalls* calls, SEXP outer);

    /**
     * @brief      Fixes environment and constant pool accesses inside the
     * callee's body.
     *
     * @param      toInline  The LLVM IR of the function to inline.
     * @param      fc        The function call correspondng to @toInline.
     * @param      newrho    The dedicated rho created in the caller.
     * @param[in]  cpOffset  The size of the caller's constant pool.
     * @param      ret       Accumulator for return instructions.
     */
    static void prepareCodeToInline(Function* toInline, FunctionCall* fc,
                                    CallInst* newrho, int cpOffset,
                                    Return_List* ret);

    /**
     * @brief      Inserts a callee body inside the caller.
     * The return instructions in the callee are removed and the corresponding
     * values are forwared to a phi node inserted after the inlined body.
     *
     * @param      toOpt         The caller.
     * @param      toInline      The callee.
     * @param      toInstrument  The model for the continuation function.
     * @param      fc            The function call.
     * @param      ret           The return instructions inside the callee.
     */
    void insertBody(Function* toOpt, Function* toInline, Function* toInstrument,
                    FunctionCall* fc, Return_List* ret);

    /**
     * @brief      Constructs a valid OSR condition, i.e., vector of
     * instructions.
     * The generated condition tests if the result of the getFunction call
     * returns the same closure as the one seen during the inlining.
     *
     * @param      fc    The function call.
     *
     * @return     A pointer to an instruction vector.
     */
    static Inst_Vector* getOSRCondition(FunctionCall* fc);

    /**
     * @brief      Creates a new environment corresponding to a function call.
     *
     * @param      fc    The function call.
     *
     * @return     Returns the vector of instructions to generate the
     * environment.
     */
    CallInst* createNewRho(FunctionCall* fc);

    /**
     * @brief      Creates the compensation code for an inlined function.
     *
     * @param[in]  fun      The toInstrument function SEXP.
     * @param[in]  closure  The wrapping closure SEXP.
     *
     * @return     Vector of instructions calling the fix closure with correct
     * id.
     */
    Inst_Vector* createCompensation(SEXP fun, SEXP closure);
    /**
    * @brief      Put the f's basic blocks into a vector
    *
    * @param      f     llvm::Function*
    *
    * @return     An std::vector of f's basic blocks
    */
    static BB_Vector* getBBs(Function* f) {
        BB_Vector* res = new BB_Vector();
        for (auto it = f->begin(); it != f->end(); ++it) {
            res->push_back(it);
        }
        return res;
    }
};

} // namespace osr

#endif