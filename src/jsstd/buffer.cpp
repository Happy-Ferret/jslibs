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

//
// RULES:
// - the buffer MUST NOT contains an empty chunk ( else memory leak during buffer life time )
// - reading a greater amount that the buffer length make the *source* object (argument of the constructor) to be used to get data until the size of the buffer do not grows any more.
// - reading only one chunk of data ( with .read(undefined); ) on an empty buffer just make one try to read data through the *source* object (argument of the constructor)
//

#include "stdafx.h"

#include "buffer.h" // beware: this is not ../common/buffer.h !

#define SLOT_SOURCE 0

DECLARE_CLASS( Buffer )


struct BufferPrivate {

	size_t length;
	jl::Queue *queue;
};


JSBool NativeInterfaceStreamRead( JSContext *cx, JSObject *obj, char *buf, size_t *amount ) {

	return ReadRawDataAmount(cx, obj, amount, buf);
}


inline JSBool PushJsval( JSContext *cx, jl::Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)JS_malloc(cx, sizeof(jsval));
	JL_CHK( pItem );
	*pItem = value;
	jl::QueuePush( queue, pItem ); // no need to JS_AddRoot *pItem, see Tracer callback
	return JS_TRUE;
	JL_BAD;
}


inline JSBool UnshiftJsval( JSContext *cx, jl::Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)JS_malloc(cx, sizeof(jsval));
	JL_CHK( pItem );
	*pItem = value;
	jl::QueueUnshift( queue, pItem ); // no need to JS_AddRoot *pItem, see Tracer callback
	return JS_TRUE;
	JL_BAD;
}


inline JSBool ShiftJsval( JSContext *cx, jl::Queue *queue, jsval *value ) {

	jsval *pItem = (jsval*)QueueShift(queue);  // no need to JS_RemoveRoot *pItem, see Tracer callback
	if ( value != NULL )
		*value = *pItem;
	JS_free(cx, pItem);
	return JS_TRUE;
}

inline JSBool PeekJsval( JSContext *cx, jl::QueueCell *cell, jsval *value ) {

	JL_IGNORE(cx);

	*value = *(jsval*)QueueGetData(cell);  // no need to JS_RemoveRoot *pItem, see Tracer callback
	return JS_TRUE;
}


JSBool WriteDataChunk( JSContext *cx, JSObject *obj, jsval chunk ) {

	JLData str;

	BufferPrivate *pv;
	pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_OBJECT_STATE( pv, JL_CLASS_NAME(Buffer) );

	//if ( !JSVAL_IS_STRING(chunk) && !JL_JsvalIsBlob(cx, chunk) && !( !JSVAL_IS_PRIMITIVE(chunk) && JL_ObjectIsString(cx, JSVAL_TO_OBJECT(chunk)) ) ) {
	if ( !JL_ValueIsData(cx, chunk) ) {

		JSString *jsstr = JS_ValueToString(cx, chunk);
		JL_ASSERT( jsstr != NULL, E_VALUE, E_CONVERT, E_TY_STRING );
		chunk = STRING_TO_JSVAL(jsstr);
	}
	// here, chunk is an immutable data container ( string, JSString, Blob, ... ) excluding BufferGetInterface.

//	size_t strLen;
//	JL_CHK( JL_JsvalToStringLength(cx, chunk, &strLen) );
	JL_CHK( JL_JsvalToNative(cx, chunk, &str) );

//	JL_CHK( JL_JsvalToStringLength(cx, chunk, &strLen) );
	if ( str.Length() == 0 ) // optimization & RULES
		return JS_TRUE;
	JL_CHK( PushJsval(cx, pv->queue, chunk) );
	pv->length += str.Length();
	return JS_TRUE;
	JL_BAD;
}


JSBool WriteRawDataChunk( JSContext *cx, JSObject *obj, size_t amount, const char *str ) {

	BufferPrivate *pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_OBJECT_STATE( pv, JL_CLASS_NAME(Buffer) );
	jsval bstr;
	JL_CHK( JL_NewBufferCopyN(cx, str, amount, &bstr) );
	JL_CHK( PushJsval(cx, pv->queue, bstr) );
	pv->length += amount;
	return JS_TRUE;
	JL_BAD;
}


