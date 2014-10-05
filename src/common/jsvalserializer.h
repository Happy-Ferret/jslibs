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

#undef JL_CHK
#define JL_CHK( CONDITION ) \
	JL_MACRO_BEGIN \
		if (unlikely( !(CONDITION) )) { \
			goto bad; \
		} \
		ASSUME(!!(CONDITION)); \
	JL_MACRO_END


JL_BEGIN_NAMESPACE

typedef enum {
	JLSTHole,
	JLSTVoid,
	JLSTNull,
	JLSTBool,
	JLSTInt,
	JLSTDouble,
	JLSTString,
//	JLSTLatin1String,
//	JLSTTwoByteString,
	JLSTFunction,
	JLSTArray,
	JLSTObject,
	JLSTProtolessObject,
	JLSTStopIteration,
	JLSTErrorObject,
	JLSTObjectValue,
	JLSTSerializableNativeObject,
	JLSTSerializableScriptObject,
	JLSTArrayBuffer,
	JLSTTypedArray
} JLSerializeType;


class SerializerConstBufferInfo {
	const void *_data;
	size_t _length;
public:
	SerializerConstBufferInfo() {}

	SerializerConstBufferInfo( const void *data, size_t length )
	: _data(static_cast<const void *>(data)), _length(length) {
	}

	template <class T>
	SerializerConstBufferInfo( const T *data, size_t count )
	: _data(static_cast<const void *>(data)), _length(sizeof(T)*count) {
	}

	const void * Data() const {

		return _data;
	}

	size_t Length() const {

		return _length;
	}
};


class SerializerObjectOwnProperties {
	JS::PersistentRootedObject _obj;
private:
	SerializerObjectOwnProperties();
	SerializerObjectOwnProperties( const SerializerObjectOwnProperties & );
	SerializerObjectOwnProperties & operator=( const SerializerObjectOwnProperties & );
public:
	SerializerObjectOwnProperties( JSContext *cx, JS::HandleObject obj )
	: _obj(cx, obj) {
	}

	operator JS::HandleObject() const {

		return _obj;
	}
};


class SerializerObjectReservedSlots {
	JS::PersistentRootedObject _obj;
private:
	SerializerObjectReservedSlots();
	SerializerObjectReservedSlots( const SerializerObjectReservedSlots & );
	SerializerObjectReservedSlots & operator=( const SerializerObjectReservedSlots & );
public:
	SerializerObjectReservedSlots(JSContext *cx, JS::HandleObject obj)
	: _obj(cx, obj) {
	}

	operator JS::HandleObject() const {

		return _obj;
	}
};


/////////////////////////////////////////////////////////////////////////////

class Serializer : public jl::CppAllocators {

	JS::Heap<JS::Value> _serializerObj; // must be used to declare data members of heap classes only

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

private:
	Serializer( const Serializer & );

public:

	~Serializer() {

		if ( _start != NULL )
			jl_free(_start); // jl_free(NULL) is legal, but this is an optimization, since usually one use GetBufferOwnership() that set _start to NULL
	}

	explicit Serializer( JSContext *cx, JS::HandleValue serializerObj = JS::NullPtr() )
	: _serializerObj(serializerObj), _start(NULL), _pos(NULL), _length(0) {

		PrepareBytes(JL_PAGESIZE / 2);
	}

	bool GetBufferOwnership( void **data, size_t *length ) {

		// add '\0'
		JL_CHK( PrepareBytes(1, true) );
		*_pos = 0;

		// get length
		*length = _pos - _start;

		// maybe realloc the buffer
		if ( maybeRealloc(_length, _pos - _start) )
			_start = (uint8_t*)jl_realloc(_start, _pos - _start + 1);
		*data = _start;

		// reset
		_start = NULL; // loose the ownership
		_pos = NULL;
		_length = 0;

		return true;
		JL_BAD;
	}

	template <class T>
	bool WriteRaw( JSContext *cx, const T &value ) {

		JL_IGNORE( cx );
		JL_CHK( PrepareBytes( sizeof( T ) ) );
		*(T*)_pos = value;
		_pos += sizeof( T );
		return true;
		JL_BAD;
	}

	bool Write( JSContext *cx, const double &value ) {

		return WriteRaw( cx, value );
	}

	bool Write( JSContext *cx, const uint32_t &value ) {

		return WriteRaw( cx, value );
	}

	bool Write( JSContext *cx, const int32_t &value ) {

		return WriteRaw( cx, value );
	}
	
	bool Write( JSContext *cx, const uint8_t &value ) {

		return WriteRaw( cx, value );
	}

	bool Write( JSContext *cx, js::Scalar::Type &type ) {

		return WriteRaw( cx, type );
	}

