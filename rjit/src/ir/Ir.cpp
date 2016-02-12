#include "Ir.h"
#include "ir/Pass.h"

using namespace llvm;

namespace rjit {
namespace ir {

char const* const Pattern::MD_NAME = "r_ir_type";

llvm::Instruction* const ir::Pattern::Sentinel::singleton =
    (new ir::Nop())->ins_;

llvm::Value * Predicate::constantPool(ir::Pass & p) {
    return p.constantPool;
}



bool VectorGetElement::FromConstantPool::match(ir::Pass & p, VectorGetElement * vge) {
    return  vge->vector() == constantPool(p) and llvm::isa<llvm::ConstantInt>(vge->index());
}


} // namespace ir

} // namespace rjit
