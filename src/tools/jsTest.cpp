#include "../common/jlhelper.h"
#include "../common/buffer.h"

/*
#undef ALWAYS_INLINE
#define ALWAYS_INLINE
*/

#include <limits>

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
#include <vector>

template <class T>
class StlAlloc {
public:

	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef T*        pointer;
	typedef const T*  const_pointer;
	typedef T&        reference;
	typedef const T&  const_reference;
	typedef T         value_type;

	template <class U>
	struct rebind { typedef StlAlloc<U> other; };

	ALWAYS_INLINE StlAlloc() {};
	ALWAYS_INLINE StlAlloc(const StlAlloc&) {}

	ALWAYS_INLINE pointer allocate(size_type n, const void * = 0) {

		printf("* alloc %d\n", n);
		return (pointer)jl_malloc(n * sizeof(value_type));
	}
  
	ALWAYS_INLINE void deallocate(void* p, size_type n) {
	
		printf("* free %d\n", n);
		jl_free(p);
	}

	ALWAYS_INLINE pointer address(reference x) const { return &x; }

	ALWAYS_INLINE const_pointer address(const_reference x) const { return &x; }

	ALWAYS_INLINE StlAlloc<T>& operator=(const StlAlloc&) { return *this; }

	ALWAYS_INLINE void construct(pointer p, const T& val) { new ((T*) p) T(val); }

	ALWAYS_INLINE void destroy(pointer p) { p->~T(); }

	ALWAYS_INLINE size_type max_size() const { return size_t(-1); }

	template <class U>
	ALWAYS_INLINE StlAlloc(const StlAlloc<U>&) {}

	template <class U>
	ALWAYS_INLINE StlAlloc& operator=(const StlAlloc<U>&) { return *this; }
};

	
//#define log printf
#define log
	
	
#include "../common/stack.h"	
	

	struct abc {
		int _a, _b, _c;
		~abc() {
			log("destruct\n");
		}
		abc(int a, int b, int c) : _a(a), _b(b), _c(c) {
			log("construct(...)\n");
		}
		abc() {
			log("construct()\n");
		}
		abc( const abc &src ) : _a(src._a), _b(src._b), _c(src._c) {
			log("construct( const& )\n");
		}
		abc& operator =( const abc &src ) {
			_a = src._a;
			_b = src._b;
			_c = src._c;
			log("copy\n");
			return *this;
		}
		bool operator ==( const abc &src ) {
			log("copy\n");
			return _a == src._a && _b == src._b && _c == src._c;
		}
	};

	ALWAYS_INLINE bool iter1( abc &value ) {
			
		log("%d\n", value._a);
		return false; // do not cancel iteration
	}

template <class T>
void* NewBack(T &o) {

	o.push_back(T::value_type());
	return &o.back();
}


int main(int argc, char* argv[]) {


	jl::StaticBuffer<1024> b;
	b.Write("test", 1000);
	b.Write("test", 1000);



/*
	std::deque<abc> q;
	//new (&q.at(1)) abc(1,2,3);

	q.push_back(abc());
	new (NewBack(q)) abc(1,2,3);
*/


	jl::Stack<abc, jl::StaticAlloc<>> s;
	jl::Stack<abc> s1;


	new(++s1) abc(2,0,0);
	new(++s1) abc(2,0,1);
	new(++s) abc(1,0,0);

	s += s1;

	size_t myeip = JLIP();

	++s;
	new(++s) abc(1,1,1);
	--s;
	s->_a = 5;

	myeip = JLIP() - myeip;

	printf("**************** code length: %d\n", myeip);

	s->_c = 5;

	new(++s) abc(1,2,3);
	

	new(s) abc(4,5,6);

	new(s[0]) abc(7,8,9);

	s->_a;
	s[2]->_a;

	++s;
	++s;
	--s;
	++s;
	++s;
	--s;
	++s;
	++s;
	--s;
	--s;
	++s;
	--s;
	--s;
	++s;
	--s;
	--s;
	++s;

	


//	jl::Stack<int> *p = new jl::Stack<int>[5];
//	delete[] p;


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

	JSObject *o1 = JS_NewArrayObject(cx, 0, NULL);
	JSScopeProperty *jssp;
	jssp = NULL;
	JS_PropertyIterator(o1, &jssp);



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
