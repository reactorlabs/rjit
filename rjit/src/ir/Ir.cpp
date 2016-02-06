#include "Ir.h"

using namespace llvm;

namespace rjit {
namespace ir {

char const* const Pattern::MD_NAME = "r_ir_type";

Car* Car::create(Builder& b, ir::Value sexp) {
    ConstantInt* int_0 =
        ConstantInt::get(b.getContext(), APInt(32, StringRef("0"), 10));
    ConstantInt* int_4 =
        ConstantInt::get(b.getContext(), APInt(32, StringRef("4"), 10));
    auto consValuePtr = GetElementPtrInst::Create(
        t::SEXPREC, sexp, std::vector<llvm::Value*>({int_0, int_4}), "", b.block());
    auto ptr = GetElementPtrInst::Create(t::SEXP_u1, consValuePtr,
                                         std::vector<llvm::Value*>({int_0, int_0}),
                                         "", b.block());
    auto value = new LoadInst(ptr, "", false, b.block());
    return new Car(consValuePtr, ptr, value);
}

Cdr* Cdr::create(Builder& b, ir::Value sexp) {
    ConstantInt* int_0 =
        ConstantInt::get(b.getContext(), APInt(32, StringRef("0"), 10));
    ConstantInt* int_1 =
        ConstantInt::get(b.getContext(), APInt(32, StringRef("1"), 10));
    ConstantInt* int_4 =
        ConstantInt::get(b.getContext(), APInt(32, StringRef("4"), 10));
    auto consValuePtr = GetElementPtrInst::Create(
        t::SEXPREC, sexp, std::vector<llvm::Value*>({int_0, int_4}), "", b.block());
    auto ptr = GetElementPtrInst::Create(t::SEXP_u1, consValuePtr,
                                         std::vector<llvm::Value*>({int_0, int_1}),
                                         "", b.block());
    auto value = new LoadInst(ptr, "", false, b.block());
    return new Cdr(consValuePtr, ptr, value);
}

Tag* Tag::create(Builder& b, ir::Value sexp) {
    ConstantInt* int_2 =
        ConstantInt::get(b.getContext(), APInt(32, StringRef("2"), 10));
    ConstantInt* int_0 =
        ConstantInt::get(b.getContext(), APInt(32, StringRef("0"), 10));
    ConstantInt* int_4 =
        ConstantInt::get(b.getContext(), APInt(32, StringRef("4"), 10));
    auto consValuePtr = GetElementPtrInst::Create(
        t::SEXPREC, sexp, std::vector<llvm::Value*>({int_0, int_4}), "", b.block());
    auto ptr = GetElementPtrInst::Create(t::SEXP_u1, consValuePtr,
                                         std::vector<llvm::Value*>({int_0, int_2}),
                                         "", b.block());
    auto value = new LoadInst(ptr, "", false, b.block());
    return new Tag(consValuePtr, ptr, value);
}

VectorGetElement* VectorGetElement::create(Builder & b, ir::Value vector,
                                           ir::Value index) {
    LLVMContext & c = b.getContext();
    ConstantInt* int64_1 = ConstantInt::get(c, APInt(64, StringRef("1"), 10));
    auto realVector = new BitCastInst(
        vector, PointerType::get(t::VECTOR_SEXPREC, 1), "", b.block());
    auto payload =
        GetElementPtrInst::Create(t::VECTOR_SEXPREC, realVector,
                                  std::vector<llvm::Value*>({int64_1}), "", b.block());
    auto payloadPtr =
        new BitCastInst(payload, PointerType::get(t::SEXP, 1), "", b.block());
    GetElementPtrInst* el_ptr =
        GetElementPtrInst::Create(t::SEXP, payloadPtr, { index }, "", b.block());
    auto res = new LoadInst(el_ptr, "", false, b.block());
    res->setAlignment(8);
    VectorGetElement* p = new VectorGetElement(realVector, payload, payloadPtr, el_ptr, res);
    return p;
}

} // namespace ir

} // namespace rjit
