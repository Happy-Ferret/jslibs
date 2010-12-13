#define XP_WIN
#include <jsapi.h>
#include <jsvalue.h>
#include <string.h>

JSClass global_class = {
	 "global", JSCLASS_GLOBAL_FLAGS, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
};

int count = 0;

static JSBool next(JSContext *cx, uintN argc, jsval *vp) {

	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	if ( ++count == 5 )
		return JS_ThrowStopIteration(cx);
	return JS_TRUE;
}

static JSObject* IteratorObject(JSContext *cx, JSObject *obj, JSBool keysonly) {

	JSObject *itObj = JS_NewObject(cx, NULL, NULL, NULL);
	JS_DefineFunction(cx, itObj, "next", next, 0, 0);
	count = 0;
	return itObj;
}

JSBool constructor( JSContext *cx, uintN argc, jsval *vp ) {

	JSObject *obj;
	obj = JS_NewObjectForConstructor(cx, vp);
	if ( obj == NULL )
		return JS_FALSE;
	JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}

js::Class jl_BlobClass = {
    "Blob",
	0,
	js::PropertyStub,   /* addProperty */
	js::PropertyStub,   /* delProperty */
	js::PropertyStub,   /* getProperty */
	js::PropertyStub,   /* setProperty */
	js::EnumerateStub,
	js::ResolveStub,
	js::ConvertStub,
    NULL,
    NULL,           /* reserved0   */
    NULL,           /* checkAccess */
    NULL,           /* call        */
    NULL,           /* construct   */
    NULL,           /* xdrObject   */
    NULL,           /* hasInstance */
    NULL,           /* mark        */
    {
		NULL,
		NULL,
		NULL,
		IteratorObject,
		NULL
	}
};


#define FUNDEF(ret, name) ret name

FUNDEF(JSBool, f1)(JSContext *cx, int i) {
	
	return JS_TRUE;
}



/*
template <
#define DEF(N) typename N##_t,
#include "exportlist.h"
#undef DEF
typename END
>
struct api {
#define DEF(N) N##_t N;
#include "exportlist.h"
#undef DEF
} api;
*/


#define DLLEXPORT


//Is it possible for a DLL to access some symbols of the application that loaded it ?


//I need to export some API functions from an application (exe) to its plugins (dll) without using a intermediate dll to export application's API.
//My idea is to create a struct that contain all API function that my application want to export.


#define EXP(x,y) x y

// in the app.
EXP(bool ApiFun1(int arg1, bool b), {
	return true;
})

bool ApiFun2(double arg1) {
	return true;
}

class HostApi {
public:
	virtual bool Log(const char *) = 0;
};


HostApi& GetHostApi();

int main(int argc, char* argv[]) {

	HostApi &api = GetHostApi();

	api.Log("123");

	HostApi *d = &api;



	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_JIT);
	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class);
	JS_InitStandardClasses(cx, globalObject);

/*
	JS_InitClass(cx, globalObject, NULL, js::Jsvalify(&jl_BlobClass), constructor, 0, NULL, NULL, NULL, NULL);
	jsval rval;
	char *script = 
		"for ( var i = 0; i < 2; i++ )"
		"  [ 0 for ( it in Blob() ) ];";
	if ( !JS_EvaluateScript(cx, globalObject, script, strlen(script), "<inline>", 0, &rval) )
		return EXIT_FAILURE;
*/

	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

	return EXIT_SUCCESS;
}
