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
ALWAYS_INLINE bool FASTCALL
getStringChars(JSContext *cx, JSString *str) {

    JS::AutoCheckCannotGC nogc;
	JSLinearString *lstr = js::StringToLinearString(cx, str);
    if (!lstr)
        return false;
    if ( js::LinearStringHasLatin1Chars(lstr) ) {

        const JS::Latin1Char *src = js::GetLatin1LinearStringChars(nogc, lstr);
	} else {
	
		const jschar *src = js::GetTwoByteLinearStringChars(nogc, lstr);
	}
	return true;
}
*/

namespace pv {

template <class T>
INLINE NEVER_INLINE bool FASTCALL
getNumberValue_slow( JSContext *cx, const JS::HandleValue &val, bool valIsDouble, T *num ) {

	double d;

	if ( valIsDouble ) {

		d = val.toDouble();

		// if T is float
		if ( jl::isTypeFloat32(*num) ) {

			if ( abs(d) < ::std::numeric_limits<T>::max() ) { //if ( isInBounds<T>(d) ) {

				*num = static_cast<T>(d);
				return true;
			}
			goto bad_range;
		}

	} else {

		// try getPrimitive instead ?
		JL_CHK( JS::ToNumber(cx, val, &d) );
		ASSERT( mozilla::IsNaN(JS_GetNaNValue(cx).toDouble()) );
		JL_CHKM( !mozilla::IsNaN(d), E_VALUE, E_TYPE, E_TY_NUMBER );
	}

	// optimization when T is double
	if ( jl::isTypeFloat64(*num) ) {

		*num = (T)d;
		return true;
	}

	if ( isInBounds<T>(d) ) {

		*num = static_cast<T>(d);
		// only warn for precision lost if num is integral
		JL_ASSERT_WARN( !::std::numeric_limits<T>::is_exact || jl::IsIntegerValue(d), E_VALUE, E_PRECISION );
		return true;
	}

bad_range:
	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR(SignificandStringValue<T>::min(), SignificandStringValue<T>::max()) );
	JL_BAD;
}


// error if out of range, warning if loss of precision
template <class T>
ALWAYS_INLINE bool FASTCALL
getNumberValue( JSContext *cx, const JS::HandleValue &val, T *num ) {

	if ( val.isInt32() ) {

		int32_t intVal = val.toInt32();
		if ( jl::isInBounds<T>(intVal) ) {

			*num = static_cast<T>(intVal);
			return true;
		}
		goto bad_range;
	}

	bool isDouble = val.isDouble();
	
	// optimization when T and val are double
	if ( jl::isTypeFloat64(*num) && isDouble ) {

		*num = static_cast<T>(val.toDouble());
		return true;
	}

	return getNumberValue_slow(cx, val, isDouble, num);
bad_range:
	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR(SignificandStringValue<T>::min(), SignificandStringValue<T>::max()) );
	JL_BAD;
}

} // namespace pv


namespace pv {

// bool
INLINE NEVER_INLINE bool FASTCALL
getBoolValue_slow( JSContext *cx, JS::HandleValue val, bool *b ) {

	ASSERT( !val.isBoolean() );
	return !( val.isUndefined() || val.isNull() || (val.isInt32() && val.toInt32() == 0) || (val.isString() && val == JL_GetEmptyStringValue(cx)) || (val.isDouble() && val.toDouble() == 0) );
}


// BufString
INLINE NEVER_INLINE bool FASTCALL
getStringValue_slow( JSContext *cx, JS::HandleValue val, jl::BufString* data ) {

	if ( val.isObject() ) {

		JS::RootedObject obj(cx, &val.toObject());
		NIBufferGet fct = jl::bufferGetInterface(cx, obj); // BufferGetNativeInterface
		if ( fct )
			return fct(cx, obj, data);

		if ( JS_IsArrayBufferObject(obj) ) {

			uint32_t length = JS_GetArrayBufferByteLength(obj);
			if ( length ) {
				
				data->get(static_cast<const uint8_t*>(JS_GetArrayBufferData(obj)), length, false);
			} else {
			
				data->setEmpty();
			}
			ASSERT( !data->owner() );
			return true;
		}

		if ( JS_IsTypedArrayObject(obj) ) {

			uint32_t length = JS_GetTypedArrayLength(obj);
			if ( length ) {

				if ( JS_GetArrayBufferViewType(obj) == js::Scalar::Uint16 )
					data->get(reinterpret_cast<const jschar*>(JS_GetUint16ArrayData(obj)), length, false);
				else
					data->get(static_cast<const uint8_t*>(JS_GetArrayBufferViewData(obj)), length, false);
			} else {

				data->setEmpty();
			}
			ASSERT( !data->owner() );
			return true;
		}
	}

	// fallback
	JS::RootedString jsstr(cx, JS::ToString(cx, val));
	JL_CHKM( jsstr != NULL, E_VALUE, E_CONVERT, E_TY_STRING );
	data->get(cx, jsstr);
	return true;
	JL_BAD;
}

}


////



////


bool setValue( JSContext *cx, JS::MutableHandleValue rval, const void * ); // forbidden case