JSBool UnReadDataChunk( JSContext *cx, JSObject *obj, jsval chunk ) {

	JLData str;

	BufferPrivate *pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_OBJECT_STATE( pv, JL_CLASS_NAME(Buffer) );

	//if ( !JSVAL_IS_STRING(chunk) && !JL_JsvalIsBlob(cx, chunk) && !(!JSVAL_IS_PRIMITIVE(chunk) && JL_ObjectIsString(cx, JSVAL_TO_OBJECT(chunk))) ) {
	if ( !JL_ValueIsData(cx, chunk) ) {


		JSString *jsstr = JS_ValueToString(cx, chunk);
		JL_ASSERT( jsstr != NULL, E_VALUE, E_CONVERT, E_TY_STRING );
		chunk = STRING_TO_JSVAL(jsstr);
	}
	// here, chunk is an immutable data container ( string, JSString, Blob, ... ) excluding BufferGetInterface.

	//size_t strLen;
	//JL_CHK( JL_JsvalToStringLength(cx, chunk, &strLen) );
	JL_CHK( JL_JsvalToNative(cx, chunk, &str) );
	if ( str.Length() == 0 ) // optimization && RULES
		return JS_TRUE;
	JL_CHK( UnshiftJsval(cx, pv->queue, chunk) );
	pv->length += str.Length();
	return JS_TRUE;
	JL_BAD;
}




/*
inline JSBool BufferRefill( JSContext *cx, JSObject *obj, size_t amount ) { // amount = total needed amount

	BufferPrivate *pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	jsval rval, fctVal;
	unsigned int bufferLengthBeforeRefillRequest;
	JL_CHK( JS_GetProperty(cx, obj, "onunderflow", &fctVal) );
	if ( !JsvalIsCallable(cx, fctVal) )
		return JS_TRUE; // cannot refil, onunderflow is not defined
	do {

		bufferLengthBeforeRefillRequest = pv->length;
		jsval argv[] = { OBJECT_TO_JSVAL(obj), INT_TO_JSVAL(amount - pv->length) }; // missing amount of data to complete the request at once.
		JL_CHK( JS_CallFunctionValue(cx, obj, fctVal, sizeof(argv)/sizeof(*argv), argv, &rval) );
	} while( pv->length < amount && pv->length > bufferLengthBeforeRefillRequest ); // see RULES ( at the top of this file )
	return JS_TRUE;
}
*/

inline JSBool BufferRefill( JSContext *cx, JSObject *obj, size_t amount ) { // amount = total needed amount

	char *buf;
	BufferPrivate *pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_OBJECT_STATE( pv, JL_CLASS_NAME(Buffer) );

	jsval srcVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_SOURCE, &srcVal) );

	if ( JSVAL_IS_VOID( srcVal ) ) // no source for refill
		return JS_TRUE;

	JL_ASSERT_IS_OBJECT(srcVal, "source");

	JSObject *srcObj;
	srcObj = JSVAL_TO_OBJECT( srcVal );

	NIStreamRead nisr;
	nisr = StreamReadInterface(cx, srcObj); // get native or non-native StreamRead function
	JL_ASSERT( nisr != NULL, E_OBJ, E_NAME("source"), E_INVALID );

	size_t len, prevBufferLength;

	do {

		prevBufferLength = pv->length;
		len = amount - pv->length;

		buf = (char*)JS_malloc(cx, len);
		JL_CHK( buf );

		JL_CHKB( nisr(cx, srcObj, buf, &len), bad_freebuf );
		if ( len > 0 )
			JL_CHKB( WriteRawDataChunk(cx, obj, len, buf), bad_freebuf );
		JS_free(cx, buf);

	} while( pv->length < amount && pv->length > prevBufferLength ); // see RULES ( at the top of this file )

	return JS_TRUE;

bad_freebuf:
	JS_free(cx, buf);
bad:
	return JS_FALSE;
}



JSBool UnReadRawDataChunk( JSContext *cx, JSObject *obj, char *data, size_t length ) {

	if ( length == 0 ) // optimization & RULES
		return JS_TRUE;
	jsval bstr;
	JL_CHK( JL_NewBufferCopyN(cx, data, length, &bstr) );
	JL_CHK( UnReadDataChunk(cx, obj, bstr) );
	return JS_TRUE;
	JL_BAD;
}


JSBool ReadChunk( JSContext *cx, JSObject *obj, jsval *rval ) {

	JLData str;

	BufferPrivate *pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_OBJECT_STATE( pv, JL_CLASS_NAME(Buffer) );

	if ( pv->length == 0 ) { // if buffer is empty, try to refill it.

		JL_CHK( BufferRefill(cx, obj, 1) ); // at least 1 more byte
		if ( pv->length == 0 ) { // the data are (definitively/temporarily) exhausted.

			*rval = JL_GetEmptyStringValue(cx);
			return JS_TRUE;
		}
	}

	JL_CHK( ShiftJsval(cx, pv->queue, rval) );
//	size_t len;
//	JL_CHK( JL_JsvalToStringLength(cx, *rval, &len) );
	JL_CHK( JL_JsvalToNative(cx, *rval, &str) );
	pv->length -= str.Length();
	return JS_TRUE;
	JL_BAD;
}


