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

#include "buffer.h"

// current method
#define INFLATE 0
#define DEFLATE 1

// used slots
#define SLOT_METHOD 0


/**doc
----
== jsz::Z class ==
**/

BEGIN_CLASS( Z )

DEFINE_FINALIZE() {

	z_streamp stream = (z_streamp)JS_GetPrivate( cx, obj );
	if ( stream == NULL ) {

		free(stream);
		JS_SetPrivate(cx,obj,NULL);
	}
}


/**doc
 * *_Constructor_*( method [, compressionLevel ] )
  Constructs a new inflater or deflater object.
  = =
  The _method_ can be Z.DEFLATE to compress data or Z.INFLATE to decompress data.
  The compression level is like this: 0 <= _compressionLevel_ <= 9.
  ===== note: =
   For Z.INFLATE method, _compressionLevel_ argument is useless ( no sense )
  ===== example: =
  {{{
  var compress = new Z( Z.DEFLATE, 9 );
  }}}
**/
DEFINE_CONSTRUCTOR() {

	if ( !JS_IsConstructing(cx) ) {

		JS_ReportError( cx, "construction is needed" );
		return JS_FALSE;
	}

	if ( argc < 1 ) {

		JS_ReportError( cx, "missing argument" );
		return JS_FALSE;
	}

	z_streamp stream = (z_streamp)malloc( sizeof(z_stream) );
	stream->zalloc = Z_NULL;
	stream->zfree = Z_NULL;
	stream->opaque = (voidpf) false; // use this private member to store the "stream_end" status (eof)

	int32 method;
	JS_ValueToInt32( cx, argv[0], &method );

	int32 level = Z_DEFAULT_COMPRESSION; // default value
	if ( argc >= 2 ) {

		JS_ValueToInt32( cx, argv[1], &level );
		if ( level < Z_NO_COMPRESSION || level > Z_BEST_COMPRESSION ) {

			JS_ReportError( cx, "level too low or too high" );
			return JS_FALSE;
		}
	}

	int status;
	if ( method == DEFLATE )
		status = deflateInit2( stream, level, Z_DEFLATED, MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY ); //  int level, int method, int windowBits, int memLevel, int strategy
	else
		status = inflateInit2( stream, MAX_WBITS );

	if ( status < 0 )
		return ThrowZError( cx, status, stream->msg );

	JS_SetPrivate( cx, obj, stream );
	JS_SetReservedSlot( cx, obj, SLOT_METHOD, INT_TO_JSVAL( method ) );
//	JS_SetReservedSlot( cx, obj, SLOT_EOF, BOOLEAN_TO_JSVAL( JS_FALSE ) ); // eof
	return JS_TRUE;
}


/**doc
=== Methods ===
**/