ALWAYS_INLINE bool FASTCALL
setValue( JSContext *cx, JS::MutableHandleValue rval, IN jl::BufString &data, bool toArrayBuffer = true ) {

	return toArrayBuffer ? data.toString(cx, rval) : data.toArrayBuffer(cx, rval);
}


ALWAYS_INLINE bool FASTCALL
setValue( JSContext *cx, JS::MutableHandleValue rval, const bool b ) {

	rval.setBoolean(b);
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const int8_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const uint8_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const int16_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const uint16_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const int32_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const uint32_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const int64_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const uint64_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const long num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const unsigned long num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const float num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const double num) {

	rval.set(JS::NumberValue(num));
	return true;
}

/*
// unfortunately calling setValue("foo") use the following function instead of setValue(pv::StrSpec).
ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const void *ptr) {

	if ( ptr == NULL ) {

		rval.setNull();
	} else {

		if ( ((uint32_t)ptr & 1) == 0 ) { // see PRIVATE_PTR_TO_JSVAL_IMPL()

			vp.address()->setPrivate(ptr);
		} else { // rare since pointers are alligned (except function ptr in DBG mode ?)

			JS::RootedObject obj(cx, jl::newObjectWithoutProto(cx));



//		jl::HandlePrivate pv = new HandlePrivate();
//		JL_CHK( HandleCreate(cx, pv, rval) );

		rval.address()->setPrivate(const_cast<void*>(ptr));
	}
	return true;
}
*/


ALWAYS_INLINE bool FASTCALL
setValue( JSContext *cx, JS::MutableHandleValue rval, IN OwnerlessWCStrSpec &str ) {

	if ( str.str() != NULL ) {
		
		if ( str.len() > 0 ) {

			ASSERT( msize(str.str()) >= str.len()+1 );
			ASSERT( str.str()[str.len()] == 0 );
			JS::RootedString jsstr(cx, JL_NewUCString(cx, str.str(), str.len()));
			JL_CHK( jsstr );
			rval.setString(jsstr);
		} else {

			rval.set(JL_GetEmptyStringValue(cx));
		}
	} else {

		rval.setUndefined();
	}
	
	return true;
	JL_BAD;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const CStrSpec &s) {

	if ( s.str() != NULL ) {

		JS::RootedString str(cx, JS_NewStringCopyN(cx, s.str(), s.len())); // !length is checked
		JL_CHK(str);
		rval.setString(str);
	} else {

		rval.setUndefined();
	}
	return true;
	JL_BAD;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const WCStrSpec &s) {

	if ( s.str() != NULL ) {

		JS::RootedString str(cx, JS_NewUCStringCopyN(cx, s.str(), s.len()));
		JL_CHK(str);
		rval.setString(str);
	} else {

		rval.setUndefined();
	}
	return true;
	JL_BAD;
}


// since implicit constructors are not applied during template deduction, we have to force the char* to pv::StrSpec conversion here.
ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const char* s) {

	return setValue(cx, rval, CStrSpec(s));
}

// since implicit constructors are not applied during template deduction, we have to force the jschar* to pv::StrSpec conversion here.
ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const jschar* s) {

	return setValue(cx, rval, WCStrSpec(s));
}


ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const JS::HandleValue val) {

	rval.set(val);
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const JS::HandleObject obj) {

	rval.setObject(*obj);
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const JS::HandleId id) {

	rval.set(js::IdToValue(id)); // -or- return JS_IdToValue(cx, id, val);
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const JS::HandleString str) {

	rval.setString(str);
	return true;
}

// rooted IN : JS::Rooted& / JS::Rooted* / JS::MutableHandle / JS::Handle


// since neither implicit constructors nor conversion operators are applied during template deduction, we have to force the JS::Rooted to JS::Handle conversion here.
template <typename T>
ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const JS::Rooted<T>& rv) {

	return setValue(cx, rval, JS::Handle<T>(rv));
}

// since neither implicit constructors nor conversion operators are applied during template deduction, we have to force the MutableHandle to JS::Handle conversion here.
template <typename T>
ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const JS::MutableHandle<T> rv) {

	return setValue(cx, rval, JS::Handle<T>(rv));
}


// since neither implicit constructors nor conversion operators are applied during template deduction, we have to force the MutableHandle to JS::Handle conversion here.
template <typename T>
ALWAYS_INLINE bool FASTCALL
setValue( JSContext *cx, JS::MutableHandleValue rval, JS::Rooted<T>*pv ) {

	return setValue( cx, rval, JS::Handle<T>( pv ) );
}
// eg. JS::RootedValue rtVal(cx);
//     jl::setValue(cx, obj, "foo", &rtVal) );



////////


ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT jl::BufBase* str );


ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT jl::BufString* str ) {

	if ( val.isString() ) { // for string literals

		JS::RootedString tmp(cx, val.toString());
		str->get(cx, tmp);
		return true;
	}
	return pv::getStringValue_slow(cx, val, str);
}


ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT bool *b ) {

	if ( val.isBoolean() ) {
		
		*b = val.toBoolean();
		return true;
	}
	return pv::getBoolValue_slow(cx, val, b);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT int8_t *num ) {

	return pv::getNumberValue(cx, val, num);
}


ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT uint8_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT int16_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT uint16_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT int32_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT uint32_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT int64_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT uint64_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT long *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT unsigned long *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT float *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT double *num ) {

	return pv::getNumberValue(cx, val, num);
}

