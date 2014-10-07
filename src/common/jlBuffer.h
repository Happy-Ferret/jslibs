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

/*
typedef char * StrZ;
typedef char16_t * WStrZ;
typedef uint8_t * Bytes;
*/


class StrDataSrc {
public:
	virtual bool isSet() const = 0; // the object has been set with data (may be empty).
	virtual size_t length() const = 0; // number on elements ( an element may be 1 or 2-bytes length), use ( length() * isWide() ? 2 : 1 ) to compute byteLength
	virtual bool isWide() const = 0; // the underlying data has 2-bytes per element.
	virtual bool isData() const = 0; // 
	
	// to*()

	virtual const char * toStrZ(const JS::AutoCheckCannotGC &nogc) = 0; // c-string with string terminator ('\0')
	virtual const char16_t * toWStrZ(const JS::AutoCheckCannotGC &nogc) = 0; // unicode string with string terminator ('\0')


	virtual const char * toStr(const JS::AutoCheckCannotGC &nogc) { // c-string without terminator
		
		return toStrZ(nogc); // Str is included in StrZ
	}

	virtual const char16_t * toWStr(const JS::AutoCheckCannotGC &nogc) { // unicode string without terminator

		return toWStrZ(nogc); // WStr is included in WStrZ
	}

	virtual const uint8_t * toBytes(const JS::AutoCheckCannotGC &nogc) { // raw data
		
		return reinterpret_cast<const uint8_t *>(toStr(nogc)); // Bytes is included in Str
	}


	// cast operators

private:
	operator bool();
public:

	operator const char *() {

		return toStrZ(JS::AutoCheckCannotGC());
	}

	operator const char16_t *() {

		return toWStrZ(JS::AutoCheckCannotGC());
	}

	operator const uint8_t *() {

		return toBytes(JS::AutoCheckCannotGC());
	}

	operator const void *() {

		return toBytes(JS::AutoCheckCannotGC());
	}


	// toOwn*()

	// if canTransferOwnership is true, the callee may give its raw data to the pointer

	virtual char16_t * toOwnWStrZ( bool canTransferOwnership = true ) {

		JS::AutoCheckCannotGC nogc;
		size_t size = sizeof(char16_t) * (length() + 1);
		char16_t *tmp = static_cast<char16_t*>(jl_malloc(size));
		if ( tmp )
			jl::memcpy(tmp, toWStrZ(nogc), size);
		return tmp;
	}

	virtual char16_t * toOwnWStr( bool canTransferOwnership = true ) {

		JS::AutoCheckCannotGC nogc;
		size_t size = sizeof(char16_t) * length();
		char16_t *tmp = static_cast<char16_t*>(jl_malloc(size));
		if ( tmp )
			jl::memcpy(tmp, toWStr(nogc), size);
		return tmp;
	}

	virtual char * toOwnStrZ( bool canTransferOwnership = true ) {

		JS::AutoCheckCannotGC nogc;
		size_t size = sizeof(char) * (length() + 1);
		char *tmp = static_cast<char*>(jl_malloc(size));
		if ( tmp )
			jl::memcpy(tmp, toStrZ(nogc), size);
		return tmp;
	}
	
	virtual char * toOwnStr( bool canTransferOwnership = true ) {

		JS::AutoCheckCannotGC nogc;
		size_t size = sizeof(char) * length();
		char *tmp = static_cast<char*>(jl_malloc(size));
		if ( tmp )
			jl::memcpy(tmp, toStr(nogc), size);
		return tmp;
	}
	
	virtual uint8_t * toOwnBytes( bool canTransferOwnership = true ) {

		return reinterpret_cast<uint8_t*>(toOwnStr(canTransferOwnership));
	}


	// getCharAt

	virtual char16_t getWCharAt(size_t index) {

		JS::AutoCheckCannotGC nogc;
		ASSERT( index < length() ); // out of string range
		return isWide() ? toWStr(nogc)[index] : static_cast<char16_t>(toStr(nogc)[index]);
	}

	virtual char getCharAt(size_t index) {

		return static_cast<char>(getWCharAt(index));
	}

	virtual uint8_t getByteAt(size_t index) {

		return static_cast<uint8_t>(getCharAt(index));
	}


	// copyTo

	virtual size_t copyTo( char *dst, size_t maxLength ) {

		JS::AutoCheckCannotGC nogc;
		size_t len = jl::min(length(), maxLength);
		jl::memcpy(dst, toStr(nogc), len * sizeof(*dst));
		return len;
	}

	virtual size_t copyTo( char16_t *dst, size_t maxLength ) {

		JS::AutoCheckCannotGC nogc;
		size_t len = jl::min(length(), maxLength);
		jl::memcpy(dst, toWStr(nogc), len * sizeof(*dst));
		return len;
	}

	virtual size_t copyTo( uint8_t *dst, size_t maxLength ) {
		
		JS::AutoCheckCannotGC nogc;
		size_t len = jl::min(length(), maxLength);
		jl::memcpy(dst, toBytes(nogc), len * sizeof(*dst));
		return len;
	}


	// equals
	virtual bool equals( const char *src ) {

		JS::AutoCheckCannotGC nogc;
		size_t len = length();
		return isWide() ? jl::tstreqUnsigned(toWStr(nogc), len, src, -1) : jl::tstreqUnsigned(toStr(nogc), len, src, -1);
	}

	virtual bool equals( const char16_t *src ) {

		JS::AutoCheckCannotGC nogc;
		size_t len = length();
		return isWide() ? jl::tstreqUnsigned(toWStr(nogc), len, src, -1) : jl::tstreqUnsigned(toStr(nogc), len, src, -1);
	}

	virtual bool equals( const uint8_t *src ) {

		JS::AutoCheckCannotGC nogc;
		return jl::MemCompare(toBytes(nogc), src, length()) == 0;
	}

	// operator ==
	bool operator ==( const char *src ) {
	
		return equals(src);
	}

	bool operator ==( const char16_t *src ) {
	
		return equals(src);
	}

	bool operator ==( const uint8_t *src ) {
	
		return equals(src);
	}


	// to JS things

