#include "Builder.h"

using namespace llvm;

namespace rjit {
namespace ir {

Builder::Context::Context(std::string const & name, Module * m, bool isPromise) {
    // TODO the type is ugly
    f = Function::Create(t::sexp_sexpsexpint, Function::ExternalLinkage, name, m);

    functionId = StackMap::nextStackmapId++;

    f->setGC("statepoint-example");
    auto attrs = f->getAttributes();
    attrs = attrs.addAttribute(f->getContext(), AttributeSet::FunctionIndex,
                               "no-frame-pointer-elim", "true");
    attrs = attrs.addAttribute(f->getContext(), AttributeSet::FunctionIndex,
                               "statepoint-id", std::to_string(functionId));
    f->setAttributes(attrs);

    // get rho value into context->rho for easier access
    llvm::Function::arg_iterator args = f->arg_begin();
    llvm::Value* body = args++;
    body->setName("body");
    rho = args++;
    rho->setName("rho");
    llvm::Value* useCache = args++;
    useCache->setName("useCache");

    // create first basic block
    b = llvm::BasicBlock::Create(llvm::getGlobalContext(), "start", f, nullptr);

    isReturnJumpNeeded = isPromise;
}

SEXP Builder::createNativeSXP(RFunctionPtr fptr, SEXP ast,
                     std::vector<SEXP> const& objects, Function* f) {
    SEXP objs = allocVector(VECSXP, objects.size() + 1);
    PROTECT(objs);
    SET_VECTOR_ELT(objs, 0, ast);
    for (size_t i = 0; i < objects.size(); ++i)
        SET_VECTOR_ELT(objs, i + 1, objects[i]);
    SEXP result = CONS(reinterpret_cast<SEXP>(fptr), objs);
    UNPROTECT(
        objects.size() +
        1); // all objects in objects + objs itself which is now part of result
    SET_TAG(result, reinterpret_cast<SEXP>(f));
    SET_TYPEOF(result, NATIVESXP);
    return result;
}




} // namespace ir
} //namespace rjit
