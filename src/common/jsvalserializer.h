/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */

#include "jsbool.h"

#define JL_JSVAL_IS_ARRAY_HOLE(val) (js::Valueify(val).isMagic(JS_ARRAY_HOLE))
#define JL_JSVAL_SET_ARRAY_HOLE(val) (js::Valueify(val).setMagic(JS_ARRAY_HOLE))

namespace jl {

	class SerializerConstBufferInfo {
	private:
		const void *_data;
		size_t _length;
	public:
		SerializerConstBufferInfo() {}

		SerializerConstBufferInfo( const void *data, size_t length ) : _data(static_cast<const void *>(data)), _length(length) {}

		template <class T>
		SerializerConstBufferInfo( const T *data, size_t count ) : _data(static_cast<const void *>(data)), _length(sizeof(T)*count) {}

		const void * Data() const {

			return _data;
		}

		size_t Length() const {

			return _length;
		}
	};

/*
	class SerializerBufferInfo {
	private:
		void *_data;
		size_t _length;
	public:
		SerializerBufferInfo() {}

		SerializerBufferInfo( void *data, size_t length ) : _data(data), _length(length) {}

		template <class T>
		SerializerBufferInfo( T *data, size_t count ) : _data((void*)data), _length(sizeof(T)*count) {}

		void * Data() const {

			return _data;
		}

		size_t Length() const {

			return _length;
		}
	};
*/

	class SerializerObjectProperties {
		JSObject *&_obj;
	public:
		SerializerObjectProperties( JSObject *&obj ) : _obj(obj) {
		}

		operator JSObject*() const {

			return _obj;
		}
	};


	enum JLSerializeType : char {

		JLSTHole,
		JLSTVoid,
		JLSTNull,
		JLSTBool,
		JLSTInt,
		JLSTDouble,
		JLSTString,
		JLSTFunction,
		JLSTArray,
		JLSTObject,
		JLSTProtolessObject,
		JLSTObjectValue,
//		JLSTInterpretedObject,
		JLSTSerializableNativeObject,
		JLSTSerializableScriptObject
	};


	class Serializer {
	private:

		jsval _serializerObj;

		uint8_t *_start;
		uint8_t *_pos;
		size_t _length;

		bool PrepareBytes( size_t length, bool exact = false ) {

			size_t offset = _pos - _start;
			if ( offset + length > _length ) {

				_length = (exact ? 0 : _length * 2) + length;
				_start = (uint8_t*)jl_realloc(_start, _length); // if ( _start == NULL ) jl_alloc(_length)
				if ( _start == NULL )
					return false;
				_pos = _start + offset;
			}
			return true;
		}

		Serializer();
		Serializer( const Serializer & );

	public:

		~Serializer() {

			if ( _start != NULL )
				jl_free(_start);
		}

		Serializer( jsval serializerObj )
		: _serializerObj(serializerObj), _start(NULL), _pos(NULL), _length(0) {

			PrepareBytes(JL_PAGESIZE);
		}

		JSBool GetBufferOwnership( void **data, size_t *length ) {

			// add '\0'
			JL_CHK( PrepareBytes(1, true) );
			*_pos = 0;

			// get length
			*length = _pos - _start;

			// maybe realloc the buffer
			if ( JL_MaybeRealloc(_length, _pos - _start) )
				_start = (uint8_t*)jl_realloc(_start, _pos - _start + 1);
			*data = _start;

			// reset
			_start = NULL; // loose the ownership
			_pos = NULL;
			_length = 0;
			return JS_TRUE;
			JL_BAD;
		}


		JSBool Write( JSContext *cx, const char *buf ) {

			size_t length = strlen(buf) + 1; // + 1 for the '\0' 
			JL_CHK( Write(cx, length) );
			JL_CHK( PrepareBytes(length) );
			memcpy(_pos, buf, length);
			_pos += length;
			return JS_TRUE;
			JL_BAD;
		}

		JSBool Write( JSContext *cx, const SerializerConstBufferInfo &buf ) {

			JL_CHK( Write(cx, buf.Length()) );
			if ( buf.Length() > 0 ) {

				JL_CHK( PrepareBytes(buf.Length()) );
				memcpy(_pos, buf.Data(), buf.Length());
				_pos += buf.Length();
			}
			return JS_TRUE;
			JL_BAD;
		}