	bool Write(JSContext *cx, const JLSerializeType type) {

		ASSERT(type >= 0 && type <= 255);
		return Write(cx, (uint8_t)type);
	}

	bool Write( JSContext *cx, const char *cstr ) {

		uint32_t length = strlen( cstr ) + 1; // + 1 for the '\0' 
		JL_CHK( Write(cx, length) );
		JL_CHK( PrepareBytes(length) );
		jl::memcpy(_pos, cstr, length);
		_pos += length;
		return true;
		JL_BAD;
	}

	bool Write( JSContext *cx, JS::HandleString str ) {

		JS::AutoCheckCannotGC nogc; // ok

		size_t length;
		if ( JS_StringHasLatin1Chars(str) ) {
			
			JL_CHK( WriteRaw<uint8_t>(cx, 0) ); // !isWide
			const JS::Latin1Char *chars = JS_GetLatin1StringCharsAndLength(cx, nogc, str, &length); // doc. not null-terminated.
			JL_CHK( Write(cx, SerializerConstBufferInfo(chars, length)) );
		} else {

			JL_CHK( WriteRaw<uint8_t>(cx, 1) ); // isWide
			const jschar *chars = JS_GetTwoByteStringCharsAndLength(cx, nogc, str, &length); // doc. not null-terminated.
			JL_CHK( Write(cx, SerializerConstBufferInfo(chars, length)) );
		}
		return true;
		JL_BAD;
	}

	bool Write( JSContext *cx, const SerializerConstBufferInfo &buf ) {

		JL_CHK( Write(cx, buf.Length()) );
		if ( buf.Length() > 0 ) {

			JL_CHK( PrepareBytes(buf.Length()) );
			jl::memcpy(_pos, buf.Data(), buf.Length());
			_pos += buf.Length();
		}
		return true;
		JL_BAD;
	}

	bool Write( JSContext *cx, const SerializerObjectOwnProperties &sop ) {

		JS::RootedValue name(cx);
		JS::RootedValue value(cx);

		JS::AutoIdArray aia(cx, JL_Enumerate(cx, sop)); // Get an array of the all *own* enumerable properties of a given object.
		uint32_t len = aia.length();
		JL_CHK(Write(cx, len));
		for (uint32_t i = 0; i < len; ++i) {

			JS::RootedId item(cx, aia[i]);
			JL_CHK( JS_IdToValue(cx, item, &name) );
			JS::RootedString jsstr(cx, name.isString() ? name.toString() : JS::ToString(cx, name));

			JL_CHK( jsstr );
			JL_CHK( JS_GetPropertyById(cx, sop, item, &value) );
			JL_CHK( Write(cx, jsstr) );
			JL_CHK( Write(cx, value) );
		}

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
		return true;
		JL_BAD;
	}

	bool Write( JSContext *cx, const SerializerObjectReservedSlots &srs ) {

		JS::RootedValue value(cx);
		uint32_t reservedSlotsCount = JSCLASS_RESERVED_SLOTS(JL_GetClass(srs));
		JL_CHK( Write(cx, reservedSlotsCount) );
		for ( uint32_t i = 0; i < reservedSlotsCount; ++i ) {

			JL_CHK( JL_GetReservedSlot(srs, i, &value) );
			JL_CHK( Write(cx, value) );
		}
		return true;
		JL_BAD;
	}

