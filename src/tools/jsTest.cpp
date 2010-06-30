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


class JLBuffer {
private:
	const void *_data;
	size_t _length;
public:
	template <class T>
	JLBuffer( T *data, size_t count ) : _data(data), _length(sizeof(T)*count) {}

	const void *Data() const {
		
		return _data;
	}

	size_t Length() const {
		
		return _length;
	}

};

class JLSerializationBuffer {

	uint8_t *_start;
	uint8_t *_pos;
	size_t _length;

	void ReserveBytes( size_t length ) {

		size_t offset = _pos - _start;
		if ( offset + length > _length ) {
			
			_length = _length * 2 + length;
			_start = (uint8_t*)jl_realloc(_start, _length);
			JL_ASSERT( _start != NULL );
			_pos = _start + offset;
		}
	}

public:
	~JLSerializationBuffer() {
		
		jl_free(_start);
	}

	JLSerializationBuffer() {

		_length = 4096;
		_start = (uint8_t*)jl_malloc(_length);
		JL_ASSERT( _start != NULL );
		_pos = _start;
	}

	uint8_t *Data() {
	
		return _start;
	}

	size_t Length() const {
	
		return _pos - _start;
	}

	template <class T>
	JLSerializationBuffer& operator <<( T value ) {

		ReserveBytes(sizeof(T));
		*(T*)_pos = value;
		_pos += sizeof(T);
		return *this;
	}

	JLSerializationBuffer& operator <<( JLBuffer &buf ) {

		*this << buf.Length();
		ReserveBytes(buf.Length());
		memcpy(_pos, buf.Data(), buf.Length());
		_pos += buf.Length();
		return *this; 
	}
};


class JLUnSerializationBuffer {

	uint8_t *_start;
	uint8_t *_pos;
	size_t _length;

public:

	JLUnSerializationBuffer( uint8_t *data, size_t length ) {

		_length = length;
		_start = data;
		_pos = _start;
	}

	bool AssertData( size_t length ) const {
		
		return (_pos - _start) + length <= _length;
	}

	template <class T>
	JLSerializationBuffer & operator >>( T &value ) {

		JL_ASSERT( !_serialize );
		JL_ASSERT( AssertData(sizeof(T)) );
		value = *(T*)_pos;
		_pos += sizeof(T);
		return *this;
	}


	template <class T>
	bool Process( T &value ) {
		
		if ( !AssertData(sizeof(T)) )
			return false;
		value = *(T*)_pos;
		_pos += sizeof(T);
		return true;
	}

	template <class T>
	bool Process( T *buffer, size_t count ) {
		
		size_t length = count * sizeof(T);
		if ( !AssertData(length) )
			return false;
		memcpy(buffer, _pos, length);
		_pos += length;
		return true;
	}
};



class JLSerializer {
private:
	JLSerializationBuffer buf;

public:

	JLSerializer( uint8_t *data, size_t length ) : buf() {
	}

	JSBool Process( JSContext *cx, jsval *val ) {

		if ( JSVAL_IS_VOID(*val) ) {
			
			buf << 0;
		}

		if ( JSVAL_IS_DOUBLE(*val) ) {
			
			buf << 1 << *JSVAL_TO_DOUBLE(*val);
		}


		if ( JSVAL_IS_OBJECT(*val) ) {

			JSObject *obj = JSVAL_TO_OBJECT(*val);

			jsval fctVal;
			JL_CHK( obj->getProperty(cx, JLID(cx, _Serialize), &fctVal) );
			if ( JsvalIsFunction(cx, fctVal) ) {

				//obj->getClass()->name
			} else {
				
				buf << JLBuffer(obj->getClass()->name, strlen(obj->getClass()->name));
			}
			
			return JS_TRUE;
			JL_BAD;
		}
	}
};



int main(int argc, char* argv[]) {

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

	uint8_t data[10];
	JLSerializer(data, 6);


return 0;




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
