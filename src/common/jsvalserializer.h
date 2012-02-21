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


#pragma once

namespace jl {

	typedef enum JLSerializeType {

		JLSTRawJsval,
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
		JLSTErrorObject,
		JLSTObjectValue,
		JLSTSerializableNativeObject,
		JLSTSerializableScriptObject
	} JLSerializeType;


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


	class SerializerObjectOwnProperties {
		JSObject *&_obj;

		SerializerObjectOwnProperties();
		SerializerObjectOwnProperties( const SerializerObjectOwnProperties & );
		SerializerObjectOwnProperties & operator=( const SerializerObjectOwnProperties & );
	public:
		SerializerObjectOwnProperties( JSObject *&obj ) : _obj(obj) {}

		operator JSObject*() const {

			return _obj;
		}
	};


	class SerializerObjectReservedSlots {
		JSObject *&_obj;

		SerializerObjectReservedSlots();
		SerializerObjectReservedSlots( const SerializerObjectReservedSlots & );
		SerializerObjectReservedSlots & operator=( const SerializerObjectReservedSlots & );
	public:
		SerializerObjectReservedSlots( JSObject *&obj ) : _obj(obj) {}

		operator JSObject*() const {

			return _obj;
		}
	};


	class Serializer : public CppAllocators {

		jsval _serializerObj;

		uint8_t *_start;
		uint8_t *_pos;
		size_t _length;

