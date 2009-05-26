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
#include "blobPub.h"

// SLOT_BLOB_LENGTH is the size of the content of the blob OR JSVAL_VOID if the blob has been invalidated (see Blob::Free() method)
#define SLOT_BLOB_LENGTH 0


ALWAYS_INLINE JSBool InvalidateBlob( JSContext *cx, JSObject *blobObject ) {

	return JS_SetReservedSlot(cx, blobObject, SLOT_BLOB_LENGTH, JSVAL_VOID);
}

ALWAYS_INLINE bool IsBlobValid( JSContext *cx, JSObject *blobObject ) {

	jsval lengthVal;
	JL_CHK( JS_GetReservedSlot(cx, blobObject, SLOT_BLOB_LENGTH, &lengthVal) );
	return JSVAL_IS_INT( lengthVal );
bad:
	return false;
}


ALWAYS_INLINE JSBool BlobLength( JSContext *cx, JSObject *blobObject, size_t *length ) {

	JL_S_ASSERT_CLASS(blobObject, BlobJSClass( cx ));
	jsval lengthVal;
	JL_CHK( JS_GetReservedSlot(cx, blobObject, SLOT_BLOB_LENGTH, &lengthVal) );
//	JL_S_ASSERT_INT( lengthVal );
	JL_SAFE(
		JL_CHKM( JSVAL_IS_INT( lengthVal ), "Invalidated blob." );
	);
	*length = JSVAL_TO_INT( lengthVal );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool BlobBuffer( JSContext *cx, JSObject *blobObject, const char **buffer ) {

	JL_S_ASSERT_CLASS(blobObject, BlobJSClass( cx ));
	*buffer = (char*)JL_GetPrivate(cx, blobObject);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Blob )

JSBool NativeInterfaceBufferGet( JSContext *cx, JSObject *obj, const char **buf, size_t *size ) {

	if ( JL_GetClass(obj) == classBlob ) {
		
		if ( !IsBlobValid(cx, obj) )
			JL_REPORT_ERROR("Invalid Blob object.");
		JL_CHK( BlobLength(cx, obj, size) );
		JL_CHK( BlobBuffer(cx, obj, buf) );
		return JS_TRUE;
	}

	JSString *jsstr;
	jsstr = JS_ValueToString(cx, OBJECT_TO_JSVAL(obj));
	*buf = JS_GetStringBytes(jsstr);
	*size = JL_GetStringLength(jsstr);
	return JS_TRUE;
	JL_BAD;
}


/*
inline JSBool JsvalToBlob( JSContext *cx, jsval val, JSObject **obj ) {

	size_t srcLen;
	void *src, *dst = NULL;

	if ( JsvalIsBlob(cx, val) ) {

		BlobGetBufferAndLength(cx, JSVAL_TO_OBJECT( val ), &src, &srcLen);
		if ( srcLen > 0 ) {

			dst = JS_malloc(cx, srcLen +1);
			JL_CHK( dst );
			((char*)dst)[srcLen] = '\0';
			memcpy(dst, src, srcLen);
		}
	} else {

		JSString *jsstr = JS_ValueToString(cx, val);
		JL_S_ASSERT( jsstr != NULL, "Unable to convert to string." );
		srcLen = JL_GetStringLength(jsstr);
		if ( srcLen > 0 ) {

			dst = JS_malloc(cx, srcLen +1);
			JL_CHK( dst );
			((char*)dst)[srcLen] = '\0';

			// (TBD) try to know if the string is deflated befor using JS_GetStringChars ??
			jschar *chars = JS_GetStringChars(jsstr);
			for ( size_t i = 0; i < srcLen; i++ )
				((char*)dst)[i] = (uint8)chars[i];
		}
	}

	*obj = NewBlob(cx, dst, srcLen);
	return JS_TRUE;
	JL_BAD;
}
*/


DEFINE_FINALIZE() {

	void *pv = JL_GetPrivate(cx, obj);
	if ( !pv )
		return;
	JS_free(cx, pv);
}


/**doc
$TOC_MEMBER $INAME
 $INAME( [data] )
  Creates an object that can contain binary data.
  $H note
  When called in a non-constructor context, Object behaves identically.
**/
DEFINE_CONSTRUCTOR() {

	void *dBuffer = NULL; // keep on top

	if ( !JS_IsConstructing(cx) ) { // supports this form (w/o new operator) : result.param1 = Blob('Hello World');

		obj = JS_NewObject(cx, _class, NULL, NULL);
		JL_S_ASSERT( obj != NULL, "Blob construction failed." );
		*rval = OBJECT_TO_JSVAL(obj);
	} else {
		
//		JSClass *tmp = JL_GetRegistredNativeClass(cx, "Blob");
//		printf("\n***DBG:%d\n", JS_GetClass(obj) == tmp);
		JL_S_ASSERT_THIS_CLASS();
	}

	if ( JL_ARGC >= 1 ) {

		size_t length;
		const char *sBuffer;
		JL_CHK( JsvalToStringAndLength(cx, &JL_ARG(1), &sBuffer, &length) ); // warning: GC on the returned buffer !

		dBuffer = JS_malloc(cx, length +1);
		JL_CHK( dBuffer );
		((char*)dBuffer)[length] = '\0';
		memcpy(dBuffer, sBuffer, length);
		JL_CHK( JL_SetPrivate(cx, obj, dBuffer) );
		JL_CHK( JS_SetReservedSlot(cx, obj, SLOT_BLOB_LENGTH, INT_TO_JSVAL(length) ) );
	} else {

		JL_CHK( JL_SetPrivate(cx, obj, NULL) );
		JL_CHK( JS_SetReservedSlot(cx, obj, SLOT_BLOB_LENGTH, INT_TO_JSVAL(0) ) );
	}

	JL_CHK( ReserveBufferGetInterface(cx, obj) );
	JL_CHK( SetBufferGetInterface(cx, obj, NativeInterfaceBufferGet) );
	return JS_TRUE;

bad:
	if ( dBuffer )
		JS_free(cx, dBuffer);
	return JS_FALSE;
}


/**doc
=== Methods ===
**/


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [ wipe = false ] )
  Frees the memory allocated by the blob and invalidates the blob.
  $H arguments
   $ARG $BOOL wipe: clears the buffer before freeing it. This is useful when the blob contains sensitive data.
  $H note
   Any access to a freed Blob will rise an error.
