#ifndef RIR_CODE
#define RIR_CODE

#include <cassert>
#include <map>

#include <queue>
#include "CodeVerifier.h"
#include "BC_inc.h"

namespace rjit {
namespace rir {

// CodeObject, holds a bytecode array and the associated debug information
// Use CodeStream to build bytecode
//
class Code {
  public:
    /** Calling convention.
     */
    enum class CC : char {
        envLazy,
        stackLazy,
        stackEager,
    };

  private:
    friend class RBytecode;
    friend class CodeStream;

    class AstMap {

        friend class RBytecode;

        size_t size;
        unsigned* pos;
        SEXP* ast;
        // Indice is only set when a new AstMap is created
        // this probably should create, i.e. in CodeStream.h
        // the indice will be updated when a new ast is added in

      public:
        AstMap& operator=(AstMap&& from) {
            delete pos;
            delete ast;
            size = from.size;
            pos = from.pos;
            ast = from.ast;
            from.size = 0;
            from.pos = nullptr;
            from.ast = nullptr;
            return *this;
        }

        AstMap(int size, unsigned* pos, SEXP* ast)
            : size(size), pos(pos), ast(ast) {}

        AstMap(std::map<unsigned, SEXP>& astMap) {
            size = astMap.size();
            pos = new unsigned[size];
            ast = new SEXP[size];
            unsigned i = 0;
            for (auto e : astMap) {
                pos[i] = e.first;
                ast[i] = e.second;
            }
        }

        ~AstMap() {
            delete pos;
            delete ast;
        }

        SEXP at(unsigned p) {
            if (size == 0)
                return nullptr;

            size_t f = 0;

            while (f < size && pos[f] < p)
                f++;

            if (pos[f] != p)
                return nullptr;

            return ast[f];
        }

        size_t astSize(){
            return size;
        }
    };

    Code(size_t size, BC_t* bc, SEXP ast, size_t astSize, unsigned* astPos,
         SEXP* astAst)
        : size(size), bc(bc), ast(ast), astMap(astSize, astPos, astAst) {}

  public:
    size_t size;
    BC_t* bc;
    SEXP ast;
    AstMap astMap;
    size_t instrSize;
    std::vector<int> astInstr;

    /** Promises used in the code object. */
    std::vector<Code*> children;

    Code() : size(0), bc(nullptr), ast(nullptr), astMap(0, nullptr, nullptr), instrSize(0) {}

    Code(size_t size, BC_t* bc, SEXP ast, std::map<unsigned, SEXP>& astMap, size_t instrSize, std::vector<int> astInstr)
        : size(size), bc(bc), ast(ast), astMap(astMap), instrSize(instrSize) {this->astInstr = astInstr;}
    ~Code() { delete bc; }

    void print();

    BC_t* end() { return (BC_t*)((uintptr_t)bc + size); }

    SEXP getAst(BC_t* pc) { return astMap.at((uintptr_t)pc - (uintptr_t)bc); }

    fun_idx_t next() {
        children.push_back(nullptr);
        assert(children.size() < MAX_FUN_IDX);
        return children.size() - 1;
    }

    void addCode(fun_idx_t pos, Code* c) {
        assert(pos < children.size() and children[pos] == nullptr);
        children[pos] = c;
    }

    Code& operator=(Code&& from) {
        delete bc;
        size = from.size;
        instrSize = from.instrSize;
        bc = from.bc;
        ast = from.ast;
        astMap = std::move(from.astMap);
        children = std::move(from.children);
        astInstr = std::move(from.astInstr);
        from.size = 0;
        from.bc = nullptr;
        return *this;
    }

    /** Serializes the code object into a Function SEXP.
     */
    SEXP toFunction();

    unsigned calcSize(unsigned size);

    ::Code * createCode();

    size_t codeSize(size_t count);

    size_t * createAst();

    size_t * createTransform(std::queue<Code*> que, size_t* transform);

    size_t * createBlock(size_t* transform);

};
}
}
#endif