	bool Write( JSContext *cx, JS::HandleValue val ) {

		if ( val.isPrimitive() ) {

			if ( val.isInt32() ) {

				JL_CHK( Write(cx, JLSTInt) );
				JL_CHK( Write(cx, val.toInt32()) );
			} else
			if ( val.isString() ) {

				JS::RootedString str(cx, val.toString());
				JL_CHK( Write(cx, JLSTString) );
				JL_CHK( Write(cx, str) );
/*				
				JS::RootedString str(cx, val.toString());
				if ( JS_StringHasLatin1Chars(str) ) {
					
					JL_CHK( Write(cx, JLSTLatin1String) );
					JL_CHK( Write(cx, str) );
				} else {

					JL_CHK( Write(cx, JLSTTwoByteString) );
					JL_CHK( Write(cx, str) );
				}
*/
			} else
			if ( val.isUndefined() ) {

				JL_CHK( Write(cx, JLSTVoid) );
			} else
			if ( val.isBoolean() ) {

				JL_CHK( Write(cx, JLSTBool) );
				JL_CHK( Write(cx, uint8_t(val.toBoolean())) );
			} else
			if ( val.isDouble() ) {

				JL_CHK( Write(cx, JLSTDouble) );
				JL_CHK( Write(cx, val.toDouble()) );
			} else
				if ( val.isMagic(JS_ELEMENTS_HOLE) ) {

				JL_CHK( Write(cx, JLSTHole) );
			} else
			if ( val.isNull() ) {

				JL_CHK( Write(cx, JLSTNull) );
			} else {

				JL_ERR( E_STR("serializer"), E_STATE, E_INVALID );
			}
			return true;
		}

		// objects
		{
			JS::RootedObject obj(cx, &val.toObject());

			if ( JS_IsArrayBufferObject(obj) ) {

				uint32_t length = JS_GetArrayBufferByteLength(obj);
				uint8_t *data = length ? JS_GetArrayBufferData(obj) : NULL;
				JL_CHK( Write(cx, JLSTArrayBuffer) );
				JL_CHK( Write(cx, SerializerConstBufferInfo(data, length)) );
				return true;
			}

			if ( JS_IsTypedArrayObject(obj) ) {

				uint32_t length = JS_GetTypedArrayByteLength(obj);
				void *data = length ? JS_GetArrayBufferViewData(obj) : NULL;
				JL_CHK( Write(cx, JLSTTypedArray) );
				JL_CHK( Write(cx, JS_GetArrayBufferViewType(obj)) );
				JL_CHK( Write(cx, SerializerConstBufferInfo(data, length)) );
				return true;
			}

			if ( jl::isArray(cx, obj) ) { // real array object, not array-like !!

				uint32_t length;
				JL_CHK( JS_GetArrayLength(cx, obj, &length) );
				JL_CHK( Write(cx, JLSTArray) );
				JL_CHK( Write(cx, length) );

				bool found;
				JS::RootedValue tmp(cx);
				for ( uint32_t i = 0; i < length; ++i ) {

					JL_CHK( JL_GetElement(cx, obj, i, &tmp) );
					if ( tmp.isUndefined() ) {

						JL_CHK( JS_HasElement(cx, obj, i, &found) );
						if ( !found )
							tmp.setMagic(JS_ELEMENTS_HOLE);
					}
					JL_CHK( Write(cx, tmp) );
				}
				return true;
			}

			if ( JS_ObjectIsFunction(cx, obj) ) {

				/* with XDR API */
				uint32_t length;
				void *data = JS_EncodeInterpretedFunction(cx, obj, &length);
				JL_CHK( Write(cx, JLSTFunction) );
				JL_CHK( Write(cx, SerializerConstBufferInfo(data, length)) );
				js_free(data);
				
				/* without XDR API
				JS::RootedString str(cx, JS_ValueToSource(cx, val));
				JL_CHK( Write(cx, JLSTFunction) );
				JL_CHK( Write(cx, str) );
				*/

				return true;
			}

			{

				JS::RootedValue serializeFctVal(cx);

				JL_CHK(JS_GetPropertyById(cx, obj, JLID(cx, _serialize), &serializeFctVal)); // JL_CHK( JS_GetProperty(cx, obj, "_serialize", &serializeFctVal) );

				if ( !serializeFctVal.isUndefined() ) {

					JS::RootedValue arg(cx);

					JL_ASSERT( jl::isCallable(cx, serializeFctVal), E_OBJ, E_NAME(JL_GetClassName(obj)), E_INTERNAL, E_SEP, E_TY_FUNC, E_NAME("_serialize"), E_DEFINED );

					if ( !jl::isObjectObject(cx, obj) ) {

						JL_CHK(Write(cx, JLSTSerializableNativeObject));
						JL_CHK(Write(cx, JL_GetClassName(obj)));
					} else {

						JS::RootedValue unserializeFctVal(cx);
						JL_CHK(JS_GetPropertyById(cx, obj, JLID(cx, _unserialize), &unserializeFctVal));
						JL_ASSERT(jl::isCallable(cx, unserializeFctVal), E_OBJ, E_NAME(JL_GetClassName(obj)), E_INTERNAL, E_SEP, E_TY_FUNC, E_NAME("_unserialize"), E_DEFINED);

						JL_CHK(Write(cx, JLSTSerializableScriptObject));
						JL_CHK(Write(cx, unserializeFctVal));
					}

					JS::RootedObject serializerWrapper(cx, NULL);

					if ( _serializerObj.get().isNull() ) {

						// create a temporary wrapper to this serializer c++ object.

						// (TBD) enhance this by creating an API (eg. serializerPub.h ?)

						//serializerWrapper = JL_NewJslibsObject(cx, "Serializer");
						serializerWrapper = jl::Global::getGlobal(cx)->newJLObject(cx, "Serializer");
						JL_ASSERT_ALLOC( serializerWrapper );
						arg.setObject(*serializerWrapper);
						JL_SetPrivate(serializerWrapper, this);
					} else {

						arg.set(_serializerObj);
					}

					bool ok;
					ok = jl::callNoRval(cx, obj, serializeFctVal, arg);

					if ( serializerWrapper != NULL )
						JL_SetPrivate(serializerWrapper, NULL);

					JL_CHK(ok);

					return true;
				}
			}

			// prototype-less object
			if ( JL_GetPrototype(cx, obj) == NULL ) {

				JL_CHK( Write(cx, JLSTProtolessObject) );
				JL_CHK( Write(cx, SerializerObjectOwnProperties(cx, obj)) );
				return true;
			}

			// StopIteration object
			if ( jl::isStopIteration(cx, obj) ) {
			
				JL_CHK( Write(cx, JLSTStopIteration) );
				return true;
			}

			// simple object
			if ( jl::isObjectObject(cx, obj) ) {

				JL_CHK( Write(cx, JLSTObject) );
				JL_CHK( Write(cx, SerializerObjectOwnProperties(cx, obj)) );
				return true;
			}

			// all JS errors objects
			if ( jl::isError(cx, obj) ) {

				JS::RootedValue tmp(cx);

				JL_CHK(Write(cx, JLSTErrorObject));
				JS::RootedString errCtorName(cx, JS_GetFunctionId(JS_GetObjectFunction(JL_GetConstructor(cx, obj))));
				JL_CHK(Write(cx, errCtorName));
				JL_CHK( JS_GetPropertyById(cx, obj, JLID(cx, message), &tmp) );
				JL_CHK( Write(cx, tmp) );
				JL_CHK( JS_GetPropertyById(cx, obj, JLID(cx, fileName), &tmp) );
				JL_CHK( Write(cx, tmp) );
				JL_CHK( JS_GetPropertyById(cx, obj, JLID(cx, lineNumber), &tmp) );
				JL_CHK( Write(cx, tmp) );
				JL_CHK( JS_GetPropertyById(cx, obj, JLID(cx, stack), &tmp) );
				JL_CHK( Write(cx, tmp) );
				return true;
			}

			// fallback (Date, Number, String, ...)
		
			JS::RootedValue tmp(cx);
			tmp.setObject(*obj);
			JL_CHK( jl::getPrimitive(cx, tmp, &tmp) );
			JL_CHK( Write(cx, JLSTObjectValue) );
			JL_CHK( Write(cx, JL_GetClass(obj)->name) );
			JL_CHK( Write(cx, tmp) );
			return true;
		}

		JL_BAD;
	}
};