	virtual bool toArrayBuffer( JSContext *cx, JS::MutableHandleValue rval ) {
		
		size_t len = length();
		rval.setObjectOrNull( len > 0 ? JS_NewArrayBufferWithContents(cx, len, toOwnBytes()) : JS_NewArrayBuffer(cx, 0));
		JL_CHK( !rval.isNull() );
		return true;
		JL_BAD;
	}

	virtual bool toJSString( JSContext *cx, JS::MutableHandleValue rval ) {
		
		size_t len = length();
		if ( len > 0 ) {

			JS::RootedString str( cx, isWide() ? JL_NewUCString(cx, toOwnWStr(), len) : JL_NewString(cx, toOwnStr(), len) );
			JL_CHK( str );
			rval.setString( str );
			JL_CHK( !rval.isNull() );
		} else {

			rval.set(JL_GetEmptyStringValue(cx));
		}
		return true;
		JL_BAD;
	}

	virtual bool toJSValue( JSContext *cx, JS::MutableHandleValue rval ) {

		return isData() ? toArrayBuffer(cx, rval) : toJSString(cx, rval);
	}

};



class StrDataDst {
public:
	static const size_t UnknownLength = size_t(~0);
	enum Ownership {
		TransferOwnership
	};
	virtual bool isSet() const = 0; // the object has been set with data (may be empty).
	virtual size_t length() const = 0; // number on elements
	virtual bool set( JSContext *cx, JS::HandleValue val ) = 0;
	
	virtual bool set( const char *strZ, size_t len = UnknownLength ) = 0;
	virtual bool set( const char16_t *wStrZ, size_t len = UnknownLength ) = 0;
	
	virtual bool set( char *strZ, Ownership, size_t len = UnknownLength ) = 0;
	virtual bool set( char16_t *wStrZ, Ownership, size_t len = UnknownLength ) = 0;
};





class StrData : public StrDataSrc, public StrDataDst {

	enum {
		None,
		Latin1String,
		TwoByteString,
		ArrayBuffer,
		ArrayBufferView,
		Uint16ArrayBuffer,
		StrZ,
		WStrZ,
		OwnStrZ,
		OwnWStrZ,
	} _type;

	JSContext *_cx;
	JS::RootedString _str;
	JS::RootedObject _obj;
	void *_data;
	mutable size_t _length;

	uint8_t _staticBuffer[128]; 

private:

	void freeData() {

		if ( ( _type == OwnStrZ || _type == OwnWStrZ ) && _data != _staticBuffer )
			js_free( _data );
	}

	void setEmpty() {

		freeData();
		_data = "";
		_length = 0;
		_type = StrZ;
	}

	template<class D, class S>
	D *makeOwnCopy( S* src ) {

		JL_CHK( src );
		size_t len = length();
		ASSERT( len != UnknownLength );

		D *dst;
		size_t dstSize = (len + 1) * sizeof(D);

		bool doRealloc = src == _data && ( _type == OwnStrZ || _type == OwnWStrZ ) && _data != _staticBuffer;

		if ( doRealloc ) {
				
			dst = reinterpret_cast<D*>( sizeof(D) > sizeof(S) ? jl_realloc(_data, dstSize) : _data);
			JL_CHK( dst );
		}
		else if ( dstSize <= sizeof(_staticBuffer) ) {

			dst = reinterpret_cast<D*>(_staticBuffer);
			freeData();
		} else {

			ASSERT_IF( src == _data, _type != OwnStrZ && _type != OwnWStrZ );
			ASSERT( _data == _staticBuffer || (_type != OwnStrZ && _type != OwnWStrZ) ); // assert no need to freeData
			
			dst = reinterpret_cast<D*>(jl_malloc(dstSize));
			//freeData();
		}

		jl::reinterpretBuffer<D, S>(dst, src, len);
		dst[len] = 0;

		if ( sizeof(D) < sizeof(S) && doRealloc ) {

			dst = reinterpret_cast<D*>(jl_realloc(_data, dstSize));
			JL_CHK( dst );
		}

		ASSERT( sizeof(D) == 1 || sizeof(D) == 2 );
		_type = sizeof(D) == 2 ? OwnWStrZ : OwnStrZ;
		_data = dst;
		return dst;
		JL_BADVAL(nullptr);
	}


public:

	~StrData() {

		freeData();
	}

	StrData( JSContext *cx ) : _cx(cx), _str(cx), _obj(cx), _data(nullptr), _length(UnknownLength), _type(StrData::None) {
	}

	// transfer semantic
	StrData( StrData &strData ) :
		_cx(strData._cx),
		_str(strData._cx, strData._str),
		_obj(strData._cx, strData._obj),
		_data(strData._data),
		_length(strData._length),
		_type(strData._type)
	{
		
		if ( _data == strData._staticBuffer ) {

			ASSERT( _length != UnknownLength );
			jl::memcpy(_staticBuffer, strData._staticBuffer, _length + 1);
			_data = _staticBuffer;
		} else
		if ( _type == OwnStrZ )
			strData._type = StrZ; // ownership is lost
		else
		if ( _type == OwnWStrZ )
			strData._type = WStrZ; // ownership is lost
	}

	bool set( const char *strZ, size_t len = UnknownLength ) {
		
		freeData();
		_data = const_cast<char*>(strZ);
		_type = StrZ;
		_length = len;
		return true;
	}

	bool set( char *strZ, Ownership, size_t len = UnknownLength ) {

		freeData();
		_data = strZ;
		_type = OwnStrZ;
		_length = len;
		return true;
	}

	bool set( const char16_t *wStrZ, size_t len = UnknownLength ) {

		freeData();
		_data = const_cast<char16_t*>(wStrZ);
		_type = WStrZ;
		_length = len;
		return true;
	}

	bool set( char16_t *wStrZ, Ownership, size_t len = UnknownLength ) {

		freeData();
		_data = wStrZ;
		_type = OwnWStrZ;
		_length = len;
		return true;
	}

	bool set( JSContext *, JS::HandleString str ) {
	
		freeData();
		_str.set(str);
		_type = JL_StringHasLatin1Chars(str) ? Latin1String : TwoByteString;
		_length = UnknownLength;
		return true;
	}

