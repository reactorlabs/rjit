#include "interpreter.h"

CodeObject * begin(FunctionObject * f) {
    return (CodeObject *)(f + 1);
}

CodeObject * end(FunctionObject * f) {
    return (CodeObject *)((char *)f + f->size);
}

FunctionObject * function(CodeObject * c) {
    return (FunctionObject *)((char *)c - c->offset);
}

BC_t * code(CodeObject * c) {
    return (BC_t *)(c + 1);
}

unsigned * debugInfo(CodeObject *c) {
    return (unsigned *)(c + 1) + sizeInInts(c->size);
}

CodeObject * next(CodeObject * c) {
   return (CodeObject *)((char*)(c + 1) + sizeInInts(c->size) * 4 + c->numInsns);
}

unsigned sizeInInts(unsigned sizeInBytes) {
    unsigned x = sizeInBytes % 4;
    if (x != 0)
        sizeInBytes += (4 - x);
    return sizeInBytes / 4;
}
