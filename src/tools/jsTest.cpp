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


class JLBufferInfo {
private:
	const uint8_t *_data;
	size_t _length;
public:
	JLBufferInfo() {};
	JLBufferInfo( const void *data, size_t count )
		:_data((const uint8_t *)data), _length(count) {
	}
	template <class T>
	JLBufferInfo( const T *data, size_t count )
		:_data((const uint8_t *)data), _length(sizeof(T)*count) {
	}

	const uint8_t *Data() const {
		
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

	const uint8_t *Data() const {
	
		return _start;
	}

	size_t Length() const {
	
		return _pos - _start;
	}

	JLSerializationBuffer& operator <<( const char *buf ) {

		size_t length = strlen(buf)+1;
		*this << length;
		ReserveBytes(length);
		memcpy(_pos, buf, length);
		_pos += length;
		return *this; 
	}

	JLSerializationBuffer& operator <<( const JLBufferInfo &buf ) {

		*this << buf.Length();
		ReserveBytes(buf.Length());
		memcpy(_pos, buf.Data(), buf.Length());
		_pos += buf.Length();
		return *this; 
	}

	template <class T>
	JLSerializationBuffer& operator <<( const T value ) {

		ReserveBytes(sizeof(T));
		*(T*)_pos = value;
		_pos += sizeof(T);
		return *this;
	}
};


class JLUnSerializationBuffer {

	const uint8_t *_start;
	const uint8_t *_pos;
	size_t _length;

public:

	JLUnSerializationBuffer( const uint8_t *data, size_t length )
		:_length(length), _start(data), _pos(_start) {
	}

	bool AssertData( size_t length ) const {
		
		return (_pos - _start) + length <= _length;
	}

	JLUnSerializationBuffer& operator >>( const char *&buf ) {

		size_t length;
		*this >> length;
		JL_ASSERT( AssertData(length) );
		buf = (const char*)_pos;
		_pos += length;
		return *this; 
	}

	JLUnSerializationBuffer& operator >>( JLBufferInfo &buf ) {

		size_t length;
		*this >> length;
		JL_ASSERT( AssertData(length) );
		buf = JLBufferInfo(_pos, length);
		_pos += length;
		return *this; 
	}

	template <class T>
	JLUnSerializationBuffer& operator >>( T &value ) {

		JL_ASSERT( AssertData(sizeof(T)) );
		value = *(T*)_pos;
		_pos += sizeof(T);
		return *this;
	}
};


enum JLSerializeType {

	JLTVoid = 0,
	JLTNull,
	JLTBool,
	JLTInt,
	JLTDouble,
	JLTString,
	JLTFunction,
	JLTArray,
	JLTObject,
	JLTObjectValue,
	JLTCustomObject,

};


class JLUnserializer {
private:
	JLUnSerializationBuffer buffer;

public:
	JLUnserializer( const uint8_t *data, size_t length )
		: buffer(data, length) {
	}

	JSBool Unserialize( JSContext *cx, jsval &val ) {

		unsigned char type;
		buffer >> type;

		switch (type) {
			case JLTVoid:
				val = JSVAL_VOID;
				break;
			case JLTNull:
				val = JSVAL_NULL;
				break;
			case JLTBool: {
				char b;
				buffer >> b;
				val = BOOLEAN_TO_JSVAL(b);
				break;
			}
			case JLTInt: {
				jsint i;
				buffer >> i;
				val = INT_TO_JSVAL(i);
				break;
			}
			case JLTDouble: {
				jsdouble d;
				buffer >> d;
				JL_CHK( JS_NewDoubleValue(cx, d, &val) );
				break;
			}
			case JLTString: {
				JLBufferInfo buf;
				buffer >> buf;
				JSString *jsstr;
				jsstr = JS_NewUCStringCopyN(cx, (const jschar *)buf.Data(), buf.Length()/2);
				val = STRING_TO_JSVAL(jsstr);
				break;
			}
			case JLTFunction: {
				JLBufferInfo buf;
				buffer >> buf;
				JSXDRState *xdr;
				xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
				JS_XDRMemSetData(xdr, (void*)buf.Data(), buf.Length());
				JL_CHK( JS_XDRValue(xdr, &val) );
				JS_XDRMemSetData(xdr, NULL, 0);
				JS_XDRDestroy(xdr);
				break;
			}
			case JLTArray: {

				jsuint length;
				buffer >> length;
				JSObject *arr;
				arr = JS_NewArrayObject(cx, length, NULL);
				val = OBJECT_TO_JSVAL(arr);

				jsval tmp;
				for ( jsuint i = 0; i < length; ++i ) {
					
					JL_CHK( Unserialize(cx, tmp) );
					JL_CHK( JS_SetElement(cx, arr, i, &tmp) );
				}
				break;
			}
			case JLTObject: {

				jsint length;
				buffer >> length;

				JSObject *obj = JS_NewObject(cx, NULL, NULL, NULL);
				val = OBJECT_TO_JSVAL(obj);

				const char *name;

				for ( int i = 0; i < length; ++i ) {
					
					buffer >> name;
					jsval value;
					JL_CHK( Unserialize(cx, value) );
					JL_CHK( JS_SetProperty(cx, obj, name, &value) );
				}
				break;
			}
			case JLTObjectValue: {

				const char *className;
				buffer >> className;
				jsval value;
				JL_CHK( Unserialize(cx, value) );
				JSObject *scope = JS_GetScopeChain(cx); // JS_GetGlobalForScopeChain
				jsval prop;
				JL_CHK( JS_GetProperty(cx, scope, className, &prop) );
				JSObject *newObj = JS_New(cx, JSVAL_TO_OBJECT(prop), 1, &value);
				val = OBJECT_TO_JSVAL(newObj);
				break;
			}
			case JLTCustomObject: {

				const char *className;
				buffer >> className;
				jsval value;
				JL_CHK( Unserialize(cx, value) );
				JSObject *scope = JS_GetScopeChain(cx); // JS_GetGlobalForScopeChain
				jsval prop;
				JL_CHK( JS_GetProperty(cx, scope, className, &prop) );
				JSObject *obj = JSVAL_TO_OBJECT(prop);
				jsval argv[] = { JSVAL_NULL, value };
				js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);

//				JS_GetMethod(cx, obj->getProto(), "_unserialize", NULL, &fctVal);

				//JL_CHK( JS_CallFunctionName(cx, obj->getProto(), "_unserialize", COUNTOF(argv-1), argv+1, argv) );
				val = *argv;
				break;
			}

		}

