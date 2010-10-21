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

	class SerializerBufferInfo {
	private:
		const void *_data;
		size_t _length;
	public:
		SerializerBufferInfo() {}

		SerializerBufferInfo( const void *data, size_t length ) : _data(static_cast<const void *>(data)), _length(length) {}

		template <class T>
		SerializerBufferInfo( const T *data, size_t count ) : _data(static_cast<const void *>(data)), _length(sizeof(T)*count) {}

		const void * Data() const {

			return _data;
		}

		size_t Length() const {

			return _length;
		}
	};

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
		JSContext *cx;

		uint8_t *_start;
		uint8_t *_pos;
		size_t _length;

		void PrepareBytes( size_t length ) {

			size_t offset = _pos - _start;
			if ( offset + length > _length ) {

				_length = _length * 2 + length;
				_start = (uint8_t*)jl_realloc(_start, _length); // if ( _start == NULL ) jl_alloc(_length)
				JL_ASSERT( _start != NULL );
				_pos = _start + offset;
			}
		}

		Serializer();
		Serializer( const Serializer & );

	public:

		~Serializer() {

			jl_free(_start);
		}

		Serializer( JSContext *cx ) : cx(cx), _start(NULL), _pos(NULL), _length(0) {

			PrepareBytes(JL_PAGESIZE);
		}

		void Free() {

			jl_free(_start);
			_start = NULL;
			_pos = NULL;
			_length = 0;
		}

		const void * Data() const {

			return _start;
		}

		size_t Length() const {

			return _pos - _start;
		}

		Serializer& operator <<( const char *buf ) {

			size_t length = strlen(buf)+1;
			*this << length;
			PrepareBytes(length);
			memcpy(_pos, buf, length);
			_pos += length;
			return *this;
		}

		Serializer& operator <<( const SerializerBufferInfo &buf ) {

			*this << buf.Length();
			if ( buf.Length() > 0 ) {

				PrepareBytes(buf.Length());
				memcpy(_pos, buf.Data(), buf.Length());
				_pos += buf.Length();
			}
			return *this;
		}

		Serializer& operator <<( const SerializerObjectProperties &sop ) {

			JSObject *obj = sop;

			JSIdArray *idArray = JS_Enumerate(cx, obj); // Get an array of the all own enumerable properties of a given object.
			*this << idArray->length;
			jsval name, value;
			for ( int i = 0; i < idArray->length; ++i ) {

				name = ID_TO_VALUE( idArray->vector[i] ); // JL_CHK( JS_IdToValue(cx, idArray->vector[i], &name) );
				JSString *jsstr = JSVAL_IS_STRING(name) ? JSVAL_TO_STRING(name) : JS_ValueToString(cx, name);
				*this << JS_GetStringBytesZ(cx, jsstr);
				JL_CHK( obj->getProperty(cx, idArray->vector[i], &value) );
				*this << value;
			}
			JS_DestroyIdArray(cx, idArray);
			return *this;
		bad:
			throw JS_FALSE;
		}

		Serializer& operator <<( const jsval &val ) {

			char type;
			if ( JSVAL_IS_PRIMITIVE(val) ) {

//				switch (JS_TypeOfValue(cx, val)) {
//				}


				if ( JSVAL_IS_INT(val) ) {

					type = JLTInt;
					*this << type << JSVAL_TO_INT(val);
				} else
				if ( JSVAL_IS_STRING(val) ) {

					type = JLTString;
					JSString *jsstr = JSVAL_TO_STRING(val);
					*this << type << SerializerBufferInfo(JS_GetStringChars(jsstr), JS_GetStringLength(jsstr));
				} else
				if ( JSVAL_IS_VOID(val) ) {

					type = JLTVoid;
					*this << type;
				} else
				if ( JSVAL_IS_BOOLEAN(val) ) {

					type = JLTBool;
					*this << type << char(JSVAL_TO_BOOLEAN(val));
				} else
				if ( JSVAL_IS_DOUBLE(val) ) {

					type = JLTDouble;
					*this << type << *JSVAL_TO_DOUBLE(val);
				} else
				if ( val == JSVAL_HOLE ) {

					type = JLTHole;
					*this << type;
				} else
				if ( JSVAL_IS_NULL(val) ) {

					type = JLTNull;
					*this << type;
				} else
					JL_REPORT_ERROR("Unsupported value.");

			} else { // objects

				JSObject *obj = JSVAL_TO_OBJECT(val);

				if ( JS_IsArrayObject(cx, obj) ) {

					type = JLTArray;
					jsuint length;
					JL_CHK( JS_GetArrayLength(cx, obj, &length) );
					*this << type << length;

					JSBool found;
					jsval elt;
					for ( jsint i = 0; i < jl::SafeCast<jsint>(length); ++i ) {

						JL_CHK( JS_GetElement(cx, obj, i, &elt) );
						if ( JSVAL_IS_VOID( elt ) ) {

							JL_CHK( JS_HasElement(cx, obj, i, &found) );
							if ( !found )
								elt = JSVAL_HOLE;
						}
						*this << elt;
					}
				} else
				if ( obj->isFunction() ) { // if ( JS_ObjectIsFunction(cx, obj) ) { // JsvalIsFunction(cx, val)

					type = JLTFunction;
					JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
					JL_CHK( JS_XDRValue(xdr, const_cast<jsval*>(&val)) ); // JSXDR_ENCODE, de-const can be done
					uint32 length;
					void *buf = JS_XDRMemGetData(xdr, &length);
					JL_ASSERT( buf );
					*this << type << SerializerBufferInfo(buf, length);
					JS_XDRDestroy(xdr);
				} else {

					jsval serializeFctVal;
					JL_CHK( JS_GetMethodById(cx, obj, JLID(cx, _serialize), NULL, &serializeFctVal) ); // JL_CHK( JS_GetProperty(cx, obj, "_serialize", &serializeFctVal) );

					if ( !JSVAL_IS_VOID( serializeFctVal ) ) {

						JL_ASSERT( JsvalIsFunction(cx, serializeFctVal) );

						JSFunction *fct = JS_ValueToFunction(cx, serializeFctVal);
						if ( fct->isInterpreted() ) { // weakly unreliable

							type = JLTInterpretedObject;
							*this << type;
							jsval unserializeFctVal;
							JL_CHK( JS_GetMethodById(cx, obj, JLID(cx, _unserialize), NULL, &unserializeFctVal) );
							JL_S_ASSERT_FUNCTION( unserializeFctVal );
							*this << unserializeFctVal;
							jsval argv[] = { JSVAL_NULL };
							js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
							JL_CHK( JS_CallFunctionValue(cx, obj, serializeFctVal, COUNTOF(argv-1), argv+1, argv) );
							*this << *argv;
						} else {

							type = JLTNativeObject;
							*this << type << obj->getClass()->name;
							jsval argv[] = { JSVAL_NULL, PRIVATE_TO_JSVAL(this) };
							js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
							JL_CHK( JS_CallFunctionValue(cx, obj, serializeFctVal, COUNTOF(argv-1), argv+1, argv) );
						}

					} else
					if ( JL_ObjectIsObject(cx, obj) ) {

						type = JLTObject;
						*this << type << SerializerObjectProperties(obj);
					} else { // fallback

						type = JLTObjectValue;
						*this << type << obj->getClass()->name;
						jsval value;
						JL_CHK( obj->defaultValue(cx, JSTYPE_VOID, &value) ); // JS_ConvertValue(cx, val, JSTYPE_VOID, &value);
						*this << value;
					}
				}
			}

			return *this;
		bad:
			throw JS_FALSE;
		}

		template <class T>
		Serializer& operator <<( const T value ) {

			PrepareBytes(sizeof(T));
			*(T*)_pos = value;
			_pos += sizeof(T);
			return *this;
		}

	};


	class Unserializer {
	private:
		JSContext *cx;

		const uint8_t *_start;
		const uint8_t *_pos;
		size_t _length;

		bool AssertData( size_t length ) const {

			return (_pos - _start) + length <= _length;
		}

		Unserializer( const Unserializer & );

	public:
		Unserializer( JSContext *cx, const void *data, size_t length )
			: cx(cx), _start((const uint8_t *)data), _pos(_start), _length(length) {
		}

		Unserializer( JSContext *cx, const Serializer &ser )
			: cx(cx), _start((const uint8_t *)ser.Data()), _pos(_start), _length(ser.Length()) {
		}

		Unserializer& operator >>( const char *&buf ) {

			size_t length;
			*this >> length;
			if ( !AssertData(length) )
				throw JS_FALSE;
			buf = (const char*)_pos;
			_pos += length;
			return *this;
		}

		Unserializer& operator >>( SerializerBufferInfo &buf ) {

			size_t length;
			*this >> length;
			if ( length > 0 ) {

				if ( !AssertData(length) )
					throw JS_FALSE;
				buf = SerializerBufferInfo(_pos, length);
				_pos += length;
			} else {

				buf = SerializerBufferInfo(NULL, 0);
			}
			return *this;
		}

		Unserializer& operator >>( SerializerObjectProperties &sop ) {

			jsint length;
			*this >> length;
//			JSObject *obj = JS_NewObject(cx, NULL, NULL, NULL);

			const char *name;
			for ( int i = 0; i < length; ++i ) {

				*this >> name;
				jsval value;
				*this >> value;
				JL_CHK( JS_SetProperty(cx, /*obj*/sop, name, &value) );
			}

//			sop = SerializerObjectProperties(obj);

			return *this;
		bad:
			throw JS_FALSE;
		}

		Unserializer& operator >>( jsval &val ) {

			SerializerBufferInfo buf;
			char type;
			*this >> type;

			switch (type) {

				case JLTInt: {

					jsint i;
					*this >> i;
					val = INT_TO_JSVAL(i);
					break;
				}
				case JLTString: {

					*this >> buf;
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
					*this >> b;
					val = BOOLEAN_TO_JSVAL(b);
					break;
				}
				case JLTDouble: {

					jsdouble d;
					*this >> d;
					JL_CHK( JS_NewDoubleValue(cx, d, &val) );
					break;
				}
				case JLTHole: {

					val = JSVAL_HOLE; // jsbool.h
					break;
				}
				case JLTNull: {

					val = JSVAL_NULL;
					break;
				}

				case JLTArray: {

					jsuint length;
					*this >> length;
					JSObject *arr;
					arr = JS_NewArrayObject(cx, length, NULL);
					JL_S_ASSERT_ALLOC( arr );
					val = OBJECT_TO_JSVAL(arr);

					jsval elt;
					for ( jsuint i = 0; i < length; ++i ) {

						*this >> elt;
						if ( elt != JSVAL_HOLE ) // optional check
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
					*this >> sop;
					break;
				}
				case JLTObjectValue: {

					const char *className;
					*this >> className;
					jsval value;
					*this >> value;
					JSObject *scope = JS_GetScopeChain(cx); // JS_GetGlobalForScopeChain
					jsval prop;
					JL_CHK( JS_GetProperty(cx, scope, className, &prop) );
					JSObject *newObj = JS_New(cx, JSVAL_TO_OBJECT(prop), 1, &value);
					val = OBJECT_TO_JSVAL(newObj);
					break;
				}
				case JLTInterpretedObject: {

					jsval unserializeFctVal;
					*this >> unserializeFctVal;
					JL_S_ASSERT_FUNCTION( unserializeFctVal );
					JSObject *globalObj;
					globalObj = JS_GetGlobalObject(cx);
					JL_CHK( JS_SetParent(cx, JSVAL_TO_OBJECT(unserializeFctVal), globalObj) );
					jsval value;
					*this >> value;
					jsval argv[] = { JSVAL_NULL, value };
					js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
					JL_CHK( JS_CallFunctionValue(cx, globalObj, unserializeFctVal, COUNTOF(argv-1), argv+1, argv) );
					val = *argv;
					break;
				}
				case JLTNativeObject: {

					const char *className;
					*this >> className;

					JSObject *scope = JS_GetScopeChain(cx); // JS_GetGlobalForScopeChain
					jsval constructorVal;
					JL_CHK( JS_GetProperty(cx, scope, className, &constructorVal) );
					JSObject *constructorObj = JSVAL_TO_OBJECT(constructorVal); //JSFunction *fun = JS_ValueToConstructor(cx, constructor);
					jsval prototypeVal;
					JL_CHK( JS_GetPropertyById(cx, constructorObj, ATOM_TO_JSID(cx->runtime->atomState.classPrototypeAtom) /* "prototype" */, &prototypeVal) );
					JSObject *proto = JSVAL_TO_OBJECT(prototypeVal);

					jsval argv[] = { JSVAL_NULL, PRIVATE_TO_JSVAL(this) };
					js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
					JL_CHK( JS_CallFunctionId(cx, proto, JLID(cx, _unserialize), COUNTOF(argv-1), argv+1, argv) );
					val = *argv;
					break;
				}
				case JLTFunction: {

					*this >> buf;
					JSXDRState *xdr;
					xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
					JS_XDRMemSetData(xdr, const_cast<void*>(buf.Data()), buf.Length());
					JL_CHK( JS_XDRValue(xdr, &val) );
					JL_S_ASSERT_FUNCTION( val );
					JS_XDRMemSetData(xdr, NULL, 0);
					JS_XDRDestroy(xdr);
					JL_CHK( JS_SetParent(cx, JSVAL_TO_OBJECT(val), JS_GetGlobalObject(cx)) );
					break;
				}
			}

			return *this;
		bad:
			throw JS_FALSE;
		}

		Unserializer& operator >>( jsval *&val ) {

			return operator >>(*val);
		}


		template <class T>
		Unserializer& operator >>( T &value ) {

			if ( !AssertData(sizeof(T)) )
				throw JS_FALSE;
			value = *(T*)_pos;
			_pos += sizeof(T);
			return *this;
		}

	};



	inline Serializer& JsvalToSerializer(jsval &val) {

		return *static_cast<Serializer*>(JSVAL_TO_PRIVATE(val));
	}

	inline Unserializer& JsvalToUnserializer(jsval &val) {

		return *static_cast<Unserializer*>(JSVAL_TO_PRIVATE(val));
	}

}
