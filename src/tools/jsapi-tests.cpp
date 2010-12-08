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

#include "jsapi-tests/tests.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


#include "jsxdrapi.h"

JSBool soubokTest_fct( JSContext *cx, uintN argc, jsval *vp ) {

	JSXDRState *w = JS_XDRNewMem(cx, JSXDR_ENCODE);
	JSFunction *fun = JS_ValueToFunction(cx, *JS_ARGV(cx, vp));
	JSScript *funScript = FUN_SCRIPT(fun);
	JS_XDRScript(w, &funScript);
	return JS_TRUE;
}

BEGIN_TEST(soubokTest)
{
	CHECK( JS_DefineFunction(cx, global, "fct", soubokTest_fct, 0, NULL) );
	EXEC("fct( function() { return [] } )");
	EXEC("fct( function() { return {} } )");
	return true;
}
END_TEST(soubokTest)