		return JS_TRUE;
		JL_BAD;
	}

};




class JLSerializer {
private:
	JLSerializationBuffer buffer;

public:

	JLSerializer()
		: buffer() {
	}

	const uint8_t *Data() const {
		
		return buffer.Data();
	}

	size_t Length() const {
		
		return buffer.Length();
	}

	JSBool Serialize( JSContext *cx, jsval val ) {

		unsigned char type;
		if ( JSVAL_IS_PRIMITIVE(val) ) {

			if ( JSVAL_IS_VOID(val) ) {
				
				type = JLTVoid;
				buffer << type;
			} else
			if ( JSVAL_IS_NULL(val) ) {
				
				type = JLTNull;
				buffer << type;
			} else
			if ( JSVAL_IS_BOOLEAN(val) ) {

				type = JLTBool;
				buffer << type << char(JSVAL_TO_BOOLEAN(val));
			} else
			if ( JSVAL_IS_INT(val) ) {
				
				type = JLTInt;
				buffer << type << JSVAL_TO_INT(val);
			} else
			if ( JSVAL_IS_DOUBLE(val) ) {
				
				type = JLTDouble;
				buffer << type << *JSVAL_TO_DOUBLE(val);
			} else
			if ( JSVAL_IS_STRING(val) ) {

				type = JLTString;
				JSString *jsstr = JSVAL_TO_STRING(val);
				buffer << type << JLBufferInfo(JS_GetStringChars(jsstr), JS_GetStringLength(jsstr));
			}
		} else { // !JSVAL_IS_PRIMITIVE

			JSObject *obj = JSVAL_TO_OBJECT(val);

			if ( JS_IsArrayObject(cx, obj) ) {
				
				type = JLTArray;
				jsuint length;
				JL_CHK( JS_GetArrayLength(cx, obj, &length) );
				buffer << type << length;

				jsval elt;
				for ( jsint i = 0; i < jl::SafeCast<jsint>(length); ++i ) {

					JL_CHK( JS_GetElement(cx, obj, i, &elt) );
					JL_CHK( Serialize(cx, elt) );
				}
			} else
			if ( JS_ObjectIsFunction(cx, obj) ) {
				
				type = JLTFunction;
				JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
				JL_CHK( JS_XDRValue(xdr, &val) );
				uint32 length;
				void *buf = JS_XDRMemGetData(xdr, &length);
				JL_ASSERT( buf );
				buffer << type << JLBufferInfo(buf, length);
				JS_XDRDestroy(xdr);
			} else {

				jsval fctVal;
//				JL_CHK( obj->getProperty(cx, JLID(cx, _Serialize), &fctVal) );
//				JL_CHK( JS_GetProperty(cx, obj, "_serialize", &fctVal) ); // JS_GetMethod(cx, obj, "_serialize", 
				JL_CHK( JS_GetMethod(cx, obj, "_serialize", NULL, &fctVal) );

				if ( fctVal != JSVAL_VOID ) {
					
					type = JLTCustomObject;
					JL_ASSERT( JsvalIsFunction(cx, fctVal) );
					jsval argv[] = { JSVAL_NULL };
					js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
					JL_CHK( JS_CallFunctionValue(cx, obj, fctVal, COUNTOF(argv-1), argv+1, argv) );
//					JSString *jsstr = JS_ValueToString(cx, *argv);
//					buffer << type << obj->getClass()->name << JLBufferInfo(JS_GetStringBytes(jsstr), JS_GetStringLength(jsstr));
					buffer << type << obj->getClass()->name;
					JL_CHK( Serialize(cx, *argv) );
				} else
				if ( JL_ObjectIsObject(cx, obj) ) {

					type = JLTObject;
					buffer << type;

					JSIdArray *idArray = JS_Enumerate(cx, obj);
					buffer << idArray->length;
					jsval name, value;
					for ( int i = 0; i < idArray->length; ++i ) {
						
						name = ID_TO_VALUE( idArray->vector[i] ); // JL_CHK( JS_IdToValue(cx, idArray->vector[i], &name) );
						JSString *jsstr = JSVAL_IS_STRING(name) ? JSVAL_TO_STRING(name) : JS_ValueToString(cx, name);
						buffer << JS_GetStringBytesZ(cx, jsstr);
						JL_CHK( obj->getProperty(cx, idArray->vector[i], &value) );
						JL_CHK( Serialize(cx, value) );
					}
					JS_DestroyIdArray(cx, idArray);
				} else {

					type = JLTObjectValue;
					buffer << type << obj->getClass()->name;
					jsval value;
					JL_CHK( obj->defaultValue(cx, JSTYPE_VOID, &value) );
					JL_CHK( Serialize(cx, value) );
				}
			}
		}
		
		return JS_TRUE;
		JL_BAD;
	}
};


void BufferTest() {


	JLSerializationBuffer s;
	s << "test";

	const char *buf;

	JLUnSerializationBuffer u(s.Data(), s.Length());
	u >> buf;
}



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