	bool set( JSContext *cx, JS::HandleObject obj ) {
		
		freeData();
/*
		NIBufferGet fct = bufferGetNativeInterface(cx, obj);
		if ( fct ) {

		} else
*/
		if ( JS_IsArrayBufferObject(obj) ) {

			_obj.set(obj);
			_type = ArrayBuffer;
		} else
		if ( JS_IsTypedArrayObject(obj) ) {

			_obj.set(obj);
			_type = JS_GetArrayBufferViewType(obj) != js::Scalar::Uint16 ? ArrayBufferView : Uint16ArrayBuffer;
		} else {

			JS::RootedValue val(cx, JS::ObjectValue(*obj));
			_str.set(JS::ToString(cx, val));
			JL_CHK( _str );
			_type = JL_StringHasLatin1Chars(_str) ? Latin1String : TwoByteString;
		}
		_length = UnknownLength;
		return true;
		JL_BAD;
	}

	bool set( JSContext *cx, JS::HandleValue val ) {

		if ( val.isObject() ) {

			_obj.set(val.toObjectOrNull());
			JL_CHK( set(cx, _obj) );
		} else {

			_str.set(JS::ToString(cx, val));
			JL_CHKM( _str != nullptr, E_CONVERT, E_TY_STRING );
			JL_CHK( set(cx, _str) );
		}
		return true;
		JL_BAD;
	}

	bool set( JSContext *cx, JS::HandleId id ) {
		
		JS::RootedValue tmp(cx);
		return JS_IdToValue(cx, id, &tmp) && set(cx, tmp);
	}

	bool isSet() const {

		return _type != None;
	}

	size_t length() const {

		if ( _length == UnknownLength ) {

			switch ( _type ) {
				case None:
					_length = 0;
					break;
				case Latin1String:
				case TwoByteString:
					_length = js::GetStringLength(_str);
					break;
				case ArrayBuffer:
					_length = JS_GetArrayBufferByteLength(_obj);
					break;
				case ArrayBufferView:
					_length = JS_GetTypedArrayByteLength(_obj);
					break;
				case Uint16ArrayBuffer:
					_length = JS_GetTypedArrayByteLength(_obj) / 2;
					break;
				case OwnStrZ:
				case StrZ:
					_length = jl::tstrlen(static_cast<char*>(_data));
					break;
				case OwnWStrZ:
				case WStrZ:
					_length = jl::tstrlen(static_cast<char16_t*>(_data));
					break;
			}
		}
		ASSERT( _length != UnknownLength );
		return _length;
	}

	bool isWide() const {

		return _type == TwoByteString || _type == OwnWStrZ || _type == Uint16ArrayBuffer || _type == WStrZ;
	}

	bool isData() const {
		
		return _type == ArrayBuffer || _type == ArrayBufferView;
	}

	char16_t * toOwnWStrZ( bool canTransferOwnership = true ) {

		if ( canTransferOwnership && _type == OwnWStrZ && _data != _staticBuffer ) {
			
			_type = WStrZ;
			return static_cast<char16_t*>(_data);
		}
		return StrDataSrc::toOwnWStrZ(canTransferOwnership);
	}

	char16_t * toOwnWStr( bool canTransferOwnership = true ) {
	
		return toOwnWStrZ(canTransferOwnership);
	}

	char * toOwnStrZ( bool canTransferOwnership = true ) {
		
		if ( canTransferOwnership && _type == OwnStrZ && _data != _staticBuffer ) {
			
			_type = StrZ;
			return static_cast<char*>(_data);
		}
		return StrDataSrc::toOwnStrZ(canTransferOwnership);
	}

	char * toOwnStr( bool canTransferOwnership = true ) {
		
		return toOwnStrZ(canTransferOwnership);
	}

	uint8_t * toOwnBytes( bool canTransferOwnership = true ) {
		
		return reinterpret_cast<uint8_t*>(toOwnStr(canTransferOwnership));
	}


	// to wide c-string
	const jschar *toWStrZ(const JS::AutoCheckCannotGC &nogc) {

		JSFlatString *fstr;
		JSLinearString *lstr;
		switch ( _type ) {
			case None:
				return nullptr;	
			case Latin1String:
				lstr = js::StringToLinearString(_cx, _str);
				JL_CHK( lstr );
				return makeOwnCopy<char16_t>(js::GetLatin1LinearStringChars(nogc, lstr));
			case TwoByteString:
				// doc: Flat strings and interned strings are always null-terminated. JS_FlattenString can be used to get a null-terminated string.
				fstr = JS_StringIsFlat(_str) ? JS_ASSERT_STRING_IS_FLAT(_str) : JS_FlattenString(_cx, _str);
				JL_CHK( fstr );
				return JS_GetTwoByteFlatStringChars(nogc, fstr);
			case ArrayBuffer:
				return makeOwnCopy<char16_t>(JS_GetArrayBufferData(_obj));
			case ArrayBufferView:
				return makeOwnCopy<char16_t>(reinterpret_cast<int8_t*>(JS_GetArrayBufferViewData(_obj)));
			case Uint16ArrayBuffer:
				return reinterpret_cast<const jschar*>(JS_GetUint16ArrayData(_obj));
			case StrZ:
			case OwnStrZ:
				return makeOwnCopy<char16_t>(static_cast<char*>(_data));
			case WStrZ:
			case OwnWStrZ:
				return static_cast<char16_t*>(_data);
		}
		JL_BADVAL(nullptr);
	}

	// to c-string
	const char *toStrZ(const JS::AutoCheckCannotGC &nogc) {

		JSFlatString *fstr;
		JSLinearString *lstr;
		switch ( _type ) {
			case None:
				return nullptr;	
			case Latin1String:
				// doc: Flat strings and interned strings are always null-terminated. JS_FlattenString can be used to get a null-terminated string.
				fstr = JS_StringIsFlat(_str) ? JS_ASSERT_STRING_IS_FLAT(_str) : JS_FlattenString(_cx, _str);
				JL_CHK( fstr );
				return reinterpret_cast<const char*>(JS_GetLatin1FlatStringChars(nogc, fstr));
			case TwoByteString:
				lstr = js::StringToLinearString(_cx, _str);
				JL_CHK( lstr );
				return makeOwnCopy<char>(js::GetTwoByteLinearStringChars(nogc, lstr));
			case ArrayBuffer:
				return makeOwnCopy<char>(JS_GetArrayBufferData(_obj));
			case ArrayBufferView:
				return makeOwnCopy<char>(reinterpret_cast<uint8_t*>(JS_GetArrayBufferViewData(_obj)));
			case Uint16ArrayBuffer:
				return makeOwnCopy<char>(JS_GetUint16ArrayData(_obj));
			case StrZ:
			case OwnStrZ:
				return static_cast<char*>(_data);
			case WStrZ:
			case OwnWStrZ:
				return makeOwnCopy<char>(static_cast<char16_t*>(_data));
		}
		JL_BADVAL(nullptr);
	}

