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




#define jl_free free
#define jl_malloc malloc


class SharedBuffer {

	struct Shared {
		size_t count;
		size_t length;
		char buffer[1]; // first char of the buffer
	};
	Shared *_shared;

	void AddRef() {
		
		++_shared->count;
	}

	void DelRef() {

		if ( !--_shared->count )
			jl_free(_shared);
	}

public:
	~SharedBuffer() {
		
		DelRef();
	}

	SharedBuffer( size_t length ) {

		_shared = (Shared*)jl_malloc(sizeof(*_shared)-1 + length);
//		JL_ASSERT( _shared );
		_shared->count = 1;
		_shared->length = length;
	}

	SharedBuffer( const SharedBuffer &other ) {

		_shared = other._shared;
		AddRef();
	}

	const SharedBuffer & operator =( const SharedBuffer &other ) {

		DelRef();
		_shared = other._shared;
		AddRef();
		return *this;
	}

	size_t Length() const {
	
		return _shared->length;
	}

	char *Data() const {
	
		return _shared->buffer;
	}

private:
	SharedBuffer();
};



SharedBuffer Test() {

	SharedBuffer sb(100);
	SharedBuffer test(sb);

	return sb;
}


int main(int argc, char* argv[]) {

	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_JIT);

	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class);
	JS_InitStandardClasses(cx, globalObject);

	SharedBuffer sb1(Test());

	SharedBuffer sb2 = sb1;
	sb2 = sb1;
	sb1 = SharedBuffer(200);



	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

	return EXIT_SUCCESS;
}

