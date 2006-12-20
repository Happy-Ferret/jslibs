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

#define ASSERT(e)

#define XP_WIN
#include <jsapi.h>
#include <zlib.h>
#include <stdlib.h>

#include "zError.h"
#include "z.h"

#include "buffer.h"

// current method
#define INFLATE 0
#define DEFLATE 1

// used slots
#define SLOT_METHOD 0
//#define SLOT_EOF 1

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void z_Finalize(JSContext *cx, JSObject *obj) {

	z_streamp stream = (z_streamp)JS_GetPrivate( cx, obj );
	if ( stream == NULL ) {

		free(stream);
		JS_SetPrivate(cx,obj,NULL);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool z_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ...
	// (TBD) check JS_InstanceOf( cx, thisObj, &NativeProc, NULL )

	z_streamp stream = (z_streamp)JS_GetPrivate( cx, thisObj );
	if ( stream == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}

// manage this call if eof is already reach : return an empty string
	if ( (bool)stream->opaque == true ) {

		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

// get the action to do
	jsval jsvalMethod;
	int method;
	JS_GetReservedSlot( cx, thisObj, SLOT_METHOD, &jsvalMethod );
	method = JSVAL_TO_INT(jsvalMethod);

// prepare input data
	char *inputData = NULL;
	size_t inputLength = 0;
	if ( argc >= 1 ) {

		JSString *jssData = JS_ValueToString( cx, argv[0] );
		inputLength = JS_GetStringLength( jssData );
		inputData = JS_GetStringBytes( jssData ); // no copy is done, we read directly in the string hold by SM
	}
	stream->avail_in = inputLength;
	stream->next_in = (Bytef*)inputData;

// force finish
	JSBool forceFinish = JS_FALSE;
	if ( argc >= 2 )
		JS_ValueToBoolean( cx, argv[1], &forceFinish );

	int flushType = inputLength == 0 || forceFinish == JS_TRUE ? Z_FINISH : Z_SYNC_FLUSH;

	Buffer buffer( method == DEFLATE ? 12 + 1.001f * stream->avail_in : 1.5f * stream->avail_in ); // if DEFLATE, dest. buffer must be at least 0.1% larger than sourceLen plus 12 bytes

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
	          ( flushType == Z_FINISH && xflateStatus != Z_STREAM_END ) || // the last datas are not read
	          ( xflateStatus == Z_OK && outputLength == 0 ) // we need to output something
	        );

// assamble chunks
	unsigned char *outputData = (unsigned char*)JS_malloc( cx, outputLength +1 ); // +1 for '\0' char
	outputData[outputLength] = 0; // (TBD) understand WHY !? outputLength info is not enough ??
	buffer.Read(outputData);
	JSString *jssOutputData = JS_NewString( cx, (char*)outputData, outputLength );
	if ( jssOutputData == NULL ) {

		JS_free( cx, outputData ); // JS_NewString takes ownership of bytes on success, avoiding a copy; but on error (signified by null return), it leaves bytes owned by the caller. So the caller must free bytes in the error case, if it has no use for them.
		JS_ReportError( cx, "out of memory" );
		return JS_FALSE;
	}
	*rval = STRING_TO_JSVAL(jssOutputData);

// close the stream and free resources
	if ( xflateStatus == Z_STREAM_END ) {

		stream->opaque = (voidp)true; // store the eof status
		int status = method == DEFLATE ? deflateEnd(stream) : inflateEnd(stream); // free(stream) is done the Finalize
		if ( status < 0 )
			return ThrowZError( cx, status, stream->msg );
	}
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass z_class = { "Z", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1), // SLOT_METHOD
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, z_Finalize,
  0,0, z_call
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool z_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	if ( !JS_IsConstructing(cx) ) {

		JS_ReportError( cx, "construction is needed" );
		return JS_FALSE;
	}

	if ( argc < 1 ) {

		JS_ReportError( cx, "missing argument" );
		return JS_FALSE;
	}

	z_streamp stream = (z_streamp)malloc( sizeof z_stream );
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JSBool z_transform(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec z_FunctionSpec[] = { // *name, call, nargs, flags, extra
// { "Transform"   , z_transform  , 0, 0, 0 },
 { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool z_getter_eof(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	z_streamp stream = (z_streamp)JS_GetPrivate( cx, obj );
	if ( stream == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}

	*vp = BOOLEAN_TO_JSVAL( (bool)stream->opaque );

//	JS_GetReservedSlot( cx, obj, SLOT_EOF, vp ); // eof

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool z_getter_adler32(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	z_streamp stream = (z_streamp)JS_GetPrivate( cx, obj );
	if ( stream == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}

	jsdouble adler32 = stream->adler;
	JS_NewNumberValue( cx, adler32, vp );

	return JS_TRUE;
}

#define LENGTH_IN 1
#define LENGTH_OUT 2
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool z_getter_length(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	z_streamp stream = (z_streamp)JS_GetPrivate( cx, obj );
	if ( stream == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}

	jsdouble length = JSVAL_TO_INT(id) == LENGTH_IN ? stream->total_in : stream->total_out;
	JS_NewNumberValue( cx, length, vp );

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec z_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "eof"              , 0         , JSPROP_SHARED | JSPROP_PERMANENT | JSPROP_READONLY, z_getter_eof              , NULL },
	{ "adler32"          , 0         , JSPROP_SHARED | JSPROP_PERMANENT | JSPROP_READONLY, z_getter_adler32          , NULL },
	{ "lengthIn"         , LENGTH_IN , JSPROP_SHARED | JSPROP_PERMANENT | JSPROP_READONLY, z_getter_length           , NULL },
	{ "lengthOut"        , LENGTH_OUT, JSPROP_SHARED | JSPROP_PERMANENT | JSPROP_READONLY, z_getter_length           , NULL },
  { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool z_static_getter_idealInputLength(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JS_NewNumberValue( cx, Buffer.staticBufferLength, vp );
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool z_static_getter_constant(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	int32 i;
	JS_ValueToInt32( cx, id, &i );
	*vp = INT_TO_JSVAL(i);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec z_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "idealInputLength" , 0      , JSPROP_SHARED | JSPROP_PERMANENT | JSPROP_READONLY, z_static_getter_idealInputLength  , NULL },
	{ "INFLATE"          , INFLATE, JSPROP_SHARED | JSPROP_PERMANENT | JSPROP_READONLY, z_static_getter_constant          , NULL },
	{ "DEFLATE"          , DEFLATE, JSPROP_SHARED | JSPROP_PERMANENT | JSPROP_READONLY, z_static_getter_constant          , NULL },
  { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *zInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &z_class, z_construct, 1, z_PropertySpec, z_FunctionSpec, z_static_PropertySpec, NULL );
}


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
