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
		JSObject *_obj;
	public:
		SerializerObjectProperties() {}

		SerializerObjectProperties( JSObject *obj ) : _obj(obj) {}

		operator JSObject*() const {

			return _obj;
		}
	};


	enum JLSerializeType {

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

		void PrepareBytes( size_t length, bool exact = false ) {

			size_t offset = _pos - _start;
			if ( offset + length > _length ) {

				_length = (exact ? 0 : _length * 2) + length;

				_start = (uint8_t*)jl_realloc(_start, _length); // if ( _start == NULL ) jl_alloc(_length)
				JL_ASSERT( _start != NULL );
				_pos = _start + offset;
			}
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
/*
		void Free() {

			jl_free(_start);
			_start = NULL;
			_pos = NULL;
			_length = 0;
		}
*/
		void GetBufferOwnership( void **data, size_t *length ) {

			PrepareBytes(1, true);
			*_pos = 0;
			*data = _start;
			*length = _pos - _start;
			_start = NULL;
			_pos = NULL;
			_length = 0;
		}

		void Write( JSContext *cx, const char *buf ) {

			size_t length = strlen(buf)+1;
			Write(cx, length);
			PrepareBytes(length);
			memcpy(_pos, buf, length);
			_pos += length;
		}

		JSBool Write( JSContext *cx, const SerializerConstBufferInfo &buf ) {

			Write(cx, buf.Length());
			if ( buf.Length() > 0 ) {

				PrepareBytes(buf.Length());
				memcpy(_pos, buf.Data(), buf.Length());
				_pos += buf.Length();
			}
			return JS_TRUE;
		}

		JSBool Write( JSContext *cx, const SerializerObjectProperties &sop ) {

			JSObject *obj = sop;

			JSIdArray *idArray = JS_Enumerate(cx, obj); // Get an array of the all own enumerable properties of a given object.
			Write(cx, idArray->length);
			jsval name, value;
			for ( int i = 0; i < idArray->length; ++i ) {

				//name = ID_TO_VALUE( idArray->vector[i] ); // JL_CHK( JS_IdToValue(cx, idArray->vector[i], &name) );
				//name = js::Value(idArray->vector[i]);
				//name = js::IdToJsval(idArray->vector[i]);
				JL_CHK( JS_IdToValue(cx, idArray->vector[i], &name) );

				JSString *jsstr = JSVAL_IS_STRING(name) ? JSVAL_TO_STRING(name) : JS_ValueToString(cx, name);
				JL_CHK( jsstr );
//				const char *s = JL_GetStringBytesZ(cx, jsstr);
				const jschar *s = JS_GetStringCharsZ(cx, jsstr);
				if ( s == NULL )
					JL_REPORT_ERROR_NUM(cx, JLSMSG_EXPECT_TYPE, "a valid string");


				// *this << SerializerConstBufferInfo(JS_GetStringChars(jsstr), JS_GetStringLength(jsstr));
				size_t length;
				const jschar *chars;
				chars = JS_GetStringCharsAndLength(jsstr, &length);
				Write(cx, SerializerConstBufferInfo(chars, length));

//				*this << s;
				//JL_CHK( obj->getProperty(cx, idArray->vector[i], &value) );
				JL_CHK( JS_GetPropertyById(cx, obj, idArray->vector[i], &value) );
				Write(cx, value);
			}
			JS_DestroyIdArray(cx, idArray);
			return JS_TRUE;
			JL_BAD;
		}

		JSBool Write( JSContext *cx, const jsval &val ) {

			char type;
			if ( JSVAL_IS_PRIMITIVE(val) ) {

//				switch (JS_TypeOfValue(cx, val)) {
//				}


				if ( JSVAL_IS_INT(val) ) {

					type = JLTInt;
					Write(cx, type);
					Write(cx, JSVAL_TO_INT(val));
				} else
				if ( JSVAL_IS_STRING(val) ) {

					type = JLTString;
					JSString *jsstr = JSVAL_TO_STRING(val);
					// *this << type << SerializerConstBufferInfo(JS_GetStringChars(jsstr), JS_GetStringLength(jsstr));
					size_t length;
					const jschar *chars;
					chars = JS_GetStringCharsAndLength(jsstr, &length);
					Write(cx, type);
					Write(cx, SerializerConstBufferInfo(chars, length));
				} else
				if ( JSVAL_IS_VOID(val) ) {

					type = JLTVoid;
					Write(cx, type);
				} else
				if ( JSVAL_IS_BOOLEAN(val) ) {

					type = JLTBool;
					Write(cx, type);
					Write(cx, char(JSVAL_TO_BOOLEAN(val)));
				} else
				if ( JSVAL_IS_DOUBLE(val) ) {

					type = JLTDouble;
					Write(cx, type);
					Write(cx, JSVAL_TO_DOUBLE(val));
				} else
					if ( js::Valueify(val).isMagic(JS_ARRAY_HOLE) ) {

					type = JLTHole;
					Write(cx, type);
				} else
				if ( JSVAL_IS_NULL(val) ) {

					type = JLTNull;
					Write(cx,type);
				} else
					JL_REPORT_ERROR("Unsupported value.");

			} else { // objects

				JSObject *obj = JSVAL_TO_OBJECT(val);

				if ( JS_IsArrayObject(cx, obj) ) {

					type = JLTArray;
					jsuint length;
					JL_CHK( JS_GetArrayLength(cx, obj, &length) );
					Write(cx, type);
					Write(cx, length);

					JSBool found;
					jsval elt;
					for ( jsint i = 0; i < jl::SafeCast<jsint>(length); ++i ) {

						JL_CHK( JS_GetElement(cx, obj, i, &elt) );
						if ( JSVAL_IS_VOID( elt ) ) {

							JL_CHK( JS_HasElement(cx, obj, i, &found) );
							if ( !found )
								elt = js::Jsvalify(js::MagicValue(JS_ARRAY_HOLE));
						}
						Write(cx, elt);
					}
				} else
				if ( obj->isFunction() ) { // if ( JS_ObjectIsFunction(cx, obj) ) { // JL_JsvalIsFunction(cx, val)

					type = JLTFunction;
					JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
					JL_CHK( JS_XDRValue(xdr, const_cast<jsval*>(&val)) ); // JSXDR_ENCODE, de-const can be done
					uint32 length;
					void *buf = JS_XDRMemGetData(xdr, &length);
					JL_ASSERT( buf );
					Write(cx, type);
					Write(cx, SerializerConstBufferInfo(buf, length));
					JS_XDRDestroy(xdr);
				} else {

					jsval serializeFctVal;
					JL_CHK( JS_GetMethodById(cx, obj, JLID(cx, _serialize), NULL, &serializeFctVal) ); // JL_CHK( JS_GetProperty(cx, obj, "_serialize", &serializeFctVal) );

					if ( !JSVAL_IS_VOID( serializeFctVal ) ) {

						JL_ASSERT( JL_JsvalIsFunction(cx, serializeFctVal) );

						JSFunction *fct = JS_ValueToFunction(cx, serializeFctVal);
						if ( fct->isInterpreted() ) { // weakly unreliable
/*
							type = JLTInterpretedObject;
							Write(cx, type);
							jsval unserializeFctVal;
							JL_CHK( JS_GetMethodById(cx, obj, JLID(cx, _unserialize), NULL, &unserializeFctVal) );
//							JL_S_ASSERT_FUNCTION( unserializeFctVal ); // assert object can be unserialized
							if ( !JL_JsvalIsFunction(cx, unserializeFctVal) )
								JL_REPORT_ERROR("unserializer function not found.");
							Write(cx, unserializeFctVal);
							jsval argv[] = { JSVAL_NULL, _serializerObj };
							js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
							JL_CHK( JS_CallFunctionValue(cx, obj, serializeFctVal, COUNTOF(argv)-1, argv+1, argv) );
//							Write(cx, *argv);
*/

						} else {

							type = JLTNativeObject;
							Write(cx, type);
							Write(cx, obj->getClass()->name);
							jsval argv[] = { JSVAL_NULL, _serializerObj };
							js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
							JL_CHK( JS_CallFunctionValue(cx, obj, serializeFctVal, COUNTOF(argv)-1, argv+1, argv) );
						}

					} else
					if ( JL_ObjectIsObject(cx, obj) ) {

						type = JLTObject;
						Write(cx, type);
						Write(cx, SerializerObjectProperties(obj));
					} else { // fallback

						type = JLTObjectValue;
						Write(cx, type);
						Write(cx, obj->getClass()->name);
						jsval value;
						JL_CHK( JL_ValueOf(cx, OBJECT_TO_JSVAL(obj), &value) );
						Write(cx, value);
					}
				}
			}

			return JS_TRUE;
			JL_BAD;
		}

		template <class T>
		JSBool Write( JSContext *cx, const T value ) {

			PrepareBytes(sizeof(T));
			*(T*)_pos = value;
			_pos += sizeof(T);
			return JS_TRUE;
		}

	};




	class Unserializer {
	private:
		jsval _unserializerObj;

		const uint8_t *_start;
		const uint8_t *_pos;
		size_t _length;

		bool AssertData( size_t length ) const {

			return (_pos - _start) + length <= _length;
		}

		Unserializer();
		Unserializer( const Unserializer & );

	public:
		Unserializer( jsval unserializerObj, const void *data, size_t length )
		: _unserializerObj(unserializerObj), _start((const uint8_t *)data), _pos(_start), _length(length) {

		}

		JSBool Read( JSContext *cx, const char *&buf ) {

			size_t length;
			Read(cx, length);
			if ( !AssertData(length) )
				return JS_FALSE;
			buf = (const char*)_pos;
			_pos += length;
			return JS_TRUE;
		}

		JSBool Read( JSContext *cx, SerializerConstBufferInfo &buf ) {

			size_t length;
			Read(cx, length);
			if ( length > 0 ) {

				if ( !AssertData(length) )
					return JS_FALSE;
				buf = SerializerConstBufferInfo(_pos, length);
				_pos += length;
			} else {

				buf = SerializerConstBufferInfo(NULL, 0);
			}
			return JS_TRUE;
		}
/*
		JSBool Read( JSContext *cx, SerializerBufferInfo &buf ) {

			size_t length;
			Read(cx, length);
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
			Read(cx, length);
//			JSObject *obj = JS_NewObject(cx, NULL, NULL, NULL);

			for ( int i = 0; i < length; ++i ) {

				SerializerConstBufferInfo name;
				jsval value;
				Read(cx, name);
				Read(cx, value);
				JL_CHK( JS_SetUCProperty(cx, /*obj*/sop, (jschar*)name.Data(), name.Length(), &value) );
			}

