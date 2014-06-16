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


///////////////////////////////////////////////////////////////////////////////
// NativeInterface API

ALWAYS_INLINE bool
ReserveNativeInterface( JSContext *cx, JS::HandleObject obj, JS::HandleId id ) {

	JSID_IS_STRING(id) && !JSID_IS_ZERO(id);

	return JS_DefinePropertyById(cx, obj, id, JL_UNDEFINED(), JSPROP_READONLY | JSPROP_PERMANENT);
}


template <class T>
ALWAYS_INLINE bool
SetNativeInterface( JSContext *cx, JS::HandleObject obj, JS::HandleId id, const T nativeFct ) {

	JSID_IS_STRING(id) && !JSID_IS_ZERO(id);

	if ( nativeFct != NULL ) {

		JL_CHK(JS_DefinePropertyById(cx, obj, id, JL_TRUE(), JSPROP_READONLY | JSPROP_PERMANENT, NULL, (JSStrictPropertyOp)nativeFct)); // hacking the setter of a read-only property seems safe.
	} else {

		JL_CHK( ReserveNativeInterface(cx, obj, id) );
	}
	return true;
	JL_BAD;
}


template <class T>
ALWAYS_INLINE const T
GetNativeInterface( JSContext *cx, JS::HandleObject obj, JS::HandleId id ) {

	ASSERT( JSID_IS_STRING(id) && !JSID_IS_ZERO(id) );
	
	JS::Rooted<JSPropertyDescriptor> desc(cx);
	if ( JS_GetPropertyDescriptorById(cx, obj, id, &desc) ) {

		return desc.object() == obj && desc.setter() != JS_StrictPropertyStub ? (const T)desc.setter() : NULL; // is JS_PropertyStub when eg. Stringify({_NI_BufferGet:function() {} })
	} else {

		return NULL;
	}
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface StreamRead

ALWAYS_INLINE bool
reserveStreamReadInterface( JSContext *cx, JS::HandleObject obj ) {

	return ReserveNativeInterface(cx, obj, JLID(cx, _NI_StreamRead) );
}


ALWAYS_INLINE bool
setStreamReadInterface( JSContext *cx, JS::HandleObject obj, NIStreamRead pFct ) {

	return SetNativeInterface( cx, obj, JLID(cx, _NI_StreamRead), pFct );
}


ALWAYS_INLINE NIStreamRead
streamReadNativeInterface( JSContext *cx, JS::HandleObject obj ) {

	return GetNativeInterface<NIStreamRead>(cx, obj, JLID(cx, _NI_StreamRead));
}


INLINE bool
JSStreamRead( JSContext * RESTRICT cx, JS::HandleObject obj, char * RESTRICT buffer, size_t * RESTRICT amount ) {

	JS::RootedValue rval(cx);
	JL_CHK( call(cx, obj, JLID(cx, read), &rval, *amount) );
	if ( rval.isUndefined() ) { // (TBD) with sockets, undefined mean 'closed', that is not supported by NIStreamRead.

		*amount = 0;
	} else {

		BufString str;
		JL_CHK( getValue(cx, rval, &str) );
		size_t len = str.length();
		JL_ASSERT( len <= *amount, E_DATASIZE, E_MAX, E_NUM(*amount) );
		ASSERT( len <= *amount );
		*amount = len;
		str.copyTo(buffer);
	}
	return true;
	JL_BAD;
}


ALWAYS_INLINE NIStreamRead
streamReadInterface( JSContext *cx, JS::HandleObject obj ) {

	NIStreamRead fct = streamReadNativeInterface(cx, obj);
	if (likely( fct != NULL ))
		return fct;
	bool found;
	if ( JS_HasPropertyById(cx, obj, JLID(cx, read), &found) && found ) // JS_GetPropertyById(cx, obj, JLID(cx, Read), &res) != true || !JL_IsCallable(cx, res)
		return JSStreamRead;
	return NULL;
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface BufferGet

ALWAYS_INLINE bool
reserveBufferGetInterface( JSContext *cx, JS::HandleObject obj ) {

	return ReserveNativeInterface(cx, obj, JLID(cx, _NI_BufferGet) );
}


ALWAYS_INLINE bool
setBufferGetInterface( JSContext *cx, JS::HandleObject obj, NIBufferGet pFct ) {

	return SetNativeInterface( cx, obj, JLID(cx, _NI_BufferGet), pFct );
}


ALWAYS_INLINE NIBufferGet
bufferGetNativeInterface( JSContext *cx, JS::HandleObject obj ) {

	return GetNativeInterface<NIBufferGet>(cx, obj, JLID(cx, _NI_BufferGet));
}


INLINE bool
JSBufferGet( JSContext *cx, JS::HandleObject obj, BufString *str ) {

	JS::RootedValue tmp(cx);
	return call(cx, obj, JLID(cx, get), &tmp) && getValue(cx, tmp, str);
}


ALWAYS_INLINE NIBufferGet
bufferGetInterface( JSContext *cx, JS::HandleObject obj ) {

	NIBufferGet fct = bufferGetNativeInterface(cx, obj);
	if (likely( fct != NULL ))
		return fct;
	bool found;
	if ( JS_HasPropertyById(cx, obj, JLID(cx, get), &found) && found ) // JS_GetPropertyById(cx, obj, JLID(cx, Get), &res) != true || !JL_IsCallable(cx, res)
		return JSBufferGet;
	return NULL;
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface Matrix44Get

ALWAYS_INLINE bool
reserveMatrix44GetInterface( JSContext *cx, JS::HandleObject obj ) {

	return ReserveNativeInterface(cx, obj, JLID(cx, _NI_Matrix44Get) );
}


ALWAYS_INLINE bool
setMatrix44GetInterface( JSContext *cx, JS::HandleObject obj, NIMatrix44Get pFct ) {

	return SetNativeInterface( cx, obj, JLID(cx, _NI_Matrix44Get), pFct );
}


ALWAYS_INLINE NIMatrix44Get
matrix44GetNativeInterface( JSContext *cx, JS::HandleObject obj ) {

	return GetNativeInterface<NIMatrix44Get>(cx, obj, JLID(cx, _NI_Matrix44Get));
}


INLINE bool
JSMatrix44Get( JSContext *, JS::HandleObject , float ** ) {

	ASSERT( false ); // ???

	return false;
}


ALWAYS_INLINE NIMatrix44Get
matrix44GetInterface( JSContext *cx, JS::HandleObject obj ) {

	NIMatrix44Get fct = matrix44GetNativeInterface(cx, obj);
	if (likely( fct != NULL ))
		return fct;
	bool found;
	if ( JS_HasPropertyById(cx, obj, JLID(cx, getMatrix44), &found) && found ) // JS_GetPropertyById(cx, obj, JLID(cx, GetMatrix44), &res) != true || !JL_IsCallable(cx, res)
		return JSMatrix44Get;
	return NULL;
}

JL_END_NAMESPACE
