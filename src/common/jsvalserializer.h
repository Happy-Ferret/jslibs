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

		JLTHole,
		JLTVoid,
		JLTNull,
		JLTBool,
		JLTInt,
		JLTDouble,
		JLTString,
		JLTFunction,
		JLTArray,
		JLTObject,
		JLTObjectValue,
		JLTInterpretedObject,
		JLTNativeObject
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

			JL_CHK( PrepareBytes(1, true) );
			*_pos = 0;
			*data = _start;
			*length = _pos - _start;
			_start = NULL;
			_pos = NULL;
			_length = 0;
			return JS_TRUE;
			JL_BAD;
		}

		JSBool Write( JSContext *cx, const char *buf ) {

			size_t length = strlen(buf)+1;
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
				chars = JS_GetStringCharsAndLength(jsstr, &length);
				JL_CHK( Write(cx, SerializerConstBufferInfo(chars, length)) );

				JL_CHK( JS_GetPropertyById(cx, obj, idArray->vector[i], &value) );
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

//				switch (JS_TypeOfValue(cx, val)) {
//				}


				if ( JSVAL_IS_INT(val) ) {

					JL_CHK( Write(cx, JLTInt) );
					JL_CHK( Write(cx, JSVAL_TO_INT(val)) );
				} else
				if ( JSVAL_IS_STRING(val) ) {

					JSString *jsstr = JSVAL_TO_STRING(val);
					// *this << type << SerializerConstBufferInfo(JS_GetStringChars(jsstr), JS_GetStringLength(jsstr));
					size_t length;
					const jschar *chars;
					chars = JS_GetStringCharsAndLength(jsstr, &length);
					JL_CHK( Write(cx, JLTString) );
					JL_CHK( Write(cx, SerializerConstBufferInfo(chars, length)) );
				} else
				if ( JSVAL_IS_VOID(val) ) {

					JL_CHK( Write(cx, JLTVoid) );
				} else
				if ( JSVAL_IS_BOOLEAN(val) ) {

					JL_CHK( Write(cx, JLTBool) );
					JL_CHK( Write(cx, char(JSVAL_TO_BOOLEAN(val))) );
				} else
				if ( JSVAL_IS_DOUBLE(val) ) {

					JL_CHK( Write(cx, JLTDouble) );
					JL_CHK( Write(cx, JSVAL_TO_DOUBLE(val)) );
				} else
				if ( js::Valueify(val).isMagic(JS_ARRAY_HOLE) ) {

					JL_CHK( Write(cx, JLTHole) );
				} else
				if ( JSVAL_IS_NULL(val) ) {

					JL_CHK( Write(cx, JLTNull) );
				} else
					JL_REPORT_ERROR("Unsupported value.");

			} else { // objects

				JSObject *obj = JSVAL_TO_OBJECT(val);

				if ( JS_IsArrayObject(cx, obj) ) {

					jsuint length;
					JL_CHK( JS_GetArrayLength(cx, obj, &length) );
					JL_CHK( Write(cx, JLTArray) );
					JL_CHK( Write(cx, length) );

					JSBool found;
					jsval elt;
					for ( jsint i = 0; i < jl::SafeCast<jsint>(length); ++i ) {

						JL_CHK( JS_GetElement(cx, obj, i, &elt) );
						if ( JSVAL_IS_VOID( elt ) ) {

							JL_CHK( JS_HasElement(cx, obj, i, &found) );
							if ( !found )
								elt = js::Jsvalify(js::MagicValue(JS_ARRAY_HOLE));
						}
						JL_CHK( Write(cx, elt) );
					}
				} else
				if ( obj->isFunction() ) { // if ( JS_ObjectIsFunction(cx, obj) ) { // JL_JsvalIsFunction(cx, val)

					JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
					JL_CHK( JS_XDRValue(xdr, const_cast<jsval*>(&val)) ); // JSXDR_ENCODE, de-const can be done
					uint32 length;
					void *buf = JS_XDRMemGetData(xdr, &length);
					JL_ASSERT( buf );
					JL_CHK( Write(cx, JLTFunction) );
					JL_CHK( Write(cx, SerializerConstBufferInfo(buf, length)) );
					JS_XDRDestroy(xdr);
				} else {

					jsval serializeFctVal;
					JL_CHK( JS_GetMethodById(cx, obj, JLID(cx, _serialize), NULL, &serializeFctVal) ); // JL_CHK( JS_GetProperty(cx, obj, "_serialize", &serializeFctVal) );

					if ( !JSVAL_IS_VOID( serializeFctVal ) ) {

						JL_ASSERT( JL_JsvalIsFunction(cx, serializeFctVal) );

						JSFunction *fct = JS_ValueToFunction(cx, serializeFctVal);
						if ( fct->isInterpreted() ) { // weakly unreliable

							JL_CHK( Write(cx, JLTInterpretedObject) );

/*							jsval unserializeFctVal;
							JL_CHK( JS_GetMethodById(cx, obj, JLID(cx, _unserialize), NULL, &unserializeFctVal) );
//							JL_S_ASSERT_FUNCTION( unserializeFctVal ); // assert object can be unserialized
							if ( !JL_JsvalIsFunction(cx, unserializeFctVal) )
								JL_REPORT_ERROR("unserializer function not found.");
							JL_CHK( Write(cx, unserializeFctVal);
							jsval argv[] = { JSVAL_NULL, _serializerObj };
							js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
							JL_CHK( JS_CallFunctionValue(cx, obj, serializeFctVal, COUNTOF(argv)-1, argv+1, argv) );
//							JL_CHK( Write(cx, *argv);
*/
						} else {

							JL_CHK( Write(cx, JLTNativeObject) );
							JL_CHK( Write(cx, obj->getClass()->name) );
							jsval argv[] = { JSVAL_NULL, _serializerObj };
							js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
							JL_CHK( JS_CallFunctionValue(cx, obj, serializeFctVal, COUNTOF(argv)-1, argv+1, argv) );
						}

					} else
					if ( JL_ObjectIsObject(cx, obj) ) {

						JL_CHK( Write(cx, JLTObject) );
						JL_CHK( Write(cx, SerializerObjectProperties(obj)) );
					} else { // fallback


						JL_CHK( Write(cx, JLTObjectValue) );
						JL_CHK( Write(cx, obj->getClass()->name) );
						js::AutoValueRooter avr(cx);
						JL_CHK( JL_JsvalToPrimitive(cx, OBJECT_TO_JSVAL(obj), avr.jsval_addr()) );
						JL_CHK( Write(cx, avr.jsval_value()) );
					}
				}
			}

			return JS_TRUE;
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

				case JLTInt: {

					jsint i;
					JL_CHK( Read(cx, i) );
					val = INT_TO_JSVAL(i);
					break;
				}
				case JLTString: {

					JL_CHK( Read(cx, buf) );
					JSString *jsstr;
					jsstr = JS_NewUCStringCopyN(cx, (const jschar *)buf.Data(), buf.Length()/2);
					val = STRING_TO_JSVAL(jsstr);
					break;
				}
				case JLTVoid: {

					val = JSVAL_VOID;
					break;
				}
				case JLTBool: {

					char b;
					JL_CHK( Read(cx, b) );
					val = BOOLEAN_TO_JSVAL(b);
					break;
				}
				case JLTDouble: {

					jsdouble d;
					JL_CHK( Read(cx, d) );
					val = DOUBLE_TO_JSVAL(d);
					break;
				}
				case JLTHole: {

					val = js::Jsvalify(js::MagicValue(JS_ARRAY_HOLE));
					break;
				}
				case JLTNull: {

					val = JSVAL_NULL;
					break;
				}

				case JLTArray: {

					jsuint length;
					JL_CHK( Read(cx, length) );
					JSObject *arr;
					arr = JS_NewArrayObject(cx, length, NULL);
					JL_S_ASSERT_ALLOC( arr );
					val = OBJECT_TO_JSVAL(arr);

					jsval elt;
					for ( jsuint i = 0; i < length; ++i ) {

						JL_CHK( Read(cx, elt) );
						JL_CHK( JS_SetElement(cx, arr, i, &elt) );
					}
					break;
				}
				case JLTObject: {
/*
					jsint length;
					*this >> length;
					JSObject *obj = JS_NewObject(cx, NULL, NULL, NULL);
					val = OBJECT_TO_JSVAL(obj);
					const char *name;
					for ( int i = 0; i < length; ++i ) {

						*this >> name;
						jsval value;
						*this >> value;
						JL_CHK( JS_SetProperty(cx, obj, name, &value) );
					}
*/

					JSObject *obj = JS_NewObject(cx, NULL, NULL, NULL);
					val = OBJECT_TO_JSVAL(obj);
					SerializerObjectProperties sop(obj);
					JL_CHK( Read(cx, sop) );
					break;
				}
				case JLTObjectValue: {

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
				case JLTInterpretedObject: {

/*
					jsval unserializeFctVal;
					JL_CHK( Read(cx, unserializeFctVal) );
					JL_S_ASSERT_FUNCTION( unserializeFctVal );
					JSObject *globalObj;
					globalObj = JL_GetGlobalObject(cx);
					JL_CHK( JS_SetParent(cx, JSVAL_TO_OBJECT(unserializeFctVal), globalObj) );
					jsval value;
					JL_CHK( Read(cx, value) );
					jsval argv[] = { JSVAL_NULL, value };
					js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
					JL_CHK( JS_CallFunctionValue(cx, globalObj, unserializeFctVal, COUNTOF(argv)-1, argv+1, argv) );
					val = *argv;
*/
					break;
				}
				case JLTNativeObject: {

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
					JL_CHKM( cpc->clasp, "Class %s not found.", className );
					JSObject *newObject;
					newObject = JS_NewObjectWithGivenProto(cx, cpc->clasp, cpc->proto, NULL);
					JL_CHK( newObject );
 					jsval argv[] = { JSVAL_NULL, _unserializerObj };
					js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
					JL_CHK( JL_CallFunctionId(cx, newObject, JLID(cx, _unserialize), COUNTOF(argv)-1, argv+1, argv) );
					val = OBJECT_TO_JSVAL(newObject);
					break;
				}
				case JLTFunction: {

					JL_CHK( Read(cx, buf) );
					JSXDRState *xdr;
					xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
					JS_XDRMemSetData(xdr, const_cast<void*>(buf.Data()), buf.Length());
					JL_CHK( JS_XDRValue(xdr, &val) );
					JL_S_ASSERT_FUNCTION( val );
					JS_XDRMemSetData(xdr, NULL, 0);
					JS_XDRDestroy(xdr);
					JL_CHK( JS_SetParent(cx, JSVAL_TO_OBJECT(val), JL_GetGlobalObject(cx)) );
					break;
				default:
					JL_REPORT_ERROR("Invalid data type.");
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
