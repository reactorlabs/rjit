#ifndef API_H_
#define API_H_

#define REXPORT extern "C"

extern int RJIT_COMPILE;
extern int R_ENABLE_JIT;
extern int RJIT_DEBUG;

// For inlining
extern int OSR_INLINE;
extern int INLINE_ALL;

#endif // API_H_