/////////////////////////////////////////////////////////////////////////////


class Unserializer : public jl::CppAllocators {

	JS::Heap<JSObject*> _unserializerObj; // must be used to declare data members of heap classes only

	uint8_t *_start;
	uint8_t *_pos;
	size_t _length;

	bool AssertData( size_t length ) const {

		return (_pos - _start) + length <= _length;
	}

private:
	Unserializer( const Unserializer & );

public:

	~Unserializer() {

		jl_free(_start);
	}
	
	explicit Unserializer(JSContext *cx, void *dataOwnership, size_t length, JS::HandleObject unserializerObj = JS::NullPtr()) :
		_unserializerObj(unserializerObj),
		_start((uint8_t*)dataOwnership),
		_pos(_start),
		_length(length) {
	}

	bool IsEmpty() const {
		
		ASSERT( _pos >= _start );
		return  size_t(_pos - _start) < _length;
	}

	template <class T>
	bool ReadRaw( JSContext *cx, T &value ) {

		JL_IGNORE( cx );
		if ( !AssertData( sizeof( T ) ) )
			return false;
		value = *(T*)_pos;
		_pos += sizeof( T );
		return true;
	}

	bool Read( JSContext *cx, double &value ) {

		return ReadRaw( cx, value );
	}

	bool Read( JSContext *cx, uint32_t &value ) {
		
		return ReadRaw( cx, value );
	}

	bool Read( JSContext *cx, int32_t &value ) {

		return ReadRaw( cx, value );
	}

	bool Read( JSContext *cx, uint8_t &value ) {

		return ReadRaw( cx, value );
	}

	bool Read( JSContext *cx, js::Scalar::Type &type ) {
		
		return ReadRaw( cx, type );
	}

	bool Read( JSContext *cx, const char *&buf ) {

		uint32_t length;
		JL_CHK( Read(cx, length) );
		JL_CHK( AssertData(length) );
		buf = (const char*)_pos;
		_pos += length;
		return true;
		JL_BAD;
	}

	bool Read(JSContext *cx, JLSerializeType &type) {

		return ReadRaw(cx, type);
	}

