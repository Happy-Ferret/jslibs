# ifdef DEBUG
extern JS_PUBLIC_DATA(uint32_t) OOM_maxAllocations; /* set from shell/js.cpp */
extern JS_PUBLIC_DATA(uint32_t) OOM_counter; /* data race, who cares. */
#  define JS_OOM_POSSIBLY_FAIL() \
    do \
    { \
        if (OOM_counter++ >= OOM_maxAllocations) { \
            return NULL; \
        } \
    } while (0)

# else
#  define JS_OOM_POSSIBLY_FAIL() do {} while(0)
# endif


static JS_INLINE void* js_malloc(size_t bytes)
{
    JS_OOM_POSSIBLY_FAIL();
    return malloc(bytes);
}

static JS_INLINE void* js_calloc(size_t bytes)
{
    JS_OOM_POSSIBLY_FAIL();
    return calloc(bytes, 1);
}

static JS_INLINE void* js_realloc(void* p, size_t bytes)
{
    JS_OOM_POSSIBLY_FAIL();
    return realloc(p, bytes);
}

static JS_INLINE void js_free(void* p)
{
    free(p);
}