//			sop = SerializerObjectProperties(obj);

			return JS_TRUE;
			JL_BAD;
		}

		JSBool Read( JSContext *cx, jsval &val ) {

			SerializerConstBufferInfo buf;
			char type;
			Read(cx, type);

			switch (type) {

				case JLTInt: {

					jsint i;
					Read(cx, i);
					val = INT_TO_JSVAL(i);
					break;
				}
				case JLTString: {

					Read(cx, buf);
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
					Read(cx, b);
					val = BOOLEAN_TO_JSVAL(b);
					break;
				}
				case JLTDouble: {

					jsdouble d;
					Read(cx, d);
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
					Read(cx, length);
					JSObject *arr;
					arr = JS_NewArrayObject(cx, length, NULL);
					JL_S_ASSERT_ALLOC( arr );
					val = OBJECT_TO_JSVAL(arr);

					jsval elt;
					for ( jsuint i = 0; i < length; ++i ) {

						Read(cx, elt);
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
					Read(cx, sop);
					break;
				}
				case JLTObjectValue: {

					const char *className;
					jsval value;
					Read(cx, className);
					Read(cx, value);
					JSObject *scope = JS_GetScopeChain(cx); // JS_GetGlobalForScopeChain
					jsval prop;
					JL_CHK( JS_GetProperty(cx, scope, className, &prop) );
					JSObject *newObj = JS_New(cx, JSVAL_TO_OBJECT(prop), 1, &value);
					val = OBJECT_TO_JSVAL(newObj);
					break;
				}
				case JLTInterpretedObject: {

					jsval unserializeFctVal;
					Read(cx, unserializeFctVal);
					JL_S_ASSERT_FUNCTION( unserializeFctVal );
					JSObject *globalObj;
					globalObj = JL_GetGlobalObject(cx);
					JL_CHK( JS_SetParent(cx, JSVAL_TO_OBJECT(unserializeFctVal), globalObj) );
					jsval value;
					Read(cx, value);
					jsval argv[] = { JSVAL_NULL, value };
					js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
					JL_CHK( JS_CallFunctionValue(cx, globalObj, unserializeFctVal, COUNTOF(argv)-1, argv+1, argv) );
					val = *argv;
					break;
				}
				case JLTNativeObject: {

					const char *className;
					Read(cx, className);
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
					JL_S_ASSERT( cpc->clasp, "Class %s not found.", className );
					JSObject *newObject;
					newObject = JS_NewObjectWithGivenProto(cx, cpc->clasp, cpc->proto, NULL);
 					jsval argv[] = { JSVAL_NULL, _unserializerObj };
					js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
					JL_CHK( JL_CallFunctionId(cx, newObject, JLID(cx, _unserialize), COUNTOF(argv)-1, argv+1, argv) );
					val = *argv;
					break;
				}
				case JLTFunction: {

					Read(cx, buf);
					JSXDRState *xdr;
					xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
					JS_XDRMemSetData(xdr, const_cast<void*>(buf.Data()), buf.Length());
					JL_CHK( JS_XDRValue(xdr, &val) );
					JL_S_ASSERT_FUNCTION( val );
					JS_XDRMemSetData(xdr, NULL, 0);
					JS_XDRDestroy(xdr);
					JL_CHK( JS_SetParent(cx, JSVAL_TO_OBJECT(val), JL_GetGlobalObject(cx)) );
					break;
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



	inline Serializer& JsvalToSerializer( JSContext *cx, jsval &val ) {

//		return *static_cast<Serializer*>(JSVAL_TO_PRIVATE(val));
		return *static_cast<Serializer*>(JL_GetPrivate(cx, JSVAL_TO_OBJECT(val)));
	}

	inline Unserializer& JsvalToUnserializer( JSContext *cx, jsval &val ) {

//		return *static_cast<Unserializer*>(JSVAL_TO_PRIVATE(val));
		return *static_cast<Unserializer*>(JL_GetPrivate(cx, JSVAL_TO_OBJECT(val)));
	}

}