/*
ALWAYS_INLINE bool FASTCALL
getValue(JSContext *cx, JS::HandleValue val, OUT JS::RootedValue* rval) {

	(*rval).set(val);
	return true;
}
*/

ALWAYS_INLINE bool FASTCALL
getValue(JSContext *cx, JS::HandleValue val, OUT JS::MutableHandleValue* rval) {

	(*rval).set(val);
	return true;
}

ALWAYS_INLINE bool FASTCALL
getValue(JSContext *cx, JS::HandleValue val, OUT JS::MutableHandleObject* obj) {

	JL_ASSERT( val.isObject(), E_VALUE, E_TYPE, E_TY_OBJECT );
	(*obj).set(&val.toObject());
	return true;
	JL_BAD;
}


// rooted OUT: JS::Rooted& / JS::Rooted* / JS::MutableHandle

// since neither implicit constructors nor conversion operators are applied during template deduction, we have to force the JS::Rooted* to JS::MutableHandle conversion here.
template <typename T>
ALWAYS_INLINE bool FASTCALL
getValue(JSContext *cx, JS::HandleValue val, OUT JS::Rooted<T>* rval) {

	JS::MutableHandle<T> tmp(rval);
	return getValue(cx, val, &tmp);
}


// since neither implicit constructors nor conversion operators are applied during template deduction, we have to force the MutableHandle& to JS::MutableHandle conversion here.
template <typename T>
ALWAYS_INLINE bool FASTCALL
getValue(JSContext *cx, JS::HandleValue val, OUT JS::MutableHandle<T> rval) {

	return getValue(cx, val, &rval);
}


////


// example:
//   bool requestIncrementalGC = jl::getValueDefault(cx, JL_SARG(1), false);
//   int64_t sliceMillis = jl::getValueDefault<int64_t>(cx, JL_SARG(2), 0);

template <typename T>
ALWAYS_INLINE T FASTCALL
getValueDefault(JSContext *cx, JS::HandleValue val, IN const T defaultValue) {
	
	T value;
	if ( !val.isUndefined() && getValue(cx, val, &value) )
		return value;
	if ( JS_IsExceptionPending(cx) )
		JS_ClearPendingException(cx); // JS_ReportPendingException(cx);
	return defaultValue;
}


////////


template <class T>
ALWAYS_INLINE bool FASTCALL
setElement( JSContext *cx, JS::HandleObject objArg, uint32_t index, const T &v /*, JS::MutableHandleValue tmpRoot*/ ) {

	JS::RootedValue value(cx);
	JL_CHK( setValue(cx, &value, v) );
	return JS_SetElement(cx, objArg, index, value);
	JL_BAD;
}

template <class T>
ALWAYS_INLINE bool FASTCALL
setElement( JSContext *cx, JS::HandleValue objArg, uint32_t index, const T &v ) {

	ASSERT( objArg.isObject() );
	JS::RootedObject obj(cx, &objArg.toObject());
	return setElement(cx, obj, index, v);
}

//

template <class T>
ALWAYS_INLINE bool FASTCALL
pushElement( JSContext *cx, JS::HandleObject objArg, const T &v ) {

	uint32_t length;
	JL_CHK( JS_GetArrayLength(cx, objArg, &length) );
	JL_CHK( jl::setElement( cx, objArg, length, v ) );
	JL_BAD;
}


////


template <class T>
ALWAYS_INLINE bool FASTCALL
getElement( JSContext *cx, const JS::HandleObject &objArg, uint32_t index, T* v ) {

	JS::RootedValue value(cx);
	JL_CHK( JS_ForwardGetElementTo(cx, objArg, index, objArg, &value) ); //JL_CHK( JS_GetElement(cx, objArg, index, &value) );
	return getValue(cx, value, v);
	JL_BAD;
}

template <class T>
ALWAYS_INLINE bool FASTCALL
getElement( JSContext *cx, const JS::HandleObject &objArg, uint32_t index, JS::MutableHandle<T> v ) {

	return getElement(cx, objArg, index, &v);
}

//

template <class T>
ALWAYS_INLINE bool FASTCALL
getElement( JSContext *cx, const JS::HandleValue &objArg, uint32_t index, T v ) {

	ASSERT( objArg.isObject() );
	JS::RootedObject obj(cx, &objArg.toObject());
	return getElement(cx, obj, index, v);
}



////


template <class T>
ALWAYS_INLINE bool FASTCALL
setSlot( JSContext *cx, JS::HandleObject obj, size_t slotIndex, const T &val ) {
	
	ASSERT( JS_IsNative(obj) );
	JS::RootedValue tmp(cx);
	JL_CHK( setValue(cx, &tmp, val) );
	//return JL_SetReservedSlot(obj, slotIndex, v);
	js::SetReservedSlot(obj, slotIndex, tmp); // jsfriendapi
	return true;
	JL_BAD;
}

ALWAYS_INLINE bool FASTCALL
setSlot( JSContext *cx, JS::HandleObject obj, size_t slotIndex, JS::HandleValue val ) {
	
	ASSERT( JS_IsNative(obj) );
	js::SetReservedSlot(obj, slotIndex, val); // jsfriendapi
	return true;
}

