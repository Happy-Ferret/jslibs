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
		setAllocSize(size);
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
	empty() {

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
			
			rval.setObject(*JS_NewArrayBuffer(cx, 0));
		} else {

			if ( !owner() )
				own();
			rval.setObject(*JS_NewArrayBufferWithContents(cx, used(), data()));
			dropOwnership();
		}
		JL_CHK( !rval.isNull() );
		return true;
		JL_BAD;
	}

};


////////


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
	wide() const {

		return _charSize == sizeof(WideChar);
	}

	bool
	nt() const {

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
	empty() {

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
		setNt( buf.nt() );
	}

	void
	get( JSContext *cx, JS::HandleString str ) {

		size_t len;
		const jschar *chars = JS_GetStringCharsAndLength(cx, str, &len);
		get(chars, len, false);
		_charSize = sizeof(*chars);
		_terminatorLength = 0;
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
			ASSERT_IF( used() == UnknownSize, nt() );

			size_t len;
			if ( used() == UnknownSize ) {

				len = wide() ? jl::strlen(dataAs<WideChar*>()) : jl::strlen(dataAs<char*>());
			} else {
				
				len = used() / charSize() - _terminatorLength;
			}

			ASSERT_IF( used() != UnknownSize && wide(), used() % 2 == 0 );
			ASSERT_IF( owner() && wide() && !nt(), jl_msize(data()) >= len*2 );
			ASSERT_IF( owner() && !wide() && !nt(), jl_msize(data()) >= len );
			ASSERT_IF( owner() && wide() && nt(), jl_msize(data()) >= len*2+2 );
			ASSERT_IF( owner() && !wide() && nt(), jl_msize(data()) >= len+1 );
			ASSERT_IF( nt() && wide(), ((jschar*)data())[len] == 0 );
			ASSERT_IF( nt() && !wide(), ((char*)data())[len] == 0 );
		}
	}
	
	template <class T>
	explicit BufString( T *str, size_t len = UnknownSize, bool nullTerminated = true )
	: BufBase(str, len + ((nullTerminated && len != UnknownSize) ? sizeof(T) : 0)), _charSize(sizeof(T)), _terminatorLength(nullTerminated ? 1 : 0) {
		assertIntegrity();
	}

	BufString( const BufBase& buf, bool withOwnership = true )
		: BufBase( buf, withOwnership ), _charSize( 1 ), _terminatorLength( 0 ) {
	}

	BufString( const BufString& buf, bool withOwnership = true )
	: BufBase(buf, withOwnership), _charSize(buf._charSize), _terminatorLength(buf._terminatorLength) {
	}

	explicit BufString( JSContext *cx, JS::HandleString str )
	: _charSize(sizeof(jschar)), _terminatorLength(0) {

		size_t len;
		const jschar *chars = JS_GetStringCharsAndLength(cx, str, &len);
		get(chars, len, false);
		assertIntegrity();
	}

//

	INLINE size_t
	length() const {

		size_t len;
		if ( used() == UnknownSize ) {

			len = wide() ? jl::strlen(dataAs<WideChar*>()) : jl::strlen(dataAs<char*>());
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

		if ( !str && !*this )
			return true;

		if ( str == !*this )
			return false;

		size_t len = length();
		if ( len == str.length() ) {

			if ( wide() ) {
			
				return str.wide() ? jl::tstrncmp(dataAs<WideChar*>(), str.dataAs<WideChar*>(), len) == 0 : jl::tstrncmpUnsigned(dataAs<WideChar*>(), str.dataAs<char*>(), len) == 0;
			} else {

				return str.wide() ? jl::tstrncmp( dataAs<char*>(), str.dataAs<WideChar*>(), len ) == 0 : jl::tstrncmpUnsigned( dataAs<char*>(), str.dataAs<char*>(), len) == 0;
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
	
		if ( str == nullptr && !*this )
			return true;

		if ( (str != nullptr) == !*this )
			return false;

		if ( nt() ) {

			return dataAs<T*>() == str || (wide() ? jl::tstrcmpUnsigned( dataAs<WideChar*>(), str ) == 0 : jl::tstrcmpUnsigned( dataAs<char*>(), str ) == 0);
		} else {

			size_t len = length();
			return jl::strlen( str ) == len && (wide() ? jl::tstrncmpUnsigned( dataAs<WideChar*>(), str, len ) == 0 : jl::tstrncmpUnsigned( dataAs<char*>(), str, len ) == 0);
		}
	}

	template <class T>
	bool
	operator !=( const T *str ) const {

		return !(operator ==(str));
	}

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
			retChar = wide() ? dataAs<WideChar*>()[index] : dataAs<NarrowChar*>()[index];
		else
		if ( sizeof(T) == sizeof(WideChar) )
			retChar = wide() ? dataAs<WideChar*>()[index] : dataAs<NarrowChar*>()[index];
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

		ASSERT( this->operator bool() );
		ASSERT( charSize() > 0 && charSize() <= 2 );

		const size_t len = length();

		ASSERT_IF( len == UnknownSize, nt() );
		ASSERT_IF( len != UnknownSize && owner() && wide() && !nt(), jl_msize(data()) >= len*2 );
		ASSERT_IF( len != UnknownSize && owner() && !wide() && !nt(), jl_msize(data()) >= len );
		ASSERT_IF( len != UnknownSize && owner() && wide() && nt(), jl_msize(data()) >= len*2+2 );
		ASSERT_IF( len != UnknownSize && owner() && !wide() && nt(), jl_msize(data()) >= len+1 );
		ASSERT_IF( len != UnknownSize && nt() && wide(), ((jschar*)data())[len] == 0 );
		ASSERT_IF( len != UnknownSize && nt() && !wide(), ((char*)data())[len] == 0 );
		ASSERT_IF( wide(), used() % 2 == 0 );

		if ( data() == nullptr )
			return nullptr;

		// something to do ?
		if ( sizeof(Base) != charSize() || (!asConst && !owner()) || (nullTerminated && !nt()) ) {

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
	operator const char *() {

		return toStringZOrNull<const char*>();
	}

	ALWAYS_INLINE
	operator const WideChar *() {

		return toStringZOrNull<const WideChar*>();
	}

	bool
	toString( JSContext *cx, JS::MutableHandleValue rval ) {

		size_t len = length();
		if ( len == 0 )
			rval.set(JL_GetEmptyStringValue(cx));
		else
			rval.setString(JL_NewUCString(cx, toStringZ<jschar*>(), len));
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
jl::BufBase
UTF16LEToUTF8( jl::BufString src ) {

	ASSERT( src.wide() );

	size_t srcSize = src.length() * 2;
	const wchar_t *wsrc = src.toData<const wchar_t *>();

	jl::BufBase dst;
	dst.alloc( srcSize * 3 / 2 );
	size_t dstLen = dst.allocSize();
	UTF16LEToUTF8( dst.dataAs<unsigned char *>(), &dstLen, reinterpret_cast<const unsigned char *>(wsrc), &srcSize );
	dst.setUsed( dstLen );
	dst.maybeCrop();
	return dst;
	//return jl::BufString( dst.dataAs<char *>(), dstLen, false );
}



JL_END_NAMESPACE