	bool Read( JSContext *cx, JS::MutableHandleString jsstr ) {

		SerializerConstBufferInfo buf;
		uint8_t isWide;
		JL_CHK( Read(cx, isWide) );
		JL_CHK( Read(cx, buf) );
		if ( isWide ) {
		
			jsstr.set( JS_NewUCStringCopyN(cx, (const jschar *)buf.Data(), buf.Length() / 2) );
		} else {
		
			jsstr.set( JS_NewStringCopyN(cx, (const char *)buf.Data(), buf.Length()) );
		}
		JL_CHK( jsstr );
		return true;
		JL_BAD;
	}

	bool Read( JSContext *cx, SerializerConstBufferInfo &buf ) {

		size_t length;
		JL_CHK( Read(cx, length) );
		if ( length > 0 ) {

			if ( !AssertData(length) )
				return false;
			buf = SerializerConstBufferInfo(_pos, length);
			_pos += length;
		} else {

			buf = SerializerConstBufferInfo(NULL, 0);
		}
		return true;
		JL_BAD;
	}

	bool Read( JSContext *cx, SerializerObjectOwnProperties &sop ) {

		uint32_t length;
		JL_CHK( Read(cx, length) );
		for ( uint32_t i = 0; i < length; ++i ) {

			JS::RootedString name(cx);
			JS::RootedId nameId(cx);
			JS::RootedValue value(cx);
			JL_CHK( Read(cx, &name) );
			JL_CHK( Read(cx, &value) );
			JL_CHK( JS_StringToId(cx, name, &nameId) );
			JL_CHK( JS_SetPropertyById(cx, sop, nameId, value) );
		}

		return true;
		JL_BAD;
	}

	bool Read( JSContext *cx, SerializerObjectReservedSlots &srs ) {

		JS::RootedValue value(cx);
		uint32_t reservedSlotsCount;
		JL_CHK( Read(cx, reservedSlotsCount) );
		for ( uint32_t i = 0; i < reservedSlotsCount; ++i ) {

			JL_CHK( Read(cx, &value) );
			JL_CHK( JL_SetReservedSlot(srs, i, (JS::HandleValue)&value) );
		}
		return true;
		JL_BAD;
	}


