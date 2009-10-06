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

#include <zlib.h>

#include "zError.h"
#include "z.h"

using namespace jl;
#include "../common/buffer.h"

// current method
enum Method {
	INFLATE,
	DEFLATE
};


/**doc
----
== jsz::Z class ==
**/

BEGIN_CLASS( Z )

struct Private {

	z_stream stream;
	int method;
	int level;
};



//static JSBool NativeInterfaceStreamRead( JSContext *cx, JSObject *obj, char *buf, size_t *amount ) {
//
//	return ReadRawAmount(cx, obj, amount, buf);
//}

DEFINE_FINALIZE() {
	
	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	if ( !pv )
		return;
	
	if ( pv->stream.state != Z_NULL ) {
		
		if ( pv->method == DEFLATE )
			deflateEnd(&pv->stream);
		else
			inflateEnd(&pv->stream);
	}
	JS_free(cx, pv);
}


/**doc
$TOC_MEMBER $INAME
 *_Constructor_*( method [, compressionLevel ] )
  Constructs a new inflater or deflater object.
  $LF
  The _method_ can be Z.DEFLATE to compress data or Z.INFLATE to decompress data.
  The compression level is like this: 0 <= _compressionLevel_ <= 9.
  $H note
   For Z.INFLATE method, _compressionLevel_ argument is useless ( no sense )
  $H example
  {{{
  var compress = new Z( Z.DEFLATE, 9 );
  }}}
**/


voidpf jsz_alloc(voidpf opaque, uInt items, uInt size) {
	
	return jl_malloc(items*size);
}

void jsz_free(voidpf opaque, voidpf address) {
	
	jl_free(address);
}

DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_ARG_MIN(1);

	Private *pv;
	pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK(pv);

	JL_SetPrivate(cx, obj, pv);
	pv->stream.state = Z_NULL; // mendatory
	
