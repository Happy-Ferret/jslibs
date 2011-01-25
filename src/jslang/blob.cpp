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

#include "../common/jsvalserializer.h"

#include <cstring>

#include "blobPub.h"

#include "jsproxy.h"


ALWAYS_INLINE jsdouble
js__DoubleToInteger(jsdouble d) // from jsnum.h
{
    if (d == 0)
        return d;

    if (!finite(d)) {

        if (isnan(d))
            return 0;
        return d;
    }

    JSBool neg = (d < 0);
    d = floor(neg ? -d : d);
    return neg ? -d : d;
}


struct MemCmp { // from jsstr.cpp
    typedef size_t Extent;
    static ALWAYS_INLINE Extent computeExtent(const char *, size_t patlen) {
        return (patlen - 1) * sizeof(char);
    }
    static ALWAYS_INLINE bool match(const char *p, const char *t, Extent extent) {
        return memcmp(p, t, extent) == 0;
    }
};


struct ManualCmp { // from jsstr.cpp
    typedef const char *Extent;
	 static ALWAYS_INLINE Extent computeExtent(const char *pat, size_t patlen) {
        return pat + patlen;
    }
    static ALWAYS_INLINE bool match(const char *p, const char *t, Extent extent) {
        for (; p != extent; ++p, ++t) {
            if (*p != *t)
                return false;
        }
        return true;
    }
};


template <class InnerMatch> // from jsstr.cpp
ssize_t
UnrolledMatch(const char *text, size_t textlen, const char *pat, size_t patlen)
{
    JL_ASSERT(patlen > 0 && textlen > 0);
    const char *textend = text + textlen - (patlen - 1);
    const char p0 = *pat;
    const char *const patNext = pat + 1;
    const typename InnerMatch::Extent extent = InnerMatch::computeExtent(pat, patlen);
    uint8_t fixup;

    const char *t = text;
    switch ((textend - t) & 7) {
      case 0: if (*t++ == p0) { fixup = 8; goto match; }
      case 7: if (*t++ == p0) { fixup = 7; goto match; }
      case 6: if (*t++ == p0) { fixup = 6; goto match; }
      case 5: if (*t++ == p0) { fixup = 5; goto match; }
      case 4: if (*t++ == p0) { fixup = 4; goto match; }
      case 3: if (*t++ == p0) { fixup = 3; goto match; }
      case 2: if (*t++ == p0) { fixup = 2; goto match; }
      case 1: if (*t++ == p0) { fixup = 1; goto match; }
    }
    while (t != textend) {
      if (t[0] == p0) { t += 1; fixup = 8; goto match; }
      if (t[1] == p0) { t += 2; fixup = 7; goto match; }
      if (t[2] == p0) { t += 3; fixup = 6; goto match; }
      if (t[3] == p0) { t += 4; fixup = 5; goto match; }
      if (t[4] == p0) { t += 5; fixup = 4; goto match; }
      if (t[5] == p0) { t += 6; fixup = 3; goto match; }
      if (t[6] == p0) { t += 7; fixup = 2; goto match; }
      if (t[7] == p0) { t += 8; fixup = 1; goto match; }
        t += 8;
        continue;
        do {
            if (*t++ == p0) {
              match:
                if (!InnerMatch::match(patNext, t, extent))
                    goto failed_match;
                return t - text - 1;
            }
          failed_match:;
        } while (--fixup > 0);
    }
    return -1;
}


ALWAYS_INLINE ssize_t
Match(const char *text, size_t textlen, const char *pat, size_t patlen) {

	return 
#if !defined(__linux__)
		patlen > 128 ? UnrolledMatch<MemCmp>(text, textlen, pat, patlen) :
#endif
		UnrolledMatch<ManualCmp>(text, textlen, pat, patlen);
}


// SLOT_BLOB_LENGTH is the size of the content of the blob OR JSVAL_VOID if the blob has been invalidated (see Blob::Free() method)
#define SLOT_BLOB_LENGTH 0

// kind of cache
#define SLOT_BLOB_JSSTRING 1


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Blob )

// invalid blob: see Blob::Free()
ALWAYS_INLINE bool
IsBlobValid( JSContext *cx, JSObject *blobObject ) {

	return JL_HasPrivate(cx, blobObject) && JL_GetPrivate(cx, blobObject) != NULL;
}


ALWAYS_INLINE JSBool
BlobLength( JSContext *cx, JSObject *blobObject, size_t *length ) {

	jsval lengthVal;
	return JL_GetReservedSlot(cx, blobObject, SLOT_BLOB_LENGTH, &lengthVal) && JL_JsvalToNative(cx, lengthVal, length);
}


