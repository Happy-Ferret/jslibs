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


#include <../common/jslibsModule.cpp>


#include <deque>

template <class T>
class my_allocator {
public:

	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef T*        pointer;
	typedef const T*  const_pointer;
	typedef T&        reference;
	typedef const T&  const_reference;
	typedef T         value_type;

	my_allocator() {};
	my_allocator(const my_allocator&) {}

	inline pointer allocate(size_type n, const void * = 0) {

		T* t = (T*) malloc(n * sizeof(T));
		return t;
	}
  
	inline void deallocate(void* p, size_type) {
	
		if (p)
			free(p);
	}

	inline pointer address(reference x) const { return &x; }

	inline const_pointer address(const_reference x) const { return &x; }

	inline my_allocator<T>& operator=(const my_allocator&) { return *this; }

	inline void construct(pointer p, const T& val) { new ((T*) p) T(val); }

	inline void destroy(pointer p) { p->~T(); }

	inline size_type max_size() const { return size_t(-1); }

	template <class U>
	struct rebind { typedef my_allocator<U> other; };

	template <class U>
	inline my_allocator(const my_allocator<U>&) {}

	template <class U>
	inline my_allocator& operator=(const my_allocator<U>&) { return *this; }
};

	
	
	
	
#include "../common/stack.h"	
	

  		bool iter1( int &value ) {
				
			printf("%d\n", value);
			return false; // do not cancel iteration
		}
	
int main(int argc, char* argv[]) {


	std::deque<int, my_allocator<int>> test;

	test.push_back(10);
	test.push_back(11);
	test.push_back(12);


	jl::Stack<int> s;

	*++s = 1;
	*++s = 2;
	*++s = 3;
	*++s = 4;
	*++s = 5;

	s.Revert();


	int x;

	x = *s;
	x = *--s;
	x = *--s;
	x = *--s;
	x = *--s;
	x = *--s;
	x = *--s;




/*
	JLSerializationBuffer ser;

	wchar_t buf[] = L"Hello";
	char value = -1;
	//ser.Process(value);
	ser << value;
	value = 123;
	ser.Process(value);
	ser.Process(buf,5);

	JLSerializationBuffer ser2(ser.Data(), ser.Length());

	char xxx;
	ser2 >> xxx;
	ser2.Process(xxx);

	wchar_t buf2[6];
	ser2.Process(buf2,5);
	ser2.Process(xxx);
*/

	
//	uint8_t data[10];



	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_JIT);
	JSObject *globalObject = JS_NewObject(cx, &global_class, NULL, NULL);
	JS_InitStandardClasses(cx, globalObject);

	


	JS_InitClass(cx, globalObject, NULL, &myClass, NULL, 0, NULL, NULL, NULL, NULL);


	JSObject *obj1 = JS_NewObject(cx, &myClass, NULL, NULL);



	JSObject *o = JS_NewObject(cx, NULL, NULL, NULL);


	JSString *tmpstr = JS_NewStringCopyZ(cx, "Hello world");
	jsval tmp = STRING_TO_JSVAL(tmpstr);
/*
	JLSerializer s1;
	s1.Serialize(cx, tmp);
	JLUnserializer	u1(s1.Data(), s1.Length() );
	jsval r1;
	u1.Unserialize(cx, r1);


	JS_SetProperty(cx, o, "test", &tmp);

	tmp = OBJECT_TO_JSVAL(o);
*/


//	BufferTest();


/*
	jsval rval;
//	char *script = "({ o:123, p:new Date(), c:function() {}, d:new Number(123.567), e:{ _Serialize:function() { return '99999' } }   })";
	char *script = "({ _serialize:function() { return '99999' }   })";
	JS_EvaluateScript(cx, globalObject, script, strlen(script), "<inline>", 0, &rval);

	JLSerializer ser;
	ser.Serialize(cx, rval);
	JLUnserializer	unser(ser.Data(), ser.Length() );
	jsval res;
	unser.Unserialize(cx, res);
*/

	jsval rval;
	char *script = "Class1.prototype = { _serialize:function(){} }; function Class1() { }; new Class1()";
	JS_EvaluateScript(cx, globalObject, script, strlen(script), "<inline>", 0, &rval);

	jsval v;
	JS_GetMethod(cx, JSVAL_TO_OBJECT(rval), "_serialize", NULL, &v);








//	JS_CallFunctionName(cx, JSVAL_TO_OBJECT(res), "toSource", 0, NULL, &res);
//	const char * str = JS_GetStringBytesZ(cx, JS_ValueToSource(cx, res));
//	printf( "%s\n", str );




/*
	try {

		try {
			int i = jl::JsvalTo<int>(cx, jl::JsvalFrom<double>(cx, 1234));
		} catch ( jl::JSAPIError err ) {}

	} catch ( jl::JSAPIError err ) {}
*/

/*

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
*/

	return EXIT_SUCCESS;
}


/***
// The following function wil only works if the class is defined in the global namespace (say global object)
inline JSClass *GetGlobalClassByName( JSContext *cx, const char *className ) {

	// see.  js_FindClassObject(cx, NULL, INT_TO_JSID(JSProto_StopIteration), &v)) / JS_GetClassObject

	JSObject *globalObj = JS_GetGlobalObject(cx);
	if ( globalObj == NULL )
		return NULL;
	jsval classConstructor;
	if ( JS_LookupProperty(cx, globalObj, className, &classConstructor) != JS_TRUE )
		return NULL;
	if ( JsvalIsFunction(cx, classConstructor) ) {

		JSFunction *fun = JS_ValueToFunction(cx, classConstructor);
		if ( fun == NULL )
			return NULL;
		if ( !FUN_SLOW_NATIVE(fun) )
			return NULL;
		return fun->u.n.u.clasp; // return fun->u.n.clasp; // (TBD) replace this by a jsapi.h call and remove dependency to jsarena.h and jsfun.h
	} else
	if ( JSVAL_IS_OBJECT(classConstructor) ) {

		return OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(classConstructor));
	}
	return NULL;
}

***/
