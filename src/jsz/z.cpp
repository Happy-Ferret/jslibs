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
#include "z.h"
#include <buffer.h>


// current method
enum Method {
	INFLATE,
	DEFLATE
};


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/

BEGIN_CLASS( Z )

struct Private {
	z_stream stream;
	int method;
	int level;
};


DEFINE_FINALIZE() {

	if ( jl::Host::getHost(fop->runtime()).hostRuntime().skipCleanup() )
		return;

	Private *pv = (Private*)js::GetObjectPrivate(obj);
	if ( !pv )
		return;

	if ( pv->stream.state != Z_NULL ) {

		int status;
		if ( pv->method == DEFLATE ) {

			status = deflateEnd(&pv->stream);
		} else {

			status = inflateEnd(&pv->stream);
		}

		// cannot report an error while GC is running

		//JL_ASSERT_WARN( status == Z_OK, E_OBJ, E_STR(JL_THIS_CLASS_NAME), E_FIN, E_COMMENT(pv->stream.msg ? pv->stream.msg : "") ); // "Unable to finalize zlib stream (%s).", pv->stream.msg ); // (TBD) send to log !
	}
	JS_freeop(fop, pv);
bad:
	return;
}


/**doc
$TOC_MEMBER $INAME
 $INAME( method [, compressionLevel ] )
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

NOALIAS voidpf jsz_alloc(voidpf, uInt items, uInt size) NOTHROW {

	return jl_malloc(items*size);
}

void jsz_free(voidpf, voidpf address) NOTHROW {

	jl_free(address);
}

DEFINE_CONSTRUCTOR() {

	Private *pv = NULL;

	JL_DEFINE_ARGS;

	JL_ASSERT_ARGC_RANGE(1,2);
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK(pv);

	pv->stream.state = Z_NULL; // mendatory
	pv->stream.zalloc = jsz_alloc;
	pv->stream.zfree = jsz_free;

	JL_CHK( jl::getValue(cx, JL_ARG(1), &pv->method) );
	JL_ASSERT_ARG_VAL_RANGE(pv->method, INFLATE, DEFLATE, 1);

	if ( JL_ARG_ISDEF(2) ) {

		JL_CHK( jl::getValue(cx, JL_ARG(1), &pv->level) );
		JL_ASSERT_ARG_VAL_RANGE( pv->level, Z_NO_COMPRESSION, Z_BEST_COMPRESSION, 1 );
		JL_ASSERT_WARN( pv->method == DEFLATE, E_ARG, E_NUM(2), E_IGNORED ); // "The second argument is overmuch for this method."
	} else {

		pv->level = Z_DEFAULT_COMPRESSION; // default value
	}

	JL_SetPrivate(JL_OBJ, pv);
	return true;

bad:
	if ( pv ) {
	
		if ( pv->stream.state != Z_NULL ) {

			if ( pv->method == DEFLATE )
				deflateEnd(&pv->stream);
			else
				inflateEnd(&pv->stream);
		}
		JS_free(cx, pv);
	}
	return false;
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
  var compressedData = Stringify(compress.process( 'Hello '));
  compressedData += Stringify(compress.process( 'world' ));
  compressedData += Stringify(compress.process()); // flush
  }}}
  $H example
  {{{
  var compress = new Z( Z.DEFLATE );
  var compressedData = Stringify(compress.process( 'Hello '));
  compressedData += Stringify(compress.process( 'world', true )); // flush
  }}}
  $H example
  {{{
  var compressedData = new Z( Z.DEFLATE ).process( 'Hello world', true ); // flush
  }}}
**/
DEFINE_FUNCTION( process ) {

	jl::Buf<Bytef> resultBuffer;
	JLData inputData;
	int flushType;

	JL_DEFINE_ARGS;

	JL_ASSERT_INSTANCE(JL_OBJ, JL_THIS_CLASS);

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	// force finish
	bool forceFinish;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &forceFinish) );
	else
		forceFinish = false;

	if ( JL_ARG_ISDEF(1) )
		JL_CHK( jl::getValue(cx, JL_ARG(1), &inputData) ); // warning: GC on the returned buffer !
	else
		forceFinish = true;

	// doc. Z_SYNC_FLUSH: all pending output is flushed to the output buffer and the output is aligned on a byte boundary, so that the decompressor can get all input data available so far.
	// doc. Z_FINISH: pending input is processed, pending output is flushed and deflate returns with Z_STREAM_END if there was enough output space; if deflate returns with Z_OK, this function must be called again with Z_FINISH and more output space.
	flushType = forceFinish ? Z_FINISH : Z_SYNC_FLUSH;

	if ( pv->stream.state == Z_NULL ) {

		int status;
		if ( pv->method == DEFLATE )
			status = deflateInit2(&pv->stream, pv->level, Z_DEFLATED, MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
		else
			status = inflateInit2(&pv->stream, MAX_WBITS);

		if ( status < 0 )
			JL_CHK( ThrowZError(cx, status, pv->stream.msg) );
	}

	ASSERT( inputData.LengthOrZero() <= UINT_MAX );
	pv->stream.avail_in = (uInt)inputData.LengthOrZero();
	pv->stream.next_in = (Bytef*)inputData.GetStrConstOrNull();

	// first length is a guess.
	size_t length;
	length = pv->method == DEFLATE ? (size_t)(12 + 1.001f * pv->stream.avail_in) : (size_t)(1.5f * pv->stream.avail_in); // if DEFLATE, dest. buffer must be at least 0.1% larger than sourceLen plus 12 bytes

	int xflateStatus;
	for (;;) {

//		length = jl::max( length, BufferGetOptimalLength(&resultBuffer) );

		ASSERT( length <= (uInt)-1 );
		pv->stream.avail_out = (uInt)length;
		resultBuffer.Reserve(pv->stream.avail_out);
		pv->stream.next_out = resultBuffer.Ptr();

//		printf("D%d, ca%d ai%d ao%d ti%d to%d", method == DEFLATE, chunk->avail, stream->avail_in, stream->avail_out, stream->total_in, stream->total_out );
		xflateStatus = pv->method == DEFLATE ? deflate(&pv->stream, flushType) : inflate(&pv->stream, flushType); // Before the call of inflate()/deflate(), the application should ensure that at least one of the actions is possible, by providing more input and/or consuming more output, ...
//		printf("..ai%d ca%d ao%d ti%d to%d\n", 			method == DEFLATE, 			chunk->avail,			stream->avail_in, 			stream->avail_out, 			stream->total_in, 			stream->total_out		);

		if ( xflateStatus < Z_OK && xflateStatus != Z_BUF_ERROR ) // fatal error ?
			JL_CHK( ThrowZError(cx, xflateStatus, pv->stream.msg) );
		resultBuffer.Advance(length - pv->stream.avail_out);
		if ( xflateStatus == Z_STREAM_END || pv->stream.avail_in == 0 )
			break;
		// doc. Z_BUF_ERROR if no progress is possible (for example avail_in or avail_out was zero). Note that Z_BUF_ERROR is not fatal, and deflate() can be called again with more input and more output space to continue compressing.
		length = pv->stream.avail_in * pv->stream.total_out / pv->stream.total_in;

//		if ( xflateStatus == Z_BUF_ERROR )
//			length = length * 2 + 4096;
	}

	JL_CHK( JL_NewBufferGetOwnership(cx, resultBuffer.GetDataOwnership(), resultBuffer.Length(), JL_RVAL) );

	// close the stream and free resources
	if ( xflateStatus == Z_STREAM_END && flushType == Z_FINISH ) {

		int status;
		status = pv->method == DEFLATE ? deflateEnd(&pv->stream) : inflateEnd(&pv->stream); // free(stream) is done the Finalize
		if ( status != Z_OK )
			JL_CHK( ThrowZError(cx, status, pv->stream.msg) );
		JL_ASSERT( pv->stream.state == Z_NULL, E_THISOBJ, E_STATE );
	}
	return true;

bad:
	return false;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
  Is $TRUE if the redy to process new data.
**/
DEFINE_PROPERTY_GETTER( idle ) {

	JL_DEFINE_PROP_ARGS;

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_CHK( jl::setValue(cx, JL_RVAL, pv->stream.state == Z_NULL) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Contains the adler32 checksum of the data.
  [http://en.wikipedia.org/wiki/Adler_checksum more].
**/
DEFINE_PROPERTY_GETTER( adler32 ) {

	JL_DEFINE_PROP_ARGS;

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_CHK( jl::setValue(cx, JL_RVAL, pv->stream.adler) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Contains the current total amount of input data.
**/
DEFINE_PROPERTY_GETTER( lengthIn ) {

	JL_DEFINE_PROP_ARGS;

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_CHK( jl::setValue(cx, JL_RVAL, pv->stream.total_in) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Contains the current total amount of output data.
**/
DEFINE_PROPERTY_GETTER( lengthOut ) {

	JL_DEFINE_PROP_ARGS;

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_CHK( jl::setValue(cx, JL_RVAL, pv->stream.total_out) );
	return true;
	JL_BAD;
}


/**doc
=== Static Properties ===
**/

/* *doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  This is the ideal size of input data to avoid buffer management overload.
**/
/*
DEFINE_PROPERTY( idealInputLength ) {

	JL_NewNumberValue(cx, Buffer::staticBufferLength, vp);
	return true;
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

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( process, 1 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( idle )
		PROPERTY_GETTER( adler32 )
		PROPERTY_GETTER( lengthIn )
		PROPERTY_GETTER( lengthOut )
	END_PROPERTY_SPEC

	BEGIN_CONST
		CONST_INTEGER_SINGLE( INFLATE )
		CONST_INTEGER_SINGLE( DEFLATE )
		CONST_INTEGER( BEST_SPEED, Z_BEST_SPEED )
		CONST_INTEGER( BEST_COMPRESSION, Z_BEST_COMPRESSION )
	END_CONST

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

print( 'ratio:' + compressedData.length + ': ' + Math.round( 100 * compressedData.length / clearData.length ) + '%','\n');
if ( clearData2 != clearData )

   print('error!!!','\n');
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