ALWAYS_INLINE JSBool
BlobBuffer( JSContext *cx, const JSObject *blobObject, const char **buffer ) {

	*buffer = (char*)JL_GetPrivate(cx, blobObject);
	JL_ASSERT( *buffer != NULL );
	return JS_TRUE;
}


JSBool NativeInterfaceBufferGet( JSContext *cx, JSObject *obj, JLStr *str ) {

	JL_ASSERT( JL_GetClass(obj) == JL_CLASS(Blob) );
		
	if (unlikely( !IsBlobValid(cx, obj) ))
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);
	
	const char *buf;
	size_t len;
	JL_CHK( BlobLength(cx, obj, &len) );
	JL_CHK( BlobBuffer(cx, obj, &buf) );

	*str = JLStr(buf, len, true);
	return JS_TRUE;
	JL_BAD;
}


/*
inline JSBool JL_JsvalToBlob( JSContext *cx, jsval val, JSObject **obj ) {

	size_t srcLen;
	void *src, *dst = NULL;

	if ( JL_JsvalIsBlob(cx, val) ) {

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

	if ( JL_GetHostPrivate(cx)->canSkipCleanup ) // do not cleanup in unsafe mode.
		return;
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
  When called in a non-constructor context, Object behaves identically. (TBD) update
**/
DEFINE_CONSTRUCTOR() {

	void *dBuffer = NULL; // keep on top (see bad:)

	if ( !JS_IsConstructing(cx, vp) && ( JL_ARGC == 0 || JL_ARG(1) == JL_GetEmptyStringValue(cx) ) ) {

		*JL_RVAL = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	JL_DEFINE_CONSTRUCTOR_OBJ;

	// supports this form (w/o new operator) : result.param1 = Blob('Hello World');

	// see. "planning to remove non-fast natives" (http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/91ee3f1f5642e05b?pli=1)

	if ( JL_ARGC != 0 && JL_ARG(1) != JL_GetEmptyStringValue(cx) ) {

		
//		JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &sBuffer, &length) ); // warning: GC on the returned buffer !
		JLStr str;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) ); // warning: GC on the returned buffer !

		size_t length = str.Length();
		const char *sBuffer = str.GetConstStr();

//		JL_S_ASSERT( length >= JSVAL_INT_MIN && length <= JSVAL_INT_MAX, "Blob too long." );

		dBuffer = JS_malloc(cx, length +1);
		JL_CHK( dBuffer );
		((char*)dBuffer)[length] = '\0';
		memcpy(dBuffer, sBuffer, length);
		JL_SetPrivate(cx, obj, dBuffer);

//		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_BLOB_LENGTH, INT_TO_JSVAL(jl::SafeCast<int>(length))) );
		jsval tmp;
		JL_CHK( JL_NativeToJsval(cx, length, &tmp) );
		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_BLOB_LENGTH, tmp) );
	} else {

		dBuffer = JS_malloc(cx, 1);
		JL_CHK( dBuffer );
		((char*)dBuffer)[0] = '\0';
		JL_SetPrivate(cx, obj, dBuffer);
		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_BLOB_LENGTH, INT_TO_JSVAL(0) ) );

//		JL_SetPrivate(cx, obj, NULL); // |Blob()| is an invalidated object ???
	}

//	JL_CHK( ReserveBufferGetInterface(cx, obj) );
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
   Any access to a freed Blob will rise an error.$LF
   Use this function to free huge amounts of memory, like images or sounds, before the GC does it for you.
**/
DEFINE_FUNCTION( Free ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS);

	if (unlikely( !IsBlobValid(cx, obj) ))
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

	*JL_RVAL = JSVAL_VOID;

	void *pv;
	pv = JL_GetPrivate(cx, obj);

	if ( JL_ARG_ISDEF(1) ) {

		bool wipe;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &wipe ) );
		if ( wipe ) {

			size_t length;
			JL_CHK( BlobLength(cx, obj, &length) );
			memset(pv, 0, length);
		}
	}

	JS_free(cx, pv);
	JL_SetPrivate(cx, obj, NULL); // InvalidateBlob(cx, obj)
	JL_CHK( JL_SetReservedSlot(cx, JL_OBJ, SLOT_BLOB_JSSTRING, JSVAL_VOID) );

	// removes all of obj's own properties, except the special __proto__ and __parent__ properties, in a single operation.
	// Properties belonging to objects on obj's prototype chain are not affected.
	JS_ClearScope(cx, obj);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Blob $INAME( data [,data1 [,...]] )
  Combines the data of two or more blobs and returns a new blob.
  $H details
   [http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/concat Mozilla]