	bool Read( JSContext *cx, JS::MutableHandleValue val ) {

		uint8_t type;
		JL_CHK( Read(cx, type) );

		switch ( type ) {

			case JLSTInt: {

				int32_t i;
				JL_CHK( Read(cx, i) );
				val.setInt32(i);
				break;
			}
			case JLSTVoid: {

				val.setUndefined();
				break;
			}
			case JLSTBool: {

				uint8_t b;
				JL_CHK( Read(cx, b) );
				val.setBoolean(b);
				break;
			}
			case JLSTDouble: {

				double d;
				JL_CHK( Read(cx, d) );
				val.setDouble(d);
				break;
			}
			case JLSTHole: {

				val.setMagic(JS_ELEMENTS_HOLE);
				break;
			}
			case JLSTNull: {

				val.setNull();
				break;
			}
			case JLSTString: {

				JS::RootedString jsstr(cx);
				JL_CHK( Read(cx, &jsstr) );
				JL_CHK( jsstr );
				val.setString(jsstr);
				break;
			}
/*
				SerializerConstBufferInfo buf;
				uint8_t isWide;
				JL_CHK( Read(cx, isWide) );
				JL_CHK( Read(cx, buf) );
				if ( isWide ) {

					JS::RootedString jsstr(cx, JS_NewUCStringCopyN(cx, (const jschar *)buf.Data(), buf.Length() / 2));
					JL_CHK( jsstr );
					val.setString(jsstr);
				} else {

					JS::RootedString jsstr(cx, JS_NewStringCopyN(cx, (const char *)buf.Data(), buf.Length()));
					JL_CHK( jsstr );
					val.setString(jsstr);
				}
			}
*/
/*
			case JLSTLatin1String: {

				SerializerConstBufferInfo buf;
				JL_CHK( Read(cx, buf) );
				JS::RootedString jsstr(cx, JS_NewStringCopyN(cx, (const char *)buf.Data(), buf.Length()));
				JL_CHK( jsstr );
				val.setString(jsstr);
				break;
			}
			case JLSTTwoByteString: {

				SerializerConstBufferInfo buf;
				JL_CHK( Read(cx, buf) );
				JS::RootedString jsstr(cx, JS_NewUCStringCopyN(cx, (const jschar *)buf.Data(), buf.Length()));
				JL_CHK( jsstr );
				val.setString(jsstr);
				break;
			}
*/
			case JLSTArray: {

				JS::RootedValue tmp(cx);
				uint32_t length;
				JL_CHK( Read(cx, length) );
				JS::RootedObject arr(cx, JS_NewArrayObject(cx, length));
				JL_ASSERT_ALLOC( arr );
				val.setObject(*arr);
				for (uint32_t i = 0; i < length; ++i) {

					JL_CHK( Read(cx, &tmp) );
					if ( !tmp.isMagic(JS_ELEMENTS_HOLE) ) // if ( !JL_JSVAL_IS_ARRAY_HOLE(*avr.jsval_addr()) )
						JL_CHK( JL_SetElement(cx, arr, i, tmp) );
				}
				break;
			}
			case JLSTObject: {

				JS::RootedObject obj(cx, jl::newObject(cx));
				JL_ASSERT_ALLOC( obj );
				val.setObject(*obj);
				SerializerObjectOwnProperties sop(cx, obj);
				JL_CHK( Read(cx, sop) );
				break;
			}
			case JLSTProtolessObject: {

				JS::RootedObject obj(cx, jl::newObjectWithoutProto(cx));
				JL_ASSERT_ALLOC( obj );
				val.setObject(*obj);
				SerializerObjectOwnProperties sop(cx, obj);
				JL_CHK( Read(cx, sop) );
				break;
			}
			case JLSTStopIteration: {

				JS::RootedObject proto(cx);
				JL_CHK( JL_GetClassPrototype(cx, JSProto_StopIteration, &proto) );
				val.setObject(*proto);
				break;
			}
			case JLSTObjectValue: {

				const char *className;
				JS::AutoValueArray<1> ava(cx);
				JS::RootedValue prop(cx);
				JL_CHK( Read(cx, className) );
				JL_CHK( Read(cx, ava[0]) );
				
				JS::RootedObject scope(cx, JL_GetGlobal(cx));
				JL_CHK( JS_GetProperty(cx, scope, className, &prop) );
				JL_CHK( prop.isObject() );

				JS::RootedObject propObj(cx, &prop.toObject());
				JS::RootedObject newObj(cx, JS_New(cx, propObj, ava));

				JL_ASSERT_ALLOC( newObj );
				val.setObject(*newObj);
				break;
			}
			case JLSTSerializableNativeObject: {

				const char *className;
				JL_CHK( Read(cx, className) );
				ASSERT(strlen(className) > 0);
				ASSERT(strlen(className) < 64);

				const jl::ClassInfo* cpc = jl::Global::getGlobal(cx)->getCachedClassInfo(className);

				//const ClassProtoCache *cpc = JL_GetCachedClassInfo(JL_GetHostPrivate(cx), className);

				JL_CHKM( cpc != NULL, E_CLASS, E_NAME(className), E_NOTFOUND );
				JS::RootedObject newObj(cx, jl::newObjectWithGivenProto(cx, cpc->clasp, cpc->proto));
				JL_ASSERT_ALLOC( newObj );

				JS::RootedValue arg(cx);

				JS::RootedObject unserializerWrapper(cx);
				if ( !_unserializerObj ) {

					// (TBD) enhance this by creating an API (eg. serializerPub.h ?)
					//unserializerWrapper = JL_NewJslibsObject(cx, "Unserializer");
					unserializerWrapper = jl::Global::getGlobal(cx)->newJLObject(cx, "Unserializer");
					JL_CHK( unserializerWrapper );
					JL_SetPrivate( unserializerWrapper, this);
					arg.setObject(*unserializerWrapper);
				} else {

					//unserializerWrapper is null by default
					arg.setObject( *_unserializerObj );
				}

				bool ok = jl::callNoRval(cx, newObj, JLID(cx, _unserialize), arg); // rval not used

				if ( unserializerWrapper != NULL )
					JL_SetPrivate( unserializerWrapper, NULL);
				JL_CHK( ok );

				val.setObject(*newObj);
				break;
			}
			case JLSTSerializableScriptObject: {

				JS::RootedValue funVal(cx);
				JL_CHK( Read(cx, &funVal) );
				JL_ASSERT( jl::isCallable(cx, funVal), E_STR("unserializer"), E_STATE, E_INVALID ); // JLSMSG_INVALID_OBJECT_STATE, "Unserializer"

				JS::RootedValue arg_1(cx);
				JS::RootedObject unserializerWrapper(cx);

				if ( !_unserializerObj ) {

					// (TBD) enhance this by creating an API (eg. serializerPub.h ?)
					//unserializerWrapper = JL_NewJslibsObject(cx, "Unserializer");
					unserializerWrapper = jl::Global::getGlobal(cx)->newJLObject(cx, "Unserializer");
					JL_CHK( unserializerWrapper );
					JL_SetPrivate(unserializerWrapper, this);
					arg_1.setObject(*unserializerWrapper);
				} else {

					unserializerWrapper = NULL;
					arg_1.setObject( *_unserializerObj );
				}
				
				JS::RootedObject globalObject(cx, JL_GetGlobal(cx));

				bool ok = jl::call(cx, globalObject, funVal, val, arg_1);

				if ( unserializerWrapper != NULL )
					JL_SetPrivate( unserializerWrapper, NULL);

				JL_CHK( ok );


				JL_ASSERT( val.isObject(), E_STR("unserializer"), E_RETURNVALUE, E_TYPE, E_TY_OBJECT );
				break;
			}
			case JLSTArrayBuffer: {

				SerializerConstBufferInfo data;
				JL_CHK( Read(cx, data) );

				//JL_CHK( JL_NewBufferCopyN(cx, data.Data(), data.Length(), val) );
				//JL_CHK( jl::Buffer((jl::Buffer::DataType)data.Data(), data.Length()).toArrayBufferObject(cx, val) );  need to copy !!!
				void *arrayBufferContents = jl_malloc(data.Length());
				jl::memcpy(arrayBufferContents, data.Data(), data.Length());
				val.setObjectOrNull(JS_NewArrayBufferWithContents(cx, data.Length(), arrayBufferContents));
				JL_ASSERT_ALLOC( !val.isNull() );
				break;
			}
			case JLSTTypedArray: {

				SerializerConstBufferInfo data;

				js::Scalar::Type type;
				JL_CHK( Read(cx, type) );
				JL_CHK( Read(cx, data) );

				//JL_CHK( JL_NewBufferCopyN(cx, data.Data(), data.Length(), val) );
				//JL_CHK( jl::Buffer((jl::Buffer::DataType)data.Data(), data.Length()).toArrayBufferObject(cx, val) );  need to copy !!!
				void *arrayBufferContents = jl_malloc(data.Length());
				jl::memcpy(arrayBufferContents, data.Data(), data.Length());
				JS::RootedObject arrayBuffer(cx, JS_NewArrayBufferWithContents(cx, data.Length(), arrayBufferContents));
				JS::RootedObject typedArray(cx);
				switch ( type ) {
				case js::Scalar::Int8:
						typedArray = JS_NewInt8ArrayWithBuffer(cx, arrayBuffer, 0, -1);
						break;
					case js::Scalar::Uint8:
						typedArray = JS_NewUint8ArrayWithBuffer(cx, arrayBuffer, 0, -1);
						break;
					case js::Scalar::Int16:
						typedArray = JS_NewInt16ArrayWithBuffer(cx, arrayBuffer, 0, -1);
						break;
					case js::Scalar::Uint16:
						typedArray = JS_NewUint16ArrayWithBuffer(cx, arrayBuffer, 0, -1);
						break;
					case js::Scalar::Int32:
						typedArray = JS_NewInt32ArrayWithBuffer(cx, arrayBuffer, 0, -1);
						break;
					case js::Scalar::Uint32:
						typedArray = JS_NewUint32ArrayWithBuffer(cx, arrayBuffer, 0, -1);
						break;
					case js::Scalar::Float32:
						typedArray = JS_NewFloat32ArrayWithBuffer(cx, arrayBuffer, 0, -1);
						break;
					case js::Scalar::Float64:
						typedArray = JS_NewFloat64ArrayWithBuffer(cx, arrayBuffer, 0, -1);
						break;
					case js::Scalar::Uint8Clamped:
						typedArray = JS_NewUint8ClampedArrayWithBuffer(cx, arrayBuffer, 0, -1);
						break;
				}

				JL_CHK( typedArray );
				val.setObject(*typedArray);
				break;
			}
			case JLSTErrorObject: {

				JS::RootedString constructorName(cx);
				JS::RootedId constructorNameId(cx);
				//SerializerConstBufferInfo constructorName;
				//jsval constructor, constructorArgs[3], stack;
				JS::RootedValue constructor(cx);
				JS::RootedValue stack(cx);
				
				//JS::AutoValueVector constructorArgs(cx);
				JS::AutoValueArray<3> constructorArgs( cx );

				JS::RootedObject globalObject(cx, JL_GetGlobal(cx));

				JL_CHK( Read(cx, &constructorName) );
				JL_CHK( JS_StringToId(cx, constructorName, &constructorNameId) );
				JL_CHK( JS_GetPropertyById(cx, globalObject, constructorNameId, &constructor) );
				JL_ASSERT( constructor.isObject(), E_TY_ERROR, E_NOTCONSTRUCT );

				//JSClass *cl = JL_GetErrorClaspByProtoKey(cx, JSProto_Error, JL_GetGlobal(cx));
				//JSObject *errorObj = JS_NewObjectForConstructor(cx, &constructor);

				JL_CHK( Read(cx, constructorArgs[0]) ); // message
				JL_CHK( Read(cx, constructorArgs[1]) ); // fileName
				JL_CHK( Read(cx, constructorArgs[2]) ); // lineNumber
				JL_CHK( Read(cx, &stack) );

				JS::RootedObject constructorObj(cx, &constructor.toObject());

				JS::RootedObject errorObj(cx, JS_New(cx, constructorObj, constructorArgs));

				JL_ASSERT_ALLOC( errorObj );
				val.setObject(*errorObj);

				JL_CHK( JS_SetPropertyById(cx, errorObj, JLID(cx, stack), stack) );
				break;
			}
			case JLSTFunction: {

				/* with XDR API */

				SerializerConstBufferInfo encodedFunction;
				JL_CHK( Read(cx, encodedFunction) );
			
				JS::RootedObject fctObj(cx, JS_DecodeInterpretedFunction(cx, encodedFunction.Data(), encodedFunction.Length(), NULL));
				ASSERT( JS_ObjectIsFunction(cx, fctObj) );
				
				JS::RootedObject parent(cx, JS_GetParent(_unserializerObj));
				val.setObjectOrNull(JS_CloneFunctionObject(cx, fctObj, parent)); // (TBD) remove this wen bz#741597 will be fixed. -> seems ok before 741597 fix -> no!
				ASSERT( !val.isNull() );

				//val.setObject(*fctObj);
				

				/* without XDR API

				JS::RootedString fctStr(cx);
				JL_CHK( Read(cx, &fctStr) );

				JS::CompileOptions compOpt(cx);
				compOpt
					.setCompileAndGo(false)
					.setCanLazilyParse(false)
					.setSourceIsLazy(false)
				;

				JS::RootedObject global(cx, JL_GetGlobal(cx));
				JS::RootedValue rval(cx);

				if ( JS_StringHasLatin1Chars(fctStr) ) {
				
					size_t len;
					const char *chars = (const char *)JS_GetLatin1StringCharsAndLength(cx, JS::AutoCheckCannotGC(), fctStr, &len);
					JL_CHK( JS::Evaluate(cx, global, compOpt, chars, len, &rval) );
				} else {
					size_t len;
					const jschar *chars = (const jschar *)JS_GetTwoByteStringCharsAndLength(cx, JS::AutoCheckCannotGC(), fctStr, &len);
					JL_CHK( JS::Evaluate(cx, global, compOpt, chars, len, &rval) );
				}

				val.set(rval);
				*/

				break;
			}
			default:
				JL_ERR( E_STR("unserializer"), E_STATE, E_INVALID );
		}

		return true;
		JL_BAD;
	}
};



