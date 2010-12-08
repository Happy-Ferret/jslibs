#include "malloc.h"

#include "jsapi.h"
#include "jsscript.h"
#include "jscntxt.h"
#include "jsarena.h"
#include "jsemit.h"

JS_BEGIN_EXTERN_C
void* (*custom_malloc)( size_t ) = malloc;
void* (*custom_calloc)( size_t, size_t ) = calloc;
void* (*custom_realloc)( void*, size_t ) = realloc;
void (*custom_free)( void* ) = free;
JS_END_EXTERN_C


static const jsbytecode emptyScriptCode[] = {JSOP_STOP, SRC_NULL};

/* static */ const JSScript JSScript::emptyScriptConst = {
    JS_INIT_STATIC_CLIST(NULL),
    const_cast<jsbytecode*>(emptyScriptCode),
    1, JSVERSION_DEFAULT, 0, 0, 0, 0, 0, 0, 0, true, false, false, false, false,
    false,      /* usesEval */
    false,      /* usesArguments */
    true,       /* warnedAboutTwoArgumentEval */
#ifdef JS_METHODJIT
    false,      /* debugMode */
#endif
    const_cast<jsbytecode*>(emptyScriptCode),
    {0, jsatomid(0)}, NULL, NULL, 0, 0, 0,
    0,          /* nClosedArgs */
    0,          /* nClosedVars */
    NULL, {NULL},
#ifdef CHECK_SCRIPT_OWNER
    reinterpret_cast<JSThread*>(1)
#endif
};


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


#include "jsapi-tests/tests.h"
#include "jsscript.h"
#include "jsxdrapi.h"

#include "jsfun.h"

BEGIN_TEST(soubokTest)
{
    const char *s = "(function() { return {} })";

    // compile
    JSScript *script = JS_CompileScript(cx, global, s, strlen(s), __FILE__, __LINE__);
    CHECK(script);
    JSObject *scrobj = JS_NewScriptObject(cx, script);
    CHECK(scrobj);
    jsvalRoot v(cx, OBJECT_TO_JSVAL(scrobj));

    jsvalRoot res(cx);
    CHECK(JS_ExecuteScript(cx, global, script, res.addr()));

	CHECK(VALUE_IS_FUNCTION(cx, res.value()));

    // freeze
    JSXDRState *w = JS_XDRNewMem(cx, JSXDR_ENCODE);
    CHECK(w);
    CHECK(JS_XDRValue(w, res.addr()));

    uint32 nbytes;
    void *p = JS_XDRMemGetData(w, &nbytes);
    CHECK(p);
    void *frozen = JS_malloc(cx, nbytes);
    CHECK(frozen);
    memcpy(frozen, p, nbytes);
    JS_XDRDestroy(w);

    return true;
}
END_TEST(soubokTest)