**/
DEFINE_FUNCTION( concat ) {

	JL_DEFINE_FUNCTION_OBJ;
	char *dst = NULL;
	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS);
	if (unlikely( !IsBlobValid(cx, obj) ))
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

	// note: var a = new String(123);  a.concat() !== a

	size_t thisLength;
	const char *thisBuffer;
	JL_CHK( BlobBuffer(cx, obj, &thisBuffer) );
	JL_CHK( BlobLength(cx, obj, &thisLength) );

	size_t dstLen;
	dstLen = thisLength;

	uintN arg;
	for ( arg = 1; arg <= JL_ARGC; arg++ ) {
	
		if ( JL_JsvalIsBlob(cx, JL_ARG(arg)) ) {

			size_t tmp;
			JL_CHK( BlobLength(cx, JSVAL_TO_OBJECT( JL_ARG(arg) ), &tmp) );
			dstLen += tmp;
		
		} else {

			JSString *jsstr = JS_ValueToString(cx, JL_ARG(arg));
			JL_ARG(arg) = STRING_TO_JSVAL(jsstr);
			dstLen += JL_GetStringLength(jsstr);
		}
	}

	dst = (char*)JS_malloc(cx, dstLen +1);
	JL_CHK( dst );

	char *tmp;
	tmp = dst;

	if ( thisLength > 0 ) {

		memcpy(tmp, thisBuffer, thisLength);
		tmp += thisLength;
	}

	for ( arg = 1; arg <= JL_ARGC; arg++ ) {
		
//		const char *buffer;
//		size_t length;
//		JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(arg), &buffer, &length) );

		JLStr str;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(arg), &str) );
		memcpy(tmp, str.GetConstStr(), str.Length());
		tmp += str.Length();
	}

	dst[dstLen] = '\0';
	JL_CHK( JL_NewBlob(cx, dst, dstLen, JL_RVAL) );
	return JS_TRUE;

bad:
	if ( dst )
		JS_free(cx, dst);
	return JS_FALSE;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Blob $INAME( start [, length ] )
  Returns the bytes in a blob beginning at the specified location through the specified number of characters.
  $H arguments
   $ARG $INT start: location at which to begin extracting characters (an integer between 0 and one less than the length of the blob).
   $ARG $INT length: the number of characters to extract.
  $H details
   fc. [http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/substr Mozilla]
**/

DEFINE_FUNCTION( substr ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS);
	if (unlikely( !IsBlobValid(cx, obj) ))
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

	const char *buffer;
	JL_CHK( BlobBuffer(cx, obj, &buffer) );

	double length;
	size_t tmp;
	JL_CHK( BlobLength(cx, obj, &tmp) );
	length = tmp;

	if ( JL_ARGC == 0 )
		return JL_NewBlobCopyN(cx, buffer, size_t(length), JL_RVAL);

	double begin, end;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &begin) );
	begin = js__DoubleToInteger(begin);

	if ( begin < 0 ) {
		
		begin += length;
		if ( begin < 0 )
			begin = 0;
	} else if ( begin > ssize_t(length) ) {
		
		begin = length;
	}

	if ( !JL_ARG_ISDEF(2) ) { // -or- if ( JL_ARGC == 1 ) {
		
		end = length;
	} else {
		
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &end) );
		end = js__DoubleToInteger(end);

		if ( end < 0 )
			end = 0;

      end += begin;
      if ( end > ssize_t(length) )
          end = length;
	}

	return JL_NewBlobCopyN(cx, buffer + size_t(begin), size_t(end - begin), JL_RVAL);
	JL_BAD;
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
DEFINE_FUNCTION( substring ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS);
	if (unlikely( !IsBlobValid(cx, obj) ))
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

	const char *buffer;
	JL_CHK( BlobBuffer(cx, obj, &buffer) );

	double length;
	size_t tmp;
	JL_CHK( BlobLength(cx, obj, &tmp) );
	length = tmp;

	if ( JL_ARGC == 0 )
		return JL_NewBlobCopyN(cx, buffer, size_t(length), JL_RVAL);

	double begin, end;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &begin) );
	begin = js__DoubleToInteger(begin);

	if ( !JL_ARG_ISDEF(2) ) {
		
		end = length;
	} else {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &end) );
		end = js__DoubleToInteger(end);
	}

    if (begin < 0)
        begin = 0;
    else if (begin > length)
        begin = length;

    if (end < 0)
        end = 0;
    else if (end > length)
        end = length;
    if (end < begin) {
        /* ECMA emulates old JDK1.0 java.lang.String.substring. */
        jsdouble tmp = begin;
        begin = end;
        end = tmp;
    }

	return JL_NewBlobCopyN(cx, buffer + size_t(begin), size_t(end - begin), JL_RVAL);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( searchValue [, fromIndex] )
  Returns the index within the calling Blob object of the first occurrence of the specified value, starting the search at fromIndex, or -1 if the value is not found.
  $H arguments
   $ARG $STR searchValue: A string representing the value to search for.
   $ARG $INT fromIndex: The location within the calling blob to start the search from. It can be any integer between 0 and the length of the blob. The default value is 0.
  $H details
   fc. [http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/indexOf Mozilla]
