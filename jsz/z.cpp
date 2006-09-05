#include "stdafx.h"

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

	int32 level = Z_DEFAULT_COMPRESSION;
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

	z_streamp stream = (z_streamp)JS_GetPrivate( cx, obj );
	if ( stream == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}

	jsval jsvalMethod;
	int method;
	JS_GetReservedSlot( cx, obj, 0, &jsvalMethod );
	method = JSVAL_TO_INT(jsvalMethod);

	if ( argc >= 1 ) {

		JSString *jssData = JS_ValueToString( cx, argv[0] );
		stream->next_in = (Bytef *)JS_GetStringBytes( jssData ); // no copy is done, we read directly in the string hold by SM
		stream->avail_in = JS_GetStringLength( jssData );
	} else {

		stream->next_in = NULL;
		stream->avail_in = 0;
	}

	int flushType;

	if ( argc == 0 )
		flushType = Z_FINISH;
	else
		flushType = Z_SYNC_FLUSH;

	typedef struct {

		int length;
		void* data;
	} queueItemType;

  int queueSize = 16;
	
	queueItemType *queue = (queueItemType*)malloc( queueSize * sizeof(queueItemType) );

	int totalLength = 0;
	int queueEndIndex = 0;

	int status; // Z_STREAM_END, Z_OK
	do {
	//for ( ; stream->avail_in > 0; queueEndIndex++ ) { // while the input data are not exhausted ...

// check if the queue can contain more elements
		if ( queueEndIndex >= queueSize ) {

			queueSize *= 2;
			queue = (queueItemType*)realloc( queue, queueSize * sizeof(queueItemType) );
		}

// compute the length of the next chunk
		int chunkSize;
		if ( method == DEFLATE )
			chunkSize = 12 + stream->avail_in + stream->avail_in / 10;
		else
			chunkSize = stream->avail_in * ( stream->total_out / stream->total_in ) + 100;
//			chunkSize = inputLength * 2; // can be more accurate using total_in and total_out ratio

// store these infos in the structures
		queue[queueEndIndex].data   = stream->next_out  = (Bytef *)malloc( chunkSize );
		queue[queueEndIndex].length = stream->avail_out = chunkSize;

// compress or uncompress
		if ( method == DEFLATE )
			status = deflate( stream, flushType );
		else
			status = inflate( stream, flushType );

		if ( status < 0 ) {
			// free the queue !! and queue data !!
			return ThrowZError( cx, status, stream->msg );
		}

// compute the effective memory used
		totalLength += queue[queueEndIndex].length = chunkSize - stream->avail_out; // if avail_out == 0, all the chunk space has been used ( help: http://www.cppreference.com/operator_precedence.html )

		queueEndIndex++;

	} while( status == Z_OK && stream->avail_out == 0  );


// assamble chunks 
//	int totalSize = stream->total_out; // total_out = total nb of bytes output so far

	char *data = (char*)JS_malloc( cx, totalLength );
	char *buf = data;

	for ( int i=0; i<queueEndIndex; ++i ) {

		memcpy( buf, queue[i].data, queue[i].length );
		free( queue[i].data ); // chunk is processed, now it is useless
		buf += queue[i].length;
	}

	free( queue ); // queue is processed, now it is useless

	*rval = STRING_TO_JSVAL(JS_NewString( cx, data, totalLength ));

	if ( argc == 0 ) {

		if ( method == DEFLATE )
			status = deflateEnd(stream);
		else
			status = inflateEnd(stream);

		free(stream);
		JS_SetPrivate(cx,obj,NULL);

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
//JSBool z_getter_myProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
//
//  return JS_TRUE;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec z_PropertySpec[] = { // *name, tinyid, flags, getter, setter
//	{ "myProperty"    , 0, JSPROP_PERMANENT|JSPROP_READONLY, z_getter_myProperty    , NULL },
  { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool z_static_getter_myStatic(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

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
	{ "myStatic"   , 0, JSPROP_PERMANENT|JSPROP_READONLY, z_static_getter_myStatic         , NULL },
	{ "INFLATE" ,    INFLATE, JSPROP_PERMANENT|JSPROP_READONLY, z_static_getter_constant   , NULL },
	{ "DEFLATE" ,    DEFLATE, JSPROP_PERMANENT|JSPROP_READONLY, z_static_getter_constant   , NULL },
  { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *zInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &z_class, z_construct, 1, z_PropertySpec, z_FunctionSpec, z_static_PropertySpec, NULL );
}


/****************************************************************

*/