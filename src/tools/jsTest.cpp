#define XP_WIN
#include <jsapi.h>
#include "jsxdrapi.h"

JSClass global_class = {
	 "global", JSCLASS_GLOBAL_FLAGS, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
};

JSBool MyXDRObject(JSXDRState *xdr, JSObject **objp);

static JSClass myClass = { 
	"MyClass", 0, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	NULL, NULL, NULL, NULL, MyXDRObject, NULL, NULL, NULL
};

JSBool MyXDRObject(JSXDRState *xdr, JSObject **objp) {

	if ( xdr->mode == JSXDR_ENCODE ) {

		// ...
		return JS_TRUE;
	}

	if ( xdr->mode == JSXDR_DECODE ) {

		*objp = JS_NewObject(xdr->cx, &myClass, NULL, NULL);
		// ...
		return JS_TRUE;
	}

	return JS_TRUE;
}


int main(int argc, char* argv[]) {

	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_JIT);
	JSObject *globalObject = JS_NewObject(cx, &global_class, NULL, NULL);
	JS_InitStandardClasses(cx, globalObject);

	JS_InitClass(cx, globalObject, NULL, &myClass, NULL, 0, NULL, NULL, NULL, NULL);
	JSObject *obj1 = JS_NewObject(cx, &myClass, NULL, NULL);


	jsval val1 = OBJECT_TO_JSVAL( obj1 );
	jsval val2;

	JSXDRState *xdr1, *xdr2;
	
	xdr1 = JS_XDRNewMem(cx, JSXDR_ENCODE);
	JS_XDRValue(xdr1, &val1);
	uint32 length;
	void *buffer;
	buffer = JS_XDRMemGetData(xdr1, &length);
	
	xdr2 = JS_XDRNewMem(cx, JSXDR_DECODE);
	JS_XDRMemSetData(xdr2, buffer, length);
	JS_XDRValue(xdr2, &val2);
	JS_XDRMemSetData(xdr2, NULL, 0); // <- Access violation reading location 0x00000000.
	JS_XDRDestroy(xdr2);

	JS_XDRDestroy(xdr1);


	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

	return EXIT_SUCCESS;
}
