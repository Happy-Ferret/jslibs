#define XP_WIN

#ifdef _MSC_VER
#pragma warning( push, 1 )
#endif // _MSC_VER

#include <jsapi.h>
#include <jsvalue.h>

#include <jsproxy.h>
#include <jswrapper.h>

#ifdef _MSC_VER
#pragma warning( pop )
#endif // _MSC_VER

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

const jschar *ToString(JSContext *cx, jsval val) {
	
	JSString *str = JS_ValueToString(cx, val);
	return JS_GetStringCharsZ(cx, str);
}


const jschar *ToString(JSContext *cx, jsid id) {
	
	jsval val;
	JS_IdToValue(cx, id, &val);
	return ToString(cx, val);
}



class BlobProxyHandler : public js::JSProxyHandler {

public:

	BlobProxyHandler() : js::JSProxyHandler(NULL) {
	}

	virtual ~BlobProxyHandler() {
	}

private:

	bool getPropertyDescriptor(JSContext *cx, JSObject *proxy, jsid id, bool set, js::PropertyDescriptor *desc) {

		wprintf(L"getPropertyDescriptor %s ", ToString(cx, id));

		JSObject *obj = &proxy->getProxyPrivate().toObject();

		if ( !JS_GetPropertyDescriptorById(cx, obj, id, JSRESOLVE_QUALIFIED, Jsvalify(desc)) )
			return false;

		wprintf(L":= %s\n", ToString(cx, Jsvalify(desc->value)));
		return true;
	}

	bool getOwnPropertyDescriptor(JSContext *cx, JSObject *proxy, jsid id, bool set, js::PropertyDescriptor *desc) {

		wprintf(L"getOwnPropertyDescriptor %s ", ToString(cx, id));

		JSObject *obj = &proxy->getProxyPrivate().toObject();

		if ( !JS_GetPropertyDescriptorById(cx, obj, id, JSRESOLVE_QUALIFIED, Jsvalify(desc)) )
			return false;

		wprintf(L":= %s\n", ToString(cx, Jsvalify(desc->value)));

		if (desc->obj != obj)
			desc->obj = NULL;
		return true;
	}

	bool defineProperty(JSContext *cx, JSObject *proxy, jsid id, js::PropertyDescriptor *desc) {

		wprintf(L"defineProperty %s ", ToString(cx, id));

		JSObject *obj = &proxy->getProxyPrivate().toObject();
		if ( !JS_DefinePropertyById(cx, obj, id, js::Jsvalify(desc->value), js::Jsvalify(desc->getter), js::Jsvalify(desc->setter), desc->attrs) )
			return false;

		wprintf(L":= %s\n", ToString(cx, Jsvalify(desc->value)));

		return true;
	}

	bool getOwnPropertyNames(JSContext *cx, JSObject *proxy, js::AutoIdVector &props) { 
		return true;
	}

	bool delete_(JSContext *cx, JSObject *proxy, jsid id, bool *bp) {
		return true;
	}

	bool enumerate(JSContext *cx, JSObject *proxy, js::AutoIdVector &props) {
		return true;
	}

	bool fix(JSContext *cx, JSObject *proxy, js::Value *vp) {
		return true;
	}
/*
	bool get(JSContext *cx, JSObject *proxy, JSObject *receiver, jsid id, js::Value *vp) {
		
		JSObject *obj = &proxy->getProxyPrivate().toObject();
		return JS_GetPropertyById(cx, obj, id, js::Jsvalify(vp));
	}
*/
	bool set(JSContext *cx, JSObject *proxy, JSObject *receiver, jsid id, js::Value *vp) {
		
		JSObject *obj = &proxy->getProxyPrivate().toObject();
		return JS_SetPropertyById(cx, obj, id, js::Jsvalify(vp));
	}

};


int main(int argc, char* argv[]) {

	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_JIT);

	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class);
	JS_InitStandardClasses(cx, globalObject);

	JS_InitClass(cx, globalObject, NULL, js::Jsvalify(&jl_BlobClass), constructor, 0, NULL, NULL, NULL, NULL);


	JSObject *arr;
	arr = JS_NewArrayObject(cx, 0, NULL);

	JSPropertyDescriptor desc;
	JS_GetPropertyDescriptorById(cx, arr, ATOM_TO_JSID(rt->atomState.lengthAtom), 0, &desc);

//	JS_GetPropertyAttrsGetterAndSetterById(cx, arr, ATOM_TO_JSID(rt->atomState.lengthAtom), 

	//JS_GetPropertyById(cx, arr, ATOM_TO_JSID(rt->atomState.lengthAtom), &tmp);





	BlobProxyHandler bph;
	JSObject *proxy = js::NewProxyObject(cx, &bph, js::Valueify(OBJECT_TO_JSVAL(arr)), JS_GetPrototype(cx, arr), JS_GetParent(cx, JS_GetPrototype(cx, arr)));
		

	double tmp;
	tmp = DOUBLE_TO_JSVAL(1.234);
//	JS_CallFunctionName(cx, proxy, "push", 1, &tmp, &tmp2);
//	JS_CallFunctionName(cx, proxy, "push", 1, &tmp, &tmp2);

	JS_GetProperty(cx, proxy, "length", &tmp);
	const jschar *str = JS_GetStringCharsZ(cx, JS_ValueToString(cx, tmp));

	wprintf(L"length = %s\n", str);

	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

	return EXIT_SUCCESS;
}
