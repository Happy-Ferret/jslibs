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

#include "stdafx.h"
#include <cstring>

#include "../common/errors.h"
#include "../common/jsHelper.h"
#include "blobapi.h"


inline JSBool BlobLength( JSContext *cx, JSObject *blobObject, size_t *length ) {

	J_S_ASSERT_CLASS(blobObject, BlobJSClass( cx ));
	jsval lengthVal;
	J_CHK( JS_GetReservedSlot(cx, blobObject, SLOT_BLOB_LENGTH, &lengthVal) );
	J_S_ASSERT_INT( lengthVal );
	*length = JSVAL_TO_INT( lengthVal );
	return JS_TRUE;
}


inline JSBool BlobBuffer( JSContext *cx, JSObject *blobObject, const char **buffer ) {

	J_S_ASSERT_CLASS(blobObject, BlobJSClass( cx ));
	*buffer = (char*)JS_GetPrivate(cx, blobObject);
	return JS_TRUE;
}


JSBool NativeInterfaceBufferGet( JSContext *cx, JSObject *obj, const char **buf, size_t *size ) {

	J_CHK( BlobLength(cx, obj, size) );
	J_CHK( BlobBuffer(cx, obj, buf) );
	return JS_TRUE;
}


/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Blob )

/*
inline JSBool JsvalToBlob( JSContext *cx, jsval val, JSObject **obj ) {

	size_t srcLen;
	void *src, *dst = NULL;

	if ( JsvalIsBlob(cx, val) ) {

		BlobGetBufferAndLength(cx, JSVAL_TO_OBJECT( val ), &src, &srcLen);
		if ( srcLen > 0 ) {

			dst = JS_malloc(cx, srcLen +1);
			J_S_ASSERT_ALLOC( dst );
			((char*)dst)[srcLen] = '\0';
			memcpy(dst, src, srcLen);
		}
	} else {

		JSString *jsstr = JS_ValueToString(cx, val);
		J_S_ASSERT( jsstr != NULL, "Unable to convert to string." );
		srcLen = J_STRING_LENGTH(jsstr);
		if ( srcLen > 0 ) {

			dst = JS_malloc(cx, srcLen +1);
			J_S_ASSERT_ALLOC( dst );
			((char*)dst)[srcLen] = '\0';

			// (TBD) try to know if the string is deflated befor using JS_GetStringChars ??
			jschar *chars = JS_GetStringChars(jsstr);
			for ( size_t i = 0; i < srcLen; i++ )
				((char*)dst)[i] = (uint8)chars[i];
		}
	}

	*obj = NewBlob(cx, dst, srcLen);
	return JS_TRUE;
}
*/


JSBool BlobToJSString( JSContext *cx, JSObject *obj, JSString **jsstr ) {

	void *pv = JS_GetPrivate(cx, obj);
	size_t length;
	J_CHK( BlobLength(cx, obj, &length) );
	if ( length == 0 ) {

		*jsstr = JSVAL_TO_STRING( JS_GetEmptyStringValue(cx) );
	} else {

		jschar *ucStr = (jschar*)JS_malloc(cx, (length + 1) * sizeof(jschar));
		ucStr[length] = 0;
		for ( size_t i = 0; i < length; i++ )
			ucStr[i] = ((unsigned char*)pv)[i]; // see js_InflateString in jsstr.c
		*jsstr = JS_NewUCString(cx, ucStr, length);
	}
	return JS_TRUE;
}



DEFINE_FINALIZE() {

	void *pv = JS_GetPrivate(cx, obj);
	if (pv != NULL)
		JS_free(cx, pv);
}