//

template <class T>
ALWAYS_INLINE bool FASTCALL
getSlot( JSContext *cx, JS::HandleObject obj, size_t slotIndex, T* rval ) {

	ASSERT( JS_IsNative(obj) );
	//JS::RootedValue v(cx);
	//JL_CHK( JL_GetReservedSlot(obj, slotIndex, v) );
	JS::RootedValue tmp(cx, js::GetReservedSlot(obj, slotIndex)); // jsfriendapi
	return getValue(cx, tmp, rval);
	JL_BAD;
}

ALWAYS_INLINE bool FASTCALL
getSlot( JSContext *cx, JS::HandleObject obj, size_t slotIndex, JS::MutableHandleValue v ) {

	v.set(js::GetReservedSlot(obj, slotIndex)); // jsfriendapi
	return true;
}


////


template <typename T>
ALWAYS_INLINE bool FASTCALL
setException( JSContext *cx, T &val ) {

	JS::RootedValue tmp(cx);
	JL_CHK( jl::setValue(cx, &tmp, val) );
	JS_SetPendingException(cx, tmp);
	return true;
	JL_BAD;
}


////////


template <typename T>
ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleObject objArg, JS::HandleId nameId, const T &v ) {

	JS::RootedValue value(cx);
	JL_CHK( setValue(cx, &value, v) );
	return JS_SetPropertyById(cx, objArg, nameId, value);
	JL_BAD;
}

template <typename T>
ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleObject objArg, const CStrSpec name, const T &v ) {

	JS::RootedValue value(cx);
	JL_CHK( setValue(cx, &value, v) );
	return JS_SetProperty(cx, objArg, name.str(), value);
	JL_BAD;
}

template <typename T>
ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleObject objArg, const WCStrSpec name, const T &v ) {

	JS::RootedValue value(cx);
	JL_CHK( setValue(cx, &value, v) );
	return JS_SetUCProperty(cx, objArg, name.str(), name.len(), value);
	JL_BAD;
}


ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleObject objArg, JS::HandleId nameId, JS::HandleValue value ) {

	return JS_SetPropertyById(cx, objArg, nameId, value);
}

ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleObject objArg, CStrSpec name, JS::HandleValue value ) {

	return JS_SetProperty(cx, objArg, name.str(), value);
}

ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleObject objArg, WCStrSpec name, JS::HandleValue value ) {

	return JS_SetUCProperty(cx, objArg, name.str(), name.len(), value);
}



// setProperty(cx, jsval, *, *)
template <typename N, typename T>
ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleValue objArg, N name, const T& v ) {

	ASSERT( objArg.isObject() );
	JS::RootedObject obj(cx, &objArg.toObject());
	return setProperty(cx, obj, name, v);
}


/*

// Define

template <class T>
ALWAYS_INLINE bool FASTCALL
JL_DefineProperty( JSContext *cx, IN JS::HandleObject obj, const char *name, const T &cval, bool visible = true, bool modifiable = true ) {

	JS::RootedValue tmp(cx);
	return jl::setValue(cx, tmp, cval) && JS_DefineProperty(cx, obj, name, tmp, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}

ALWAYS_INLINE bool FASTCALL
JL_DefineProperty( JSContext *cx, IN JS::HandleObject obj, const char *name, IN JS::HandleValue val, bool visible = true, bool modifiable = true ) {

	return JS_DefineProperty(cx, obj, name, val, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}


template <class T>
ALWAYS_INLINE bool FASTCALL
JL_DefineProperty( JSContext *cx, IN JS::HandleObject obj, jsid id, const T &cval, bool visible = true, bool modifiable = true ) {

	JS::RootedValue tmp(cx);
	return jl::setValue(cx, tmp, cval) && JS_DefinePropertyById(cx, obj, id, tmp, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}

ALWAYS_INLINE bool FASTCALL
JL_DefineProperty( JSContext *cx, IN JS::HandleObject obj, jsid id, IN JS::HandleValue val, bool visible = true, bool modifiable = true ) {

	return JS_DefinePropertyById(cx, obj, id, val, NULL, NULL, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}

*/


////


template <typename T>
ALWAYS_INLINE bool FASTCALL
getProperty( JSContext *cx, JS::HandleObject objArg, CStrSpec name, T* v ) {

	JS::RootedValue value(cx);
	JL_CHK( JS_GetProperty(cx, objArg, name.str(), &value) );
	JL_CHK( getValue(cx, value, v) );
	return true;
	JL_BAD;
}

template <typename T>
ALWAYS_INLINE bool FASTCALL
getProperty( JSContext *cx, JS::HandleObject objArg, WCStrSpec name, T* v ) {

	JS::RootedValue value(cx);
	JL_CHK( JS_GetUCProperty(cx, objArg, name.str(), name.len(), &value) );
	JL_CHK( getValue(cx, value, v) );
	return true;
	JL_BAD;
}

template <typename T>
ALWAYS_INLINE bool FASTCALL
getProperty( JSContext *cx, JS::HandleObject objArg, JS::HandleId nameId, T* v ) {

	JS::RootedValue value(cx);
	JL_CHK( JS_GetPropertyById(cx, objArg, nameId, &value) );
	JL_CHK( getValue(cx, value, v) );
	return true;
	JL_BAD;
}