		bool PrepareBytes( size_t length, bool exact = false ) {

			size_t offset = _pos - _start;
			if ( offset + length > _length ) {

				_length = _length * (exact ? 1 : 2) + length;
				_start = static_cast<uint8_t*>(jl_realloc(_start, sizeof(uint8_t) * _length));
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
				jl_free(_start); // jl_free(NULL) is legal, but here is an optimization.
		}

		Serializer( jsval serializerObj )
		: _serializerObj(serializerObj), _start(NULL), _pos(NULL), _length(0) {

			PrepareBytes(JL_PAGESIZE / 2);
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


		JSBool Write( JSContext *cx, const char *cstr ) {

			size_t length = strlen(cstr) + 1; // + 1 for the '\0' 
			JL_CHK( Write(cx, length) );
			JL_CHK( PrepareBytes(length) );
			memcpy(_pos, cstr, length);
			_pos += length;
			return JS_TRUE;
			JL_BAD;
		}

		ALWAYS_INLINE JSBool
		Write( JSContext *cx, const JLSerializeType type ) {
			
			ASSERT( type >= 0 && type <= 255 );
			return Write(cx, (const unsigned char)type);
		}

		JSBool Write( JSContext *cx, JSString *jsstr ) {

			size_t length;
			const jschar *chars;
			chars = JS_GetStringCharsAndLength(cx, jsstr, &length); // doc. not null-terminated.
			return Write(cx, SerializerConstBufferInfo(chars, length));
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

		JSBool Write( JSContext *cx, const SerializerObjectOwnProperties &sop ) {

			JSObject *obj = sop;

			JSIdArray *idArray = JS_Enumerate(cx, obj); // Get an array of the all *own* enumerable properties of a given object.
			JL_CHK( idArray );
			JL_CHK( Write(cx, idArray->length) );
			jsval name, value;
			for ( int i = 0; i < idArray->length; ++i ) {

				JL_CHK( JS_IdToValue(cx, idArray->vector[i], &name) );
				JSString *jsstr = JSVAL_IS_STRING(name) ? JSVAL_TO_STRING(name) : JS_ValueToString(cx, name);
				JL_CHK( jsstr );
				JL_CHK( JS_GetPropertyById(cx, obj, idArray->vector[i], &value) );
				JL_CHK( Write(cx, jsstr) );
				JL_CHK( Write(cx, value) );
			}
			JS_DestroyIdArray(cx, idArray);

/*
			jsid id;
			jsval value;
			JSObject *it = JS_NewPropertyIterator(cx, obj);

			for (;;) {

				JL_CHK( JS_NextProperty(cx, it, &id) );
				if ( JSID_IS_VOID(id) )
					break;

				ASSERT( JSID_IS_STRING(id) );
				// JL_CHK( JS_IdToValue(cx, id, &key) );

				JL_CHK( JS_GetPropertyById(cx, obj, id, &value) );

				JL_CHK( Write(cx, JSID_TO_STRING(id)) );
				JL_CHK( Write(cx, value) );
			}
*/
			return JS_TRUE;
		bad:

			if ( idArray )
				JS_DestroyIdArray(cx, idArray);
			return JS_FALSE;
		}

		JSBool Write( JSContext *cx, const SerializerObjectReservedSlots &srs ) {

			JSObject *obj = srs;
			jsval value;
			uint32_t reservedSlotsCount = JSCLASS_RESERVED_SLOTS(JL_GetClass(obj));
			JL_CHK( Write(cx, reservedSlotsCount) );
			for ( uint32_t i = 0; i < reservedSlotsCount; ++i ) {

				JL_CHK( JL_GetReservedSlot(cx, obj, i, &value) );
				JL_CHK( Write(cx, value) );
			}
			return JS_TRUE;
			JL_BAD;
		}
		

		JSBool Write( JSContext *cx, const jsval &val ) {

			if ( JSVAL_IS_PRIMITIVE(val) ) {

				if ( JSVAL_IS_INT(val) ) {

					JL_CHK( Write(cx, JLSTInt) );
					JL_CHK( Write(cx, JSVAL_TO_INT(val)) );
				} else
				if ( JSVAL_IS_STRING(val) ) {

					JL_CHK( Write(cx, JLSTString) );
					JL_CHK( Write(cx, JSVAL_TO_STRING(val)) );
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
					if ( val.isMagic(JS_ARRAY_HOLE) ) {

					JL_CHK( Write(cx, JLSTHole) );
				} else
				if ( JSVAL_IS_NULL(val) ) {

					JL_CHK( Write(cx, JLSTNull) );
				}

				// else {
				//	JL_CHK( Write(cx, JLSTRawJsval) );
				//	JL_CHK( Write(cx, *(uint64*)&val) ); // JL_REPORT_ERROR("Unsupported value.");
				//}

				return JS_TRUE;
			}

			// objects

			JSObject *obj;
			obj = JSVAL_TO_OBJECT(val);

			if ( JL_ObjectIsArray(cx, obj) ) { // real array object, not array-like !

				jsuint length;
				JL_CHK( JS_GetArrayLength(cx, obj, &length) );
				JL_CHK( Write(cx, JLSTArray) );
				JL_CHK( Write(cx, length) );

				JSBool found;
				jsval tmp;
				for ( jsint i = 0; i < jl::SafeCast<jsint>(length); ++i ) {

					JL_CHK( JL_GetElement(cx, obj, i, &tmp) );
					if ( JSVAL_IS_VOID(tmp) ) {

						JL_CHK( JS_HasElement(cx, obj, i, &found) );
						if ( !found )
							tmp.setMagic(JS_ARRAY_HOLE);
					}
					JL_CHK( Write(cx, tmp) );
				}
				return JS_TRUE;
			}
			
			if ( JL_ObjectIsCallable(cx, obj) ) {

/* JS_XDRValue fails because function chasp has no XDR encoding hook
				JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
				JL_CHK( JS_XDRValue(xdr, const_cast<jsval*>(&val)) ); // JSXDR_ENCODE, de-const can be done
				uint32_t length;
				void *buf = JS_XDRMemGetData(xdr, &length);
				ASSERT( buf );
				JL_CHK( Write(cx, JLSTFunction) );
				JL_CHK( Write(cx, SerializerConstBufferInfo(buf, length)) );
				JS_XDRDestroy(xdr);
*/
				JSString *src;
				src = JS_ValueToSource(cx, OBJECT_TO_JSVAL(obj));
				JL_ASSERT( src, E_JSLIBS, E_INTERNAL ); // "Unable to get function source."
				JL_CHK( Write(cx, JLSTFunction) );
				return Write(cx, src);
			}

			jsval serializeFctVal;
			JL_CHK( JS_GetMethodById(cx, obj, JLID(cx, _serialize), NULL, &serializeFctVal) ); // JL_CHK( JS_GetProperty(cx, obj, "_serialize", &serializeFctVal) );

			if ( !JSVAL_IS_VOID( serializeFctVal ) ) {

				jsval argv[] = { JSVAL_NULL, _serializerObj };
				JL_ASSERT( JL_ValueIsCallable(cx, serializeFctVal), E_OBJ, E_NAME(JL_GetClassName(obj)), E_INTERNAL, E_SEP, E_TY_FUNC, E_NAME("_serialize"), E_DEFINED );

//				JSObject *objectProto;
//				JL_CHK( JL_GetClassPrototype(cx, NULL, JSProto_Object, &objectProto) );
				if ( !JL_ObjectIsObject(cx, obj ) ) {
//				if ( JL_GetClass(obj) != JL_GetStandardClassByKey(cx, JSProto_Object) ) { // native serializable object

					JL_CHK( Write(cx, JLSTSerializableNativeObject) );
					JL_CHK( Write(cx, JL_GetClassName(obj)) );
					JL_CHK( JS_CallFunctionValue(cx, obj, serializeFctVal, COUNTOF(argv)-1, argv+1, argv) ); // rval not used
					return JS_TRUE;
				}

				// JS object
				JL_CHK( Write(cx, JLSTSerializableScriptObject) );

/*
				JSObject *objectConstructor;
				objectConstructor = JS_GetConstructor(cx, objectProto);
				ASSERT( objectConstructor != NULL );
				
				JSObject *proto;
				proto = JS_GetPrototype(cx, obj);
				JL_CHKM( proto, "Invalid class prototype." );

				JSObject *constructor;
				constructor = JS_GetConstructor(cx, proto);
				JL_CHKM( constructor && JL_ObjectIsCallable(cx, constructor), "Constructor not found." );
				JL_CHKM( constructor != objectConstructor, "Invalid constructor." );

				JSString *funName;
				funName = JS_GetFunctionId(JL_ObjectToFunction(cx, constructor)); // see also. JS_ValueToConstructor()
				JL_CHKM( funName != NULL, "Constructor name not found." );

				size_t length;
				const jschar *chars;
				chars = JS_GetStringCharsAndLength(funName, &length); // doc. not null-terminated.
				JL_CHKM( chars && *chars, "Invalid constructor name." );
				JL_CHK( Write(cx, SerializerConstBufferInfo(chars, length)) );

				JL_CHK( JS_CallFunctionValue(cx, obj, serializeFctVal, COUNTOF(argv)-1, argv+1, argv) );
*/
				jsval unserializeFctVal;
				JL_CHK( JS_GetMethodById(cx, obj, JLID(cx, _unserialize), NULL, &unserializeFctVal) );
				JL_ASSERT( JL_ValueIsCallable(cx, unserializeFctVal), E_OBJ, E_NAME(JL_GetClassName(obj)), E_INTERNAL, E_SEP, E_TY_FUNC, E_NAME("_unserialize"), E_DEFINED );

				JL_CHK( Write(cx, unserializeFctVal) );
				JL_CHK( JS_CallFunctionValue(cx, obj, serializeFctVal, COUNTOF(argv)-1, argv+1, argv) ); // rval not used
				return JS_TRUE;
			}

			// prototype-less object
			if ( JS_GetPrototype(obj) == NULL ) {

				JL_CHK( Write(cx, JLSTProtolessObject) );
				JL_CHK( Write(cx, SerializerObjectOwnProperties(obj)) );
				return JS_TRUE;
			}

			// simple object
			if ( obj != NULL && JL_ObjectIsObject(cx, obj) ) {

				JL_CHK( Write(cx, JLSTObject) );
				JL_CHK( Write(cx, SerializerObjectOwnProperties(obj)) );
				return JS_TRUE;
			}

			if ( JL_ObjectIsError(cx, obj) ) {

				JSObject *constructor;
				constructor = JL_GetConstructor(cx, obj);

				jsval constructorName;
				JL_CHK( JS_GetPropertyById(cx, constructor, JLID(cx, name), &constructorName) );
				ASSERT( JSVAL_IS_STRING(constructorName) );

				const jschar *str;
				size_t length;
				str = JS_GetStringCharsAndLength(cx, JS_ValueToString(cx, constructorName), &length);

				JL_CHK( Write(cx, JLSTErrorObject) );
				JL_CHK( Write(cx, SerializerConstBufferInfo(str, length)) );

				//JL_CHK( Write(cx, SerializerObjectOwnProperties(obj)) ); // (TBD) why this don't work on error object ?
				
				jsval tmp;
				JL_CHK( JS_GetPropertyById(cx, obj, JLID(cx, message), &tmp) );
				JL_CHK( Write(cx, tmp) );
				JL_CHK( JS_GetPropertyById(cx, obj, JLID(cx, fileName), &tmp) );
				JL_CHK( Write(cx, tmp) );
				JL_CHK( JS_GetPropertyById(cx, obj, JLID(cx, lineNumber), &tmp) );
				JL_CHK( Write(cx, tmp) );
				return JS_TRUE;
			}

			// fallback (Date, Number, String, ...)
			{
			jsval tmp;
			JL_CHK( JL_JsvalToPrimitive(cx, OBJECT_TO_JSVAL(obj), &tmp) );
			JL_CHK( Write(cx, JLSTObjectValue) );
			JL_CHK( Write(cx, JL_GetClass(obj)->name) );
			JL_CHK( Write(cx, tmp) );
			return JS_TRUE;
			}

			JL_BAD;
		}

		template <class T>
		JSBool Write( JSContext *cx, const T &value ) {

			JL_IGNORE(cx);
			JL_CHK( PrepareBytes(sizeof(T)) );
			*(T*)_pos = value;
			_pos += sizeof(T);
			return JS_TRUE;
			JL_BAD;
		}

	};




	class Unserializer : public CppAllocators {

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
			JL_CHK( AssertData(length) );
			buf = (const char*)_pos;
			_pos += length;
			return JS_TRUE;
			JL_BAD;
		}

		JSBool Read( JSContext *cx, JSString *&jsstr ) {

			SerializerConstBufferInfo buf;
			JL_CHK( Read(cx, buf) );
			jsstr = JS_NewUCStringCopyN(cx, (const jschar *)buf.Data(), buf.Length()/2);
			JL_CHK(jsstr);
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

		JSBool Read( JSContext *cx, SerializerObjectOwnProperties &sop ) {

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

		JSBool Read( JSContext *cx, SerializerObjectReservedSlots &srs ) {

			JSObject *obj = srs;
			jsval value;
			uint32_t reservedSlotsCount;
			JL_CHK( Read(cx, reservedSlotsCount) );
			for ( uint32_t i = 0; i < reservedSlotsCount; ++i ) {

				JL_CHK( Read(cx, value) );
				JL_CHK( JL_SetReservedSlot(cx, obj, i, value) );
			}
			return JS_TRUE;
			JL_BAD;
		}


		JSBool Read( JSContext *cx, jsval &val ) {

			char type;
			JL_CHK( Read(cx, type) );

			switch ( type ) {

				//case JLSTRawJsval: {
				//	
				//	JL_CHK( Read(cx, JSVAL_BITS(val)) );
				//	break;
				//}

				case JLSTInt: {

					jsint i;
					JL_CHK( Read(cx, i) );
					val = INT_TO_JSVAL(i);
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

					val.setMagic(JS_ARRAY_HOLE);
					break;
				}
				case JLSTNull: {

					val = JSVAL_NULL;
					break;
				}

				case JLSTString: {

					JSString *jsstr;
					JL_CHK( Read(cx, jsstr) );
					val = STRING_TO_JSVAL(jsstr);
					break;
				}
				case JLSTArray: {

					jsuint length;
					JL_CHK( Read(cx, length) );
					JSObject *arr;
					arr = JS_NewArrayObject(cx, length, NULL);
					JL_ASSERT_ALLOC( arr );
					val = OBJECT_TO_JSVAL(arr);

					jsval tmp;
					for ( jsuint i = 0; i < length; ++i ) {

						JL_CHK( Read(cx, tmp) );
						if ( !tmp.isMagic(JS_ARRAY_HOLE) ) // if ( !JL_JSVAL_IS_ARRAY_HOLE(*avr.jsval_addr()) )
							JL_CHK( JL_SetElement(cx, arr, i, &tmp) );
					}
					break;
				}
				case JLSTObject: {

					JSObject *obj = JL_NewObj(cx);
					JL_CHK( obj );
					val = OBJECT_TO_JSVAL(obj);
					SerializerObjectOwnProperties sop(obj);
					JL_CHK( Read(cx, sop) );
					break;
				}
				case JLSTProtolessObject: {

					JSObject *obj = JL_NewProtolessObj(cx);
					JL_CHK( obj );
					val = OBJECT_TO_JSVAL(obj);
					SerializerObjectOwnProperties sop(obj);
					JL_CHK( Read(cx, sop) );
					break;
				}
				case JLSTObjectValue: {

					const char *className;
					jsval value;
					JL_CHK( Read(cx, className) );
					JL_CHK( Read(cx, value) );
					JSObject *scope = JS_GetGlobalForScopeChain(cx); //JS_GetScopeChain(cx);
					jsval prop;
					JL_CHK( JS_GetProperty(cx, scope, className, &prop) );
					JSObject *newObj = JS_New(cx, JSVAL_TO_OBJECT(prop), 1, &value);
					val = OBJECT_TO_JSVAL(newObj);
					break;
				}
				case JLSTSerializableNativeObject: {

					const char *className;
					JL_CHK( Read(cx, className) );
					ASSERT(strlen(className) > 0);
					ASSERT(strlen(className) < 64);
					ClassProtoCache *cpc = JL_GetCachedClassProto(JL_GetHostPrivate(cx), className);
					JL_CHKM( cpc != NULL, E_CLASS, E_NAME(className), E_NOTFOUND );
					JSObject *newObj;
					newObj = JS_NewObjectWithGivenProto(cx, cpc->clasp, cpc->proto, NULL);
					JL_CHK( newObj );
 					jsval argv[] = { JSVAL_NULL, _unserializerObj };
					JL_CHK( JL_CallFunctionId(cx, newObj, JLID(cx, _unserialize), COUNTOF(argv)-1, argv+1, argv) ); // rval not used
					val = OBJECT_TO_JSVAL(newObj);
					break;
				}
				case JLSTSerializableScriptObject: {
/*
					SerializerConstBufferInfo buf;
					JL_CHK( Read(cx, buf) );

					JSObject *scope;
					scope = JS_GetScopeChain(cx);

					jsval constructorVal;
					JL_CHK( JS_GetUCProperty(cx, scope, (const jschar*)buf.Data(), buf.Length()/2, &constructorVal) );
					//JL_CHK( JS_LookupUCProperty(cx, scope, (const jschar*)buf.Data(), buf.Length()/2, &constructorVal) );

					JL_CHKM( !JSVAL_IS_VOID(constructorVal), "Constructor not found." );
					JSObject *newObj;
					newObj = JS_New(cx, JSVAL_TO_OBJECT(constructorVal), 0, NULL);
					JL_CHK( newObj );

 					jsval argv[] = { JSVAL_NULL, _unserializerObj };

					JL_CHK( JL_CallFunctionId(cx, newObj, JLID(cx, _unserialize), COUNTOF(argv)-1, argv+1, argv) );
					val = OBJECT_TO_JSVAL(newObj);
*/
 					jsval argv[] = { JSVAL_NULL, _unserializerObj };
					jsval fun;
					JL_CHK( Read(cx, fun) );
					
//					JSObject *parent = JL_GetParent(cx, JSVAL_TO_OBJECT(_unserializerObj));
//					JL_CHK( JS_SetParent(cx, JSVAL_TO_OBJECT(fun), parent) );
					JL_ASSERT( JL_ValueIsCallable(cx, fun), E_STR("unserializer"), E_STATE ); // JLSMSG_INVALID_OBJECT_STATE, "Unserializer"

					JL_CHK( JS_CallFunctionValue(cx, JL_GetGlobalObject(cx), fun, COUNTOF(argv)-1, argv+1, argv) );
					JL_ASSERT( JSVAL_IS_OBJECT(argv[0]), E_STR("unserializer"), E_RETURNVALUE, E_TYPE, E_TY_OBJECT );
					val = *argv;
					break;
				}
				case JLSTErrorObject: {

					SerializerConstBufferInfo constructorName;
					jsval constructor, tmp;
					JL_CHK( Read(cx, constructorName) );
					JL_CHK( JS_GetUCProperty(cx, JL_GetGlobalObject(cx), (const jschar *)constructorName.Data(), constructorName.Length() / 2, &constructor) );
					JSObject *errorObj = JS_NewObjectForConstructor(cx, &constructor);
					val = OBJECT_TO_JSVAL(errorObj);
					JL_CHK( Read(cx, tmp) );
					JL_CHK( JS_SetPropertyById(cx, errorObj, JLID(cx, message), &tmp) );
					JL_CHK( Read(cx, tmp) );
					JL_CHK( JS_SetPropertyById(cx, errorObj, JLID(cx, fileName), &tmp) );
					JL_CHK( Read(cx, tmp) );
					JL_CHK( JS_SetPropertyById(cx, errorObj, JLID(cx, lineNumber), &tmp) );
					break;
				}
				case JLSTFunction: {
/*
					JL_CHK( Read(cx, buf) );
					JSXDRState *xdr;
					xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
					JS_XDRMemSetData(xdr, const_cast<void*>(buf.Data()), buf.Length());
					JL_CHK( JS_XDRValue(xdr, &val) );
					JL_ASSERT_FUNCTION( val );
					JS_XDRMemSetData(xdr, NULL, 0);
					JS_XDRDestroy(xdr);
					JL_CHK( JS_SetParent(cx, JSVAL_TO_OBJECT(val), JL_GetGlobalObject(cx)) );
					JSObject *funProto;
					JL_CHK( JL_GetClassPrototype(cx, NULL, JSProto_Function, &funProto) );
					JL_CHK( JS_SetPrototype(cx, JSVAL_TO_OBJECT(val), funProto) );
*/
					JSString *source;
					JL_CHK( Read(cx, source) );
					JL_CHK( JL_Eval(cx, source, &val) ); // beware, non-functionExpression behavior 
					//JL_CHKM( !JSVAL_IS_VOID(val), 
					break;
				}
				default:
					JL_ERR( E_STR("unserializer"), E_STATE );
			}

			return JS_TRUE;
			JL_BAD;
		}

		template <class T>
		JSBool Read( JSContext *cx, T &value ) {

			JL_IGNORE(cx);
			if ( !AssertData(sizeof(T)) )
				return JS_FALSE;
			value = *(T*)_pos;
			_pos += sizeof(T);
			return JS_TRUE;
		}
	};



	ALWAYS_INLINE bool
	JsvalIsSerializer( JSContext *cx, jsval &val ) {

		return JL_ValueIsClass(val, JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Serializer")->clasp);
	}

	ALWAYS_INLINE Serializer*
	JsvalToSerializer( JSContext *cx, jsval &val ) {

		ASSERT( jl::JsvalIsSerializer(cx, val) );
		return static_cast<Serializer*>(JL_GetPrivate(cx, JSVAL_TO_OBJECT(val)));
	}


	ALWAYS_INLINE bool
	JsvalIsUnserializer( JSContext *cx, jsval &val ) {

		return JL_ValueIsClass(val, JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Unserializer")->clasp);
	}

	ALWAYS_INLINE Unserializer*
	JsvalToUnserializer( JSContext *cx, jsval &val ) {
		
		ASSERT( jl::JsvalIsUnserializer(cx, val) );
		return static_cast<Unserializer*>(JL_GetPrivate(cx, JSVAL_TO_OBJECT(val)));
	}

}

/* Template

#include <jsvalserializer.h>

DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT( jl::JsvalIsSerializer(cx, JL_ARG(1)), "Invalid serializer object." );
	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	//ser->Write(cx, globalKey);

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT( jl::JsvalIsUnserializer(cx, JL_ARG(1)), "Invalid unserializer object." );
	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	//uint32_t gKey;
	//unser->Read(cx, gKey);

	return JS_TRUE;
	JL_BAD;
}


		FUNCTION_ARGC(_serialize, 1)
		FUNCTION_ARGC(_unserialize, 1)
		
*/
