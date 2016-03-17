#ifndef COMPILER_LAYER_H
#define COMPILER_LAYER_H

#include "JITMemoryManager.h"
#include "JITModule.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"

#include <vector>
#include <unordered_map>

class StateMap;

namespace rjit {

namespace ir {
class Builder;
}

class JITCompileLayer {
  public:
    llvm::Function* osrInstrument = nullptr;
    llvm::Function* osrClone = nullptr;
    StateMap* osrStatemap = nullptr;

    typedef std::unordered_map<llvm::Function*, std::vector<uint64_t>>
        FunctionToStackmap;

    static JITCompileLayer singleton;

    ExecutionEngine* finalize(JITModule* m);
    uint64_t getSafepointId(llvm::Function* f);
    void setPatchpoint(uint64_t i, unsigned stubSize) {
        patchpoints[i] = stubSize;
    }

  private:
    void recordStackmaps(llvm::ExecutionEngine* engine, llvm::Module* m,
                         JITMemoryManager* mm);

    uint64_t nextStackmapId = 2;
    FunctionToStackmap safepoints;
    std::unordered_map<uint64_t, unsigned> patchpoints;
};
}

#endif