template <typename T>
ALWAYS_INLINE bool FASTCALL
getProperty( JSContext *cx, JS::HandleObject objArg, JS::RootedId & nameId, T* v ) {

	JS::HandleId name(nameId);
	return getProperty(cx, objArg, name, v);
}

template <typename N, typename T>
ALWAYS_INLINE bool FASTCALL
getProperty( JSContext *cx, JS::HandleObject objArg, const N &name, JS::MutableHandle<T> v ) {

	return getProperty(cx, objArg, name, &v);
}



//


// setProperty(cx, jsval, *, *)
template <typename NAME, typename T>
ALWAYS_INLINE bool FASTCALL
getProperty( JSContext *cx, JS::HandleValue objArg, NAME name, T v ) {

	ASSERT( objArg.isObject() );
	JS::RootedObject obj(cx, &objArg.toObject());
	return getProperty(cx, obj, name, v);
}


////////

// vector

template <class T>
ALWAYS_INLINE bool FASTCALL
setVector( JSContext *cx, JS::MutableHandleValue rval, const T *vector, uint32_t length, bool useValArray = false ) {

	ASSERT( vector );

	JS::RootedValue value(cx);
	JS::RootedObject arrayObj(cx);

	if (likely( useValArray )) {

		JL_ASSERT_IS_OBJECT(rval, "vector");
		arrayObj = &rval.toObject();
		JL_CHK( JS_SetArrayLength(cx, arrayObj, length) );
	} else {

		arrayObj = JS_NewArrayObject(cx, length);
		JL_ASSERT_ALLOC( arrayObj );
		rval.setObject(*arrayObj);
	}

	for ( uint32_t i = 0; i < length; ++i ) {

		JL_CHK( setValue(cx, &value, vector[i]) );
		JL_CHK( JS_SetElement(cx, arrayObj, i, value) );
	}

	return true;
	JL_BAD;
}



ALWAYS_INLINE js::Scalar::Type JLNativeTypeToTypedArrayType(const int8_t &) { return js::Scalar::Int8; }
ALWAYS_INLINE js::Scalar::Type JLNativeTypeToTypedArrayType(const uint8_t &) { return js::Scalar::Uint8; }
ALWAYS_INLINE js::Scalar::Type JLNativeTypeToTypedArrayType(const int16_t &) { return js::Scalar::Int16; }
ALWAYS_INLINE js::Scalar::Type JLNativeTypeToTypedArrayType(const uint16_t &) { return js::Scalar::Uint16; }
ALWAYS_INLINE js::Scalar::Type JLNativeTypeToTypedArrayType(const int32_t &) { return js::Scalar::Int32; }
ALWAYS_INLINE js::Scalar::Type JLNativeTypeToTypedArrayType(const uint32_t &) { return js::Scalar::Uint32; }
ALWAYS_INLINE js::Scalar::Type JLNativeTypeToTypedArrayType(const float32_t &) { return js::Scalar::Float32; }
ALWAYS_INLINE js::Scalar::Type JLNativeTypeToTypedArrayType(const float64_t &) { return js::Scalar::Float64; }

