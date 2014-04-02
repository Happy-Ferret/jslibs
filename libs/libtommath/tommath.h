#include "./src/tommath.h"

#undef XMALLOC
#undef XREALLOC
#undef XCALLOC
#undef XFREE

#define XMALLOC jl_malloc_fct
#define XCALLOC jl_calloc_fct
#define XREALLOC jl_realloc_fct
#define XFREE jl_free_fct

extern void *XMALLOC(size_t n);
extern void *XREALLOC(void *p, size_t n);
extern void *XCALLOC(size_t n, size_t s);
extern void XFREE(void *p);