**/
DEFINE_FUNCTION( indexOf ) {

	JLStr str;

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS);
	if (unlikely( !IsBlobValid(cx, obj) ))
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);
	
	if (JL_ARGC == 0)
		return JL_NativeToJsval(cx, -1, JL_RVAL);

	const char *text, *pat;
	size_t textlen, patlen;

	JL_CHK( BlobBuffer(cx, obj, &text) );
	JL_CHK( BlobLength(cx, obj, &textlen) );

//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &pat, &patlen) );
	
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	patlen = str.Length();
	pat = str.GetConstStr();


	jsuint start;
	if (JL_ARGC > 1) {

		double d;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &d) );
		d = js__DoubleToInteger(d);
		if (d <= 0) {
			start = 0;
		} else if (d > textlen) {
			start = 0;
			textlen = 0;
		} else {
			start = (jsint)d;
			text += start;
			textlen -= start;
		}
    } else {

        start = 0;
    }

    if (patlen == 0)
		 return JL_NativeToJsval(cx, 0, JL_RVAL);

    if (textlen < patlen)
        return JL_NativeToJsval(cx, -1, JL_RVAL);

	ssize_t match = Match(text, textlen, pat, patlen);
	if ( match == -1 )
		*JL_RVAL = INT_TO_JSVAL(-1);
	else 
		JL_CHK( JL_NativeToJsval(cx, start + match, JL_RVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( searchValue [, fromIndex] )
  Returns the index within the calling Blob object of the last occurrence of the specified value, or -1 if not found. The calling blob is searched backward, starting at fromIndex.
  $H arguments
   $ARG $STR searchValue: A string representing the value to search for.
   $ARG $INT fromIndex: The location within the calling blob to start the search from, indexed from left to right. It can be any integer between 0 and the length of the blob. The default value is the length of the blob.
  $H details
   fc. [http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/lastIndexOf Mozilla]
**/
DEFINE_FUNCTION( lastIndexOf ) {

	JLStr str;

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS);
	if (unlikely( !IsBlobValid(cx, obj) ))
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

	if (JL_ARGC == 0)
		return JL_NativeToJsval(cx, -1, JL_RVAL);

	const char *text, *pat;
	ssize_t i;
	size_t textlen, patlen;

	JL_CHK( BlobBuffer(cx, obj, &text) );
	JL_CHK( BlobLength(cx, obj, &textlen) );

//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &pat, &patlen) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	patlen = str.Length();
	pat = str.GetConstStr();

	i = textlen - patlen; // Start searching here
	if (i < 0) {
		 
		*JL_RVAL = INT_TO_JSVAL(-1);
		return JS_TRUE;
	}

	if (JL_ARGC > 1) {
		
		double d;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &d) );
		if (!isnan(d)) {

			d = js__DoubleToInteger(d);
			if (d <= 0)
				i = 0;
			else if (d < i)
				i = (jsint)d;
		}
	}

	if (patlen == 0)
		return JL_NativeToJsval(cx, i, JL_RVAL);

	const char *t = text + i;
	const char *textend = text - 1;
	const char p0 = *pat;
	const char *patNext = pat + 1;
	const char *patEnd = pat + patlen;

	for (; t != textend; --t) {
		if (*t == p0) {
			const char *t1 = t + 1;
			for (const char *p1 = patNext; p1 != patEnd; ++p1, ++t1) {
				if (*t1 != *p1)
					goto break_continue;
			}
			return JL_NativeToJsval(cx, t - text, JL_RVAL);
		}
		break_continue:;
	}

	*JL_RVAL = INT_TO_JSVAL(-1);
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME( [separator][, limit] )
  Splits a Blob object into an array of strings by separating the blob into substrings
  $H arguments
   $ARG $STR separator: Specifies the character to use for separating the string. The separator is treated as a string. If separator is omitted, the array returned contains one element consisting of the entire string.
   $ARG $INT limit: Integer specifying a limit on the number of splits to be found.
  $H details
   fc. [http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/split Mozilla]
