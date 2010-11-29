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


// char *test;
// JL_CHK( JL_Alloc(test,10);
template <typename T>
__forceinline bool JL_Alloc( T*&ptr, size_t count = 1 ) {

	ptr = (T*)malloc(sizeof(T)*count);
	return ptr != NULL;
}

#include <windows.h>

double AccurateTimeCounter() {

	static volatile LONGLONG initTime = 0; // initTime helps in avoiding precision waste.
	LARGE_INTEGER frequency, performanceCount;
	BOOL result = ::QueryPerformanceFrequency(&frequency);
	DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0); // manage bug in BIOS or HAL
	result = ::QueryPerformanceCounter(&performanceCount);
	if ( initTime == 0 )
		initTime = performanceCount.QuadPart;
	::SetThreadAffinityMask(::GetCurrentThread(), oldmask);
	return (double)1000 * (performanceCount.QuadPart-initTime) / (double)frequency.QuadPart;
}


int main(int argc, char* argv[]) {


	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetProcessAffinityMask(GetCurrentProcess(), 1);



	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_JIT);

	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class);
	JS_InitStandardClasses(cx, globalObject);

	JS_InitClass(cx, globalObject, NULL, js::Jsvalify(&jl_BlobClass), constructor, 0, NULL, NULL, NULL, NULL);


	JSObject *root = JS_NewObjectWithGivenProto(cx, NULL, NULL, NULL);
	int index = 0;

	jsval *value;
	
	double t, err;
	t = AccurateTimeCounter();
	err = AccurateTimeCounter() - t;
	t = AccurateTimeCounter();

	for ( int i = 0; i < 10000; i++ ) {

		value = (jsval*)malloc(sizeof(jsval));

		JS_SetPropertyById(cx, root, INT_TO_JSID(index++), value);
		//JS_AddValueRoot(cx, value);
	}


	t = AccurateTimeCounter() - t - err;

	printf("%f\n", t);



	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

	return EXIT_SUCCESS;
}