		JSBool Write( JSContext *cx, const SerializerObjectProperties &sop ) {

			JSObject *obj = sop;

			JSIdArray *idArray = JS_Enumerate(cx, obj); // Get an array of the all *own* enumerable properties of a given object.
			JL_CHK( idArray );
			JL_CHK( Write(cx, idArray->length) );
			jsval name, value;
			for ( int i = 0; i < idArray->length; ++i ) {

				JL_CHK( JS_IdToValue(cx, idArray->vector[i], &name) );
				JSString *jsstr = JSVAL_IS_STRING(name) ? JSVAL_TO_STRING(name) : JS_ValueToString(cx, name);
				JL_CHK( jsstr );
				size_t length;
				const jschar *chars;
				chars = JS_GetStringCharsAndLength(jsstr, &length); // doc. not null-terminated.
				JL_CHK( JS_GetPropertyById(cx, obj, idArray->vector[i], &value) );
				JL_CHK( Write(cx, SerializerConstBufferInfo(chars, length)) );
				JL_CHK( Write(cx, value) );
			}
			JS_DestroyIdArray(cx, idArray);
			return JS_TRUE;
		bad:
			if ( idArray )
				JS_DestroyIdArray(cx, idArray);
			return JS_FALSE;
		}

		JSBool Write( JSContext *cx, const jsval &val ) {

			if ( JSVAL_IS_PRIMITIVE(val) ) {

				if ( JSVAL_IS_INT(val) ) {

					JL_CHK( Write(cx, JLSTInt) );
					JL_CHK( Write(cx, JSVAL_TO_INT(val)) );
				} else
				if ( JSVAL_IS_STRING(val) ) {

					JSString *jsstr = JSVAL_TO_STRING(val);
					size_t length;
					const jschar *chars;
					chars = JS_GetStringCharsAndLength(jsstr, &length);
					JL_CHK( Write(cx, JLSTString) );
					JL_CHK( Write(cx, SerializerConstBufferInfo(chars, length)) );
				} else
				if ( JSVAL_IS_VOID(val) ) {

					JL_CHK( Write(cx, JLSTVoid) );
				} else
				if ( JSVAL_IS_BOOLEAN(val) ) {

					JL_CHK( Write(cx, JLSTBool) );
					JL_CHK( Write(cx, char(JSVAL_TO_BOOLEAN(val))) );
				} else
				if ( JSVAL_IS_DOUBLE(val) ) {

					JL_CHK( Write(cx, JLSTDouble) );
					JL_CHK( Write(cx, JSVAL_TO_DOUBLE(val)) );
				} else
				if ( JL_JSVAL_IS_ARRAY_HOLE(val) ) {

					JL_CHK( Write(cx, JLSTHole) );
				} else
				if ( JSVAL_IS_NULL(val) ) {

					JL_CHK( Write(cx, JLSTNull) );
				} else
					JL_REPORT_ERROR("Unsupported value.");
				return JS_TRUE;
			}

			// objects

			JSObject *obj = JSVAL_TO_OBJECT(val);

			if ( JS_IsArrayObject(cx, obj) ) {

				jsuint length;
				JL_CHK( JS_GetArrayLength(cx, obj, &length) );
				JL_CHK( Write(cx, JLSTArray) );
				JL_CHK( Write(cx, length) );

				JSBool found;
				jsval elt;
				for ( jsint i = 0; i < jl::SafeCast<jsint>(length); ++i ) {

					JL_CHK( JS_GetElement(cx, obj, i, &elt) );
					if ( JSVAL_IS_VOID( elt ) ) {

						JL_CHK( JS_HasElement(cx, obj, i, &found) );
						if ( !found )
							JL_JSVAL_SET_ARRAY_HOLE(elt);
					}
					JL_CHK( Write(cx, elt) );
				}
				return JS_TRUE;
			}

			if ( obj->isFunction() ) { // if ( JS_ObjectIsFunction(cx, obj) ) { // JL_JsvalIsFunction(cx, val)

				JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
				JL_CHK( JS_XDRValue(xdr, const_cast<jsval*>(&val)) ); // JSXDR_ENCODE, de-const can be done
				uint32 length;
				void *buf = JS_XDRMemGetData(xdr, &length);
				JL_ASSERT( buf );
				JL_CHK( Write(cx, JLSTFunction) );
				JL_CHK( Write(cx, SerializerConstBufferInfo(buf, length)) );
				JS_XDRDestroy(xdr);
				return JS_TRUE;
			}


			jsval serializeFctVal;
			JL_CHK( JS_GetMethodById(cx, obj, JLID(cx, _serialize), NULL, &serializeFctVal) ); // JL_CHK( JS_GetProperty(cx, obj, "_serialize", &serializeFctVal) );

			if ( !JSVAL_IS_VOID( serializeFctVal ) ) {

				jsval argv[] = { JSVAL_NULL, _serializerObj };
				js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);

				JL_S_ASSERT_FUNCTION( serializeFctVal );

				JSObject *objectProto;
				JL_CHK( js_GetClassPrototype(cx, NULL, JSProto_Object, &objectProto) );

				if ( JL_GetClass(obj) != JL_GetClass(objectProto) ) { // native serializable object

					JL_CHK( Write(cx, JLSTSerializableNativeObject) );
					JL_CHK( Write(cx, JL_GetClass(obj)->name) );
					JL_CHK( JS_CallFunctionValue(cx, obj, serializeFctVal, COUNTOF(argv)-1, argv+1, argv) );
					return JS_TRUE;
				}

				// JS object
				JL_CHK( Write(cx, JLSTSerializableScriptObject) );

				JSObject *objectConstructor;
				objectConstructor = JS_GetConstructor(cx, objectProto);
				JL_ASSERT( objectConstructor != NULL );
				
				JSObject *proto = JS_GetPrototype(cx, obj);
				JL_CHKM( proto, "Invalid class prototype." );

				JSObject *constructor = JS_GetConstructor(cx, proto);
				JL_CHKM( constructor && JL_ObjectIsFunction(cx, constructor), "Constructor not found." );

				JL_CHKM( constructor != objectConstructor, "Invalid constructor." );

				JSString *funName = JS_GetFunctionId(JL_ObjectToFunction(cx, constructor)); // see also. JS_ValueToConstructor()
				JL_CHKM( funName != NULL, "Constructor name not found." );

				size_t length;
				const jschar *chars;
				chars = JS_GetStringCharsAndLength(funName, &length); // doc. not null-terminated.
				JL_CHKM( chars && *chars, "Invalid constructor name." );

				JL_CHK( Write(cx, SerializerConstBufferInfo(chars, length)) );

				JL_CHK( JS_CallFunctionValue(cx, obj, serializeFctVal, COUNTOF(argv)-1, argv+1, argv) );
				return JS_TRUE;

				//JSFunction *fct = JS_ValueToFunction(cx, serializeFctVal);
				//if ( fct->isInterpreted() ) { // weakly unreliable

				//	JL_CHK( Write(cx, JLSTInterpretedObject) );

				//	//jsval unserializeFctVal;
				//	//JL_CHK( JS_GetMethodById(cx, obj, JLID(cx, _unserialize), NULL, &unserializeFctVal) );
				//	//if ( !JL_JsvalIsFunction(cx, unserializeFctVal) )
				//	//	JL_REPORT_ERROR("unserializer function not found.");
				//	//JL_CHK( Write(cx, unserializeFctVal) );
				//	//jsval argv[] = { JSVAL_NULL, _serializerObj };
				//	//js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
				//	//JL_CHK( JS_CallFunctionValue(cx, obj, serializeFctVal, COUNTOF(argv)-1, argv+1, argv) );
				//	//JL_CHK( Write(cx, *argv);
				//	return JS_TRUE;
				//} else {

				//	JL_CHK( Write(cx, JLSTSerializableNativeObject) );
				//	JL_CHK( Write(cx, obj->getClass()->name) );
				//	jsval argv[] = { JSVAL_NULL, _serializerObj };
				//	js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
				//	JL_CHK( JS_CallFunctionValue(cx, obj, serializeFctVal, COUNTOF(argv)-1, argv+1, argv) );
				//	return JS_TRUE;
				//}
			}