JSBool ReadRawDataAmount( JSContext *cx, JSObject *obj, size_t *amount, char *str ) { // this form allows one to read into a static buffer

	if ( *amount == 0 ) // optimization
		return JS_TRUE;

	BufferPrivate *pv;
	pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_OBJECT_STATE( pv, JL_CLASS_NAME(Buffer) );

	if ( pv->length < *amount )
		JL_CHK( BufferRefill(cx, obj, *amount) );

	if ( *amount == 0 ) // another optimization
		return JS_TRUE;

	char *ptr;
	ptr = str;

	*amount = JL_MIN( *amount, pv->length );

	size_t remainToRead;
	remainToRead = *amount;

	while ( remainToRead > 0 ) { // while there is something to read,

		JLData str;
		jsval item;
		JL_CHK( ShiftJsval(cx, pv->queue, &item) );

		size_t chunkLen;
		const char *chunk;
		//JL_CHK( JL_JsvalToStringAndLength(cx, &item, &chunk, &chunkLen) ); // (TBD) GC protect item ? (can be a JSString or a NIBufferGet)
		JL_CHK( JL_JsvalToNative(cx, item, &str) );
		chunkLen = str.Length();
		chunk = str.GetConstStr();

		if ( chunkLen <= remainToRead ) {

			jl_memcpy(ptr, chunk, chunkLen);
			ptr += chunkLen;
			remainToRead -= chunkLen; // adjust remaining required data length
		} else { // chunkLen > remain: this is the last chunk we have to manage. we only get a part of it chunk and we 'unread' the remaining.

			jl_memcpy(ptr, chunk, remainToRead);
			jsval bstr;
			JL_CHK( JL_NewBufferCopyN(cx, chunk + remainToRead, chunkLen - remainToRead, &bstr) );
			UnshiftJsval(cx, pv->queue, bstr);
			remainToRead = 0; // adjust remaining required data length
		}
	}
	pv->length -= *amount;
	return JS_TRUE;
	JL_BAD;
}


JSBool BufferSkipAmount( JSContext *cx, JSObject *obj, size_t *amount ) { // amount: in/out

	if ( *amount == 0 ) // optimization
		return JS_TRUE;

	BufferPrivate *pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_OBJECT_STATE( pv, JL_CLASS_NAME(Buffer) );

	if ( pv->length < *amount )
		JL_CHK( BufferRefill(cx, obj, *amount) );

	if ( pv->length < *amount ) {

		while ( !QueueIsEmpty(pv->queue) )
			JL_CHK( ShiftJsval(cx, pv->queue, NULL) );
		
		*amount -= pv->length;
		pv->length = 0;
		return JS_TRUE;
	}

	size_t remainToRead;
	remainToRead = *amount;
	while ( remainToRead > 0 ) { // while there is something to read,

		JLData str;
		jsval item;
		size_t chunkLen;
		JL_CHK( ShiftJsval(cx, pv->queue, &item) );
		//JL_CHK( JL_JsvalToStringLength(cx, item, &chunkLen) );
		JL_CHK( JL_JsvalToNative(cx, item, &str) );
		chunkLen = str.Length();

		if ( chunkLen <= remainToRead ) {

			remainToRead -= chunkLen;
		} else {

			JLData str;

			//const char *chunk;
			//JL_CHK( JL_JsvalToStringAndLength(cx, &item, &chunk, &chunkLen) );
			JL_CHK( JL_JsvalToNative(cx, item, &str) );

			jsval bstr;
			JL_CHK( JL_NewBufferCopyN(cx, str.GetConstStr() + remainToRead, str.Length() - remainToRead, &bstr) );
			UnshiftJsval(cx, pv->queue, bstr);
			remainToRead = 0;
		}
	}
	pv->length -= *amount;
	*amount = 0;
	return JS_TRUE;
	JL_BAD;
}


