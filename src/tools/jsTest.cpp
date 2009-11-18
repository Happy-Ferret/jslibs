#define XP_WIN
#include <jsapi.h>
#include <jsdbgapi.h>
#include <jscntxt.h>
#include <jsscope.h>
#include <cstring>

JSBool Print(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	
	fputs(JS_GetStringBytes(JS_ValueToString(cx, argv[0])), stdout);
	return JS_TRUE;
}

JSBool EvalVarByName(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	
	JS_ASSERT( JSVAL_IS_STRING(argv[0]) );
	
	const char *name = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	JS_ASSERT( name );


	JSStackFrame *fp = js_GetTopStackFrame(cx); //JS_GetScriptedCaller(cx, NULL);
	JS_ASSERT( fp );

	JS_FrameIterator(cx, &fp);

//	fp = JS_GetScriptedCaller(cx, fp);

	JSBool found;
	JSObject *scope = JS_GetFrameScopeChain(cx, fp);
	for ( ; scope; scope = JS_GetParent(cx, scope) ) {
/*
		JSScopeProperty *it = NULL;
		JS_PropertyIterator(scope, &it);
		const char *n;
		for ( ; it; JS_PropertyIterator(scope, &it) ) {
			
			JSPropertyDesc desc;
			JS_GetPropertyDesc(cx, scope, it, &desc);
			n = JS_GetStringBytes(JS_ValueToString(cx, desc.id));
		}
*/

		JS_HasProperty(cx, scope, name, &found);
		if ( found )
			return JS_GetProperty(cx, scope, name, rval);
	
	}

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

int main(int argc, char* argv[]) {

	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);

	JSObject *globalObject = JS_NewObject(cx, NULL, NULL, NULL);
	JS_SetGlobalObject(cx, globalObject);
	JS_InitStandardClasses(cx, globalObject);

	JS_SetVersion(cx, (JSVersion)JSVERSION_LATEST);
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_VAROBJFIX);

	JS_DefineFunction(cx, globalObject, "Print", Print, 0, 0);
	JS_DefineFunction(cx, globalObject, "EvalVarByName", EvalVarByName, 0, 0);

	char scriptSrc[] =
"var a = 'a'; function foo() { var b = 'b'; function bar() { var c = 'c'; Print( EvalVarByName('a')+' '+EvalVarByName('b')+' '+EvalVarByName('c')+'\\n' ); }; bar();};foo();";

	JSScript *script = JS_CompileScript(cx, globalObject, scriptSrc, strlen(scriptSrc), "mytest", 1);
	JS_ASSERT( script );

	jsval rval;
	JSBool st = JS_ExecuteScript(cx, globalObject, script, &rval);
	JS_ASSERT( st );

	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

	return EXIT_SUCCESS;
}