**/
DEFINE_FUNCTION( split ) {

	JLStr str;

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS);
	if (unlikely( !IsBlobValid(cx, obj) ))
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

	size_t blobLen;
	const char *blob;
	JL_CHK( BlobLength(cx, obj, &blobLen) );
	JL_CHK( BlobBuffer(cx, obj, &blob) );

	JSObject *arr;
	arr = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK( arr );
	*JL_RVAL = OBJECT_TO_JSVAL(arr);
	jsuint arrLen;
	arrLen = 0;

	jsval chunk;

	if ( JL_ARGC == 0 || JL_ARGC == 1 && JSVAL_IS_VOID(JL_ARG(1)) ) {

		//JL_CHK( JL_StringAndLengthToJsval(cx, &chunk, blob, blobLen) );
		JL_CHK( JL_NativeToJsval(cx, blob, blobLen, &chunk) );

		JL_CHK( JS_SetElement(cx, arr, 0, &chunk) );
		JL_CHK( JS_SetArrayLength(cx, arr, 1) );
		return JS_TRUE;
	}

	JL_S_ASSERT_STRING( JL_ARG(1) ); // regexp is not supported

	size_t max;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &max) );
	else
		max = SIZE_MAX;

	const char *sep;
	size_t sepLen;
	//JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &sep, &sepLen) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	sepLen = str.Length();
	sep = str.GetConstStr();

	ssize_t pos;

	for (;;) {

		if ( !max-- )
			break;

		if ( blobLen < sepLen ) {

			//JL_CHK( JL_StringAndLengthToJsval(cx, &chunk, blob, blobLen) );
			JL_CHK( JL_NativeToJsval(cx, blob, blobLen, &chunk) );
			JL_CHK( JS_SetElement(cx, arr, arrLen++, &chunk) );
			break;
		}

		if ( sepLen )
			pos = Match(blob, blobLen, sep, sepLen);
		else 
			pos = blobLen > 1 ? 1 : -1;

		if ( pos == -1 ) {

			//JL_CHK( JL_StringAndLengthToJsval(cx, &chunk, blob, blobLen) );
			JL_CHK( JL_NativeToJsval(cx, blob, blobLen, &chunk) );
			JL_CHK( JS_SetElement(cx, arr, arrLen++, &chunk) );
			break;
		}
		
		//JL_CHK( JL_StringAndLengthToJsval(cx, &chunk, blob, pos) );
		JL_CHK( JL_NativeToJsval(cx, blob, pos, &chunk) );
		JL_CHK( JS_SetElement(cx, arr, arrLen++, &chunk) );
		
		blob += pos + sepLen;
		blobLen -= pos + sepLen;
	}


	JL_CHK( JS_SetArrayLength(cx, arr, arrLen) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( index )
  Returns the specified character from a blob.
  $H details
   fc. [http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/charAt Mozilla]
**/
DEFINE_FUNCTION( charAt ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS);
	if (unlikely( !IsBlobValid(cx, obj) ))
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

    jsdouble d;

    if ( JL_ARG_ISDEF(1) ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &d) );
        d = js__DoubleToInteger(d);
    } else {

		d = 0.0;
    }

	size_t length;
	JL_CHK( BlobLength(cx, obj, &length) );

	if ( d < 0 || length <= d ) {

		*JL_RVAL = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	const char *buffer;
	JL_CHK( BlobBuffer(cx, obj, &buffer) );

	jschar chr;
	chr = buffer[size_t(d)];

	JSString *str;
	str = JS_NewUCStringCopyN(cx, &chr, 1);
	JL_CHK( str );
	*JL_RVAL = STRING_TO_JSVAL(str);

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
DEFINE_FUNCTION( charCodeAt ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS);
	if (unlikely( !IsBlobValid(cx, obj) ))
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

    jsdouble d;

    if ( JL_ARG_ISDEF(1) ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &d) );
        d = js__DoubleToInteger(d);
    } else {

		d = 0.0;
    }

	size_t length;
	JL_CHK( BlobLength(cx, obj, &length) );

	if ( d < 0 || length <= d ) {

		*JL_RVAL = JL_GetNaNValue(cx);
		return JS_TRUE;
	}

	const char *buffer;
	JL_CHK( BlobBuffer(cx, obj, &buffer) );

	*JL_RVAL = INT_TO_JSVAL( buffer[size_t(d)] );

	return JS_TRUE;
	JL_BAD;
}



