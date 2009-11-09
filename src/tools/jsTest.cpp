#define XP_WIN

#include <jsapi.h>
#include <cstring>

#include "../common/jlplatform.h"
#include "../common/jlhelper.h"

jl_malloc_t jl_malloc = malloc;
jl_calloc_t jl_calloc = calloc;
jl_memalign_t jl_memalign = memalign;
jl_realloc_t jl_realloc = realloc;
jl_msize_t jl_msize = msize;
jl_free_t jl_free = free;



int index = 1234;

static JSBool MyConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	
	return JS_TRUE;
}

JSBool MyProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	if ( *vp != JSVAL_VOID )
		return JS_TRUE;

//	JS_SetProperty(cx, obj, id, vp);
	//JS_SetProperty(cx, obj, "myProperty", vp);

	*vp = INT_TO_JSVAL(index);

//	JS_DefineProperty(cx, obj, "myProperty", *vp, NULL, NULL, 0);


	index++;
	return JS_TRUE;
}

JSBool Print(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	
	fputs(JS_GetStringBytes(JS_ValueToString(cx, argv[0])), stdout);
	return JS_TRUE;
}

JSBool JL_DefineClassProperties(JSContext *cx, JSObject *obj, JSPropertySpec *ps) {

	JSBool ok;
	for (ok = JS_TRUE; ps->name; ps++) {

		if ( ps->tinyid >= 0 ) {
			ok = JS_DefineProperty(cx, obj, ps->name, JSVAL_VOID, ps->getter, ps->setter, ps->flags);
			if (!ok)
				break;
		} else {
			ok = JS_DefinePropertyWithTinyId(cx, obj, ps->name, ps->tinyid, JSVAL_VOID, ps->getter, ps->setter, ps->flags);
			if (!ok)
				break;
		}
	}
	return ok;
}

bool _unsafeMode = false;

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

	JSClass myClass = { "MyClass", 0, JS_PropertyStub , JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub, JSCLASS_NO_OPTIONAL_MEMBERS };
	JSPropertySpec ps[] = { {"myProperty", 0, JSPROP_SHARED, MyProperty, NULL}, {NULL} };

	JS_InitClass(cx, globalObject, NULL, &myClass, MyConstructor, 0, ps, NULL, NULL, NULL);

	JLLoadScript(cx, globalObject, "test.js", true, false);


/*
	char scriptSrc[] =
"var oa = new MyClass; Print(oa.myProperty);   var ob = new MyClass; Print(oa.myProperty)"; // prints 11, expect 12

	JSScript *script = JS_CompileScript(cx, globalObject, scriptSrc, strlen(scriptSrc), "mytest", 1);
	jsval rval;
	JSBool st = JS_ExecuteScript(cx, globalObject, script, &rval);
*/
	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();
	return EXIT_SUCCESS;
}