ALWAYS_INLINE bool
JsvalIsSerializer( JSContext *cx, JS::HandleValue val ) {

	return jl::isClass(cx, val, jl::Global::getGlobal(cx)->getCachedClasp("Serializer"));
}

ALWAYS_INLINE Serializer*
JsvalToSerializer( JSContext *cx, JS::MutableHandleValue val ) {

	ASSERT( jl::JsvalIsSerializer(cx, val) );
	return static_cast<Serializer*>(JL_GetPrivate(val));
}

ALWAYS_INLINE bool
JsvalIsUnserializer( JSContext *cx, JS::HandleValue val ) {

	return jl::isClass(cx, val, jl::Global::getGlobal(cx)->getCachedClasp("Unserializer"));
}

ALWAYS_INLINE Unserializer*
JsvalToUnserializer( JSContext *cx, JS::MutableHandleValue val ) {
	
	ASSERT( jl::JsvalIsUnserializer(cx, val) );
	return static_cast<Unserializer*>(JL_GetPrivate(val));
}

JL_END_NAMESPACE

/* Template

#include <jsvalserializer.h>

DEFINE_FUNCTION( _serialize ) {

	JL_ASSERT_ARGC(1);
	JL_ASSERT( jl::JsvalIsSerializer(cx, JL_ARG(1)), "Invalid serializer object." );
	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	//ser->Write(cx, globalKey);

	return true;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_ASSERT_ARGC(1);
	JL_ASSERT( jl::JsvalIsUnserializer(cx, JL_ARG(1)), "Invalid unserializer object." );
	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	//uint32_t gKey;
	//unser->Read(cx, gKey);

	return true;
	JL_BAD;
}


		FUNCTION_ARGC(_serialize, 1)
		FUNCTION_ARGC(_unserialize, 1)
		
*/
