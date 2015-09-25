#include "JITCompileLayer.h"

#include "GCPassApi.h"

#include "JITMemoryManager.h"
#include "JITSymbolResolver.h"

#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#include "llvm/Support/DynamicLibrary.h"

using namespace llvm;

namespace rjit {

JITCompileLayer::ObjLayer JITCompileLayer::objectLayer;
std::unique_ptr<JITCompileLayer::CompileLayer> JITCompileLayer::compileLayer;

JITCompileLayer::ModuleHandle JITCompileLayer::getHandle(Module* m) {

    legacy::PassManager pm;

    pm.add(createTargetTransformInfoWrapperPass(TargetIRAnalysis()));

    pm.add(rjit::createPlaceRJITSafepointsPass());

    PassManagerBuilder PMBuilder;
    PMBuilder.OptLevel = 0;  // Set optimization level to -O0
    PMBuilder.SizeLevel = 0; // so that no additional phases are run.
    PMBuilder.populateModulePassManager(pm);

    // TODO: maybe have our own version which is not relocating?
    pm.add(rjit::createRJITRewriteStatepointsForGCPass());
    pm.run(*m);

    auto targetMachine = EngineBuilder().selectTarget();
    compileLayer = std::unique_ptr<CompileLayer>(new CompileLayer(
        objectLayer, llvm::orc::SimpleCompiler(*targetMachine)));
    std::vector<llvm::Module*> moduleSet;
    moduleSet.push_back(m);
    // Make sure we can resolve symbols in the program as well. The zero arg
    // to the function tells DynamicLibrary to load the program, not a library.
    std::string err;
    sys::DynamicLibrary::LoadLibraryPermanently(nullptr, &err);
    auto mm = new JITMemoryManager();
    return compileLayer->addModuleSet(moduleSet,
                                      std::unique_ptr<SectionMemoryManager>(mm),
                                      &JITSymbolResolver::singleton);
}
}