	// to wide c-string not null terminated
	const char *toStr(const JS::AutoCheckCannotGC &nogc) {

		JSLinearString *lstr;
		switch ( _type ) {
			case None:
				return nullptr;	
			case Latin1String:
				lstr = js::StringToLinearString(_cx, _str);
				JL_CHK( lstr );
				return reinterpret_cast<const char*>(js::GetLatin1LinearStringChars(nogc, lstr));
			case TwoByteString:
				lstr = js::StringToLinearString(_cx, _str);
				JL_CHK( lstr );
				return makeOwnCopy<char>(js::GetTwoByteLinearStringChars(nogc, lstr));
			case ArrayBuffer:
				return reinterpret_cast<const char*>(JS_GetArrayBufferData(_obj));
			case ArrayBufferView:
				return reinterpret_cast<const char*>(JS_GetArrayBufferViewData(_obj));
			case Uint16ArrayBuffer:
				return makeOwnCopy<char>(JS_GetUint16ArrayData(_obj));
			case StrZ:
			case OwnStrZ:
				return static_cast<const char*>(_data);
			case WStrZ:
			case OwnWStrZ:
				return makeOwnCopy<char>(static_cast<char16_t*>(_data));
		}
		JL_BADVAL(nullptr);
	}


	char16_t getWCharAt(size_t index) {

		jschar chr;
		switch ( _type ) {
			case OwnStrZ:
			case StrZ:
				return static_cast<char16_t>( static_cast<char*>(_data)[index] );
			case OwnWStrZ:
			case WStrZ:
				return static_cast<char16_t*>(_data)[index];
			case Latin1String:
			case TwoByteString: {
				bool res = JS_GetStringCharAt(_cx, _str, index, &chr);
				ASSERT( res );
				return chr;
			}
			case ArrayBuffer:
				return static_cast<char16_t>( reinterpret_cast<char*>(JS_GetArrayBufferData(_obj))[index] );
			case Uint16ArrayBuffer:
				return reinterpret_cast<const jschar*>(JS_GetUint16ArrayData(_obj))[index];
			case ArrayBufferView:
				return static_cast<char16_t>( reinterpret_cast<char*>(JS_GetArrayBufferViewData(_obj))[index] );
			default:
				return 0;
		}
	}


	using StrDataSrc::copyTo; // bring back copyTo() into this scope

	size_t copyTo( char16_t *dst, size_t maxLength ) {

		if ( _type == TwoByteString ) {
			size_t len = jl::min(length(), maxLength);
			bool res = js::CopyStringChars(_cx, dst, _str, len);
			ASSERT( res );
			return len;
		}
		return StrDataSrc::copyTo(dst, maxLength);
	}


	virtual bool toArrayBuffer( JSContext *cx, JS::MutableHandleValue rval ) {

		if ( _type == ArrayBuffer ) {
			
			JL_CHK( _obj );
			rval.setObject(*_obj);
			return true;
		}

		if ( _type == ArrayBufferView ) {
			
			JL_CHK( _obj );
			_obj.set(JS_GetArrayBufferViewBuffer(cx, _obj));
			JL_CHK( _obj );
			rval.setObject(*_obj);
			return true;
		}

		size_t len = length();
		if ( len > 0 ) {

			_obj.set( JS_NewArrayBufferWithContents(cx, len, toOwnBytes()) );
			JL_CHK( _obj );
			rval.setObject( *_obj );
			freeData();
		} else {

			_obj.set( JS_NewArrayBuffer(cx, 0) );
			JL_CHK( _obj );
			setEmpty();
		}
		return true;
		JL_BAD;
	}

	virtual bool toJSString( JSContext *cx, JS::MutableHandleValue rval ) {

		if ( _type == Latin1String || _type == TwoByteString ) {

			JL_CHK( _str );
			rval.setString(_str);
			return true;
		}
		
		size_t len = length();
		if ( len > 0 ) {

			_str.set(isWide() ? JL_NewUCString(cx, toOwnWStr(), len) : JL_NewString(cx, toOwnStr(), len));
			JL_CHK( _str );
			_type = JL_StringHasLatin1Chars(_str) ? Latin1String : TwoByteString;
			rval.setString(_str);
			freeData();
		} else {

			rval.set(JL_GetEmptyStringValue(cx));
			setEmpty();
		}
		return true;
		JL_BAD;
	}
};




class BufBase : virtual public jl::CppAllocators {
public:
	enum InitializationType { Uninitialized = 1 };
	typedef uint8_t* Type;
	static const size_t UnknownSize = size_t(~0);

private:
	Type _data;
	mutable size_t _allocSize;
	mutable size_t _used;
	mutable bool _owner; // true if this object is responsible of freeing the memory pointed by _data

protected:

	void
	setAllocSize( size_t allocSize ) const {

		_allocSize = allocSize;
	}

public:

	~BufBase() {

		if ( hasData() && owner() )
			free();
	}

	BufBase(InitializationType) {
	}

	BufBase()
	: _data(nullptr) {
	}

	template <class T>
	explicit BufBase( T *data, size_t size = UnknownSize )
	: _data(reinterpret_cast<Type>(const_cast<RemoveConst(T)*>(data))), _allocSize(size), _used(size), _owner(!IsConst(T)) {
	}

	BufBase( const BufBase& buf, bool withOwnership = true )
	: _data(buf._data), _allocSize(buf._allocSize), _used(buf._used) {

		if ( hasData() ) {

			if ( withOwnership ) {

				bool owner = buf.owner(); // handle when this is buf
				buf.dropOwnership(); // only one can free the buffer
				_owner = owner;
			} else {

				_owner = false;
			}
		}
	}