JSBool ReadDataAmount( JSContext *cx, JSObject *obj, size_t amount, jsval *rval ) {

	if ( amount == 0 ) { // optimization

		*rval = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	char *str;
	str = (char*)JS_malloc(cx, amount +1); // (TBD) memory leak if ReadRawAmount failed
	JL_CHK( str );

	// (TBD) IMPORTANT: here, amount should be MIN( amount, buffer_size ). This can avoid an useless memory allocation.
	size_t requestedAmount;
	requestedAmount = amount;
	JL_CHK( ReadRawDataAmount(cx, obj, &amount, str) );

	if ( amount == 0 ) { // optimization (when nothing has been read)

		JS_free(cx, str);
		*rval = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	if ( JL_MaybeRealloc(requestedAmount, amount) ) {

		str = (char*)JS_realloc(cx, str, amount +1);
		JL_CHK( str );
	}

	str[amount] = '\0'; // (TBD) explain this
	JL_CHK( JLData(str, true, amount).GetJSString(cx, rval) );
	return JS_TRUE;
	JL_BAD;
}


JSBool FindInBuffer( JSContext *cx, JSObject *obj, const char *needle, size_t needleLength, bool *found, size_t *foundAt ) {

	// (TBD) optimise this function for needleLength == 1 (eg. '\0' in a string)
	BufferPrivate *pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_OBJECT_STATE( pv, JL_CLASS_NAME(Buffer) );

	size_t chunkLength, i, j, pos;
	const char *chunk;
	pos = 0;

	char *buf, staticBuffer[128];
	buf = needleLength <= sizeof(staticBuffer) ? staticBuffer : (char*)jl_malloc(needleLength); // the "ring buffer"
	JL_ASSERT_ALLOC( buf );

	for ( jl::QueueCell *it = jl::QueueBegin(pv->queue); it; it = jl::QueueNext(it) ) {

		jsval *pNewStr = (jsval*)QueueGetData(it);
		//JL_CHK( JL_JsvalToStringAndLength(cx, pNewStr, &chunk, &chunkLength) );
		JLData str;
		JL_CHK( JL_JsvalToNative(cx, *pNewStr, &str) );
		chunkLength = str.Length();
		chunk = str.GetConstStr();

		for ( i = 0; i < chunkLength; i++ ) {

			buf[pos++ % needleLength] = chunk[i]; // store one more char of the chunk in the ring buffer
			if ( pos >= needleLength ) { // if we have enough data in the ring buffer to start the search

				for ( j = 0; j < needleLength && needle[j] == buf[(pos+j) % needleLength]; j++ ) ; // search the 'needle' starting at the right position in the ring buffer.
				if( j == needleLength ) { // if all chars of the 'needle' are found

					*found = true;
					*foundAt = pos-needleLength;
					goto end; // this is a cheap way to break all these nested loops
				}
			}
		}
	}

	*found = false;
//	*foundAt = -1;
end:
	if ( buf != staticBuffer )
		jl_free(buf); // free the "ring buffer"
	return JS_TRUE;
	JL_BAD;
}


JSBool AddBuffer( JSContext *cx, JSObject *destBuffer, JSObject *srcBuffer ) {

	JL_ASSERT_INSTANCE( destBuffer, JL_CLASS(Buffer) );
	BufferPrivate *dpv;
	dpv = (BufferPrivate*)JL_GetPrivate(cx, destBuffer);
	JL_ASSERT_OBJECT_STATE( dpv, JL_CLASS_NAME(Buffer) );

	JL_ASSERT_INSTANCE( srcBuffer, JL_CLASS(Buffer) );
	BufferPrivate *spv;
	spv = (BufferPrivate*)JL_GetPrivate(cx, srcBuffer);
	JL_ASSERT_OBJECT_STATE( spv, JL_CLASS_NAME(Buffer) );

	for ( jl::QueueCell *it = jl::QueueBegin(spv->queue); it; it = jl::QueueNext(it) )
		JL_CHK( PushJsval(cx, dpv->queue, *(jsval*)QueueGetData(it)) );
	dpv->length += spv->length;

	return JS_TRUE;
	JL_BAD;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 Buffer class is a simple buffer that allows arbitrary length input and output.
**/
BEGIN_CLASS( Buffer )

DEFINE_FINALIZE() {

	if ( JL_GetHostPrivate(cx)->canSkipCleanup )
		return;

	BufferPrivate *pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	if ( !pv )
		return;

	while ( !QueueIsEmpty(pv->queue) )
		ShiftJsval(cx, pv->queue, NULL);
	QueueDestruct(pv->queue);
	JS_free(cx, pv);
}

/* prev doc:
  If a string is given as argument, the buffer is initialized with this string.
  If buffer object is given, the buffer is initialized with this existing buffer object (kind of copy constructor).
*/

/**doc
$TOC_MEMBER $INAME
 $INAME( [source] )
  Constructs a Buffer object.
  $H arguments
   $ARG streamObject source: any object that supports NIStreamRead interface. The Buffer uses this object when its length is less than the requested amount of data.
   $H beware
    When used by the Buffer, the source MUST return the exact or less data than the required size else the remaining is lost (not stored in the buffer).
   $H note
    You can use the Write() function in the source like:
    {{{
    var buf = new Buffer({ Read:function(count) { buf.Write('some data') } });
    }}}

  $H example 1
  {{{
  var buf1 = new Buffer(new Stream('456'));
  buf1.write('123');
  print( buf1.read(6) ); // prints: '123456'
  }}}

  $H example 2
  {{{
  var buf1 = new Buffer(new Stream('123'));
  print( buf1.read(1) ,'\n'); // prints: '1'
  print( buf1.read(1) ,'\n'); // prints: '2'
  print( buf1.read(1) ,'\n'); // prints: '3'
  }}}

  $H example 3
  {{{
  var buf2 = new Buffer(new function() {

   this.read = function(count) stringRepeat('x',count);
  });
  print( buf2.read(6) ); // prints: 'xxxxxx'
  }}}

  $H example 4
   Create a long chain ( pack << buffer << buffer << stream )
  {{{
  var p = new Pack(new Buffer(new Buffer(new Stream('\x12\x34'))));
  print( (p.readInt(2, false, true)).toString(16) ); // prints: '1234'
  }}}
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_CHK( SetStreamReadInterface(cx, obj, NativeInterfaceStreamRead) );

	BufferPrivate *pv;
	pv = (BufferPrivate *)JS_malloc(cx, sizeof(BufferPrivate));
	JL_CHK( pv );
	JL_SetPrivate(cx, obj, pv);
	pv->queue = jl::QueueConstruct();
	JL_ASSERT_ALLOC(pv->queue);
	pv->length = 0;

	if ( JL_ARG_ISDEF(1) ) {

/*
		// (TBD) loop over all args
		if ( JL_IsClass(JL_ARG(1), _class) ) {

			return AddBuffer(cx, obj, JSVAL_TO_OBJECT( JL_ARG(1) ));
		} else {

			size_t length;
			JL_CHK( JL_JsvalToStringLength(cx, JL_ARG(1), &length) );
			if ( length == 0 )
				return JS_TRUE;
			pv->length += length;
			return PushJsval(cx, pv->queue, JL_ARG(1));
		}
*/

		JL_ASSERT_ARG_IS_OBJECT(1);
		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_SOURCE, JL_ARG(1)) );
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/*
DEFINE_FUNCTION( clone ) {

		// (TBD) loop over all args
		if ( JL_IsClass(JL_ARG(1), _class) ) {

			return AddBuffer(cx, obj, JSVAL_TO_OBJECT( JL_ARG(1) ));
		} else {

			size_t length;
			JL_CHK( JL_JsvalToStringLength(cx, JL_ARG(1), &length) );
			if ( length == 0 )
				return JS_TRUE;
			pv->length += length;
			return PushJsval(cx, pv->queue, JL_ARG(1));
		}

	return JS_TRUE;
	JL_BAD;
}
*/


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Empty the whole buffer.
**/
DEFINE_FUNCTION( clear ) {

	JL_IGNORE(argc);

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	BufferPrivate *pv;
	pv = (BufferPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	while ( !QueueIsEmpty(pv->queue) )
		JL_CHK( ShiftJsval(cx, pv->queue, NULL) );
	pv->length = 0;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( data [, length] )
 $VOID $INAME( buffer )
  Add _data_ in the buffer. If _length_ is used, only the first _length_ bytes of _data_ are added.
  The second form allow to add another whole buffer in the current buffer.
**/
DEFINE_FUNCTION( write ) {

	JL_DEFINE_FUNCTION_OBJ;

	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	BufferPrivate *pv;
	pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_ASSERT_ARGC_RANGE(1, 2);
	
	*JL_RVAL = JSVAL_VOID;
	jsval arg1;
	arg1 = JL_ARG(1);

// rekated to buffer markers
//	if ( JSVAL_IS_VOID(arg1) ) {
//	}

	if ( JL_ValueIsClass(arg1, JL_THIS_CLASS) ) {
		
		JL_ASSERT_ARGC(1);
		return AddBuffer(cx, obj, JSVAL_TO_OBJECT(arg1));
	}

	if ( JL_ARG_ISDEF(2) ) {

		size_t amount;
//		const char *buf;
		
		//JL_CHK( JL_JsvalToStringAndLength(cx, arg1, &buf, &strLen) ); // warning: GC on the returned buffer !
		JLData str;
		JL_CHK( JL_JsvalToNative(cx, arg1, &str) );

		if ( str.Length() == 0 )
			return JS_TRUE;

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &amount) );
		if ( amount > str.Length() )
			amount = str.Length();

		return WriteRawDataChunk(cx, obj, amount, str.GetConstStr());
	} else {

		return WriteDataChunk(cx, obj, arg1);
	}
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( str [, consume = false ] )
  Check if the given string _str_ matchs to the next data in the buffer.
  $H arguments
   $ARG $STR str
   $ARG $BOOL consume: if false, just check if it match without consuming data, else, read and check.
  $H return value
   true if it matchs, else false.
**/
DEFINE_FUNCTION( match ) {

	JLData str;

	JL_DEFINE_FUNCTION_OBJ;

	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	JL_ASSERT_ARGC_RANGE(1, 2);

//	const char *str;
	size_t len;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &str, &len) ); // warning: GC on the returned buffer !
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	len = str.Length();

	char *src;
	src = (char *)jl_malloc(len);
	JL_ASSERT_ALLOC( src );

	size_t amount;
	amount = len;
	JSBool st;
	st = ReadRawDataAmount(cx, obj, &amount, src);
	if ( st != JS_TRUE )
		goto err;

	if ( amount != len )
		*JL_RVAL = JSVAL_FALSE;
	else
		*JL_RVAL = BOOLEAN_TO_JSVAL( strncmp( str.GetConstStr(), src, len ) == 0 );

	bool consume;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &consume) );
	else
		consume = false;

	if ( !consume )
		JL_CHK( UnReadRawDataChunk(cx, obj, src, amount) );