ALWAYS_INLINE const char * JLNativeTypeToString( const int8_t & ) { return "Int8Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const uint8_t & ) { return "Uint8Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const int16_t & ) { return "Int16Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const uint16_t & ) { return "Uint16Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const int32_t & ) { return "Int32Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const uint32_t & ) { return "Uint32Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const float32_t & ) { return "Float32Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const float64_t & ) { return "Float64Array"; }


template <class T>
INLINE bool FASTCALL
getTypedArray( JSContext *cx, IN JS::HandleObject obj, OUT T * vector, IN uint32_t maxLength, OUT uint32_t &actualLength ) {

	ASSERT( JS_IsTypedArrayObject(obj) );
	JL_ASSERT( JS_GetArrayBufferViewType(obj) == JLNativeTypeToTypedArrayType(*vector), E_TY_TYPEDARRAY, E_TYPE, E_NAME(JLNativeTypeToString(*vector)) );
	void *data;
	data = JS_GetArrayBufferViewData(obj);
	actualLength = JS_GetTypedArrayLength(obj);
	maxLength = jl::min( actualLength, maxLength );
	for ( uint32_t i = 0; i < maxLength; ++i )
		vector[i] = static_cast<T*>(data)[i];
	return true;
	JL_BAD;
}


template <class T>
INLINE bool FASTCALL
getArrayBuffer( JSContext *cx, JS::HandleObject obj, OUT T * vector, IN uint32_t maxLength, OUT uint32_t &actualLength ) {

	JL_IGNORE(cx);
	ASSERT( JS_IsArrayBufferObject(obj) );
	uint8_t *buffer = JS_GetArrayBufferData(obj);
	ASSERT( buffer != NULL );
	actualLength = JS_GetArrayBufferByteLength(obj);
	maxLength = jl::min( actualLength, maxLength );
	jl::memcpy(vector, buffer, maxLength);
	return true;
	JL_BAD;
}


// supports Array-like objects and typedArray
template <class T>
ALWAYS_INLINE bool FASTCALL
getVector( JSContext *cx, IN JS::HandleValue val, OUT T * vector, IN uint32_t maxLength, OUT uint32_t *actualLength ) {

	JL_ASSERT_IS_OBJECT(val, "vector");

	{

	JS::RootedValue value(cx);
	JS::RootedObject arrayObj(cx, &val.toObject());

	if (unlikely( JS_IsTypedArrayObject(arrayObj) ))
		return getTypedArray(cx, arrayObj, vector, maxLength, *actualLength);

	if (unlikely( JS_IsArrayBufferObject(arrayObj) )) {

		if ( sizeof(*vector) == 1 )
			return getArrayBuffer(cx, arrayObj, (uint8_t*)vector, maxLength, *actualLength);
		else
			JL_ERR( E_TY_ARRAYBUFFER, E_UNEXP );
	}

	JL_CHK( JS_GetArrayLength(cx, arrayObj, actualLength) );
	maxLength = jl::min( *actualLength, maxLength );
	for ( unsigned i = 0; i < maxLength; ++i ) { // while ( maxLength-- ) { // avoid reverse walk (L1 cache issue)
		
		JL_CHK( JS_ForwardGetElementTo(cx, arrayObj, i, arrayObj, &value) ); //JL_CHK( JS_GetElement(cx, objArg, index, &value) );
		JL_CHK( getValue(cx, value, &vector[i]) );
	}

	}

	return true;
	JL_BAD;
}

template <class T>
ALWAYS_INLINE bool FASTCALL
getVector(JSContext *cx, IN JS::HandleValue val, OUT T * vector, IN uint32_t length) {

	uint32_t actualLength;
	return getVector(cx, val, vector, length, &actualLength);
}


////


// fun as function
ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleObject thisObj, JS::HandleValue fun, const JS::HandleValueArray& args, JS::MutableHandleValue rval) {

	return JS_CallFunctionValue(cx, thisObj, fun, args, rval);
}

ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleObject thisObj, JS::HandleFunction fun, const JS::HandleValueArray &args, JS::MutableHandleValue rval) {

    return JS_CallFunction(cx, thisObj, fun, args, rval);
}

// fun as name (obj.fun)
ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleObject thisObj, JS::HandleId funId, const JS::HandleValueArray& args, JS::MutableHandleValue rval) {

    JS::RootedValue funVal(cx);
	JL_CHK( JS_GetPropertyById(cx, thisObj, funId, &funVal) );
	return call(cx, thisObj, funVal, args, rval);
	JL_BAD;
}


ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleObject thisObj, const CStrSpec name, const JS::HandleValueArray& args, JS::MutableHandleValue rval) {

    return JS_CallFunctionName(cx, thisObj, name.str(), args, rval);
}


ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleObject thisObj, const WCStrSpec name, const JS::HandleValueArray& args, JS::MutableHandleValue rval) {

	JS::RootedValue fval(cx);
	JL_CHK( JS_GetUCProperty(cx, thisObj, name.str(), name.len(), &fval) );
	return call(cx, thisObj, fval, args, rval);
	JL_BAD;
}


// handle case when thisArg is a value and not an object
template <class FCT>
ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleValue thisArg, const FCT &fct, const JS::HandleValueArray& args, JS::MutableHandleValue rval) {

	JS::RootedObject thisObj(cx, &thisArg.toObject());
	return call(cx, thisObj, fct, args, rval);
}

ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleValue thisArg, const JS::RootedValue &fval, const JS::HandleValueArray& args, JS::MutableHandleValue rval) {

	return JS::Call(cx, thisArg, fval, args, rval);
}


///


template <class THIS, class FCT>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval ) {

	return call(cx, thisArg, fct, JS::HandleValueArray::empty(), rval);
}