			// simple object
			if ( JL_ObjectIsObject(cx, obj) ) {

				JL_CHK( Write(cx, JLSTObject) );
				JL_CHK( Write(cx, SerializerObjectProperties(obj)) );
				return JS_TRUE;
			}

			// prototype-less object
			if ( JS_GetPrototype(cx, obj) == NULL ) {

				JL_CHK( Write(cx, JLSTProtolessObject) );
				JL_CHK( Write(cx, SerializerObjectProperties(obj)) );
				return JS_TRUE;
			}
			
			// fallback (Date, Number, String, ...)
			{
			JL_CHK( Write(cx, JLSTObjectValue) );
			JL_CHK( Write(cx, obj->getClass()->name) );
			js::AutoValueRooter avr(cx);
			JL_CHK( JL_JsvalToPrimitive(cx, OBJECT_TO_JSVAL(obj), avr.jsval_addr()) );
			JL_CHK( Write(cx, avr.jsval_value()) );
			return JS_TRUE;
			}

			JL_BAD;
		}

		template <class T>
		JSBool Write( JSContext *cx, const T value ) {

			JL_CHK( PrepareBytes(sizeof(T)) );
			*(T*)_pos = value;
			_pos += sizeof(T);
			return JS_TRUE;
			JL_BAD;
		}

	};




	class Unserializer {
	private:
		jsval _unserializerObj;

		uint8_t *_start;
		uint8_t *_pos;
		size_t _length;

		bool AssertData( size_t length ) const {

			return (_pos - _start) + length <= _length;
		}

		Unserializer();
		Unserializer( const Unserializer & );

	public:

		~Unserializer() {

			jl_free(_start);
		}
		
		Unserializer( jsval unserializerObj, void *dataOwnership, size_t length )
		: _unserializerObj(unserializerObj), _start((uint8_t*)dataOwnership), _pos(_start), _length(length) {
		}

		JSBool Read( JSContext *cx, const char *&buf ) {

			size_t length;
			JL_CHK( Read(cx, length) );
			if ( !AssertData(length) )
				return JS_FALSE;
			buf = (const char*)_pos;
			_pos += length;
			return JS_TRUE;
			JL_BAD;
		}

		JSBool Read( JSContext *cx, SerializerConstBufferInfo &buf ) {

			size_t length;
			JL_CHK( Read(cx, length) );
			if ( length > 0 ) {

				if ( !AssertData(length) )
					return JS_FALSE;
				buf = SerializerConstBufferInfo(_pos, length);
				_pos += length;
			} else {

				buf = SerializerConstBufferInfo(NULL, 0);
			}
			return JS_TRUE;
			JL_BAD;
		}

/*
		JSBool Read( JSContext *cx, SerializerBufferInfo &buf ) {

			size_t length;
			JL_CHK( Read(cx, length);
			if ( length > 0 ) {

				if ( !AssertData(length) )
					return JS_FALSE;
				buf = SerializerBufferInfo(_pos, length);
				_pos += length;
			} else {

				buf = SerializerBufferInfo(NULL, 0);
			}
			return JS_TRUE;
		}
*/

		JSBool Read( JSContext *cx, SerializerObjectProperties &sop ) {

			jsint length;
			JL_CHK( Read(cx, length) );

			for ( int i = 0; i < length; ++i ) {

				SerializerConstBufferInfo name;
				jsval value;
				JL_CHK( Read(cx, name) );
				JL_CHK( Read(cx, value) );
				JL_CHK( JS_SetUCProperty(cx, sop, (const jschar *)name.Data(), name.Length()/2, &value) );
			}

			return JS_TRUE;
			JL_BAD;
		}


		JSBool Read( JSContext *cx, jsval &val ) {

			SerializerConstBufferInfo buf;
			char type;
			JL_CHK( Read(cx, type) );

			switch ( type ) {

				case JLSTInt: {

					jsint i;
					JL_CHK( Read(cx, i) );
					val = INT_TO_JSVAL(i);
					break;
				}
				case JLSTString: {

					JL_CHK( Read(cx, buf) );
					JSString *jsstr;
					jsstr = JS_NewUCStringCopyN(cx, (const jschar *)buf.Data(), buf.Length()/2);
					val = STRING_TO_JSVAL(jsstr);
					break;
				}
				case JLSTVoid: {

					val = JSVAL_VOID;
					break;
				}
				case JLSTBool: {

					char b;
					JL_CHK( Read(cx, b) );
					val = BOOLEAN_TO_JSVAL(b);
					break;
				}
				case JLSTDouble: {

					jsdouble d;
					JL_CHK( Read(cx, d) );
					val = DOUBLE_TO_JSVAL(d);
					break;
				}
				case JLSTHole: {

					JL_JSVAL_SET_ARRAY_HOLE(val);
					break;
				}
				case JLSTNull: {

					val = JSVAL_NULL;
					break;
				}
				case JLSTArray: {

					jsuint length;
					JL_CHK( Read(cx, length) );
					JSObject *arr;
					arr = JS_NewArrayObject(cx, length, NULL);
					JL_S_ASSERT_ALLOC( arr );
					val = OBJECT_TO_JSVAL(arr);

					jsval elt;
					for ( jsuint i = 0; i < length; ++i ) {

						JL_CHK( Read(cx, elt) );
						if ( !JL_JSVAL_IS_ARRAY_HOLE(elt) )
							JL_CHK( JS_SetElement(cx, arr, i, &elt) );
					}
					break;
				}
				case JLSTObject: {

					JSObject *obj = JS_NewObject(cx, NULL, NULL, NULL);
					val = OBJECT_TO_JSVAL(obj);
					SerializerObjectProperties sop(obj);
					JL_CHK( Read(cx, sop) );
					break;
				}
				case JLSTProtolessObject: {

					JSObject *obj = JS_NewObjectWithGivenProto(cx, NULL, NULL, NULL);
					val = OBJECT_TO_JSVAL(obj);
					SerializerObjectProperties sop(obj);
					JL_CHK( Read(cx, sop) );
					break;
				}
				case JLSTObjectValue: {

					const char *className;
					jsval value;
					JL_CHK( Read(cx, className) );
					JL_CHK( Read(cx, value) );
					JSObject *scope = JS_GetScopeChain(cx); // JS_GetGlobalForScopeChain
					jsval prop;
					JL_CHK( JS_GetProperty(cx, scope, className, &prop) );
					JSObject *newObj = JS_New(cx, JSVAL_TO_OBJECT(prop), 1, &value);
					val = OBJECT_TO_JSVAL(newObj);
					break;
				}
				//case JLSTInterpretedObject: {

				//	jsval unserializeFctVal;
				//	JL_CHK( Read(cx, unserializeFctVal) );
				//	JL_S_ASSERT_FUNCTION( unserializeFctVal );
				//	JSObject *globalObj;
				//	globalObj = JL_GetGlobalObject(cx);
				//	JL_CHK( JS_SetParent(cx, JSVAL_TO_OBJECT(unserializeFctVal), globalObj) );
				//	jsval value;
				//	JL_CHK( Read(cx, value) );
				//	jsval argv[] = { JSVAL_NULL, value };
				//	js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
				//	JL_CHK( JS_CallFunctionValue(cx, globalObj, unserializeFctVal, COUNTOF(argv)-1, argv+1, argv) );
				//	val = *argv;
				//	break;
				//}
				case JLSTSerializableNativeObject: {

					const char *className;
					JL_CHK( Read(cx, className) );
/*
					JSObject *scope = JS_GetScopeChain(cx); // JS_GetGlobalForScopeChain
					jsval constructorVal;
					JL_CHK( JS_GetProperty(cx, scope, className, &constructorVal) );
					JSObject *constructorObj = JSVAL_TO_OBJECT(constructorVal); //JSFunction *fun = JS_ValueToConstructor(cx, constructor);
					jsval prototypeVal;
					JL_CHK( JS_GetPropertyById(cx, constructorObj, JL_ATOMJSID(cx, classPrototype), &prototypeVal) );
					JSObject *proto = JSVAL_TO_OBJECT(prototypeVal);
					jsval argv[] = { JSVAL_NULL, _unserializerObj };
					js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
					JL_CHK( JL_CallFunctionId(cx, proto, JLID(cx, _unserialize), COUNTOF(argv)-1, argv+1, argv) );
					val = *argv;
*/

					ClassProtoCache *cpc = JL_GetCachedClassProto(JL_GetHostPrivate(cx), className);
					JL_CHKM( cpc != NULL, "Class %s not found.", className );
					JSObject *newObj;
					newObj = JS_NewObjectWithGivenProto(cx, cpc->clasp, cpc->proto, NULL);
					JL_CHK( newObj );
 					jsval argv[] = { JSVAL_NULL, _unserializerObj };
					js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
					JL_CHK( JL_CallFunctionId(cx, newObj, JLID(cx, _unserialize), COUNTOF(argv)-1, argv+1, argv) );
					val = OBJECT_TO_JSVAL(newObj);
					break;
				}
				case JLSTSerializableScriptObject: {

					SerializerConstBufferInfo buf;
					JL_CHK( Read(cx, buf) );

					JSObject *scope;
					scope = JS_GetScopeChain(cx); // JS_GetGlobalForScopeChain
					jsval constructorVal;
					JL_CHK( JS_GetUCProperty(cx, scope, (const jschar*)buf.Data(), buf.Length()/2, &constructorVal) );
					JSObject *newObj;
					newObj = JS_New(cx, JSVAL_TO_OBJECT(constructorVal), 0, NULL);
					JL_CHK( newObj );

 					jsval argv[] = { JSVAL_NULL, _unserializerObj };
					js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
					JL_CHK( JL_CallFunctionId(cx, newObj, JLID(cx, _unserialize), COUNTOF(argv)-1, argv+1, argv) );
					val = OBJECT_TO_JSVAL(newObj);
					break;
				}
				case JLSTFunction: {

					JL_CHK( Read(cx, buf) );
					JSXDRState *xdr;
					xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
					JS_XDRMemSetData(xdr, const_cast<void*>(buf.Data()), buf.Length());
					JL_CHK( JS_XDRValue(xdr, &val) );
					JL_S_ASSERT_FUNCTION( val );
					JS_XDRMemSetData(xdr, NULL, 0);
					JS_XDRDestroy(xdr);
					JL_CHK( JS_SetParent(cx, JSVAL_TO_OBJECT(val), JL_GetGlobalObject(cx)) );
					JSObject *funProto;
					JL_CHK( js_GetClassPrototype(cx, NULL, JSProto_Function, &funProto) );
					JL_CHK( JS_SetPrototype(cx, JSVAL_TO_OBJECT(val), funProto) );
					break;
				default:
					JL_REPORT_ERROR("Unknown data type.");
				}
			}

			return JS_TRUE;
			JL_BAD;
		}

		JSBool Read( JSContext *cx, jsval *&val ) {

			return Read(cx, *val);
		}

		template <class T>
		JSBool Read( JSContext *cx, T &value ) {

			if ( !AssertData(sizeof(T)) )
				return JS_FALSE;
			value = *(T*)_pos;
			_pos += sizeof(T);
			return JS_TRUE;
		}

	};




	ALWAYS_INLINE bool
	JsvalIsSerializer( JSContext *cx, jsval &val ) {

		return JL_JsvalIsClass(val, JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Serializer")->clasp);
	}

	ALWAYS_INLINE Serializer*
	JsvalToSerializer( JSContext *cx, jsval &val ) {

		return static_cast<Serializer*>(JL_GetPrivate(cx, JSVAL_TO_OBJECT(val)));
	}


	ALWAYS_INLINE bool
	JsvalIsUnserializer( JSContext *cx, jsval &val ) {

		return JL_JsvalIsClass(val, JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Unserializer")->clasp);
	}

	ALWAYS_INLINE Unserializer*
	JsvalToUnserializer( JSContext *cx, jsval &val ) {

		return static_cast<Unserializer*>(JL_GetPrivate(cx, JSVAL_TO_OBJECT(val)));
	}

}