err:
	jl_free(src);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( [ amount ] )
  Read _amount_ data in the buffer. If _amount_ is omited, The whole buffer is returned.
  $H beware
   This function returns a Blob or a string literal as empty string.
  $LF
  If _amount_ == undefined, an arbitrary (ideal) amount of data is returned. Use this when you don't know how many data you have to read.
  $H example
  {{{
  var chunk = buffer.read(undefined);
  }}}
  $H note
  The read operation never blocks, even if the requested amount of data is greater than the buffer length.
**/
DEFINE_FUNCTION( read ) { // Read( [ amount | <undefined> ] )

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	BufferPrivate *pv;
	pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	if ( JL_ARGC == 1 && JSVAL_IS_VOID( JL_ARG(1) ) ) // read the next chunk (of an unknown length) (read something as fast as possible)
		return ReadChunk(cx, obj, JL_RVAL);

	size_t amount;
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &amount) );
	else
		amount = pv->length;

	JL_ASSERT( (int)amount >= 0, E_ARG, E_NUM(1), E_MIN, E_NUM(0) );
	JL_CHK( ReadDataAmount(cx, obj, amount, JL_RVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( length )
  Skip _length_ bytes of data from the buffer. Returns how many bytes has been skiped.
**/
DEFINE_FUNCTION( skip ) { // Skip( amount )

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	BufferPrivate *pv;
	pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_ASSERT_ARGC(1);
	size_t amount;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &amount) );
	JL_ASSERT( (int)amount >= 0, E_ARG, E_NUM(1), E_MIN, E_NUM(0) );
	size_t tmp;
	tmp = amount;
	JL_CHK( BufferSkipAmount(cx, obj, &tmp) );
	return JL_NativeToJsval(cx, amount - tmp, JL_RVAL);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( boundaryString [, skip] )
  Reads the buffer until it match the _boundaryString_, else it returns $UNDEF.
  If _skip_ argument is $TRUE, the _boundaryString_ is skiped from the buffer.
**/
DEFINE_FUNCTION( readUntil ) {

	JLData str;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(1, 2);

//	const char *boundary;
//	size_t boundaryLength;

//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &boundary, &boundaryLength) ); // warning: GC on the returned buffer !
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );

	bool skip;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &skip) );
	else
		skip = true;
	bool found;
	size_t foundAt;
	JL_CHK( FindInBuffer(cx, obj, str.GetConstStr(), str.Length(), &found, &foundAt) );
	if ( found ) {

		JL_CHK( ReadDataAmount(cx, obj, foundAt, JL_RVAL) );
		if ( skip ) {

			jsval tmp;
			JL_CHK( ReadDataAmount(cx, obj, str.Length(), &tmp) ); // (TBD) optimization: skip without reading the data.
		}
	} else {

		*JL_RVAL = JSVAL_VOID;
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( string )
  Find _string_ in the buffer and returns the offset of the first letter. If not found, this function returns -1.
**/
DEFINE_FUNCTION( indexOf ) {

	JLData str;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

//	const char *boundary;
//	size_t boundaryLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &boundary, &boundaryLength) ); // warning: GC on the returned buffer !

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );

	bool found;
	size_t foundAt;
	JL_CHK( FindInBuffer(cx, JL_OBJ, str.GetConstStr(), str.Length(), &found, &foundAt) );
	*JL_RVAL = INT_TO_JSVAL(found ? foundAt : -1);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( _data_ )
  Insert _data_ at the begining of the buffer. This function can undo a read operation. The returned value is _data_.
  $H example
  {{{
  function peek(len) {

    return buffer.unread( buffer.read(len) );
  }
  }}}
**/
DEFINE_FUNCTION( unread ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

	JL_CHK( UnReadDataChunk(cx, JL_OBJ, JL_ARG(1)) );
	*JL_RVAL = JL_ARG(1);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME()
  Converts the whole content of the buffer to a string.
  $H note
   The buffer is not modified.
**/

// Note:  String( { toString:function() { return [1,2,3]} } );  throws the following error: "can't convert Object to string"
DEFINE_FUNCTION( toString ) {

	JL_IGNORE(argc);

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	BufferPrivate *pv;
	pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	if ( pv->length == 0 ) {

		*JL_RVAL = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	char *buffer;
	buffer = (char*)JS_malloc(cx, pv->length +1);
	JL_CHK( buffer );
	buffer[pv->length] = '\0';

	size_t pos;
	pos = 0;
//	while ( !QueueIsEmpty(pv->queue) ) {
	for ( jl::QueueCell *it = jl::QueueBegin(pv->queue); it; it = jl::QueueNext(it) ) {

		JLData str;
//		JL_CHK( ShiftJsval(cx, pv->queue, rval) );
		JL_CHK( PeekJsval(cx, it, JL_RVAL) );

//		const char *chunkBuf;
//		size_t chunkLen;
//		JL_CHK( JL_JsvalToStringAndLength(cx, JL_RVAL, &chunkBuf, &chunkLen) );
		JL_CHK( JL_JsvalToNative(cx, *JL_RVAL, &str) );

		jl_memcpy(buffer + pos, str.GetConstStr(), str.Length());
		pos += str.Length();
	}

	JL_CHK( JLData(buffer, true, pv->length).GetJSString(cx, JL_RVAL) );

//	pv->length = 0;


	// we have to return a real string !?
	//char *bstrBuf = (char*)JS_malloc(cx, pv->length);
	//JL_CHK( bstrBuf );
	//size_t amount = pv->length;
	//JL_CHK( ReadRawAmount(cx, obj, &amount, bstrBuf) );
	//JSObject *bstrObj = NewBlob(cx, bstrBuf, pv->length);
	//JL_ASSERT( bstrObj != NULL, "Unable to create the Blob." );
	//*JL_RVAL = OBJECT_TO_JSVAL(bstrObj);

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE char $INAME $READONLY
  Used to access the character in the _N_th position where _N_ is a positive integer between 0 and one less than the value of length.
**/
DEFINE_GET_PROPERTY() {

	JL_ASSERT_THIS_INSTANCE();
	if ( !JSID_IS_INT(id) )
		return JS_TRUE;

	long slot;
	slot = JSID_TO_INT( id );

	BufferPrivate *pv;
	pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	if ( slot >= 0 && (size_t)slot < pv->length ) {

		size_t offset = 0;

//		size_t chunkLength;
//		const char *chunk;

		for ( jl::QueueCell *it = jl::QueueBegin(pv->queue); it; it = jl::QueueNext(it) ) {

			jsval *pNewStr = (jsval*)QueueGetData(it);
//			JL_CHK( JL_JsvalToStringLength(cx, *pNewStr, &chunkLength) );
			JLData str;
			JL_CHK( JL_JsvalToNative(cx, *pNewStr, &str) );

			if ( (size_t)slot >= offset && (size_t)slot < offset + str.Length() ) {

				//JL_CHK( JL_JsvalToNative(cx, *pNewStr, &chunk) ); // items in the queue are GC protected.

				const jschar chr = str.GetConstStr()[slot - offset] & 0xff;
				JSString *str1 = JS_NewUCStringCopyN(cx, &chr, 1);
				JL_CHK( str1 );
				*vp = STRING_TO_JSVAL(str1);
				return JS_TRUE;
			}
			offset += str.Length();
		}
	}

	*vp = JL_GetEmptyStringValue(cx);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_SET_PROPERTY() {

	JL_IGNORE(vp, obj, strict);

	JL_ASSERT( !JSID_IS_INT(id), E_THISOBJ, E_OPERATION );

	return JS_TRUE;
	JL_BAD;
}




/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the current length of the buffer.
**/
DEFINE_PROPERTY_GETTER( length ) {

	JL_IGNORE(id);

	JL_ASSERT_THIS_INSTANCE();
	BufferPrivate *pv;
	pv = (BufferPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	return JL_NativeToJsval(cx, pv->length, vp);
	JL_BAD;
}


//DEFINE_PROPERTY( source ) { // do not support: delete buf.source
//
//	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_SOURCE, *vp) );
//	return JS_TRUE;
//}


DEFINE_TRACER() {

	BufferPrivate *pv = (BufferPrivate*)JL_GetPrivate(trc->context, obj);
	if ( pv )
		for ( jl::QueueCell *it = jl::QueueBegin(pv->queue); it; it = jl::QueueNext(it) )
			JS_CALL_VALUE_TRACER(trc, *(jsval*)QueueGetData(it), "jsstd/Buffer:bufferQueueItem");
}


/* previous doc:
 * *onunderflow*( bufferObject, missingAmount )
  This function is called by the Buffer when its length is less than the requested amount of data.
  $H arguments
   $ARG Buffer bufferObject: is the buffer object that caused the call.
   $ARG $INT missingAmount: is the  missing amount of data to complete the request at once.
  This function is called until the buffer size do not grow any more.
*/

/** prev doc
$TOC_MEMBER $INAME
 $OBJ *source*
  The source property can contains any NIStreamRead compatible object. The Buffer uses this object when its length is less than the requested amount of data. Any extra data returned by the NIStreamRead object is keept in the buffer and will be used at the next read operation.
  $H beware
   The .source function called by the Buffer MUST return the exact or less than the  required size else the remaining is lost (not stored in the buffer).
  $H note
   You can use the Write() function in .source like:
	{{{
   buf.source = { Read:function(count) { times++; buf.Write(toto) } };
   }}}

	$H example 1
   {{{
	var buf1 = new Buffer('123');
	buf1.source = new Stream('456');
	QA.ASSERT( buf1.length, 3, 'length' );
	print( buf2.read(6) ); // prints: '123456'
	}}}

	$H example 2
	{{{
	var buf2 = new Buffer('123');
	buf2.source = new function() {

		this.read = function(count) stringRepeat('x', count);
	}
	print( buf2.read(6) ); // prints: '123xxx'
	}}}

	$H example 3
	{{{
	var first = new Buffer();
	first.source = new Buffer();
	first.source.source = new Buffer('123');
	print( first.read(4) ); // prints: '123'
	}}}

	$H example 4
	 Create a long chain ( pack << buffer << buffer << stream )
	{{{
	var p = new Pack(new Buffer());
	p.buffer.source = new Buffer();
	p.buffer.source.source = new Stream('\x12\x34');
	print( (p.readInt(2, false, true)).toString(16) ); // prints: '1234'
	}}}
**/

/**doc
=== Native Interface ===
 * *NIStreamRead*
**/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_TRACER

	HAS_GET_PROPERTY
	HAS_SET_PROPERTY

	BEGIN_FUNCTION_SPEC
		FUNCTION(clear)
		FUNCTION(write)
		FUNCTION(match)
		FUNCTION(read)
		FUNCTION(unread)
		FUNCTION(readUntil)
		FUNCTION(indexOf)
		FUNCTION(skip)
		FUNCTION(toString) // used when the buffer has to be transformed into a javascript string
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( length )
//		PROPERTY_WRITE_STORE(source)
	END_PROPERTY_SPEC

END_CLASS


/**doc
=== example 1 ===
 {{{
 var buf = new Buffer();
 buf.Write('1234');
 buf.Write('5');
 buf.Write('');
 buf.Write('6789');
 print( buf.read() );
 }}}

=== example 2 ===
 {{{
 var buf = new Buffer();
 buf.write('0123456789');
 print( buf.read(4) );
 print( buf.read(1) );
 print( buf.read(1) );
 print( buf.read(4) );
 }}}


=== example 3 ===
 Buffered read from a stream.
 {{{
 function readFromFile() {

  print('*** read from the file\n');
  return stringRepeat('x',5);
 }

 var buf = new Buffer({ read:function() { buf.write(readFromFile()); }})

 for ( var i=0; i<15; i++ )
  print( buf.read(1), '\n' )
 }}}
**/
