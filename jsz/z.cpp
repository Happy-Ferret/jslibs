#include "stdafx.h"

#define ASSERT(e)

#define XP_WIN
#include <jsapi.h>
#include <zlib.h>
#include <stdlib.h>

#include "zError.h"
#include "z.h"

#define INFLATE 0
#define DEFLATE 1

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void z_Finalize(JSContext *cx, JSObject *obj) {

	z_streamp stream = (z_streamp)JS_GetPrivate( cx, obj );
	if ( stream == NULL ) {

		free(stream);
		JS_SetPrivate(cx,obj,NULL);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass z_class = { "Z", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, z_Finalize
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
		status = deflateInit( stream, level );
	else
		status = inflateInit( stream );

	if ( status < 0 )
		return ThrowZError( cx, status, stream->msg );

	JS_SetPrivate( cx, obj, stream );
	JS_SetReservedSlot( cx, obj, 0, INT_TO_JSVAL( method ) );

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool z_transform(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	char staticBuffer[8192];

	z_streamp stream = (z_streamp)JS_GetPrivate( cx, obj );
	if ( stream == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}

// get the action to do
	jsval jsvalMethod;
	int method;
	JS_GetReservedSlot( cx, obj, 0, &jsvalMethod );
	method = JSVAL_TO_INT(jsvalMethod);

// prepare input data
	int inputLength;
	int flushType;
	if ( argc != 0 ) {

		JSString *jssData = JS_ValueToString( cx, argv[0] );
		inputLength = JS_GetStringLength( jssData );
		stream->next_in = (Bytef *)JS_GetStringBytes( jssData ); // no copy is done, we read directly in the string hold by SM
		flushType = Z_SYNC_FLUSH; // Z_SYNC_FLUSH ensure that input datas will be entierly consumed
	} else {
		
		stream->next_in = NULL;
		inputLength = 0;
		flushType = Z_FINISH;
	}


	stream->avail_in = inputLength;


// queue management
	typedef struct {

		int length;
		void* data;
	} queueItemType;

	int queueSize = 8;
	queueItemType *queue = (queueItemType*)malloc( queueSize * sizeof(queueItemType) ); // [TBD] make the first buffer static ( 4096 bytes ) to avoid the first malloc
	int queueEndIndex = 0; // must be signed ( see. assamble chunks )


// main loop
	int resultLength = 0;
	int status;
	do {
		// check if the queue can contain more elements, else enlarge the queue
		if ( queueEndIndex >= queueSize ) {
			queueSize *= 2;
			queue = (queueItemType*)realloc( queue, sizeof(queueItemType) * queueSize );
		}

		// compute the length of the next output chunk
		int chunkSize; // this should contain an estimation of the output size

//		if ( queueEndIndex == 0 ) {
//
//			chunkSize = sizeof(staticBuffer);
//		} else

			if ( method == DEFLATE )
				chunkSize = 12 + stream->avail_in + stream->avail_in / 1000; // dest. buffer must be at least 0.1% larger than sourceLen plus 12 bytes
			else {
//				chunkSize = 100 + (float)stream->avail_in * (float)stream->total_out / (float)(stream->total_in+1); // +1 to avoid div by zero

				chunkSize = stream->avail_in;
			}

		ASSERT( chunkSize > 0 ); // Before the call of inflate()/deflate(), the application should ensure that at least one of the actions is possible, by providing more input and/or consuming more output,

		// store these infos in the structures
		//if ( queueEndIndex == 0 ) {

		//	queue[queueEndIndex].length  =  stream->avail_out  =  chunkSize;
		//	queue[queueEndIndex].data    =  stream->next_out   =  (Bytef*)staticBuffer;
		//} else {

			queue[queueEndIndex].length  =  stream->avail_out  =  chunkSize;
			queue[queueEndIndex].data    =  stream->next_out   =  (Bytef*)malloc( chunkSize );
		//}

		// compress or uncompress
		status = method == DEFLATE ? deflate( stream, flushType ) : inflate( stream, flushType );

if ( method == DEFLATE )
	printf("DE=%d, i=%d, lost= %d / %d, in=%d, avail_in=%d, avail_out=%d, totalIn=%d, totalOut=%d, ratio=%.2f \n", method == DEFLATE, queueEndIndex, stream->avail_out, chunkSize, inputLength, stream->avail_in, stream->avail_out, stream->total_in, stream->total_out, (float)stream->total_in / (float)stream->total_out );


		if ( status < 0 ) {
			// [TBD] free the queue !! 
			// [TBD] free the queue data !!
			return ThrowZError( cx, status, stream->msg );
		}
		
		// compute the effective memory used  ( it is possible that the buffer is not completely fill )



		resultLength += queue[queueEndIndex].length = chunkSize - stream->avail_out; // if avail_out == 0, all the chunk space has been used. operator precedence: http://www.cppreference.com/operator_precedence.html

		queueEndIndex++; // go to the next free chunk

//	} while( stream->avail_out == 0 && status == Z_OK );
	} while ( ( flushType != Z_FINISH && stream->avail_in > 0 )  ||  ( flushType == Z_FINISH && status != Z_STREAM_END ) ); // while the input data are not exhausted or the last datas are not read


// assamble chunks
	char *data = (char*)JS_malloc( cx, resultLength ) + resultLength;
	for ( --queueEndIndex ; queueEndIndex >= 0 ; --queueEndIndex ) {

		data -= queue[queueEndIndex].length;
		memcpy( data, queue[queueEndIndex].data, queue[queueEndIndex].length );
//		if ( queueEndIndex > 0 ) // queue[queueEndIndex].data points to a static area ( see. staticBuffer )
			free( queue[queueEndIndex].data );
	}

/*
	char *data = (char*)JS_malloc( cx, resultLength );
	char *dataPtr = data;

	for ( int i=0; i<queueEndIndex; ++i ) {

		memcpy( dataPtr, queue[i].data, queue[i].length );
		free( queue[i].data ); // chunk is processed, now it is useless
		dataPtr += queue[i].length; // advance in the destination buffer
	}
*/


	free( queue ); // queue is processed, now it is useless

	*rval = STRING_TO_JSVAL(JS_NewString( cx, data, resultLength ));

// close the stream and free resources
	if ( flushType == Z_FINISH ) {
		
		status = method == DEFLATE ? deflateEnd(stream) : inflateEnd(stream); // free(stream) is done the Finalize
		if ( status < 0 )
			return ThrowZError( cx, status, stream->msg );
	}

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JSBool z_finalize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//
//	return JS_TRUE;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec z_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "Transform"   , z_transform  , 0, 0, 0 },
// { "Finalize"    , z_finalize   , 0, 0, 0 },
 { 0 }
};

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
	{ "adler32"    , 0, JSPROP_PERMANENT|JSPROP_READONLY, z_getter_adler32    , NULL },
	{ "lengthIn"   , LENGTH_IN , JSPROP_PERMANENT|JSPROP_READONLY, z_getter_length    , NULL },
	{ "lengthOut"  , LENGTH_OUT, JSPROP_PERMANENT|JSPROP_READONLY, z_getter_length    , NULL },
  { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JSBool z_static_getter_myStatic(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
//
//  return JS_TRUE;
//}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool z_static_getter_constant(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	int32 i;
	JS_ValueToInt32( cx, id, &i );
	*vp = INT_TO_JSVAL(i);

  return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec z_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
//	{ "myStatic"   , 0, JSPROP_PERMANENT|JSPROP_READONLY, z_static_getter_myStatic         , NULL },
	{ "INFLATE" ,    INFLATE, JSPROP_PERMANENT|JSPROP_READONLY, z_static_getter_constant   , NULL },
	{ "DEFLATE" ,    DEFLATE, JSPROP_PERMANENT|JSPROP_READONLY, z_static_getter_constant   , NULL },
  { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *zInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &z_class, z_construct, 1, z_PropertySpec, z_FunctionSpec, z_static_PropertySpec, NULL );
}


/****************************************************************

API doc.
	http://www.zlib.net/manual.html



*/