template <class THIS, class FCT, class T1>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1 ) {

	JS::AutoValueArray<1> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

template <class THIS, class FCT, class T1, class T2>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1, const T2 &v2 ) {

	JS::AutoValueArray<2> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

template <class THIS, class FCT, class T1, class T2, class T3>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1, const T2 &v2, const T3 &v3 ) {

	JS::AutoValueArray<3> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

template <class THIS, class FCT, class T1, class T2, class T3, class T4>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4 ) {

	JS::AutoValueArray<4> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	JL_CHK( setValue(cx, ava[3], v4) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

template <class THIS, class FCT, class T1, class T2, class T3, class T4, class T5>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5 ) {

	JS::AutoValueArray<5> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	JL_CHK( setValue(cx, ava[3], v4) );
	JL_CHK( setValue(cx, ava[4], v5) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

template <class THIS, class FCT, class T1, class T2, class T3, class T4, class T5, class T6>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6 ) {

	JS::AutoValueArray<6> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	JL_CHK( setValue(cx, ava[3], v4) );
	JL_CHK( setValue(cx, ava[4], v5) );
	JL_CHK( setValue(cx, ava[5], v6) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

template <class THIS, class FCT, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7 ) {

	JS::AutoValueArray<7> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	JL_CHK( setValue(cx, ava[3], v4) );
	JL_CHK( setValue(cx, ava[4], v5) );
	JL_CHK( setValue(cx, ava[5], v6) );
	JL_CHK( setValue(cx, ava[6], v7) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

//...

template <class THIS, class FCT>
ALWAYS_INLINE bool FASTCALL
callNoRval( JSContext *cx, const THIS &thisArg, const FCT &fct ) {

	JS::RootedValue rval(cx);
	//JS::Value val; JS::MutableHandleValue::fromMarkedLocation(&val);
	return call(cx, thisArg, fct, &rval);
}

template <class THIS, class FCT, class T1>
ALWAYS_INLINE bool FASTCALL
callNoRval( JSContext *cx, const THIS &thisArg, const FCT &fct, const T1 &v1 ) {

	JS::RootedValue rval(cx);
	return call(cx, thisArg, fct, &rval, v1);
}

template <class THIS, class FCT, class T1, class T2>
ALWAYS_INLINE bool FASTCALL
callNoRval( JSContext *cx, const THIS &thisArg, const FCT &fct, const T1 &v1, const T2 &v2 ) {

	JS::RootedValue rval(cx);
	return call(cx, thisArg, fct, &rval, v1, v2);
}

template <class THIS, class FCT, class T1, class T2, class T3>
ALWAYS_INLINE bool FASTCALL
callNoRval( JSContext *cx, const THIS &thisArg, const FCT &fct, const T1 &v1, const T2 &v2, const T3 &v3 ) {

	JS::RootedValue rval(cx);
	return call(cx, thisArg, fct, &rval, v1, v2, v3);
}

template <class THIS, class FCT, class T1, class T2, class T3, class T4>
ALWAYS_INLINE bool FASTCALL
callNoRval( JSContext *cx, const THIS &thisArg, const FCT &fct, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4 ) {

	JS::RootedValue rval(cx);
	return call(cx, thisArg, fct, &rval, v1, v2, v3, v4);
}

template <class THIS, class FCT, class T1, class T2, class T3, class T4, class T5>
ALWAYS_INLINE bool FASTCALL
callNoRval( JSContext *cx, const THIS &thisArg, const FCT &fct, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5 ) {

	JS::RootedValue rval(cx);
	return call(cx, thisArg, fct, &rval, v1, v2, v3, v4, v5);
}

template <class THIS, class FCT, class T1, class T2, class T3, class T4, class T5, class T6>
ALWAYS_INLINE bool FASTCALL
callNoRval( JSContext *cx, const THIS &thisArg, const FCT &fct, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6 ) {

	JS::RootedValue rval(cx);
	return call(cx, thisArg, fct, &rval, v1, v2, v3, v4, v5, v6);
}

template <class THIS, class FCT, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
ALWAYS_INLINE bool FASTCALL
callNoRval( JSContext *cx, const THIS &thisArg, const FCT &fct, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7 ) {

	JS::RootedValue rval(cx);
	return call(cx, thisArg, fct, &rval, v1, v2, v3, v4, v5, v6, v7);
}

////

ALWAYS_INLINE JSObject* FASTCALL
construct( JSContext *cx, JS::HandleObject proto ) {

	JS::RootedObject ctor(cx, JL_GetConstructor(cx, proto));
	JL_CHK( ctor );
	return JS_New(cx, ctor, JS::HandleValueArray::empty());
	JL_BADVAL(nullptr);
}

template <class T1>
ALWAYS_INLINE JSObject* FASTCALL
construct( JSContext *cx, JS::HandleObject proto, const T1 &v1 ) {

	JS::AutoValueArray<1> ava(cx);
	JS::RootedObject ctor(cx, JL_GetConstructor(cx, proto));
	JL_CHK( ctor );
	JL_CHK( setValue(cx, ava[0], v1) );
	return JS_New(cx, ctor, ava);
	JL_BADVAL(nullptr);
}

template <class T1, class T2>
ALWAYS_INLINE JSObject* FASTCALL
construct( JSContext *cx, JS::HandleObject proto, const T1 &v1, const T2 &v2 ) {

	JS::AutoValueArray<1> ava(cx);
	JS::RootedObject ctor(cx, JL_GetConstructor(cx, proto));
	JL_CHK( ctor );
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	return JS_New(cx, ctor, ava.length(), ava.begin());
	JL_BADVAL(nullptr);
}

//...


////


ALWAYS_INLINE JSObject* FASTCALL
newArray( JSContext *cx ) {

	return JS_NewArrayObject(cx, 0);
}

template <typename T1>
ALWAYS_INLINE JSObject* FASTCALL
newArray( JSContext *cx, const T1 v1 ) {

	JS::AutoValueArray<1> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	return JS_NewArrayObject(cx, ava);
	JL_BADVAL(nullptr);
}

template <typename T1, typename T2>
ALWAYS_INLINE JSObject* FASTCALL
newArray( JSContext *cx, const T1 v1, const T2 v2 ) {

	JS::AutoValueArray<2> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	return JS_NewArrayObject(cx, ava);
	JL_BADVAL(nullptr);
}

template <typename T1, typename T2, typename T3>
ALWAYS_INLINE JSObject* FASTCALL
newArray( JSContext *cx, const T1 v1, const T2 v2, const T3 v3 ) {

	JS::AutoValueArray<3> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	return JS_NewArrayObject(cx, ava);
	JL_BADVAL(nullptr);
}

template <typename T1, typename T2, typename T3, typename T4>
ALWAYS_INLINE JSObject* FASTCALL
newArray( JSContext *cx, const T1 v1, const T2 v2, const T3 v3, const T4 v4 ) {

	JS::AutoValueArray<4> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	JL_CHK( setValue(cx, ava[3], v4) );
	return JS_NewArrayObject(cx, ava);
	JL_BADVAL(nullptr);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
ALWAYS_INLINE JSObject* FASTCALL
newArray( JSContext *cx, const T1 v1, const T2 v2, const T3 v3, const T4 v4, const T5 v5 ) {

	JS::AutoValueArray<4> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	JL_CHK( setValue(cx, ava[3], v4) );
	JL_CHK( setValue(cx, ava[4], v5) );
	return JS_NewArrayObject(cx, ava);
	JL_BADVAL(nullptr);
}


INLINE bool FASTCALL
getPrimitive( JSContext * RESTRICT cx, IN JS::HandleValue val, OUT JS::MutableHandleValue rval ) {

	if ( val.isPrimitive() ) {

		rval.set(val);
		return true;
	}
	JS::RootedObject obj(cx, &val.toObject());
	JL_CHK( jl::call(cx, obj, JLID(cx, valueOf), rval) );
	if ( !rval.isPrimitive() )
		JL_CHK( jl::call(cx, obj, JLID(cx, toString), rval) );

	return true;
	JL_BAD;
}


INLINE NEVER_INLINE bool FASTCALL
getMatrix44( JSContext * cx, IN JS::HandleValue val, OUT float32_t ** m ) {

	static float32_t Matrix44IdentityValue[16] = {
		 1.0f, 0.0f, 0.0f, 0.0f,
		 0.0f, 1.0f, 0.0f, 0.0f,
		 0.0f, 0.0f, 1.0f, 0.0f,
		 0.0f, 0.0f, 0.0f, 1.0f
	};

	if ( val.isNull() ) {

		jl::memcpy(*m, &Matrix44IdentityValue, sizeof(Matrix44IdentityValue));
		return true;
	}

	JL_ASSERT_IS_OBJECT(val, "matrix44");

	{

	JS::RootedObject matrixObj(cx, &val.toObject());

	NIMatrix44Get Matrix44Get;
	Matrix44Get = jl::matrix44GetInterface(cx, matrixObj);
	if ( Matrix44Get )
		return Matrix44Get(cx, matrixObj, m);

	if ( JS_IsFloat32Array(matrixObj) ) {

		if ( JS_GetTypedArrayLength(matrixObj) == 16 ) {

			jl::memcpy(*m, JS_GetFloat32ArrayData(matrixObj), sizeof(float32_t) * 16);
			return true;
		}
	}

	if ( jl::isArrayLike(cx, matrixObj) ) {

		uint32_t length;
		JS::RootedValue element(cx);

		JL_CHK( JL_GetElement(cx, matrixObj, 0, &element) );
		if ( jl::isArrayLike(cx, element) ) { // support for [ [1,1,1,1], [2,2,2,2], [3,3,3,3], [4,4,4,4] ] matrix

			JL_CHK( jl::getVector(cx, element, (*m)+0, 4, &length ) );
			JL_ASSERT( length == 4, E_VALUE, E_STR("matrix44[0]"), E_TYPE, E_TY_NVECTOR(4) );

			JL_CHK( JL_GetElement(cx, matrixObj, 1, &element) );
			JL_CHK( jl::getVector(cx, element, (*m)+4, 4, &length ) );
			JL_ASSERT_IS_ARRAY( element, "matrix44[1]" );
			JL_ASSERT( length == 4, E_VALUE, E_STR("matrix44[1]"), E_TYPE, E_TY_NVECTOR(4) );

			JL_CHK( JL_GetElement(cx, matrixObj, 2, &element) );
			JL_CHK( jl::getVector(cx, element, (*m)+8, 4, &length ) );
			JL_ASSERT_IS_ARRAY( element, "matrix44[2]" );
			JL_ASSERT( length == 4, E_VALUE, E_STR("matrix44[2]"), E_TYPE, E_TY_NVECTOR(4) );

			JL_CHK( JL_GetElement(cx, matrixObj, 3, &element) );
			JL_CHK( jl::getVector(cx, element, (*m)+12, 4, &length ) );
			JL_ASSERT_IS_ARRAY( element, "matrix44[3]" );
			JL_ASSERT( length == 4, E_VALUE, E_STR("matrix44[3]"), E_TYPE, E_TY_NVECTOR(4) );
			return true;
		}

		JL_CHK( jl::getVector(cx, val, *m, 16, &length ) );  // support for [ 1,1,1,1, 2,2,2,2, 3,3,3,3, 4,4,4,4 ] matrix
		JL_ASSERT( length == 16, E_VALUE, E_STR("matrix44"), E_TYPE, E_TY_NVECTOR(16) );
		return true;
	}

	}

	JL_ERR( E_VALUE, E_STR("matrix44"), E_INVALID );
	JL_BAD;
}


JL_END_NAMESPACE