/**doc
 * $STR *Call operator*( [ inputData [, forceFinish = false ] ] )
  This function process _inputData_ as a stream.
  If _forceFinish_ is true, the _inputData_ and any buffered data are flushed to the _outputData_.
  If this function is call without any argument, All remaining data are flushed.
  If eof has already reach, the function returns an empty _outputData_ string.
  ===== example: =
  {{{
  var compress = new Z( Z.DEFLATE );
  var compressedData = compress( 'Hello ');
  compressedData += compress( 'world' );
  compressedData += compress(); // flush
  }}}
  ===== example: =
  {{{
  var compress = new Z( Z.DEFLATE );
  var compressedData = compress( 'Hello ');
  compressedData += compress( 'world', true ); // flush
  }}}
  ===== example: =
  {{{
  var compressedData = new Z( Z.DEFLATE )( 'Hello world', true ); // flush
  }}}
**/
DEFINE_CALL() {

	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ...
	// (TBD) check JS_InstanceOf( cx, thisObj, &NativeProc, NULL )
//	J_S_ASSERT_CLASS(thisObj, z_class );

	z_streamp stream = (z_streamp)JS_GetPrivate( cx, thisObj );
	J_S_ASSERT_RESOURCE( stream );

// manage this call if eof is already reach : return an empty string
	if ( stream->opaque != 0 ) { // ( (bool)stream->opaque == true )

		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

// get the action to do
	jsval jsvalMethod;
	int method;
	JS_GetReservedSlot( cx, thisObj, SLOT_METHOD, &jsvalMethod );
	method = JSVAL_TO_INT(jsvalMethod);

// prepare input data
	const char *inputData = NULL;
	size_t inputLength = 0;
	if ( argc >= 1 ) {

		J_CHK( JsvalToStringAndLength(cx, &argv[0], &inputData, &inputLength) ); // warning: GC on the returned buffer !

//		JSString *jssData = JS_ValueToString( cx, argv[0] );
//		J_S_ASSERT_ALLOC( jssData );
//		argv[0] = STRING_TO_JSVAL( jssData );
//		inputLength = JS_GetStringLength( jssData );
//		inputData = JS_GetStringBytes( jssData ); // no copy is done, we read directly in the string hold by SM
	}
	stream->avail_in = inputLength;
	stream->next_in = (Bytef*)inputData;

// force finish
	JSBool forceFinish = JS_FALSE;
	if ( argc >= 2 )
		JS_ValueToBoolean( cx, argv[1], &forceFinish );

	int flushType = inputLength == 0 || forceFinish == JS_TRUE ? Z_FINISH : Z_SYNC_FLUSH;

	Buffer buffer( method == DEFLATE ? (size_t)(12 + 1.001f * stream->avail_in) : (size_t)(1.5f * stream->avail_in) ); // if DEFLATE, dest. buffer must be at least 0.1% larger than sourceLen plus 12 bytes

// deflate / inflate loop
	size_t outputLength;
	int xflateStatus;
	do {

		BufferChunk *chunk = buffer.Next(buffer.SmartLength());
		stream->avail_out = chunk->avail;
		stream->next_out = chunk->data;
//		printf("D%d, ca%d ai%d ao%d ti%d to%d", method == DEFLATE, chunk->avail, stream->avail_in, stream->avail_out, stream->total_in, stream->total_out );
		xflateStatus = method == DEFLATE ? deflate( stream, flushType ) : inflate( stream, flushType ); // Before the call of inflate()/deflate(), the application should ensure that at least one of the actions is possible, by providing more input and/or consuming more output, ...
//		printf("..ai%d ca%d ao%d ti%d to%d\n", 			method == DEFLATE, 			chunk->avail,			stream->avail_in, 			stream->avail_out, 			stream->total_in, 			stream->total_out		);
		chunk->avail = stream->avail_out;
		chunk->data = stream->next_out;
		outputLength = buffer.Length();
		if ( xflateStatus < 0 )
			return ThrowZError( cx, xflateStatus, stream->msg );
	} while ( ( flushType != Z_FINISH && stream->avail_in > 0 ) || // the input data is not exhausted
	          ( flushType == Z_FINISH && xflateStatus != Z_STREAM_END ) || // the last data are not read
	          ( xflateStatus == Z_OK && outputLength == 0 ) // we need to output something
	        );

// assamble chunks
	unsigned char *outputData = (unsigned char*)JS_malloc( cx, outputLength +1 ); // +1 for '\0' char
	outputData[outputLength] = 0; // (TBD) understand WHY !? outputLength info is not enough ??
	buffer.Read(outputData);
	J_CHK( J_NewBlob( cx, (char*)outputData, outputLength, rval ) );

// close the stream and free resources
	if ( xflateStatus == Z_STREAM_END ) {

		stream->opaque = (voidp)true; // store the eof status
		int status = method == DEFLATE ? deflateEnd(stream) : inflateEnd(stream); // free(stream) is done the Finalize
		if ( status < 0 )
			return ThrowZError( cx, status, stream->msg );
	}
	return JS_TRUE;
}


/**doc
=== Properties ===
**/

/**doc
 * *eof* $READONLY
  Is `true` if the end of the stream has been reach.
**/
DEFINE_PROPERTY( eof ) {

	z_streamp stream = (z_streamp)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( stream );
	*vp = BOOLEAN_TO_JSVAL( stream->opaque != 0 ); // (bool)stream->opaque
	return JS_TRUE;
}


/**doc
 * *adler32* $READONLY
  Contains the adler32 checksum of the data.
  [http://en.wikipedia.org/wiki/Adler_checksum more].
**/
DEFINE_PROPERTY( adler32 ) {

	z_streamp stream = (z_streamp)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( stream );
	J_CHK( JS_NewNumberValue(cx, stream->adler, vp) );
	return JS_TRUE;
}


/**doc
 * *lengthIn* $READONLY
  Contains the current total amount of input data.
**/
DEFINE_PROPERTY( lengthIn ) {

	z_streamp stream = (z_streamp)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( stream );
	J_CHK( JS_NewNumberValue(cx, stream->total_in, vp) );
	return JS_TRUE;
}


/**doc
 * *lengthOut* $READONLY
  Contains the current total amount of output data.
**/
DEFINE_PROPERTY( lengthOut ) {
	z_streamp stream = (z_streamp)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( stream );
	J_CHK( JS_NewNumberValue(cx, stream->total_out, vp) );
	return JS_TRUE;
}


/**doc
=== Static Properties ===
**

/**doc
 * *idealInputLength* $READONLY
  This is the ideal size of input data to avoid buffer management overload.
**/
DEFINE_PROPERTY( idealInputLength ) {

	JS_NewNumberValue( cx, Buffer::staticBufferLength, vp );
	return JS_TRUE;
}


/**doc
=== Constants ===
 * Z.`DEFLATE`
  Compression method.

 * Z.`INFLATE`
  Decompression method.
**/
CONFIGURE_CLASS

	HAS_RESERVED_SLOTS(1)
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_CALL

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(eof)
		PROPERTY_READ(adler32)
		PROPERTY_READ(lengthIn)
		PROPERTY_READ(lengthOut)
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ(idealInputLength)
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER_SINGLE(INFLATE)
		CONST_INTEGER_SINGLE(DEFLATE)
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