//	pv->stream.zalloc = Z_NULL;
//	pv->stream.zfree = Z_NULL;
	pv->stream.zalloc = jsz_alloc;
	pv->stream.zfree = jsz_free;

	JL_CHK( JsvalToInt(cx, JL_ARG(1), &pv->method) );

	if ( JL_ARG_ISDEF(2) ) {

		JL_S_ASSERT( pv->method == DEFLATE, "The second argument is overmuch for this method.");
		JL_CHK( JsvalToInt(cx, JL_ARG(1), &pv->level) );
		JL_S_ASSERT( pv->level >= Z_NO_COMPRESSION && pv->level <= Z_BEST_COMPRESSION, "Invalid compression level." );
	} else {

		pv->level = Z_DEFAULT_COMPRESSION; // default value
	}

	JL_CHK( ReserveStreamReadInterface(cx, obj) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME( [ inputData [, forceFinish = false ] ] )
  This function process _inputData_ as a stream.
  If _forceFinish_ is true, the _inputData_ and any buffered data are flushed to the _outputData_.
  If this function is call without any argument, All remaining data are flushed.
  Once finished, the object can be reused to process a new stream of data.
  $H example
  {{{
  var compress = new Z( Z.DEFLATE );
  var compressedData = compress( 'Hello ');
  compressedData += compress( 'world' );
  compressedData += compress(); // flush
  }}}
  $H example
  {{{
  var compress = new Z( Z.DEFLATE );
  var compressedData = compress( 'Hello ');
  compressedData += compress( 'world', true ); // flush
  }}}
  $H example
  {{{
  var compressedData = new Z( Z.DEFLATE )( 'Hello world', true ); // flush
  }}}
**/

void* wrapped_JS_malloc(void * opaqueAllocatorContext, unsigned int size) {
	
	return JS_malloc((JSContext*)opaqueAllocatorContext, size);
}

void wrapped_JS_free(void * opaqueAllocatorContext, void* address) {
	
	return JS_free((JSContext*)opaqueAllocatorContext, address);
}

void* wrapped_JS_realloc(void * opaqueAllocatorContext, void* address, unsigned int size) {
	
	return JS_realloc((JSContext*)opaqueAllocatorContext, address, size);
}

DEFINE_CALL() {

	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ...
	// (TBD) check JS_InstanceOf( cx, thisObj, &NativeProc, NULL )
	JL_S_ASSERT_CLASS(thisObj, _class);

	Private *pv;
	pv = (Private*)JL_GetPrivate(cx, thisObj);
	JL_S_ASSERT_RESOURCE(pv);


// prepare input data
	const char *inputData;
	size_t inputLength;
	if ( JL_ARG_ISDEF(1) ) {

		JL_CHK( JsvalToStringAndLength(cx, &JL_ARG(1), &inputData, &inputLength) ); // warning: GC on the returned buffer !
	} else {

		inputData = NULL;
		inputLength = 0;
	}
	
// force finish
	bool forceFinish;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JsvalToBool(cx, JL_ARG(2), &forceFinish) );
	else
		forceFinish = false;

	// doc. Z_SYNC_FLUSH: all pending output is flushed to the output buffer and the output is aligned on a byte boundary, so that the decompressor can get all input data available so far.
	// doc. Z_FINISH: pending input is processed, pending output is flushed and deflate returns with Z_STREAM_END if there was enough output space; if deflate returns with Z_OK, this function must be called again with Z_FINISH and more output space.
	int flushType;
	flushType = (JL_ARGC == 0 || forceFinish) ? Z_FINISH : Z_SYNC_FLUSH;

	if ( pv->stream.state == Z_NULL ) {

		int status;
		if ( pv->method == DEFLATE )
			status = deflateInit2(&pv->stream, pv->level, Z_DEFLATED, MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
		else
			status = inflateInit2(&pv->stream, MAX_WBITS);

		if ( status < 0 )
			JL_CHK( ThrowZError(cx, status, pv->stream.msg) );

		//	JL_CHK( SetStreamReadInterface(cx, obj, NativeInterfaceStreamRead) ); (TBD) ???
	}

	pv->stream.avail_in = inputLength;
	pv->stream.next_in = (Bytef*)inputData;

	Buffer resultBuffer;
	BufferInitialize(&resultBuffer, bufferTypeAuto, bufferGrowTypeAuto, cx, wrapped_JS_malloc, wrapped_JS_realloc, wrapped_JS_free);

	// first length is a guess.
	size_t length;
	length = pv->method == DEFLATE ? (size_t)(12 + 1.001f * pv->stream.avail_in) : (size_t)(1.5f * pv->stream.avail_in); // if DEFLATE, dest. buffer must be at least 0.1% larger than sourceLen plus 12 bytes

	int xflateStatus;
	for (;;) {

//		length = JL_MAX( length, BufferGetOptimalLength(&resultBuffer) );

		pv->stream.avail_out = length;
		pv->stream.next_out = (Bytef*)BufferNewChunk(&resultBuffer, pv->stream.avail_out);
		
//		printf("D%d, ca%d ai%d ao%d ti%d to%d", method == DEFLATE, chunk->avail, stream->avail_in, stream->avail_out, stream->total_in, stream->total_out );
		xflateStatus = pv->method == DEFLATE ? deflate(&pv->stream, flushType) : inflate(&pv->stream, flushType); // Before the call of inflate()/deflate(), the application should ensure that at least one of the actions is possible, by providing more input and/or consuming more output, ...
//		printf("..ai%d ca%d ao%d ti%d to%d\n", 			method == DEFLATE, 			chunk->avail,			stream->avail_in, 			stream->avail_out, 			stream->total_in, 			stream->total_out		);

		if ( xflateStatus < Z_OK && xflateStatus != Z_BUF_ERROR ) // fatal error ?
			JL_CHKB( ThrowZError(cx, xflateStatus, pv->stream.msg), bad_freebuf );
		BufferConfirm(&resultBuffer, length - pv->stream.avail_out);
		if ( xflateStatus == Z_STREAM_END || pv->stream.avail_in == 0 )
			break;
		// doc. Z_BUF_ERROR if no progress is possible (for example avail_in or avail_out was zero). Note that Z_BUF_ERROR is not fatal, and deflate() can be called again with more input and more output space to continue compressing.
		length = pv->stream.avail_in * pv->stream.total_out / pv->stream.total_in;

//		if ( xflateStatus == Z_BUF_ERROR )
//			length = length * 2 + 4096;
	}

//	JL_CHK( StringAndLengthToJsval(cx, rval, BufferGetData(&resultBuffer), BufferGetLength(&resultBuffer)) );

	*BufferNewChunk(&resultBuffer, 1) = '\0';
	BufferConfirm(&resultBuffer, 1);
	JL_CHKB( JL_NewBlob(cx, (void*)BufferGetDataOwnership(&resultBuffer), BufferGetLength(&resultBuffer)-1, rval), bad_freebuf );
//	js_UpdateMallocCounter(cx, 1);
	BufferFinalize(&resultBuffer);

// close the stream and free resources
	if ( xflateStatus == Z_STREAM_END && flushType == Z_FINISH ) {

		int status;
		status = pv->method == DEFLATE ? deflateEnd(&pv->stream) : inflateEnd(&pv->stream); // free(stream) is done the Finalize
		if ( status != Z_OK )
			JL_CHK( ThrowZError(cx, status, pv->stream.msg) );
		JL_S_ASSERT( pv->stream.state == Z_NULL, "Invalid state." );
	}

	return JS_TRUE;

bad_freebuf:
	BufferFinalize(&resultBuffer);

bad:
	return JS_FALSE;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $BOOL *idle* $READONLY
  Is $TRUE if the redy to process new data.
**/
DEFINE_PROPERTY( idle ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_CHK( BoolToJsval(cx, pv->stream.state == Z_NULL, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 *adler32* $READONLY
  Contains the adler32 checksum of the data.
  [http://en.wikipedia.org/wiki/Adler_checksum more].
**/
DEFINE_PROPERTY( adler32 ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_CHK( JS_NewNumberValue(cx, pv->stream.adler, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 *lengthIn* $READONLY
  Contains the current total amount of input data.
**/
DEFINE_PROPERTY( lengthIn ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_CHK( JS_NewNumberValue(cx, pv->stream.total_in, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 *lengthOut* $READONLY
  Contains the current total amount of output data.
**/
DEFINE_PROPERTY( lengthOut ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_CHK( JS_NewNumberValue(cx, pv->stream.total_out, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Static Properties ===
**/

/* *doc
$TOC_MEMBER $INAME
 *idealInputLength* $READONLY
  This is the ideal size of input data to avoid buffer management overload.
**/
/*
DEFINE_PROPERTY( idealInputLength ) {

	JS_NewNumberValue(cx, Buffer::staticBufferLength, vp);
	return JS_TRUE;
}
*/



/**doc
=== Constants ===
 * Z.`DEFLATE`
  Compression method.

 * Z.`INFLATE`
  Decompression method.
**/
CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_CALL

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(idle)
		PROPERTY_READ(adler32)
		PROPERTY_READ(lengthIn)
		PROPERTY_READ(lengthOut)
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
//		PROPERTY_READ(idealInputLength)
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER_SINGLE(INFLATE)
		CONST_INTEGER_SINGLE(DEFLATE)
		CONST_INTEGER(BEST_SPEED, Z_BEST_SPEED)
		CONST_INTEGER(BEST_COMPRESSION, Z_BEST_COMPRESSION)
	END_CONST_INTEGER_SPEC

END_CLASS


/**doc
=== example ===
{{{
var deflate = new Z( Z.DEFLATE, 9 );
var clearData;
var compressedData;

for ( var i = 10; i >= 0; --i ) {

   var chunk = randomString(10000);
   compressedData += deflate( chunk );
   clearData += chunk;
}
compressedData += deflate(); // flush


var inflate = new Z( Z.INFLATE );
var clearData2 = inflate( compressedData, true );

Print( 'ratio:' + compressedData.length + ': ' + Math.round( 100 * compressedData.length / clearData.length ) + '%','\n');
if ( clearData2 != clearData )

   Print('error!!!','\n');
}
}}}

[http://jslibs.googlecode.com/svn/trunk/jsz/debug.js code snippet]

**/



/****************************************************************

API doc.
	http://www.zlib.net/manual.html

About JS_NewString:
	You should let length be strlen() and add 1 here in the malloc call.

	> ::strcpy(buffer, pDoc->getName());
	> JSString *str = ::JS_NewString(cx, buffer, length);

	because you need to pass the length, not the size, here to JS_NewString.
	  What you are doing now wrongly informs the JS engine that the string
	has a NUL included in its characters, at the end.  That crops printing
	in the JS shell and any other 8-bit environment.

	/be




*/
