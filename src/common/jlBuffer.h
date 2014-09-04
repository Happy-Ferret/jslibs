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


class BufBase : public jl::CppAllocators {
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


class BufString : public BufBase {
	typedef jschar WideChar;
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
		if ( JS_StringHasLatin1Chars(str) ) {

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
		if ( len == str.length() ) {

			if ( isWide() ) {
			
				return str.isWide() ? jl::tstrncmp(dataAs<WideChar*>(), str.dataAs<WideChar*>(), len) == 0 : jl::tstrncmpUnsigned(dataAs<WideChar*>(), str.dataAs<NarrowChar*>(), len) == 0;
			} else {

				return str.isWide() ? jl::tstrncmp( dataAs<NarrowChar*>(), str.dataAs<WideChar*>(), len ) == 0 : jl::tstrncmpUnsigned( dataAs<NarrowChar*>(), str.dataAs<NarrowChar*>(), len) == 0;
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
	
		if ( str == nullptr && isEmpty() )
			return true;

		if ( (str != nullptr) == isEmpty() )
			return false;

		if ( isNt() ) {

			return dataAs<T*>() == str || (isWide() ? jl::tstrcmpUnsigned( dataAs<WideChar*>(), str ) == 0 : jl::tstrcmpUnsigned( dataAs<NarrowChar*>(), str ) == 0);
		} else {

			size_t len = length();
			return jl::strlen( str ) == len && (isWide() ? jl::tstrncmpUnsigned( dataAs<WideChar*>(), str, len ) == 0 : jl::tstrncmpUnsigned( dataAs<NarrowChar*>(), str, len ) == 0);
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

	ALWAYS_INLINE
	operator const NarrowChar *() {

		return toStringZOrNull<const NarrowChar*>();
	}

	ALWAYS_INLINE
	operator const WideChar *() {

		return toStringZOrNull<const WideChar*>();
	}

	bool
	toString( JSContext *cx, JS::MutableHandleValue rval ) {

		size_t len = length();
		if ( len == 0 ) {

			rval.set(JL_GetEmptyStringValue(cx));
		} else {

			if ( isWide() )
				rval.setString(JL_NewUCString(cx, toData<WideChar*>(), len));
			else
				rval.setString(JL_NewString(cx, toData<NarrowChar*>(), len));
			JL_CHK( !rval.isNull() );
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