DEFINE_FUNCTION( toSource ) {

	// (TBD) try something faster !!
	JL_DEFINE_FUNCTION_OBJ;

	if (unlikely( !IsBlobValid(cx, obj) ))
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

	if ( obj == JL_PROTOTYPE(cx, Blob) )
		*JL_RVAL = JL_GetEmptyStringValue(cx);
	else
		*JL_RVAL = STRING_TO_JSVAL( JS_ValueToString(cx, OBJECT_TO_JSVAL( obj )) );
	*JL_RVAL = STRING_TO_JSVAL( JS_ValueToSource(cx, *JL_RVAL) );
	return JS_TRUE;
	JL_BAD;
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

	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS)
	if (unlikely( !IsBlobValid(cx, obj) ))
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

	size_t length;
	JL_CHK( BlobLength(cx, obj, &length) );
	return JL_NativeToJsval(cx, length, vp);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE char $INAME $READONLY
  Used to access the character in the _N_th position where _N_ is a positive integer between 0 and one less than the value of length.
**/
DEFINE_GET_PROPERTY() {

	JL_S_ASSERT_THIS_CLASS();
	if (unlikely( !IsBlobValid(cx, obj) ))
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

	if ( !JSID_IS_INT(id) )
		return JS_TRUE;

	size_t length;
	const char *buffer;
	JL_CHK( BlobLength(cx, obj, &length) );
	JL_CHK( BlobBuffer(cx, obj, &buffer) );

	jsint slot = JSID_TO_INT(id);
	if ( slot < 0 || size_t(slot) >= length )
		return JS_TRUE;

	jschar chr;
	chr = buffer[slot];
	JSString *str1;
	str1 = JS_NewUCStringCopyN(cx, &chr, 1);
	JL_CHK( str1 );

	*vp = STRING_TO_JSVAL(str1);

	return JS_TRUE;
	JL_BAD;
}


DEFINE_SET_PROPERTY() {

	if ( JSID_IS_INT(id) )
		JL_REPORT_WARNING_NUM(cx, JLSMSG_IMMUTABLE_OBJECT, JL_THIS_CLASS->name); // see also JSMSG_READ_ONLY
	return JS_TRUE;
	JL_BAD;
}


DEFINE_EQUALITY_OP() {

	JL_CHKB( JL_IsClass(*v, JL_THIS_CLASS), not_eq );

	if ( !IsBlobValid(cx, obj) || !IsBlobValid(cx, &js::Valueify(v)->toObject()) )
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);
	
	const char *buf1, *buf2;
	size_t len1, len2;
	JL_CHK( BlobLength(cx, obj, &len1) );
	JL_CHK( BlobLength(cx, js::Valueify(v)->toObjectOrNull(), &len2) );
	JL_CHKB( len1 == len2, not_eq );
	JL_CHK( BlobBuffer(cx, obj, &buf1) );
	JL_CHK( BlobBuffer(cx, js::Valueify(v)->toObjectOrNull(), &buf2) );
	JL_CHKB( memcmp(buf1, buf2, len1) == 0, not_eq );
	*bp = JS_TRUE;
	return JS_TRUE;

not_eq:
	*bp = JS_FALSE;
	return JS_TRUE;
	JL_BAD;
}



JSBool next_for(JSContext *cx, uintN argc, jsval *vp) {

	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	jsval tmp;
	JL_CHK( JS_GetPropertyById(cx, JS_THIS_OBJECT(cx, vp), INT_TO_JSID(0), &tmp) );
	JSObject *blobObj = JSVAL_TO_OBJECT(tmp);
	if ( !IsBlobValid(cx, blobObj) )
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

	JL_CHK( JS_GetPropertyById(cx, JS_THIS_OBJECT(cx, vp), INT_TO_JSID(1), &tmp) );
	size_t pos = JSVAL_TO_INT(tmp);
	size_t len;
	JL_CHK( BlobLength(cx, blobObj, &len) );
	if ( pos >= len )
		return JS_ThrowStopIteration(cx);

	*vp = INT_TO_JSVAL(pos);

	tmp = INT_TO_JSVAL(pos+1);
	JL_CHK( JS_SetPropertyById(cx, obj, INT_TO_JSID(1), &tmp) );

	return JS_TRUE;
	JL_BAD;
}