/**doc
 * $INAME( [data] )
  Creates an object that can contain binary data.
  $H note
  When called in a non-constructor context, Object behaves identically.
**/
DEFINE_CONSTRUCTOR() {

	if ( JS_IsConstructing(cx) != JS_TRUE ) { // supports this form (w/o new operator) : result.param1 = Blob('Hello World');

		obj = JS_NewObject(cx, _class, NULL, NULL);
		J_S_ASSERT( obj != NULL, "Blob construction failed." );
		*rval = OBJECT_TO_JSVAL(obj);
	}

	if ( J_ARGC >= 1 ) {

		size_t length;
		const char *sBuffer;
		J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), &sBuffer, &length) ); // warning: GC on the returned buffer !

		void *dBuffer = JS_malloc(cx, length +1);
		J_S_ASSERT_ALLOC( dBuffer );
		((char*)dBuffer)[length] = '\0';
		memcpy(dBuffer, sBuffer, length);
		J_CHK( JS_SetPrivate(cx, obj, dBuffer) );
		J_CHK( JS_SetReservedSlot(cx, obj, SLOT_BLOB_LENGTH, INT_TO_JSVAL(length) ) );
	} else {

		J_CHK( JS_SetPrivate(cx, obj, NULL) );
		J_CHK( JS_SetReservedSlot(cx, obj, SLOT_BLOB_LENGTH, INT_TO_JSVAL(0) ) );
	}

	J_CHK( ReserveBufferGetInterface(cx, obj) );
	J_CHK( SetBufferGetInterface(cx, obj, NativeInterfaceBufferGet) );
	return JS_TRUE;
}


/**doc
=== Methods ===
**/


/**doc
 * $TYPE Blob $INAME( data [,data1 [,...]] )
  Combines the text of two or more strings and returns a new string.
  $H details
   http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Reference:Global_Objects:String:concat
**/
DEFINE_FUNCTION_FAST( concat ) {

	J_S_ASSERT_ARG_MIN( 1 );

	size_t thisLength;
	const char *thisBuffer;
	J_CHK( BlobBuffer(cx, J_FOBJ, &thisBuffer) );
	J_CHK( BlobLength(cx, J_FOBJ, &thisLength) );

	size_t dstLen = thisLength;

	unsigned int arg;
	for ( arg = 1; arg <= J_ARGC; arg++ ) {
	
		if ( JsvalIsBlob(cx, J_FARG(arg)) ) {

			size_t tmp;
			J_CHK( BlobLength(cx, JSVAL_TO_OBJECT( J_FARG(arg) ), &tmp) );
			dstLen += tmp;
		
		} else {

			JSString *jsstr = JS_ValueToString(cx, J_FARG(arg));
			J_FARG(arg) = STRING_TO_JSVAL(jsstr);
			dstLen += J_STRING_LENGTH(jsstr);
		}
	}

	char *dst = (char*)JS_malloc(cx, dstLen +1);
	J_S_ASSERT_ALLOC( dst );
	dst[dstLen] = '\0';

	char *tmp = dst;

	if ( thisLength > 0 ) {

		memcpy(tmp, thisBuffer, thisLength);
		tmp += thisLength;
	}

	for ( arg = 1; arg <= J_ARGC; arg++ ) {
		
		const char *buffer;
		size_t length;
		J_CHK( JsvalToStringAndLength(cx, &J_FARG(arg), &buffer, &length) );

		memcpy(tmp, buffer, length);
		tmp += length;
	}

	J_CHK( J_NewBlob(cx, dst, dstLen, J_FRVAL) );
	return JS_TRUE;
}


