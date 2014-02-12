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


JL_BEGIN_NAMESPACE


typedef enum JLSerializeType {
	JLDataawJsval,
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
	JLSTStopIteration,
	JLSTErrorObject,
	JLSTObjectValue,
	JLSTSerializableNativeObject,
	JLSTSerializableScriptObject,
	JLSTArrayBuffer,
	JLSTTypedArray
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


/////////////////////////////////////////////////////////////////////////////

class Serializer : public jl::CppAllocators {

	//jsval _serializerObj;
	JS::PersistentRootedValue _serializerObj; // must be used to declare data members of heap classes only

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

	Serializer( const Serializer & );

public:

	~Serializer() {

		if ( _start != NULL )
			jl_free(_start); // jl_free(NULL) is legal, but this is an optimization, since usually one use GetBufferOwnership() that set _start to NULL
	}

	explicit Serializer( JSContext *cx, JS::HandleValue serializerObj = JS::NullPtr() )
	: _serializerObj(cx, serializerObj), _start(NULL), _pos(NULL), _length(0) {

		PrepareBytes(JL_PAGESIZE / 2);
	}


	bool GetBufferOwnership( void **data, size_t *length ) {

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

		return true;
		JL_BAD;
	}


	bool Write( JSContext *cx, const char *cstr ) {

		size_t length = strlen(cstr) + 1; // + 1 for the '\0' 
		JL_CHK( Write(cx, length) );
		JL_CHK( PrepareBytes(length) );
		jl::memcpy(_pos, cstr, length);
		_pos += length;
		return true;
		JL_BAD;
	}

	ALWAYS_INLINE bool
	Write( JSContext *cx, const JLSerializeType type ) {
		
		ASSERT( type >= 0 && type <= 255 );
		return Write(cx, (const unsigned char)type);
	}

	bool Write( JSContext *cx, JS::HandleString jsstr ) {

		size_t length;
		const jschar *chars;
		chars = JS_GetStringCharsAndLength(cx, jsstr, &length); // doc. not null-terminated.
		return Write(cx, SerializerConstBufferInfo(chars, length));
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


		JS::RootedObject obj(cx, sop);
		JS::RootedValue name(cx);
		JS::RootedValue value(cx);
		
		JSIdArray *idArray = JS_Enumerate(cx, obj); // Get an array of the all *own* enumerable properties of a given object.
		int len = JS_IdArrayLength(cx, idArray);
		JL_CHK( idArray );
		JL_CHK( Write(cx, len) );
		for ( int i = 0; i < len; ++i ) {

			jsid item = JS_IdArrayGet(cx, idArray, i);
			JL_CHK( JS_IdToValue(cx, item, name.address()) );
			JS::RootedString jsstr(cx, name.isString() ? name.toString() : JS::ToString(cx, name));
			JL_CHK( jsstr );
			JL_CHK( JS_GetPropertyById(cx, obj, item, &value) );
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
		return true;
	bad:

		if ( idArray )
			JS_DestroyIdArray(cx, idArray);
		return false;
	}

	bool Write( JSContext *cx, const SerializerObjectReservedSlots &srs ) {

		JS::RootedObject obj(cx, srs);
		JS::RootedValue value(cx);
		uint32_t reservedSlotsCount = JSCLASS_RESERVED_SLOTS(JL_GetClass(obj));
		JL_CHK( Write(cx, reservedSlotsCount) );
		for ( uint32_t i = 0; i < reservedSlotsCount; ++i ) {

			JL_CHK( JL_GetReservedSlot( obj, i, &value) );
			JL_CHK( Write(cx, value) );
		}
		return true;
		JL_BAD;
	}
	
	bool Write( JSContext *cx, JS::MutableHandleValue mval ) {

		JS::HandleValue val(mval);
		return Write(cx, val);
	}

	bool Write( JSContext *cx, JS::HandleValue val ) {

		//jsval serializeFctVal;
		JS::RootedValue serializeFctVal(cx);

		if ( JSVAL_IS_PRIMITIVE(val) ) {

			if ( JSVAL_IS_INT(val) ) {

				JL_CHK( Write(cx, JLSTInt) );
				JL_CHK( Write(cx, val.toInt32()) );
			} else
			if ( val.isString() ) {

				JL_CHK( Write(cx, JLSTString) );
				JL_CHK( Write(cx, val.toString()) );
			} else
			if ( val.isUndefined() ) {

				JL_CHK( Write(cx, JLSTVoid) );
			} else
			if ( val.isBoolean() ) {

				JL_CHK( Write(cx, JLSTBool) );
				JL_CHK( Write(cx, char(val.toBoolean())) );
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

				JL_ERR( E_STR("serializer"), E_STATE );
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

		if ( JL_ObjectIsArray(cx, obj) ) { // real array object, not array-like !!

			unsigned length;
			JL_CHK( JS_GetArrayLength(cx, obj, &length) );
			JL_CHK( Write(cx, JLSTArray) );
			JL_CHK( Write(cx, length) );

			bool found;
			JS::RootedValue tmp(cx);
			for ( int i = 0; i < jl::SafeCast<int>(length); ++i ) {

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
/*				
			JSString *src;
			src = JS_ValueToSource(cx, OBJECT_TO_JSVAL(obj));
			JL_ASSERT( src, E_JSLIBS, E_INTERNAL ); // "Unable to get function source."
			JL_CHK( Write(cx, JLSTFunction) );
			return Write(cx, src);
*/
			uint32_t length;
			void *data;
			data = JS_EncodeInterpretedFunction(cx, obj, &length);
			JL_CHK( Write(cx, JLSTFunction) );
			JL_CHK( Write(cx, SerializerConstBufferInfo(data, length)) );
			js_free(data);
			return true;
		}


		JL_CHK( JS_GetPropertyById(cx, obj, JLID(cx, _serialize), &serializeFctVal) ); // JL_CHK( JS_GetProperty(cx, obj, "_serialize", &serializeFctVal) );
		if ( !serializeFctVal.isUndefined() ) {

			JS::RootedValue arg(cx);

			JL_ASSERT( JL_ValueIsCallable(cx, serializeFctVal), E_OBJ, E_NAME(JL_GetClassName(obj)), E_INTERNAL, E_SEP, E_TY_FUNC, E_NAME("_serialize"), E_DEFINED );

			if ( !JL_ObjectIsObject(cx, obj ) ) {

				JL_CHK( Write(cx, JLSTSerializableNativeObject) );
				JL_CHK( Write(cx, JL_GetClassName(obj)) );
			} else {

				JL_CHK( Write(cx, JLSTSerializableScriptObject) );
				JS::RootedValue unserializeFctVal(cx);
				JL_CHK( JS_GetPropertyById(cx, obj, JLID(cx, _unserialize), &unserializeFctVal) );
				JL_ASSERT( JL_ValueIsCallable(cx, unserializeFctVal), E_OBJ, E_NAME(JL_GetClassName(obj)), E_INTERNAL, E_SEP, E_TY_FUNC, E_NAME("_unserialize"), E_DEFINED );
				JL_CHK( Write(cx, unserializeFctVal) );
			}


			//JSObject *serializerWrapper;
			JS::RootedObject serializerWrapper(cx, NULL);


			if ( _serializerObj.get().isNull() ) {

				// create a temporary wrapper to this serializer c++ object.

				// (TBD) enhance this by creating an API (eg. serializerPub.h ?)
				serializerWrapper = JL_NewJslibsObject(cx, "Serializer");
				JL_CHK( serializerWrapper );
				arg.setObject(*serializerWrapper);
				JL_SetPrivate( serializerWrapper, this);
			} else {

				arg.set(_serializerObj);
			}

			jsval tmp; // root no need
			bool ok;
			ok = JS_CallFunctionValue(cx, obj, serializeFctVal, 1, arg.address(), &tmp); // rval not used
			
			if ( serializerWrapper != NULL ) {

				JL_SetPrivate(serializerWrapper, NULL);
			}

			JL_CHK( ok );

			return true;
		}

		// prototype-less object
		if ( JL_GetPrototype(cx, obj) == NULL ) {

			JL_CHK( Write(cx, JLSTProtolessObject) );
			JL_CHK( Write(cx, SerializerObjectOwnProperties(*obj.address())) );
			return true;
		}

		// StopIteration object
		if ( JL_IsStopIteration(cx, obj) ) {
			
			JL_CHK( Write(cx, JLSTStopIteration) );
			return true;
		}

		// simple object
		if ( JL_ObjectIsObject(cx, obj) ) {

			JL_CHK( Write(cx, JLSTObject) );
			JL_CHK( Write(cx, SerializerObjectOwnProperties(*obj.address())) );
			return true;
		}

		// all JS errors objects
		if ( JL_ObjectIsError(cx, obj) ) {

			JS::RootedObject constructor(cx, JL_GetConstructor(cx, obj));

			JS::RootedValue constructorName(cx);
			JL_CHK( JS_GetPropertyById(cx, constructor, JLID(cx, name), &constructorName) );
			ASSERT( constructorName.isString() );

			const jschar *str;
			size_t length;
			str = JS_GetStringCharsAndLength(cx, JS::ToString(cx, constructorName), &length);

			JL_CHK( Write(cx, JLSTErrorObject) );
			JL_CHK( Write(cx, SerializerConstBufferInfo(str, length)) );

			//JL_CHK( Write(cx, SerializerObjectOwnProperties(obj)) ); // (TBD) why this don't work on error object => bug 724768
			
			JS::RootedValue tmp(cx);
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
		JL_CHK( JL_JsvalToPrimitive(cx, tmp, &tmp) );
		JL_CHK( Write(cx, JLSTObjectValue) );
		JL_CHK( Write(cx, JL_GetClass(obj)->name) );
		JL_CHK( Write(cx, tmp) );
		return true;
		}

		JL_BAD;
	}

	template <class T>
	bool Write( JSContext *cx, const T &value ) {

		JL_IGNORE(cx);
		JL_CHK( PrepareBytes(sizeof(T)) );
		*(T*)_pos = value;
		_pos += sizeof(T);
		return true;
		JL_BAD;
	}

};


/////////////////////////////////////////////////////////////////////////////


class Unserializer : public jl::CppAllocators {

	//jsval _unserializerObj;
	JS::PersistentRootedValue _unserializerObj; // must be used to declare data members of heap classes only

	uint8_t *_start;
	uint8_t *_pos;
	size_t _length;

	bool AssertData( size_t length ) const {

		return (_pos - _start) + length <= _length;
	}

	Unserializer( const Unserializer & );

public:

	~Unserializer() {

		jl_free(_start);
	}
	
	explicit Unserializer( JSContext *cx, void *dataOwnership, size_t length, jsval unserializerObj = JSVAL_NULL )
	: _unserializerObj(cx, unserializerObj), _start((uint8_t*)dataOwnership), _pos(_start), _length(length) {

	}

	bool IsEmpty() const {
		
		ASSERT( _pos >= _start );
		return  size_t(_pos - _start) < _length;
	}

	bool Read( JSContext *cx, const char *&buf ) {

		size_t length;
		JL_CHK( Read(cx, length) );
		JL_CHK( AssertData(length) );
		buf = (const char*)_pos;
		_pos += length;
		return true;
		JL_BAD;
	}

	bool Read( JSContext *cx, JSString *&jsstr ) {

		SerializerConstBufferInfo buf;
		JL_CHK( Read(cx, buf) );
		jsstr = JS_NewUCStringCopyN(cx, (const jschar *)buf.Data(), buf.Length()/2);
		JL_CHK(jsstr);
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

		int length;
		JL_CHK( Read(cx, length) );

		for ( int i = 0; i < length; ++i ) {

			SerializerConstBufferInfo name;
			JS::RootedValue value(cx);
			JL_CHK( Read(cx, name) );
			JL_CHK( Read(cx, *value.address()) );
			JL_CHK( JS_SetUCProperty(cx, sop, (const jschar *)name.Data(), name.Length()/2, value) );
		}

		return true;
		JL_BAD;
	}

	bool Read( JSContext *cx, SerializerObjectReservedSlots &srs ) {

		JS::RootedObject obj(cx, srs);
		JS::RootedValue value(cx);
		uint32_t reservedSlotsCount;
		JL_CHK( Read(cx, reservedSlotsCount) );
		for ( uint32_t i = 0; i < reservedSlotsCount; ++i ) {

			JL_CHK( Read(cx, *value.address()) );
			JL_CHK( JL_SetReservedSlot( obj, i, (JS::HandleValue)&value) );
		}
		return true;
		JL_BAD;
	}


	bool Read( JSContext *cx, JS::MutableHandleValue val ) {

		char type;
		JL_CHK( Read(cx, type) );

		switch ( type ) {

			//case JLDataawJsval: {
			//	
			//	JL_CHK( Read(cx, JSVAL_BITS(val)) );
			//	break;
			//}

			case JLSTInt: {

				int i;
				JL_CHK( Read(cx, i) );
				val.setInt32(i);
				break;
			}
			case JLSTVoid: {

				val.setUndefined();
				break;
			}
			case JLSTBool: {

				char b;
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
				JL_CHK( Read(cx, jsstr) );
				val.setString(jsstr);
				break;
			}
			case JLSTArray: {

				unsigned length;
				JL_CHK( Read(cx, length) );
//				JSObject *arr;
//				arr = JS_NewArrayObject(cx, length, NULL);
//				JL_ASSERT_ALLOC( arr );
//				val = OBJECT_TO_JSVAL(arr);

				JS::RootedObject arr(cx, JS_NewArrayObject(cx, length, NULL));
				val.setObject(*arr);

				JS::RootedValue tmp(cx);
				for ( unsigned i = 0; i < length; ++i ) {

					JL_CHK( Read(cx, tmp) );
					if ( !tmp.isMagic(JS_ELEMENTS_HOLE) ) // if ( !JL_JSVAL_IS_ARRAY_HOLE(*avr.jsval_addr()) )
						JL_CHK( JL_SetElement(cx, arr, i, tmp) );
				}
				break;
			}
			case JLSTObject: {

				JS::RootedObject obj(cx, JL_NewObj(cx));
				JL_CHK( obj );
				val.setObject(*obj);
				SerializerObjectOwnProperties sop(*obj.address());
				JL_CHK( Read(cx, sop) );
				break;
			}
			case JLSTProtolessObject: {

				JS::RootedObject obj(cx, JL_NewProtolessObj(cx));
				JL_CHK( obj );
				val.setObject(*obj);
				SerializerObjectOwnProperties sop(*obj.address());
				JL_CHK( Read(cx, sop) );
				break;
			}
			case JLSTStopIteration: {

				JS::RootedObject proto(cx);
				JL_GetClassPrototype(cx, JSProto_StopIteration, &proto);
				val.setObject(*proto);
				break;
			}
			case JLSTObjectValue: {

				const char *className;
				JS::RootedValue value(cx);
				JS::RootedValue prop(cx);
				JL_CHK( Read(cx, className) );
				JL_CHK( Read(cx, value) );
				
				
				JS::RootedObject scope(cx, JL_GetGlobal(cx)); //JS_GetScopeChain(cx);
				JL_CHK( JS_GetProperty(cx, scope, className, &prop) );
				JL_CHK( prop.isObject() );

				JS::RootedObject newObj(cx, JS_New(cx, &prop.toObject(), 1, value.address()));
				JL_CHK( newObj );
				val.setObject(*newObj);
				break;
			}
			case JLSTSerializableNativeObject: {

				const char *className;
				JL_CHK( Read(cx, className) );
				ASSERT(strlen(className) > 0);
				ASSERT(strlen(className) < 64);

				jl::Host &host = jl::Host::getHost(cx);
				const jl::ClassProtoCache* cpc = host.getCachedClassProto(className);


				//const ClassProtoCache *cpc = JL_GetCachedClassProto(JL_GetHostPrivate(cx), className);

				JL_CHKM( cpc != NULL, E_CLASS, E_NAME(className), E_NOTFOUND );
				JS::RootedObject newObj(cx, JL_NewObjectWithGivenProto(cx, cpc->clasp, cpc->proto));
				JL_CHK( newObj );

				JS::RootedValue arg(cx);

				JS::RootedObject unserializerWrapper(cx);
				if ( _unserializerObj.get().isNull() ) {

					// (TBD) enhance this by creating an API (eg. serializerPub.h ?)
					unserializerWrapper = JL_NewJslibsObject(cx, "Unserializer");
					JL_CHK( unserializerWrapper );
					JL_SetPrivate( unserializerWrapper, this);
					arg = OBJECT_TO_JSVAL(unserializerWrapper);
				} else {

					//unserializerWrapper is null by default
					arg.set( _unserializerObj );
				}

				JS::RootedValue rval(cx); // note that no root needed

				//bool ok = JL_CallFunctionId(cx, newObj, JLID(cx, _unserialize), 1, &arg, &rval); // rval not used
				bool ok = JL_CallFunctionIdVA(cx, newObj, JLID(cx, _unserialize), &rval, arg); // rval not used

				if ( unserializerWrapper != NULL )
					JL_SetPrivate( unserializerWrapper, NULL);
				JL_CHK( ok );

				val.setObject(*newObj);
				break;
			}
			case JLSTSerializableScriptObject: {

				JS::RootedValue funVal(cx);
				JL_CHK( Read(cx, funVal) );
				JL_ASSERT( JL_ValueIsCallable(cx, funVal), E_STR("unserializer"), E_STATE ); // JLSMSG_INVALID_OBJECT_STATE, "Unserializer"

				JS::RootedValue arg_1(cx);
				JS::RootedObject unserializerWrapper(cx);

				if ( _unserializerObj.get().isNull() ) {

					// (TBD) enhance this by creating an API (eg. serializerPub.h ?)
					unserializerWrapper = JL_NewJslibsObject(cx, "Unserializer");
					JL_CHK( unserializerWrapper );
					JL_SetPrivate(unserializerWrapper, this);
					arg_1 = OBJECT_TO_JSVAL(unserializerWrapper);
				} else {

					unserializerWrapper = NULL;
					arg_1 = _unserializerObj;
				}
				
				JS::RootedObject globalObject(cx, JL_GetGlobal(cx));
				bool ok = JL_CallFunctionVA(cx, globalObject, funVal, val, arg_1);

				if ( unserializerWrapper != NULL )
					JL_SetPrivate( unserializerWrapper, NULL);
				JL_CHK( ok );


				JL_ASSERT( val.isObject(), E_STR("unserializer"), E_RETURNVALUE, E_TYPE, E_TY_OBJECT );
				break;
			}
			case JLSTArrayBuffer: {

				SerializerConstBufferInfo data;
				JL_CHK( Read(cx, data) );
				JL_CHK( JL_NewBufferCopyN(cx, data.Data(), data.Length(), val) );
				break;
			}
			case JLSTTypedArray: {

				SerializerConstBufferInfo data;

				JSArrayBufferViewType type;
				JL_CHK( Read(cx, type) );
				JL_CHK( Read(cx, data) );
				JL_CHK( JL_NewBufferCopyN(cx, data.Data(), data.Length(), val) );

				JS::RootedObject typedArray(cx);
				switch ( type ) {
					case js::ArrayBufferView::TYPE_INT8:
						typedArray = JS_NewInt8ArrayWithBuffer(cx, &val.toObject(), 0, -1);
						break;
					case js::ArrayBufferView::TYPE_UINT8:
						typedArray = JS_NewUint8ArrayWithBuffer(cx, &val.toObject(), 0, -1);
						break;
					case js::ArrayBufferView::TYPE_INT16:
						typedArray = JS_NewInt16ArrayWithBuffer(cx, &val.toObject(), 0, -1);
						break;
					case js::ArrayBufferView::TYPE_UINT16:
						typedArray = JS_NewUint16ArrayWithBuffer(cx, &val.toObject(), 0, -1);
						break;
					case js::ArrayBufferView::TYPE_INT32:
						typedArray = JS_NewInt32ArrayWithBuffer(cx, &val.toObject(), 0, -1);
						break;
					case js::ArrayBufferView::TYPE_UINT32:
						typedArray = JS_NewUint32ArrayWithBuffer(cx, &val.toObject(), 0, -1);
						break;
					case js::ArrayBufferView::TYPE_FLOAT32:
						typedArray = JS_NewFloat32ArrayWithBuffer(cx, &val.toObject(), 0, -1);
						break;
					case js::ArrayBufferView::TYPE_FLOAT64:
						typedArray = JS_NewFloat64ArrayWithBuffer(cx, &val.toObject(), 0, -1);
						break;
					case js::ArrayBufferView::TYPE_UINT8_CLAMPED:
						typedArray = JS_NewUint8ClampedArrayWithBuffer(cx, &val.toObject(), 0, -1);
						break;
					default:
						typedArray = NULL;
				}

				JL_CHK( typedArray );
				val.setObject(*typedArray);
				break;
			}
			case JLSTErrorObject: {

				SerializerConstBufferInfo constructorName;
				//jsval constructor, constructorArgs[3], stack;
				JS::RootedValue constructor(cx);
				JS::RootedValue stack(cx);
				JS::AutoValueVector constructorArgs(cx);

				JL_CHK( Read(cx, constructorName) );

				// js_GetClassPrototype
				JS::RootedObject globalObject(cx, JL_GetGlobal(cx));
				JL_CHK( JS_GetUCProperty(cx, globalObject, (const jschar *)constructorName.Data(), constructorName.Length() / 2, &constructor) );
				JL_ASSERT( constructor.isObject(), E_TY_ERROR, E_NOTCONSTRUCT );

				//JSClass *cl = JL_GetErrorJSClassJSClassByProtoKey(cx, JSProto_Error, JL_GetGlobal(cx));
				//JSObject *errorObj = JS_NewObjectForConstructor(cx, &constructor);

				JL_CHK( Read(cx, constructorArgs[0]) ); // message
				JL_CHK( Read(cx, constructorArgs[1]) ); // fileName
				JL_CHK( Read(cx, constructorArgs[2]) ); // lineNumber
				JL_CHK( Read(cx, stack) );

				JS::RootedObject errorObj(cx, JS_New(cx, &constructor.toObject(), constructorArgs.length(), constructorArgs.begin()));
				JL_CHK( errorObj );
				val.setObject(*errorObj);

				JL_CHK( JS_SetPropertyById(cx, errorObj, JLID(cx, stack), stack) );
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
				JL_CHK( JS_SetParent(cx, JSVAL_TO_OBJECT(val), JL_GetGlobal(cx)) );
				JSObject *funProto;
				JL_CHK( JL_GetClassPrototype(cx, NULL, JSProto_Function, &funProto) );
				JL_CHK( JS_SetPrototype(cx, JSVAL_TO_OBJECT(val), funProto) );
*/

/*
				JSString *source;
				JL_CHK( Read(cx, source) );
				JL_CHK( JL_Eval(cx, source, &val) ); // beware, non-functionExpression behavior 
				//JL_CHKM( !JSVAL_IS_VOID(val), 
*/

				SerializerConstBufferInfo encodedFunction;
				JL_CHK( Read(cx, encodedFunction) );
				
				JS::RootedObject fctObj(cx);
				fctObj = JS_DecodeInterpretedFunction(cx, encodedFunction.Data(), encodedFunction.Length(), NULL, NULL);
				
				fctObj = JS_CloneFunctionObject(cx, fctObj, JS_GetParent(fctObj)); // (TBD) remove this wen bz#741597 wil be fixed.
				
				val.setObject(*fctObj);
				break;
			}
			default:
				JL_ERR( E_STR("unserializer"), E_STATE );
		}

		return true;
		JL_BAD;
	}

	template <class T>
	bool Read( JSContext *cx, T &value ) {

		JL_IGNORE(cx);
		if ( !AssertData(sizeof(T)) )
			return false;
		value = *(T*)_pos;
		_pos += sizeof(T);
		return true;
	}
};



ALWAYS_INLINE bool
JsvalIsSerializer( JSContext *cx, JS::HandleValue val ) {

	return JL_ValueIsClass(cx, val, JL_GetCachedClass(JL_GetHostPrivate(cx), "Serializer"));
}

ALWAYS_INLINE Serializer*
JsvalToSerializer( JSContext *cx, JS::MutableHandleValue val ) {

	ASSERT( jl::JsvalIsSerializer(cx, val) );
	return static_cast<Serializer*>(JL_GetPrivate(val));
}

ALWAYS_INLINE bool
JsvalIsUnserializer( JSContext *cx, JS::HandleValue val ) {

	return JL_ValueIsClass(cx, val, JL_GetCachedClass(JL_GetHostPrivate(cx), "Unserializer"));
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

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT( jl::JsvalIsSerializer(cx, JL_ARG(1)), "Invalid serializer object." );
	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	//ser->Write(cx, globalKey);

	return true;
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

	return true;
	JL_BAD;
}


		FUNCTION_ARGC(_serialize, 1)
		FUNCTION_ARGC(_unserialize, 1)
		
*/