**/
DEFINE_FUNCTION_FAST( Free ) {

	JL_S_ASSERT_CLASS(JL_FOBJ, _class);
	void *pv;
	pv = JL_GetPrivate(cx, JL_FOBJ);

	if ( JL_FARG_ISDEF(1) ) {

		bool wipe;
		JL_CHK( JsvalToBool(cx, JL_FARG(1), &wipe ) );
		if ( wipe ) {

			size_t length;
			JL_CHK( BlobLength(cx, JL_FOBJ, &length) );
			memset(pv, 0, length);
		}
	}

	JS_free(cx, pv);
	JL_CHK( JL_SetPrivate(cx, JL_FOBJ, NULL) );
	JL_CHK( InvalidateBlob(cx, JL_FOBJ) );
	// removes all of obj's own properties, except the special __proto__ and __parent__ properties, in a single operation.
	// Properties belonging to objects on obj's prototype chain are not affected.
	JS_ClearScope(cx, JL_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Blob $INAME( data [,data1 [,...]] )
  Combines the text of two or more strings and returns a new string.
  $H details
   [http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/concat Mozilla]
**/
DEFINE_FUNCTION_FAST( concat ) {

	char *dst = NULL;
	JL_S_ASSERT_CLASS(JL_FOBJ, _class);

	// note: var a = new String(123);  a.concat() !== a

	size_t thisLength;
	const char *thisBuffer;
	JL_CHK( BlobBuffer(cx, JL_FOBJ, &thisBuffer) );
	JL_CHK( BlobLength(cx, JL_FOBJ, &thisLength) );

	size_t dstLen;
	dstLen = thisLength;

	unsigned int arg;
	for ( arg = 1; arg <= JL_ARGC; arg++ ) {
	
		if ( JsvalIsBlob(cx, JL_FARG(arg)) ) {

			size_t tmp;
			JL_CHK( BlobLength(cx, JSVAL_TO_OBJECT( JL_FARG(arg) ), &tmp) );
			dstLen += tmp;
		
		} else {

			JSString *jsstr = JS_ValueToString(cx, JL_FARG(arg));
			JL_FARG(arg) = STRING_TO_JSVAL(jsstr);
			dstLen += JL_GetStringLength(jsstr);
		}
	}

	dst = (char*)JS_malloc(cx, dstLen +1);
	JL_CHK( dst );
	dst[dstLen] = '\0';

	char *tmp;
	tmp = dst;

	if ( thisLength > 0 ) {

		memcpy(tmp, thisBuffer, thisLength);
		tmp += thisLength;
	}

	for ( arg = 1; arg <= JL_ARGC; arg++ ) {
		
		const char *buffer;
		size_t length;
		JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(arg), &buffer, &length) );

		memcpy(tmp, buffer, length);
		tmp += length;
	}

	JL_CHK( JL_NewBlob(cx, dst, dstLen, JL_FRVAL) );
	return JS_TRUE;
bad:
	if ( dst )
		JS_free(cx, dst);
	return JS_FALSE;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Blob $INAME( start [, length ] )
  Returns the bytes in a string beginning at the specified location through the specified number of characters.
  $H arguments
   $ARG $INT start: location at which to begin extracting characters (an integer between 0 and one less than the length of the string).
   $ARG $INT length: the number of characters to extract.
  $H details
   fc. [http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/substr Mozilla]
**/
DEFINE_FUNCTION_FAST( substr ) {

	JL_S_ASSERT_CLASS(JL_FOBJ, _class);
	JL_S_ASSERT_ARG_MIN(1);

	const char *bstrBuffer;
	JL_CHK( BlobBuffer(cx, JL_FOBJ, &bstrBuffer) );

	size_t dataLength;
	JL_CHK( BlobLength(cx, JL_FOBJ, &dataLength) );

	jsval arg1;
	arg1 = JL_FARG(1);

	int start;
	if ( JsvalIsPInfinity(cx, arg1) )
		start = (signed)dataLength;
	else if ( !JsvalToInt(cx, arg1, &start) )
		start = 0;

	if ( start >= (signed)dataLength ) {

		*JL_FRVAL = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	if ( start < 0 )
		start = dataLength + start;

	if ( start < 0 || start >= (int)dataLength )
		start = 0;

	// now 0 <= start < dataLength

	int length;
	if ( argc >= 2 ) {

		jsval arg2;
		arg2 = JL_FARG(2);

		if ( JsvalIsPInfinity(cx, arg2) )
			length = dataLength;
		else if ( !JsvalToInt(cx, arg2, &length) )
			length = 0;

		if ( length <= 0 ) {

			*JL_FRVAL = JS_GetEmptyStringValue(cx);
			return JS_TRUE;
		}

		if ( start + length > (int)dataLength )
			length = dataLength - start;

	} else
		length = dataLength - start;

	// now 0 <= length < dataLength - start

	void *buffer;
	buffer = JS_malloc(cx, length +1);
	JL_CHK( buffer );
	((char*)buffer)[length] = '\0';

	memcpy(buffer, ((int8_t*)bstrBuffer) + start, length);
	JL_CHKB( JL_NewBlob(cx, buffer, length, JL_FRVAL), bad_free );

	return JS_TRUE;
bad_free:
	JS_free(cx, buffer);
bad:
	return JS_FALSE;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE Blob $INAME( indexA, [ indexB ] )
  Extracts characters from indexA up to but not including indexB. In particular: 
   * If indexA equals indexB, substring returns an empty string.
   * If indexB is omitted, substring extracts characters to the end of the blob.
   * If either argument is less than 0 or is NaN, it is treated as if it were 0.
   * If either argument is greater than stringName.length, it is treated as if it were stringName.length. 
   If indexA is larger than indexB, then the effect of substring is as if the two arguments were swapped; for example, str.substring(1, 0) == str.substring(0, 1). 
  $H arguments
   $ARG $INT indexA: An integer between 0 and one less than the length of the blob.
   $ARG $INT indexB: (optional) An integer between 0 and the length of the blob.
  $H details
   fc. [http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/substring Mozilla]
**/
DEFINE_FUNCTION_FAST( substring ) {

	JL_S_ASSERT_CLASS(JL_FOBJ, _class);
	if ( JL_ARGC == 0 ) {
		
		*JL_FRVAL = JL_FARG(1);
		return JS_TRUE;
	}

	const char *bstrBuffer;
	JL_CHK( BlobBuffer(cx, JL_FOBJ, &bstrBuffer) );
	size_t dataLength;
	JL_CHK( BlobLength(cx, JL_FOBJ, &dataLength) );

	int indexA, indexB;
/*
	jsval arg1 = JL_FARG(1);
	jsval arg2 = JL_FARG(1);

	if ( JSVAL_IS_INT(arg1) )
		indexA = JSVAL_TO_INT(arg1)
	else {
		
		if ( arg1 == JS_GetPositiveInfinityValue(cx) )
	}
*/

	jsval arg1;
	arg1 = JL_FARG(1);
	if ( JsvalIsPInfinity(cx, arg1) )
		indexA = (signed)dataLength;
	else if ( !JsvalToInt(cx, JL_FARG(1), &indexA) )
		indexA = 0;

	if ( JL_ARGC < 2 || JsvalIsPInfinity(cx, JL_FARG(2)) )
		indexB = (signed)dataLength;
	else if ( !JsvalToInt(cx, JL_FARG(2), &indexB) )
		indexB = 0;

	if ( indexA < 0 )
		indexA = 0;
	else if ( indexA > (signed)dataLength )
		indexA = (signed)dataLength;

	if ( indexB < 0 )
		indexB = 0;
	else if ( indexB > (signed)dataLength )
		indexB = (signed)dataLength;

	if ( indexA > indexB ) {

		int tmp = indexB;
		indexB = indexA;
		indexA = tmp;
	}

	if ( indexA == indexB || indexA >= (signed)dataLength ) {

		*JL_FRVAL = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}


	int length;
	length = indexB - indexA;

	void *buffer;
	buffer = JS_malloc(cx, length +1);
	JL_CHK( buffer );
	((char*)buffer)[length] = '\0';

	memcpy(buffer, ((int8_t*)bstrBuffer) + indexA, length);
	JL_CHKB( JL_NewBlob(cx, buffer, length, JL_FRVAL), bad_free );

	return JS_TRUE;
bad_free:
	JS_free(cx, buffer);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( searchValue [, fromIndex] )
  Returns the index within the calling Blob object of the first occurrence of the specified value, starting the search at fromIndex, or -1 if the value is not found.
  $H arguments
   $ARG $STR searchValue: A string representing the value to search for.
   $ARG $INT fromIndex: The location within the calling string to start the search from. It can be any integer between 0 and the length of the string. The default value is 0.
  $H details
   fc. [http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/indexOf Mozilla]
**/
DEFINE_FUNCTION_FAST( indexOf ) {

	JL_S_ASSERT_CLASS(JL_FOBJ, _class);
	JL_S_ASSERT_ARG_MIN(1);

	const char *sBuffer;
	size_t sLength;
	JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(1), &sBuffer, &sLength) ); // warning: GC on the returned buffer !

	if ( sLength == 0 ) {

		*JL_FRVAL = INT_TO_JSVAL(0);
		return JS_TRUE;
	}

	size_t length;
	JL_CHK( BlobLength(cx, JL_FOBJ, &length) );

	long start;
	if ( JL_FARG_ISDEF(2) ) {

		JL_S_ASSERT_INT( JL_FARG(2) );
		start = JSVAL_TO_INT( JL_FARG(2) );

		if ( start < 0 )
			start = 0;
		else if ( start + sLength > length ) {

			*JL_FRVAL = INT_TO_JSVAL(-1);
			return JS_TRUE;
		}

	} else {

		start = 0;
	}

	const char *buffer;
	JL_CHK( BlobBuffer(cx, JL_FOBJ, &buffer) );

	for ( size_t i = start; i < length; i++ ) {

		size_t j;
		for ( j = 0; j < sLength && buffer[i+j] == sBuffer[j]; j++ );
		if ( j == sLength ) {

			JL_CHK( JS_NewNumberValue(cx, i, JL_FRVAL) );
			return JS_TRUE;
		}
	}

	*JL_FRVAL = INT_TO_JSVAL(-1);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( searchValue [, fromIndex] )
  Returns the index within the calling Blob object of the last occurrence of the specified value, or -1 if not found. The calling string is searched backward, starting at fromIndex.
  $H arguments
   $ARG $STR searchValue: A string representing the value to search for.
   $ARG $INT fromIndex: The location within the calling string to start the search from, indexed from left to right. It can be any integer between 0 and the length of the string. The default value is the length of the string.
  $H details
   fc. [http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/lastIndexOf Mozilla]
**/
DEFINE_FUNCTION_FAST( lastIndexOf ) {

	JL_S_ASSERT_CLASS(JL_FOBJ, _class);
	JL_S_ASSERT_ARG_MIN(1);

	const char *sBuffer;
	size_t sLength;
	JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(1), &sBuffer, &sLength) ); // warning: GC on the returned buffer !


	const char *buffer;
	size_t length;
	JL_CHK( BlobBuffer(cx, JL_FOBJ, &buffer) );
	JL_CHK( BlobLength(cx, JL_FOBJ, &length) );

	if ( sLength == 0 && argc < 2 ) {

		*JL_FRVAL = INT_TO_JSVAL(length);
		return JS_TRUE;
	}

	int start;
	if ( JL_FARG_ISDEF(2) ) {
		
		jsval arg2 = JL_FARG(2);

		if ( JSVAL_IS_INT(arg2) && JSVAL_TO_INT(arg2) < 0 || JsvalIsNInfinity(cx, arg2) ) {
			
			start = 0;
		} else {

			if ( JsvalIsPInfinity(cx, arg2) || JsvalIsNaN(cx, arg2) ) {
				
				start = length - sLength;
			} else {
				
				JL_CHK( JsvalToInt(cx, JL_FARG(2), &start) );
				if ( start + sLength > length ) {

					start = length - sLength;
				}
			}
		}

	} else {

		start = length - sLength;
	}

	for ( long i = start; i >= 0; i-- ) {

		size_t j;
		for ( j = 0; j < sLength && buffer[i+j] == sBuffer[j]; j++ );
		if ( j == sLength ) {

			JL_CHK( JS_NewNumberValue(cx, i, JL_FRVAL) );
			return JS_TRUE;
		}
	}

	*JL_FRVAL = INT_TO_JSVAL(-1);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( index )
  Returns the specified character from a string.
  $H details
   fc. [http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/charAt Mozilla]
**/
DEFINE_FUNCTION_FAST( charAt ) {

	JL_S_ASSERT_CLASS(JL_FOBJ, _class);
	int index;
	if ( JL_FARG_ISDEF(1) ) {

		jsval arg1 = JL_FARG(1);
		if ( !JSVAL_IS_INT(arg1) ) {

			if ( JsvalIsPInfinity(cx, arg1) || JsvalIsNInfinity(cx, arg1) || JsvalIsNaN(cx, arg1) ) {
				
				*JL_FRVAL = JS_GetEmptyStringValue(cx);
				return JS_TRUE;
			}

			JL_CHK( JsvalToInt(cx, arg1, &index) );
		} else {

			index = JSVAL_TO_INT(arg1);
		}
	} else {

		index = 0;
	}


	size_t length;
	JL_CHK( BlobLength(cx, JL_FOBJ, &length) );

	if ( length == 0 || index < 0 || (unsigned)index >= length ) {

		*JL_FRVAL = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	const char *buffer;
	JL_CHK( BlobBuffer(cx, JL_FOBJ, &buffer) );

	jschar chr;
	chr = ((char*)buffer)[index];
	JSString *str1;
	str1 = JS_NewUCStringCopyN(cx, &chr, 1);
	JL_CHK( str1 );
	*JL_FRVAL = STRING_TO_JSVAL(str1);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( index )
  Returns a number indicating the ASCII value of the character at the given index.
  $H details
   fc. [http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/charCodeAt Mozilla]
**/
DEFINE_FUNCTION_FAST( charCodeAt ) {

	JL_S_ASSERT_CLASS(JL_FOBJ, _class);
	int index;
	if ( JL_FARG_ISDEF(1) ) {

		jsval arg1 = JL_FARG(1);
		if ( !JSVAL_IS_INT(arg1) ) {

			if ( JsvalIsPInfinity(cx, arg1) || JsvalIsNInfinity(cx, arg1) ) {
				
				*JL_FRVAL = JS_GetNaNValue(cx);
				return JS_TRUE;
			}

			JL_CHK( JsvalToInt(cx, arg1, &index) );

//			JL_REPORT_ERROR( J__ERRMSG_UNEXPECTED_TYPE " Integer expected." );
			return JS_FALSE;
		}
		index = JSVAL_TO_INT(arg1);
	} else {

		index = 0;
	}

	size_t length;
	JL_CHK( BlobLength(cx, JL_FOBJ, &length) );

	if ( length == 0 || index < 0 || (unsigned)index >= length ) {

		*JL_FRVAL = JS_GetNaNValue(cx);
		return JS_TRUE;
	}

	const char *buffer;
	JL_CHK( BlobBuffer(cx, JL_FOBJ, &buffer) );
	*JL_FRVAL = INT_TO_JSVAL( buffer[index] );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME()
  Returns a JavaScript string version of the current Blob object.
  $H beware
   This function may be called automatically by the JavaScript engine when it needs to convert the Blob object to a JS string.
**/
DEFINE_FUNCTION_FAST( toString ) { // and valueOf ?

	JL_S_ASSERT_CLASS(JL_FOBJ, _class);
	void *pv;
	pv = JL_GetPrivate(cx, JL_FOBJ);
	size_t length;
	JL_CHK( BlobLength(cx, JL_FOBJ, &length) );
	JSString *jsstr;
	if ( length == 0 ) {

		jsstr = JSVAL_TO_STRING( JS_GetEmptyStringValue(cx) );
	} else {

		jschar *ucStr = (jschar*)JS_malloc(cx, (length + 1) * sizeof(jschar));
		ucStr[length] = 0;
		for ( size_t i = 0; i < length; i++ )
			ucStr[i] = ((unsigned char*)pv)[i]; // see js_InflateString in jsstr.c
		jsstr = JS_NewUCString(cx, ucStr, length);
	}

	JL_S_ASSERT( jsstr != NULL, "Unable to convert Blob to String." );
	*JL_FRVAL = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION_FAST( toSource ) {

	// (TBD) try something faster !!
	*JL_FRVAL = STRING_TO_JSVAL(JS_ValueToSource(cx, STRING_TO_JSVAL(JS_ValueToString(cx, OBJECT_TO_JSVAL(JL_FOBJ)))));
	return JS_TRUE;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  is the length of the current Blob.
**/
DEFINE_PROPERTY( length ) {

	JL_S_ASSERT_THIS_CLASS();
	size_t length;
	JL_CHK( BlobLength(cx, obj, &length) );
	*vp = INT_TO_JSVAL( length );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE char $INAME $READONLY
  Used to access the character in the _N_th position where _N_ is a positive integer between 0 and one less than the value of length.
**/
DEFINE_GET_PROPERTY() {

//	JL_S_ASSERT_THIS_CLASS(); // when mutated, this is a String
	if ( !JSVAL_IS_INT(id) )
		return JS_TRUE;

	jsint slot = JSVAL_TO_INT( id );

	void *pv = JL_GetPrivate(cx, obj);
	if ( pv == NULL )
		return JS_TRUE;

	size_t length;
	JL_CHK( BlobLength(cx, obj, &length) );

	if ( slot < 0 || slot >= (int)length )
		return JS_TRUE;

	jschar chr;
	chr = ((char*)pv)[slot];
	JSString *str1;
	str1 = JS_NewUCStringCopyN(cx, &chr, 1);
	JL_CHK( str1 );

	*vp = STRING_TO_JSVAL(str1);

	return JS_TRUE;
	JL_BAD;
}


DEFINE_SET_PROPERTY() {

	JL_S_ASSERT( !JSVAL_IS_NUMBER(id), "Cannot modify immutable objects" );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_EQUALITY() {

	if ( JsvalIsClass(v, _class) ) {

		if ( !IsBlobValid(cx, obj) || !IsBlobValid(cx, JSVAL_TO_OBJECT(v)) ) {
			
			JL_REPORT_ERROR("Invalid Blob object.");
		}
		
		const char *buf1, *buf2;
		size_t len1, len2;
		JL_CHK( BlobBuffer(cx, obj, &buf1) );
		JL_CHK( BlobLength(cx, obj, &len1) );
		JL_CHK( BlobBuffer(cx, JSVAL_TO_OBJECT(v), &buf2) );
		JL_CHK( BlobLength(cx, JSVAL_TO_OBJECT(v), &len2) );
		
		if ( len1 == len2 && memcmp(buf1, buf2, len1) == 0 ) {

			*bp = JS_TRUE;
			return JS_TRUE;
		}
	}
	*bp = JS_FALSE;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_NEW_RESOLVE() {

	// (TBD) check if needed: yes else var s = new Blob('this is a string object');Print( s.substring() ); will failed
	// check if obj is a Blob
	if ( JS_GetPrototype(cx, obj) != prototypeBlob )
		return JS_TRUE;

	if ( !IsBlobValid(cx, obj) ) {

		JL_REPORT_ERROR("Invalid Blob object.");
	}

	if ( !(flags & JSRESOLVE_QUALIFIED) || (flags & JSRESOLVE_ASSIGNING) )
		return JS_TRUE;

	jsid propId;
	JL_CHK( JS_ValueToId(cx, id, &propId) );


	// search propId in Blob's prototype
	JSProperty *prop;
	JL_CHK( OBJ_LOOKUP_PROPERTY(cx, prototypeBlob, propId, objp, &prop) );
	if ( prop ) {

		OBJ_DROP_PROPERTY(cx, *objp, prop);
		return JS_TRUE;
	}

	// search propId in String's prototype.
	JSObject *stringPrototype;
	JL_CHK( JS_GetClassObject(cx, JS_GetGlobalObject(cx), JSProto_String, &stringPrototype) );
	JL_CHK( OBJ_LOOKUP_PROPERTY(cx, stringPrototype, propId, objp, &prop) );
	if ( prop )
		OBJ_DROP_PROPERTY(cx, *objp, prop);
	else
		return JS_TRUE;

	*objp = NULL; // let the engin find the property on the String's prototype.


	const char *buffer;
	size_t length;
	JL_CHK( BlobBuffer(cx, obj, &buffer) );
	JL_CHK( BlobLength(cx, obj, &length) );
	// ownership of buffer is given to the JSString
	JSString *jsstr;
	jsstr = JS_NewString(cx, (char*)buffer, length); // JS_NewString don't accepts (const char *)
	JSObject *strObj;
	JL_CHK( JS_ValueToObject(cx, STRING_TO_JSVAL(jsstr), &strObj) );

	obj->fslots[JSSLOT_PROTO] = strObj->fslots[JSSLOT_PROTO];
	// Make sure we preserve any flags borrowing bits in JSSLOT_CLASS.

	obj->classword ^= (jsuword)OBJ_GET_CLASS(cx, obj);
	obj->classword |= (jsuword)OBJ_GET_CLASS(cx, strObj);

	obj->fslots[JSSLOT_PRIVATE] = strObj->fslots[JSSLOT_PRIVATE];
	obj->map->ops = strObj->map->ops;

//	JL_CHKM( MutateToJSString(cx, obj), "Unable to transform the Blob into a String." );
//	const char *debug_name = JS_GetStringBytes(JS_ValueToString(cx, id));
	return JS_TRUE;
	JL_BAD;
}


DEFINE_XDR() {
	
	jsid id;
	jsval key, value;

	if ( xdr->mode == JSXDR_ENCODE ) {

		const char *buffer;
		size_t length;
		JL_CHK( BlobLength(xdr->cx, *objp, &length) );
		JL_CHK( BlobBuffer(xdr->cx, *objp, &buffer) );
		uint32 tmp;
		tmp = length;
		JL_CHK( JS_XDRUint32(xdr, &tmp) );
		JL_CHK( JS_XDRBytes(xdr, (char*)buffer, length) ); // ugly but safe de-const because we are JSXDR_ENCODE.

		JSObject *it;
		it = JS_NewPropertyIterator(xdr->cx, *objp); // see JS_Enumerate that calls obj's JSClass.enumerate hook. JS_DestroyIdArray.
		JL_CHK( it );

		for (;;) {

			JL_CHK( JS_NextProperty(xdr->cx, it, &id) );
			if ( id != JSVAL_VOID ) { // ... or JSVAL_VOID if there is no such property left to visit.

				JL_CHK( JS_IdToValue(xdr->cx, id, &key) );
				JL_CHK( OBJ_GET_PROPERTY(xdr->cx, *objp, id, &value) ); // returning false on error or exception, true on success.
				JL_CHK( JS_XDRValue(xdr, &key) );
				JL_CHK( JS_XDRValue(xdr, &value) );
			} else {

				jsval tmp = JSVAL_VOID;
				JL_CHK( JS_XDRValue(xdr, &tmp) );
				break;
			}
		}
		return JS_TRUE;
	}


	if ( xdr->mode == JSXDR_DECODE ) {

		uint32 length;
		char *buffer;
		JL_CHK( JS_XDRUint32(xdr, &length) );
		buffer = (char*)JS_malloc(xdr->cx, length +1);
		buffer[length] = '\0'; // (TBD) needed ?
		JL_CHKB( JS_XDRBytes(xdr, buffer, length), bad_free_buffer );

		jsval tmp;
		JL_CHKB( JL_NewBlob(xdr->cx, buffer, length, &tmp), bad_free_buffer );
		JL_CHKB( JS_ValueToObject(xdr->cx, tmp, objp), bad_free_buffer );

		for (;;) {

			JL_CHKB( JS_XDRValue(xdr, &key), bad_free_buffer );
			if ( key != JSVAL_VOID ) {

				JS_ValueToId(xdr->cx, key, &id);
				JL_CHKB( JS_XDRValue(xdr, &value), bad_free_buffer );
				JL_CHKB( OBJ_SET_PROPERTY(xdr->cx, *objp, id, &value), bad_free_buffer );
			} else {

				break;
			}
		}
		return JS_TRUE;
	bad_free_buffer:
		JS_free(xdr->cx, buffer);
		return JS_FALSE;
	}

	if ( xdr->mode == JSXDR_FREE ) {

		// (TBD) nothing to free ?
		return JS_TRUE;
	}

	JL_BAD;
}




/**doc
=== Note ===
 Blobs are immutable. This mean that its content cannot be modified after it is created.
**/

/**doc
=== Native Interface ===
 * *NIBufferGet*
**/

CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_GET_PROPERTY
	HAS_SET_PROPERTY
	HAS_EQUALITY
	HAS_NEW_RESOLVE
	HAS_XDR

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Free)
		FUNCTION_FAST(concat)
		FUNCTION_FAST(substr)
		FUNCTION_FAST(substring)
		FUNCTION_FAST(indexOf)
		FUNCTION_FAST(lastIndexOf)
		FUNCTION_FAST(charCodeAt)
		FUNCTION_FAST(charAt)
		FUNCTION_FAST(toString)
		FUNCTION_FAST_ALIAS(valueOf, toString)
		FUNCTION_FAST(toSource)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(length)
	END_PROPERTY_SPEC

END_CLASS