	template <class T>
	void
	get( T *data, size_t size = UnknownSize ) {

		if ( hasData() && owner() )
			free();
		setData(data, !IsConst(T));
		setAllocSize(size); // at least
		setUsed(size);
	}

	void
	get( const BufBase &buf, bool withOwnership = true ) {

		if ( hasData() && owner() && !is(buf) )
			free();

		if ( withOwnership ) {

			bool tmp = buf.owner(); // handle when this is buf
			buf.dropOwnership(); // only one can free the buffer
			setData(buf.data(), tmp);
		} else {

			setData(buf.data(), false);
		}
		setAllocSize(buf.allocSize());
		setUsed(buf.used());
	}

	void operator =( const BufBase &buf ) {

		get(buf);
	}

	BufBase&
	setEmpty() {

		if ( hasData() && owner() )
			free();
		else
			setData(nullptr);
		//setUsed(0);
		// setAllocSize(0);
		// dropOwnership();
		return *this;
	}

	bool
	isEmpty() {

		return !hasData() || used() == 0;
	}

	template <class T>
	void
	setData( const T *data, bool ownership ) {

		_data = reinterpret_cast<Type>(const_cast<T*>(data));
		_owner = ownership;
	}
	
	void
	setData( nullptr_t ) {

		_data = nullptr;
	}

	bool
	hasData() const {

		return _data != nullptr;
	}

	operator bool() const {

		return _data != nullptr;
	}

	template <class T>
	const T
	dataAs() const {

		return reinterpret_cast<const T>(_data);
	}

	template <class T>
	T
	dataAs() {

		return reinterpret_cast<T>(_data);
	}

	const Type
	data() const {

		return _data;
	}

	Type
	data() {

		return _data;
	}

	size_t
	allocSize() const {

		ASSERT_IF( _data == nullptr, _allocSize == 0 );
		return _allocSize;
	}

	size_t
	used() const {

		ASSERT_IF( _data == nullptr, allocSize() == 0 );
		return _used;
	}

	void
	setUsed( size_t used ) const {
		
		ASSERT( used == 0 || used <= allocSize() );
		_used = used;
	}


	bool
	owner() const {

		return _owner;
	}

	void
	dropOwnership() const {

		_owner = false;
	}

	bool
	is( const BufBase &buf ) const {

		return buf.data() == data();
	}

	void
	own( bool crop = false ) {
		
		ASSERT( !owner() );
		Type tmp = static_cast<Type>(jl_malloc(crop ? used() : allocSize()));
		jl::memcpy(tmp, data(), used());
		setData(tmp, true);
	}

	void
	alloc( size_t size, bool usedAll = false ) {
		
		ASSERT_IF( hasData(), !owner() ); // else memory leak
		setData(jl_malloc(size), true);
		setAllocSize(size);
		setUsed(usedAll ? size : 0);
	}

	void
	realloc( size_t newSize ) {

		if ( owner() ) {

			setData(jl_realloc(data(), newSize), true);
			setAllocSize(newSize);
		} else {

			void *newData = jl_malloc(newSize);
			setAllocSize(newSize);

			if ( used() > allocSize() )
				setUsed(allocSize());

			if ( newData )
				jl::memcpy(newData, data(), used());

			setData(newData, true);
		}
	}

	void
	free( bool wipe = false ) {

		ASSERT( owner() );
		ASSERT( data() );

		if ( wipe )
			jl::zeromem(data(), used());
		jl_free(data());
		
		setData(nullptr);
//		setAllocSize(0);
//		setUsed(0);
	}

	void
	maybeCrop() {
		
		ASSERT( owner() );

		if ( owner() && maybeRealloc(allocSize(), used()) ) {
		
			realloc(used());
			ASSERT( !!*this ); // assume it always possible to reduce the size of an allocated block
		}
	}


	bool
	stealArrayBuffer( JSContext *cx, JS::HandleObject obj ) {
		
		ASSERT( !owner() );

		JL_ASSERT( JS_IsArrayBufferObject(obj), E_TY_ARRAYBUFFER, E_REQUIRED );

		size_t size = JS_GetArrayBufferByteLength(obj);
		void *data = JS_StealArrayBufferContents(cx, obj);
		get(data, size);
		JL_CHK( data != nullptr );
		return true;
		JL_BAD;
	}


	bool
	toArrayBuffer( JSContext *cx, JS::MutableHandleValue rval ) {
		
		ASSERT( used() != UnknownSize );
		if ( used() == 0 ) {
			
			rval.setObjectOrNull(JS_NewArrayBuffer(cx, 0));
		} else {

			if ( !owner() )
				own();
			rval.setObjectOrNull(JS_NewArrayBufferWithContents(cx, used(), data()));
			dropOwnership();
		}
		JL_CHK( !rval.isNull() );
		return true;
		JL_BAD;
	}

};


////////

S_ASSERT( sizeof(jschar) != sizeof(char) );
S_ASSERT( sizeof(jschar) == sizeof(wchar_t) );


class BufString : public BufBase, public jl::StrDataSrc {
	typedef char16_t WideChar;
	typedef char NarrowChar;

	uint8_t _charSize;
	uint8_t _terminatorLength;

public:

	uint8_t
	charSize() const {

		ASSERT(_charSize == sizeof(NarrowChar) || _charSize == sizeof(WideChar));
		return _charSize;
	}

	BufString&
	setCharSize( uint8_t charSize ) {

		_charSize = charSize;
		return *this;
	}

	bool
	isWide() const {

		return _charSize == sizeof(WideChar);
	}

	bool
	isNt() const {

		return _terminatorLength != 0;
	}

	BufString&
	setNt( bool nullTerminated ) {

		_terminatorLength = nullTerminated ? 1 : 0;
		return *this;
	}

	void
	alloc( size_t size ) {
		
		BufBase::alloc(size);
	}

	BufString&
	setEmpty() {

		BufBase::get("", 1);
		_charSize = sizeof(NarrowChar);
		_terminatorLength = 1;
		return *this;
	}

	bool
	isEmpty() const {

		return !hasData() || used() == 0 || used() == size_t(_terminatorLength * charSize());
	}

	bool
	isData() const {
		
		return false;
	}

