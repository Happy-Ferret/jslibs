#define NOMINMAX
#include <limits>

#define XP_WIN
#include <jsapi.h>
#include "jsxdrapi.h"

#include "../common/jlhelper.h"


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



#define ASSERT JL_ASSERT
/*
	template <class D, class S>
	inline D SafeConvert( S value );

	template <class S>
	inline int SafeConvert<int, S>( S value ) {
			
		ASSERT( value >= S(INT_MIN) && value <= S(INT_MAX) );
		return int(value);
	} /// error C2768: 'SafeConvert' : illegal use of explicit template arguments


	template <class S>
	inline size_t SafeConvert<size_t, S>( S value ) {
			
		ASSERT( value >= S(0) && value <= S(size_t(-1)) );
		return size_t(value);
	} /// error C2768: 'SafeConvert' : illegal use of explicit template arguments

*/

template <class D, class S>
D SafeConvert(S src) {

//#pragma warning(push)
//#pragma warning(disable:4018)

	ASSERT( src <= S(std::numeric_limits<D>::max()) );
	ASSERT( std::numeric_limits<D>::min() >= 0 || src >= 0 );

	ASSERT( std::numeric_limits<D>::min() < 0 || src >= 0 );


//	ASSERT(value >= std::numeric_limits<D>::min() && D(value) <= std::numeric_limits<D>::max());
//#pragma warning(pop)
  return static_cast<D>(src);
}
	// ...

	void test() {


		SafeConvert<size_t>(  (int)1  );

		SafeConvert<int>(  (size_t)UINT_MAX  );

		SafeConvert<int>(  __int64(INT_MIN)-0  );
		SafeConvert<int>(  __int64(INT_MAX)+0  );
		SafeConvert<int>(  size_t(INT_MAX)+0  );





		SafeConvert<int>(  size_t(INT_MAX)+1  );
		SafeConvert<int>(  size_t(0)  );
		SafeConvert<int>(  __int64(INT_MIN)-1  );
		SafeConvert<int>(  __int64(INT_MAX)+1  );
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

	
	test();

/*
	try {

		try {
			int i = jl::JsvalTo<int>(cx, jl::JsvalFrom<double>(cx, 1234));
		} catch ( jl::JSAPIError err ) {}

	} catch ( jl::JSAPIError err ) {}
*/

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
