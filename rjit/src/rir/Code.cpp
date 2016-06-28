
#include "Code.h"
#include "interp.h"

namespace rjit {
namespace rir {

// #define DEF_INSTR(name, imm, ...) case name : pc += sizeof(ArgT) * imm; break;
// #include "insns.h"

/** Serializes the rir::Code object into a Function.
 */

// TODO Bytecodes and indices should be unified

SEXP Code::toFunction() {
    // calculate the overall size required (in bytes)
    unsigned totalSize = 0;
    // size of the function header
    totalSize += sizeof(Function);
    totalSize += calcSize(totalSize);

    // create the vector
    assert(totalSize % sizeof (int) == 0 and "Are code objects properly padded?");
    SEXP result = Rf_allocVector(INTSXP, totalSize / sizeof (int));
    PROTECT(result);
    OpcodeT * code = reinterpret_cast<OpcodeT*>(INTEGER(result));

    // fill in the header
    ::Function * fHdr = reinterpret_cast<::Function *>(code);
    fHdr->magic = 0xCAFEBABE;
    fHdr->origin = nullptr; // this will be ptr to unoptimized version, but we do not optimize yet
    fHdr->size = totalSize;
    unsigned numInstr = codeSize(0); 
    fHdr->codeLength = numInstr + 1;
    fHdr++;

    // now serialize the codes, one by one, first itself, then all the children
    // keep in mind to update the instructions which deal push promise indices to update
    // them so that they push offsets from the beginning

    ::Code * codeObject = reinterpret_cast<::Code *>(fHdr);
    ::Code * codeO = createCode(codeObject);
    memcpy(codeObject, codeO, size);
    codeObject++;

    // calculate the padding - the type of the padding is unsigned
    unsigned * padding = reinterpret_cast<unsigned *>(codeObject);
    padding += pad4(size);
    padding++;

    // add the AST map - an array of size_t
    size_t * astMapping = reinterpret_cast<size_t *> (padding);
    size_t * astPos = createAst();
    memcpy(astMapping, astPos, sizeof(astPos));
    astMapping++;

    std::queue<Code *> que;
    for (Code * c : children){
        que.push(c);

        // Create the code
        // codeObject = reinterpret_cast<::Code *>(astMapping);
        // ::Code * tempCO = c->createCode();
        // memcpy(codeObject, tempCO, c->size);
        // tempCO++;
        // // Pad the code and the ast
        // padding = reinterpret_cast<unsigned *>(codeObject);
        // padding += pad4(c->size);
        // padding++;
        // // Create the AST
        // astMapping = reinterpret_cast<size_t *> (padding);
        // astPos = c->createAst();
        // memcpy(astMapping, astPos, sizeof(astPos));
        // astMapping++;
    }
    size_t * pointerRes = createTransform(que, astMapping);
    result = reinterpret_cast<SEXP>(pointerRes);

    // assigning to the result vector is achieved by using the memcpy function. 
    // memcpy(dest, src, totalSize);
    UNPROTECT(1);
    return nullptr;
}


size_t * Code::createBlock(size_t * transform){

    ::Code * codeObject = reinterpret_cast<::Code *>(transform);
    ::Code * codeO = createCode();
    memcpy(codeObject, codeO, size);
    codeObject++;

    // calculate the padding - the type of the padding is unsigned
    unsigned * padding = reinterpret_cast<unsigned *>(codeObject);
    padding += pad4(size);
    padding++;

    // add the AST map - an array of size_t
    size_t * astMapping = reinterpret_cast<size_t *> (padding);
    size_t * astPos = createAst();
    memcpy(astMapping, astPos, sizeof(astPos));
    astMapping++;

    return astMapping;
}

size_t * Code::createTransform(std::queue<Code *> que, size_t * transform){
    if(que.empty()){
        return transform;
    }

    Code * c = que.front();
    que.pop();
    transform = c->createBlock(transform);

    for (Code * code : c->children){
        que.push(code);
    }
    return createTransform(que, transform);
}

    // Create the Code object as defined in interp.h
    // from the code object generated from the CodeStream
::Code * Code::createCode(::Code * code){

    // unsigned header; /// offset to Function object // calculate back to the function header
    code->header = sizeof();
    // unsigned src; /// AST of the function (or promise) represented by the code
    code->src = ast;
    // unsigned codeSize; /// bytes of code (not padded)
    code->codeSize = size;
    // unsigned srcLength; /// number of instructions
    code->srcLength = instrSize;

    memcpy(code->data, bc, size);

    verifyStack(code);

    return code;
}

    // Create the ast map 
    // size and astMap->size are two very different things
    // size is the number of instructions
    // astMap->size is the number of instructions that have ast.
size_t * Code::createAst(){

    size_t * astPos;
    // the new src context
    Context * context = globalContext();
    for (size_t i = 0; i < instrSize; ++i){
        int flag = astInstr[i]; 
        if (flag && flag != -1){
            astPos[i] = src_pool_add(context, astMap.at(flag));
        } else {
            astPos[i] = 0; 
        }
    }
    return astPos;
}

// Used to count the number of code object in the function
size_t Code::codeSize(){

    size_t count = 0;
    count += children.size();

    for (Code * c: children){
        count += c->codeSize();
    }
    return count;
}

// Used to figure out the total size of all code object in the function
unsigned Code::calcSize(){
    unsigned totalSize += sizeof(::Code);
    totalSize += size;
    totalSize += pad4(sizeof(::Code)+size);
    totalSize += astMap.size;

    for (Code * c : children){
        totalSize += c->calcSize();
    }

    return totalSize;
}

} // namespace rir
} // namespace rjit