	bool
	isSet() const {

		return hasData();
	}

	template <class T>
	void
	get( T *str, size_t len = UnknownSize, bool nullTerminated = true ) {
	
		ASSERT_IF( len == UnknownSize, nullTerminated == true );

		_charSize = sizeof(*str);
		_terminatorLength = nullTerminated ? 1 : 0;
		BufBase::get(str, len != UnknownSize ? sizeof(*str) * (len + _terminatorLength) : UnknownSize);
	}

	void
	get( const BufBase & buf, bool withOwnership = true ) {

		BufBase::get(buf, withOwnership);
	}

	void
	get( const BufString & buf, bool withOwnership = true ) {

		BufBase::get(buf, withOwnership);
		setCharSize( buf.charSize() );
		setNt( buf.isNt() );
	}

	void
	get( JSContext *cx, JS::HandleString str, const JS::AutoCheckCannotGC &nogc ) {

		size_t len;
		// doc: Flat strings and interned strings are always null-terminated so JS_FlattenString can be used to get a null-terminated string.
		bool isFlatOrInterned = JS_StringIsFlat(str) || JS_StringHasBeenInterned(cx, str);
		if ( JL_StringHasLatin1Chars(str) ) {

			const JS::Latin1Char *chars = JS_GetLatin1StringCharsAndLength(cx, nogc, str, &len);
			get(chars, len, isFlatOrInterned);
			ASSERT( !isWide() );
		} else {

			const jschar *chars = JS_GetTwoByteStringCharsAndLength(cx, nogc, str, &len);
			get(chars, len, isFlatOrInterned);
			ASSERT( isWide() );
		}

		assertIntegrity();
	}

//

	void operator =( const BufString &buf ) {
		
		get(buf);
	}

	ALWAYS_INLINE
	BufString() {
	}

	ALWAYS_INLINE
	void
	assertIntegrity() const {
		
		if ( IS_DEBUG ) {

			ASSERT( hasData() );
			ASSERT_IF( used() == UnknownSize, isNt() );

			size_t len;
			if ( used() == UnknownSize ) {

				len = isWide() ? jl::strlen(dataAs<WideChar*>()) : jl::strlen(dataAs<NarrowChar*>());
			} else {
				
				len = used() / charSize() - _terminatorLength;
			}

			ASSERT_IF( used() != UnknownSize && isWide(), used() % 2 == 0 );
			ASSERT_IF( owner() && isWide() && !isNt(), jl_msize(data()) >= len*2 );
			ASSERT_IF( owner() && !isWide() && !isNt(), jl_msize(data()) >= len );
			ASSERT_IF( owner() && isWide() && isNt(), jl_msize(data()) >= len*2+2 );
			ASSERT_IF( owner() && !isWide() && isNt(), jl_msize(data()) >= len+1 );
			ASSERT_IF( isNt() && isWide(), ((jschar*)data())[len] == 0 );
			ASSERT_IF( isNt() && !isWide(), ((char*)data())[len] == 0 );
			ASSERT_IF( used() != UnknownSize, used() / charSize() >= _terminatorLength );
		}
	}
	
	// len argument does not include the possible \0
	template <class T>
	explicit BufString( T *str, size_t len = UnknownSize, bool nullTerminated = true )
	: BufBase( str, len != UnknownSize ? (len + (nullTerminated ? 1 : 0)) * sizeof( T ) : UnknownSize ), _charSize( sizeof( T ) ), _terminatorLength( nullTerminated ? 1 : 0 ) {
		assertIntegrity();
	}

	BufString( const BufBase& buf, bool withOwnership = true )
		: BufBase( buf, withOwnership ), _charSize( 1 ), _terminatorLength( 0 ) {
	}

	BufString( const BufString& buf, bool withOwnership = true )
	: BufBase(buf, withOwnership), _charSize(buf._charSize), _terminatorLength(buf._terminatorLength) {
	}

	explicit BufString( JSContext *cx, JS::HandleString str, const JS::AutoCheckCannotGC &nogc ) {

		get(cx, str, nogc);
	}

//

	INLINE size_t
	length() const {

		size_t len;
		if ( used() == UnknownSize ) {

			len = isWide() ? jl::strlen(dataAs<WideChar*>()) : jl::strlen(dataAs<NarrowChar*>());
			setUsed((len + 1) * charSize());
			setAllocSize(used());
		} else {
				
			len = used() / charSize() - _terminatorLength;
		}

		assertIntegrity();

		return len;
	}


	size_t
	lengthOrZero() const {

		return hasData() ? length() : 0;
	}

//

	bool
	operator ==( const BufString &str ) const {

		if ( str.isEmpty() && isEmpty() )
			return true;

		if ( !str.isEmpty() == isEmpty() )
			return false;

		size_t len = length();
		size_t strLen = str.length();
		if ( len == strLen ) {

			if ( isWide() ) {
			
				return str.isWide() ? jl::tstreq(dataAs<WideChar*>(), len, str.dataAs<WideChar*>(), strLen) : jl::tstreqUnsigned(dataAs<WideChar*>(), len, str.dataAs<NarrowChar*>(), strLen);
			} else {

				return str.isWide() ? jl::tstreq( dataAs<NarrowChar*>(), len, str.dataAs<WideChar*>(), strLen) : jl::tstreqUnsigned( dataAs<NarrowChar*>(), len, str.dataAs<NarrowChar*>(), strLen);
			}
		} else {

			return false;
		}
	}

	bool
	operator !=( const BufString &str ) const {
	
		return !(operator ==(str));
	}


	template <class T>
	bool
	operator ==( const T *str ) const {
	
		if ( ( str == nullptr || *str == 0 ) && isEmpty() )
			return true;

		if ( (str != nullptr && *str != 0 ) == isEmpty() )
			return false;

		if ( isNt() ) {

			return dataAs<T*>() == str || ( isWide() ? jl::tstreqUnsigned( dataAs<WideChar*>(), -1, str, -1 ) : jl::tstreqUnsigned( dataAs<NarrowChar*>(), -1, str, -1 ) );
		} else {

			size_t len = length();
			return jl::strlen( str ) == len && ( isWide() ? jl::tstreqUnsigned( dataAs<WideChar*>(), len, str, -1 ) : jl::tstreqUnsigned( dataAs<NarrowChar*>(), len, str, -1 ) );
		}
	}