/**doc
 * $TYPE Blob $INAME( start [, length ] )
  Returns the bytes in a string beginning at the specified location through the specified number of characters.
  $H arguments
   $ARG integer start: location at which to begin extracting characters (an integer between 0 and one less than the length of the string).
   $ARG integer length: the number of characters to extract.
  $H details
   fc. [http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Reference:Global_Objects:String:substr JavaScript substr]
**/
DEFINE_FUNCTION_FAST( substr ) {

	J_S_ASSERT_ARG_MIN(1);

	const char *bstrBuffer;
	J_CHK( BlobBuffer(cx, J_FOBJ, &bstrBuffer) );

	size_t dataLength;
	J_CHK( BlobLength(cx, J_FOBJ, &dataLength) );

	int start;
	J_JSVAL_TO_INT32( J_FARG(1), start );

	if ( start >= (int)dataLength || start < -(int)dataLength ) {

		*J_FRVAL = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	if ( start < 0 )
		start = dataLength + start;

	if ( start < 0 || start >= (int)dataLength )
		start = 0;

	// now 0 <= start < dataLength

	int length;
	if ( J_FARG_ISDEF(2) ) {

		J_JSVAL_TO_INT32( J_FARG(2), length );
		if ( length <= 0 ) {

			*J_FRVAL = JS_GetEmptyStringValue(cx);
			return JS_TRUE;
		}

		if ( start + length > (int)dataLength )
			length = dataLength - start;

	} else
		length = dataLength - start;

	// now 0 <= length < dataLength - start

	void *buffer = JS_malloc(cx, length +1);
	J_S_ASSERT_ALLOC( buffer );
	((char*)buffer)[length] = '\0';

	memcpy(buffer, ((int8_t*)bstrBuffer) + start, length);
	J_CHK( J_NewBlob(cx, buffer, length, J_FRVAL) );

	return JS_TRUE;
}


/**doc
 * $INT $INAME( searchValue [, fromIndex] )
  Returns the index within the calling Blob object of the first occurrence of the specified value, starting the search at fromIndex, or -1 if the value is not found.
  $H arguments
   $ARG string searchValue: A string representing the value to search for.
   $ARG integer fromIndex: The location within the calling string to start the search from. It can be any integer between 0 and the length of the string. The default value is 0.
  $H details
   fc. [http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Reference:Global_Objects:String:indexOf]
**/
DEFINE_FUNCTION_FAST( indexOf ) {

	J_S_ASSERT_ARG_MIN(1);

	const char *sBuffer;
	size_t sLength;
	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &sBuffer, &sLength) ); // warning: GC on the returned buffer !

	if ( sLength == 0 ) {

		*J_FRVAL = INT_TO_JSVAL(0);
		return JS_TRUE;
	}

	size_t length;
	J_CHK( BlobLength(cx, J_FOBJ, &length) );

	long start;
	if ( J_FARG_ISDEF(2) ) {

		J_S_ASSERT_INT( J_FARG(2) );
		start = JSVAL_TO_INT( J_FARG(2) );

		if ( start < 0 )
			start = 0;
		else if ( start + sLength > length ) {

			*J_FRVAL = INT_TO_JSVAL(-1);
			return JS_TRUE;
		}

	} else {

		start = 0;
	}

	const char *buffer;
	J_CHK( BlobBuffer(cx, J_FOBJ, &buffer) );

	for ( size_t i = start; i < length; i++ ) {

		size_t j;
		for ( j = 0; j < sLength && buffer[i+j] == sBuffer[j]; j++ );
		if ( j == sLength ) {

			J_CHK( JS_NewNumberValue(cx, i, J_FRVAL) );
			return JS_TRUE;
		}
	}

	*J_FRVAL = INT_TO_JSVAL(-1);
	return JS_TRUE;
}


/**doc
 * $INT $INAME( searchValue [, fromIndex] )
  Returns the index within the calling Blob object of the last occurrence of the specified value, or -1 if not found. The calling string is searched backward, starting at fromIndex.
  $H arguments
   $ARG string searchValue: A string representing the value to search for.
   $ARG integer fromIndex: The location within the calling string to start the search from, indexed from left to right. It can be any integer between 0 and the length of the string. The default value is the length of the string.
  $H details
   fc. [http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Reference:Objects:String:lastIndexOf]
**/
DEFINE_FUNCTION_FAST( lastIndexOf ) {

	J_S_ASSERT_ARG_MIN(1);

	const char *sBuffer;
	size_t sLength;
	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &sBuffer, &sLength) ); // warning: GC on the returned buffer !

	if ( sLength == 0 ) {

		*J_FRVAL = INT_TO_JSVAL(0);
		return JS_TRUE;
	}

	const char *buffer;
	size_t length;
	J_CHK( BlobBuffer(cx, J_FOBJ, &buffer) );
	J_CHK( BlobLength(cx, J_FOBJ, &length) );

	long start;
	if ( J_FARG_ISDEF(2) ) {

		J_S_ASSERT_INT( J_FARG(2) );
		start = JSVAL_TO_INT( J_FARG(2) );

		if ( start < 0 ) {

			*J_FRVAL = INT_TO_JSVAL(-1);
			return JS_TRUE;
		}

		if ( start + sLength > length ) {

			start = length - sLength;
		}

	} else {

		start = length - sLength;
	}

	for ( long i = start; i >= 0; i-- ) {

		size_t j;
		for ( j = 0; j < sLength && buffer[i+j] == sBuffer[j]; j++ );
		if ( j == sLength ) {

			J_CHK( JS_NewNumberValue(cx, i, J_FRVAL) );
			return JS_TRUE;
		}
	}

	*J_FRVAL = INT_TO_JSVAL(-1);
	return JS_TRUE;
}