JSBool next_foreach(JSContext *cx, uintN argc, jsval *vp) {

	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	jsval tmp;
	JL_CHK( JS_GetPropertyById(cx, JS_THIS_OBJECT(cx, vp), INT_TO_JSID(0), &tmp) );
	JSObject *blobObj = JSVAL_TO_OBJECT(tmp);
	if ( !IsBlobValid(cx, blobObj) )
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

	JL_CHK( JS_GetPropertyById(cx, JS_THIS_OBJECT(cx, vp), INT_TO_JSID(1), &tmp) );
	size_t pos = JSVAL_TO_INT(tmp);
	size_t len;
	JL_CHK( BlobLength(cx, blobObj, &len) );
	if ( pos >= len )
		return JS_ThrowStopIteration(cx);

	const char *buf;
	JL_CHK( BlobBuffer(cx, blobObj, &buf) );

	jschar chr;
	chr = buf[pos];
	JSString *str;
	str = JS_NewUCStringCopyN(cx, &chr, 1);
	JL_CHK( str );
	*vp = STRING_TO_JSVAL(str);

	tmp = INT_TO_JSVAL(pos+1);
	JL_CHK( JS_SetPropertyById(cx, obj, INT_TO_JSID(1), &tmp) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_ITERATOR_OBJECT() {

	JSObject *itObj = JS_NewObjectWithGivenProto(cx, NULL, NULL, NULL);
	JL_CHK( itObj );
	JL_CHK( JS_DefineFunctionById(cx, itObj, JL_ATOMJSID(cx, next), keysonly ? next_for : next_foreach, 0, 0) );

	jsval v;
	v = OBJECT_TO_JSVAL(obj);
	JL_CHK( JS_SetPropertyById(cx, itObj, INT_TO_JSID(0), &v) );
	v = INT_TO_JSVAL(0);
	JL_CHK( JS_SetPropertyById(cx, itObj, INT_TO_JSID(1), &v) );
	return itObj;
bad:
	return NULL;
}



JSBool GetBlobString( JSContext *cx, JSObject *obj, jsval *str ) {

	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_BLOB_JSSTRING, str) );
	if ( JSVAL_IS_VOID(*str) ) {

		JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS);

		if (unlikely( !IsBlobValid(cx, obj) ))
			JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

		size_t length;
		JL_CHK( BlobLength(cx, obj, &length) );

		if ( length == 0 ) {

			*str = JL_GetEmptyStringValue(cx);

		} else {

			const char *buffer;
			JL_CHK( BlobBuffer(cx, obj, &buffer) );
			JSString *jsstr;
			jsstr = JS_NewStringCopyN(cx, buffer, length);
			JL_CHK( jsstr );
			*str = STRING_TO_JSVAL(jsstr);
		}

		JL_CHK( JL_SetReservedSlot(cx, JL_OBJ, SLOT_BLOB_JSSTRING, *str) );
	}
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
DEFINE_FUNCTION( toString ) { // and valueOf ?

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS);

	JL_CHK( GetBlobString(cx, obj, JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}



/*
DEFINE_NEW_RESOLVE() {

	JSObject *blobProto = JL_PROTOTYPE(cx, Blob);

	// check if obj is a Blob
	if ( JS_GetPrototype(cx, obj) != blobProto )
		return JS_TRUE;

	if ( !IsBlobValid(cx, obj) )
		JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALIDATED_OBJECT, JL_CLASS(Blob)->name);

	if ( !(flags & JSRESOLVE_QUALIFIED) || (flags & JSRESOLVE_ASSIGNING) )
		return JS_TRUE;

	// search id in Blob's prototype
	JSProperty *prop;
	JL_CHK( blobProto->lookupProperty(cx, id, objp, &prop) );
	if ( prop )
		return JS_TRUE;

	// search id in String's prototype.
	JSObject *stringProto;
	JL_CHK( JS_GetClassObject(cx, JS_GetGlobalObject(cx), JSProto_String, &stringProto) );

	JL_CHK( stringProto->lookupProperty(cx, id, objp, &prop) );
	if ( !prop )
		return JS_TRUE;

	return JS_TRUE;
	JL_BAD;
}

DEFINE_OPS_LOOKUP_PROPERTY() {

	return JS_TRUE;
}

const jschar *ToString(JSContext *cx, jsval val) {
	
	JSString *str = JS_ValueToString(cx, val);
	return JS_GetStringCharsZ(cx, str);
}

const jschar *ToString(JSContext *cx, jsid id) {
	
	jsval val;
	JS_IdToValue(cx, id, &val);
	return ToString(cx, val);
}
*/


DEFINE_FUNCTION( valueOf ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS);
	JL_CHK( GetBlobString(cx, obj, vp) );
	return JS_TRUE;
	JL_BAD;
}