	template <class T>
	bool
	operator !=( const T *str ) const {

		return !(operator ==(str));
	}

/*
	template <class T>
	bool
	operator icmp( const T *str ) const {
	
		if ( str == nullptr && isEmpty() )
			return true;

		if ( (str != nullptr) == isEmpty() )
			return false;

		if ( isNt() ) {

			return dataAs<T*>() == str || (isWide() ? jl::tstricmpUnsigned( dataAs<WideChar*>(), str ) == 0 : jl::tstricmpUnsigned( dataAs<NarrowChar*>(), str ) == 0);
		} else {

			size_t len = length();
			return jl::strlen( str ) == len && (isWide() ? jl::tstrnicmpUnsigned( dataAs<WideChar*>(), str, len ) == 0 : jl::tstrnicmpUnsigned( dataAs<NarrowChar*>(), str, len ) == 0);
		}
	}
*/


//

	template <class T>
	T
	charAt( size_t index ) const {

		T retChar;
		
		DISABLE_SMALLER_TYPE_CHECK;

		if ( sizeof(T) == charSize() )
			retChar = dataAs<T*>()[index];
		else
		if (sizeof(T) == sizeof(NarrowChar))
			retChar = isWide() ? dataAs<WideChar*>()[index] : dataAs<NarrowChar*>()[index];
		else
		if ( sizeof(T) == sizeof(WideChar) )
			retChar = isWide() ? dataAs<WideChar*>()[index] : dataAs<NarrowChar*>()[index];
		else
			ASSERT(false);
		
		RESTORE_SMALLER_TYPE_CHECK;
		
		return retChar;
	}

//

	template <class T>
	size_t
	copyTo( T *dst, size_t maxLength = UnknownSize ) const {

		size_t len = length();
		maxLength = maxLength == UnknownSize ? len : jl::min(len, maxLength);
		if ( charSize() == sizeof(T) )
			jl::memcpy(dst, data(), len * sizeof(T));
		else
		if (charSize() == sizeof(NarrowChar))
			reinterpretBufferUnsigned<T, NarrowChar>(dst, data(), len);
		else
		if ( charSize() == sizeof(WideChar) )
			reinterpretBufferUnsigned<T, WideChar>( dst, data(), len);
		else
			ASSERT(false);
		return maxLength;
	}

//

	template<class T, bool nullTerminated>
	T
	to() {

		typedef RemovePointer(T) Base;
		typedef RemoveConst(Base) MutableBase;
		const bool asConst = IsConst(Base);

		ASSERT( this-operator bool() );
		ASSERT( charSize() > 0 && charSize() <= 2 );

		const size_t len = length();

		ASSERT_IF( len == UnknownSize, isNt() );
		ASSERT_IF( len != UnknownSize && owner() && isWide() && !isNt(), jl_msize(data()) >= len*2 );
		ASSERT_IF( len != UnknownSize && owner() && !isWide() && !isNt(), jl_msize(data()) >= len );
		ASSERT_IF( len != UnknownSize && owner() && isWide() && isNt(), jl_msize(data()) >= len*2+2 );
		ASSERT_IF( len != UnknownSize && owner() && !isWide() && isNt(), jl_msize(data()) >= len+1 );
		ASSERT_IF( len != UnknownSize && isNt() && isWide(), ((jschar*)data())[len] == 0 );
		ASSERT_IF( len != UnknownSize && isNt() && !isWide(), ((char*)data())[len] == 0 );
		ASSERT_IF( isWide(), used() % 2 == 0 );
		ASSERT_IF( len != UnknownSize, used() / charSize() >= _terminatorLength );

		if ( data() == nullptr )
			return nullptr;

		// something to do ?
		if ( sizeof(Base) != charSize() || (!asConst && !owner()) || (nullTerminated && !isNt()) ) {

			const size_t requiredSize = (len + (nullTerminated ? 1 : 0)) * sizeof(Base);

			BufBase dst(*this, false);

			// two reasons to allocate
			if ( allocSize() < requiredSize || !owner() ) {

				if ( sizeof(Base) != charSize() ) { // avoid useless copy made by realloc

					dst.alloc(requiredSize);
				} else {

					realloc(requiredSize);
					dst.get(*this, false); // sync
				}
			}

			if ( charSize() == sizeof(Base) )
				;// nothing
			else
			if (charSize() == sizeof(NarrowChar))
				reinterpretBufferUnsigned<MutableBase, NarrowChar>(dst.data(), data(), len);
			else
			if ( charSize() == sizeof(WideChar) )
				reinterpretBufferUnsigned<MutableBase, WideChar>( dst.data(), data(), len);

			if ( nullTerminated )
				dst.dataAs<MutableBase*>()[len] = Base(0);

			if ( !is(dst) )
				get(dst, true);

			setUsed(requiredSize);
			setCharSize(sizeof(Base));
			setNt(nullTerminated);
		}

		if ( !asConst )
			dropOwnership();

		return dataAs<T>();
	}

//

	template<class T>
	T
	toData() {

		return to<T, false>();
	}

	template<class T>
	T
	toStringZ() {

		return to<T, true>();
	}

//

	template<class T>
	T
	toDataOrNull() {

		return hasData() ? toData<T>() : nullptr;
	}

	template<class T>
	T
	toStringZOrNull() {

		return hasData() ? toStringZ<T>() : nullptr;
	}

	// part to the StrDataSrc interface
	ALWAYS_INLINE
	const char * toStrZ(const JS::AutoCheckCannotGC &nogc) {

		return toStringZOrNull<const NarrowChar*>();
	}

	ALWAYS_INLINE
	char * toOwnStrZ(const JS::AutoCheckCannotGC &nogc) {

		return toStringZOrNull<NarrowChar*>();
	}

	ALWAYS_INLINE
	const WideChar * toWStrZ(const JS::AutoCheckCannotGC &nogc) {

		return toStringZOrNull<const WideChar*>();
	}

	ALWAYS_INLINE
	char16_t * toOwnWStrZ() {

		return toStringZOrNull<WideChar*>();
	}

	ALWAYS_INLINE
	const uint8_t * toBytes() {

		return toDataOrNull<const uint8_t*>();
	}