/**doc
 * $STR $INAME( index )
  Returns the specified character from a string.
  $H details
   fc. [http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Reference:Global_Objects:String:charAt]
**/
DEFINE_FUNCTION_FAST( charAt ) {

	long index;
	if ( J_FARG_ISDEF(1) ) {

		J_S_ASSERT_INT( J_FARG(1) );
		index = JSVAL_TO_INT( J_FARG(1) );
	} else {

		index = 0;
	}

	size_t length;
	J_CHK( BlobLength(cx, J_FOBJ, &length) );

	if ( length == 0 || index < 0 || (unsigned)index >= length ) {

		*J_FRVAL = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	const char *buffer;
	J_CHK( BlobBuffer(cx, J_FOBJ, &buffer) );

	jschar chr = ((char*)buffer)[index];
	JSString *str1 = JS_NewUCStringCopyN(cx, &chr, 1);
	J_S_ASSERT_ALLOC( str1 );
	*J_FRVAL = STRING_TO_JSVAL(str1);

	return JS_TRUE;
}


/**doc
 * $INT $INAME( index )
  Returns a number indicating the ASCII value of the character at the given index.
  $H details
   fc. [http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Reference:Global_Objects:String:charCodeAt]
**/
DEFINE_FUNCTION_FAST( charCodeAt ) {

	long index;
	if ( J_FARG_ISDEF(1) ) {

		J_S_ASSERT_INT( J_FARG(1) );
		index = JSVAL_TO_INT( J_FARG(1) );
	} else {

		index = 0;
	}

	size_t length;
	J_CHK( BlobLength(cx, J_FOBJ, &length) );

	if ( length == 0 || index < 0 || (unsigned)index >= length ) {

		*J_FRVAL = JS_GetNaNValue(cx);
		return JS_TRUE;
	}

	const char *buffer;
	J_CHK( BlobBuffer(cx, J_FOBJ, &buffer) );
	*J_FRVAL = INT_TO_JSVAL( buffer[index] );
	return JS_TRUE;
}


/**doc
 * $STR $INAME()
  Returns a JavaScript string version of the current Blob object.
  $H beware
   This function may be called automatically by the JavaScript engine when it needs to convert the Blob object to a JS string.
**/
DEFINE_FUNCTION_FAST( toString ) { // and valueOf

	JSString *jsstr;
	J_CHK( BlobToJSString(cx, J_FOBJ, &jsstr) );
	J_S_ASSERT( jsstr != NULL, "Unable to convert Blob to String." );
	*J_FRVAL = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
}


/**doc
=== Properties ===
**/

/**doc
 * $INT $INAME
  is the length of the current Blob.
**/
DEFINE_PROPERTY( length ) {

	size_t length;
	J_CHK( BlobLength(cx, obj, &length) );
	*vp = INT_TO_JSVAL( length );
	return JS_TRUE;
}


/**doc
 * $TYPE char $INAME $READONLY
  Used to access the character in the _N_th position where _N_ is a positive integer between 0 and one less than the value of length.
**/
DEFINE_GET_PROPERTY() {

	if ( !JSVAL_IS_INT(id) )
		return JS_TRUE;

	jsint slot = JSVAL_TO_INT( id );

	void *pv = JS_GetPrivate(cx, obj);
	if ( pv == NULL )
		return JS_TRUE;

	size_t length;
	J_CHK( BlobLength(cx, obj, &length) );

	if ( slot < 0 || slot >= (int)length )
		return JS_TRUE;

	jschar chr = ((char*)pv)[slot];
	JSString *str1 = JS_NewUCStringCopyN(cx, &chr, 1);
	J_S_ASSERT_ALLOC( str1 );

	*vp = STRING_TO_JSVAL(str1);

	return JS_TRUE;
}


DEFINE_SET_PROPERTY() {

	J_S_ASSERT( !JSVAL_IS_NUMBER(id), "Cannot modify immutable string" );
	return JS_TRUE;
}


DEFINE_EQUALITY() {

	if ( J_JSVAL_IS_CLASS(v, _class) ) {
		
		const char *buf1, *buf2;
		size_t len1, len2;
		J_CHK( BlobBuffer(cx, obj, &buf1) );
		J_CHK( BlobLength(cx, obj, &len1) );
		J_CHK( BlobBuffer(cx, JSVAL_TO_OBJECT(v), &buf2) );
		J_CHK( BlobLength(cx, JSVAL_TO_OBJECT(v), &len2) );
		
		if ( len1 == len2 && memcmp(buf1, buf2, len1) == 0 ) {

			*bp = JS_TRUE;
			return JS_TRUE;
		}
	}
	*bp = JS_FALSE;
	return JS_TRUE;
}


static JSBool MutateToJSString(JSContext *cx, JSObject *obj) {

	J_CHK( SetBufferGetInterface(cx, obj, NULL) );

	const char *buffer;
	size_t length;
	J_CHK( BlobBuffer(cx, obj, &buffer) );
	J_CHK( BlobLength(cx, obj, &length) );
	// ownership of buffer is given to the JSString
	JSString *jsstr = JS_NewString(cx, (char*)buffer, length); // JS_NewString don't accepts (const char *)
	JSObject *strObj;
	J_CHK( JS_ValueToObject(cx, STRING_TO_JSVAL(jsstr), &strObj) );

	obj->fslots[JSSLOT_PROTO] = strObj->fslots[JSSLOT_PROTO];
	// Make sure we preserve any flags borrowing bits in JSSLOT_CLASS.
	obj->fslots[JSSLOT_CLASS] ^= (jsval) JS_GET_CLASS(cx, obj);
	obj->fslots[JSSLOT_CLASS] |= (jsval) JS_GET_CLASS(cx, strObj);
	obj->fslots[JSSLOT_PRIVATE] = strObj->fslots[JSSLOT_PRIVATE];
	obj->map->ops = strObj->map->ops;

	return JS_TRUE;
}


DEFINE_NEW_RESOLVE() {

	if ( !(flags & JSRESOLVE_QUALIFIED) || (flags & JSRESOLVE_ASSIGNING) )
		return JS_TRUE;

	// check if obj is a Blob
	if ( JS_GetPrototype(cx, obj) != prototypeBlob )
		return JS_TRUE;

	jsid propId;
	J_CHK( JS_ValueToId(cx, id, &propId) );

	// search propId in Blob's prototype
	JSProperty *prop;
	J_CHK( OBJ_LOOKUP_PROPERTY(cx, prototypeBlob, propId, objp, &prop) );
	if ( prop ) {

		OBJ_DROP_PROPERTY(cx, *objp, prop);
		return JS_TRUE;
	}

	// search propId in String's prototype.
	JSObject *stringPrototype;
	J_CHK( JS_GetClassObject(cx, JS_GetGlobalObject(cx), JSProto_String, &stringPrototype) );
	J_CHK( OBJ_LOOKUP_PROPERTY(cx, stringPrototype, propId, objp, &prop) );
	if ( prop )
		OBJ_DROP_PROPERTY(cx, *objp, prop);
	else
		return JS_TRUE;

	*objp = NULL; // let the engin find the property on the String's prototype.

	J_CHKM( MutateToJSString(cx, obj), "Unable to transform the Blob into a String." );
//	const char *debug_name = JS_GetStringBytes(JS_ValueToString(cx, id));
	return JS_TRUE;
}


/**doc
=== Note ===
 Blobs are immutable. This mean that its content cannot be modified after it is created.
**/

/**doc
=== Native Interface ===
 *NIBufferGet*
**/

CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_GET_PROPERTY
	HAS_SET_PROPERTY
	HAS_EQUALITY
	HAS_NEW_RESOLVE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(concat)
		FUNCTION_FAST(substr)
		FUNCTION_FAST(indexOf)
		FUNCTION_FAST(lastIndexOf)
		FUNCTION_FAST(charCodeAt)
		FUNCTION_FAST(charAt)
		FUNCTION_FAST(toString)
		FUNCTION_FAST_ALIAS(valueOf, toString)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(length)
	END_PROPERTY_SPEC

END_CLASS