/*

DEFINE_OPS_GET_PROPERTY() {

//	if ( id == ATOM_TO_JSID(cx->runtime->atomState.constructorAtom) ) {

//		JSPropertyDescriptor desc;
//		JS_GetPropertyDescriptorById(cx, obj, id, JSRESOLVE_QUALIFIED, &desc);

	JSObject *blobProto = JL_PROTOTYPE(cx, Blob);

//	wprintf(ToString(cx, id));

	// search id in Blob's prototype

	JS_LookupPropertyById(cx, obj, id, js::Jsvalify(vp));

	if ( vp->isUndefined() ) {

		GetBlobString(cx, obj, js::Jsvalify(vp));
		JSObject *stringProto;
		JS_GetClassObject(cx, JS_GetGlobalObject(cx), JSProto_String, &stringProto);

		JS_LookupPropertyById(cx, stringProto, id, js::Jsvalify(vp));
//		JS_DefineFunction(cx, obj, "toString", _valueOf, 0, 0);

		JS_SetPrototype(cx, obj, stringProto);

	}

	return JS_TRUE;
}
*/




DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT( jl::JsvalIsSerializer(cx, JL_ARG(1)), "Invalid serializer object." );
	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	if ( IsBlobValid(cx, JL_OBJ) ) {

		size_t length;
		const char *buf;
		JL_CHK( BlobLength(cx, JL_OBJ, &length) );
		JL_CHK( BlobBuffer(cx, JL_OBJ, &buf) );
		JL_CHK( ser->Write(cx, true) ); // valid
		JL_CHK( ser->Write(cx, jl::SerializerConstBufferInfo(buf, length)) );
		JL_CHK( ser->Write(cx, jl::SerializerObjectOwnProperties(JL_OBJ)) );
	} else {
		
		JL_CHK( ser->Write(cx, false) ); // invalid
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT( jl::JsvalIsUnserializer(cx, JL_ARG(1)), "Invalid unserializer object." );
	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	bool validity;
	JL_CHK( unser->Read(cx, validity) );

	if ( validity ) {
		
		jl::SerializerConstBufferInfo buf;
		JL_CHK( unser->Read(cx, buf) );

		void *dBuffer;
		dBuffer = JS_malloc(cx, buf.Length() +1);
		JL_CHK( dBuffer );
		memcpy(dBuffer, buf.Data(), buf.Length());
		
		//JL_NullTerminate(dBuffer, buf.Length());
		((char*)dBuffer)[buf.Length()] = '\0';


		JL_SetPrivate(cx, obj, dBuffer);

		JL_CHK( JL_NativeToJsval(cx, buf.Length(), JL_RVAL) );
		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_BLOB_LENGTH, *JL_RVAL) );

		JL_CHK( unser->Read(cx, jl::SerializerObjectOwnProperties(obj)) );
	} else {

		JL_SetPrivate(cx, obj, NULL);
	}

	return JS_TRUE;
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

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2)
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_GET_PROPERTY
	HAS_SET_PROPERTY
	HAS_EQUALITY_OP
	HAS_ITERATOR_OBJECT

//	HAS_OPS_LOOKUP_PROPERTY
//	HAS_OPS_GET_PROPERTY

//	HAS_NEW_RESOLVE
//	HAS_NEW_RESOLVE_GETS_START

	BEGIN_FUNCTION_SPEC

		FUNCTION(Free)
		FUNCTION(concat)
		FUNCTION_ARGC(substr, 2)
		FUNCTION_ARGC(substring, 2)
		FUNCTION(indexOf)
		FUNCTION(lastIndexOf)
		FUNCTION(split)

		FUNCTION(charCodeAt)
		FUNCTION(charAt)
		FUNCTION(toString)
		FUNCTION_ALIAS(valueOf, toString)
		FUNCTION(toSource)

		FUNCTION_ARGC(_serialize, 1)
		FUNCTION_ARGC(_unserialize, 1)

	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(length)
	END_PROPERTY_SPEC

END_CLASS