	ALWAYS_INLINE
	uint8_t * toOwnBytes() {

		return toDataOrNull<uint8_t*>();
	}

	//


	ALWAYS_INLINE
	operator const NarrowChar *() {

		return toStringZOrNull<const NarrowChar*>();
	}

	ALWAYS_INLINE
	operator const WideChar *() {

		return toStringZOrNull<const WideChar*>();
	}

	ALWAYS_INLINE
	operator const uint8_t *() {

		return toDataOrNull<const uint8_t*>();
	}

	bool
	toString( JSContext *cx, JS::MutableHandleValue rval ) {

		size_t len = length();
		if ( len == 0 ) {

			rval.set(JL_GetEmptyStringValue(cx));
		} else {
			
			JS::RootedString str( cx, isWide() ? JL_NewUCString(cx, toData<WideChar*>(), len) : JL_NewString(cx, toData<NarrowChar*>(), len) );
			JL_ASSERT_ALLOC( str );
			rval.setString(str);
		}
		return true;
		JL_BAD;
	}

	bool
	toArrayBuffer( JSContext *cx, JS::MutableHandleValue rval ) {
		
		size_t len = length();
		if ( len == 0 ) {
			
			rval.setObjectOrNull(JS_NewArrayBuffer(cx, 0));
		} else {

			if ( !owner() )
				own();
			rval.setObjectOrNull(JS_NewArrayBufferWithContents(cx, len * charSize(), data())); // avoid the terminal \0
			dropOwnership();
		}
		JL_CHK( !rval.isNull() );
		return true;
		JL_BAD;
	}

};




namespace pv {

template <class T>
class StrSpec {
	static const size_t undefined = size_t(-1);
	T _str;
	mutable size_t _len;
public:
	StrSpec(T str)
	: _str(str), _len(StrSpec::undefined) {
	}

	StrSpec( T str, size_t len )
	: _str(str), _len(len) {

		ASSERT( len != StrSpec::undefined );
	}

	const T	str() const {

		return _str;
	}

	T str() {

		return _str;
	}

	size_t len() const {

		ASSERT( _str );
		if (unlikely( _len == StrSpec::undefined )) {

			_len = jl::strlen(_str);
			ASSERT( _len != StrSpec::undefined );
		}
		return _len;
	}
};

} // namespace pv



typedef pv::StrSpec<const char *> CStrSpec;

typedef pv::StrSpec<const jschar *> WCStrSpec;

typedef pv::StrSpec<jschar *> OwnerlessWCStrSpec;

ALWAYS_INLINE CStrSpec FASTCALL
strSpec( const char *str, size_t len ) {

	return CStrSpec(str, len);
}

ALWAYS_INLINE WCStrSpec FASTCALL
strSpec( const jschar *str, size_t len ) {

	return WCStrSpec(str, len);
}


ALWAYS_INLINE
bool
UTF16LEToUTF8( jl::BufString &dstBuf, jl::BufString &srcBuf ) {

	ASSERT(srcBuf.isWide());

	const uint8_t *src = reinterpret_cast<const uint8_t *>(srcBuf.toData<const wchar_t *>());
	const uint8_t *srcEnd = src + srcBuf.length() * 2;

	//jl::BufBase dstBuf;
	dstBuf.alloc((srcBuf.length() * 2) * (3/2));

	for (;;) {

		size_t srcSize = srcEnd - src;
		size_t dstSize = dstBuf.allocSize() - dstBuf.used();
		uint8_t *dst = dstBuf.dataAs<uint8_t *>() + dstBuf.used();
		int res = UTF16LEToUTF8(dst, &dstSize, src, &srcSize);
		if ( res < 0 )
			return false;
		src += srcSize;
		dstBuf.setUsed(dstBuf.used() + res);
		if (src >= srcEnd)
			break;
		dstBuf.realloc(dstBuf.allocSize() * 2);
	}
	dstBuf.maybeCrop();
	dstBuf.setCharSize( 1 );
	dstBuf.setNt( 0 );
	return true;
}



template <class T, size_t LENGTH>
class SimpleBufferBuffer {
	T _buf[LENGTH];
	T *_ptr; // > _end on error
	T *_end;

public:
	SimpleBufferBuffer()
	: _ptr( _buf ), _end( _buf + LENGTH ) {
		ASSERT( LENGTH >= 1 );
	}

	size_t
	avail() const {

		return _end - _ptr;
	}

	size_t
	length() const {

		return _ptr - _buf;
	}

	bool
	isEmpty() const {

		return length() == 0;
	}

	bool
	hasError() const {

		return _ptr > _end;
	}

	void
	reset() {

		_ptr = _buf;
	}

	const T *
	toString() {

		if ( hasError() ) {

			*(_end - 1) = 0;
		} else {

			if ( avail() < 1 )
				return nullptr;
			*_ptr = 0;
		}
		return _buf;
	}

	operator const T *() {

		return toString();
	}
	
	void
	move( ptrdiff_t mv ) {
		
		if ( _ptr + mv >= _buf && _ptr + mv < _end )
			_ptr += mv;
		else
			_ptr = _end + 1;
	}

	template <class U>
	bool
	rchr( const U chr ) {

		if ( hasError() )
			return false;

		T *tmp = _ptr;
		while ( _ptr > _buf )
			if ( *(--_ptr) == static_cast<T>(chr) )
				return true;
		_ptr = tmp;
		return false;
	}

	template <class U>
	void
	cat( const U * str ) {

		while ( _ptr < _end && *str != 0 )
			*(_ptr++) = static_cast<T>( *(str++) );
		
		if ( _ptr == _end && *str != 0 )
			_ptr++;
	}

	template <class U>
	void
	cat( const U * str, const U * strEnd ) {

		while ( _ptr < _end && str != strEnd  )
			*(_ptr++) = static_cast<T>( *(str++) );

		if ( _ptr == _end && str != strEnd )
			_ptr++;
	}

	void
	cat( long num, uint8_t base = 10 ) {

		ASSERT( avail() > 32 ); // see IToA10MaxDigits
		jl::itoa( num, _ptr, base );
		_ptr += jl::strlen( _ptr );
	}
};


JL_END_NAMESPACE
