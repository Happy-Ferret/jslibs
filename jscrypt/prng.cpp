#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>

#include "prng.h"

#include "cryptError.h"

#include <tomcrypt.h>

#include "../common/jshelper.h"

typedef struct PrivateData {
	ltc_prng_descriptor prng;
	prng_state state;
};

JSBool prng_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
void prng_Finalize(JSContext *cx, JSObject *obj);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass prng_class = { "Prng", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, prng_Finalize,
	0,0, prng_call
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void prng_Finalize(JSContext *cx, JSObject *obj) {

	PrivateData *privateData = (PrivateData *)JS_GetPrivate( cx, obj );
	if ( privateData != NULL ) {

		privateData->prng.done( &privateData->state );
		free( privateData );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool prng_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ... [TBD]: check JS_InstanceOf( cx, thisObj, &NativeProc, NULL )

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( thisObj, &prng_class );
	PrivateData *privateData = (PrivateData *)JS_GetPrivate( cx, thisObj );
	RT_ASSERT( privateData, RT_ERROR_NOT_INITIALIZED );

	int32 readCount;
	RT_JSVAL_TO_INT32( argv[0], readCount );

	char *pr = (char*)JS_malloc( cx, readCount );
	RT_ASSERT( pr != NULL, RT_ERROR_OUT_OF_MEMORY );
	unsigned long hasRead = privateData->prng.read( (unsigned char*)pr, readCount, &privateData->state );

	JSString *randomString = JS_NewString( cx, pr, hasRead );
	RT_ASSERT( randomString != NULL, "unable to create the random string." );
	*rval = STRING_TO_JSVAL(randomString);

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool prng_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT( JS_IsConstructing(cx), RT_ERROR_NEED_CONSTRUCTION );
	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, &prng_class );

	char *prngName;
	RT_JSVAL_TO_STRING( argv[0], prngName );

	int prngIndex = find_prng(prngName);
	RT_ASSERT_1( prngIndex != -1, "prng %s is not registred", prngName );

	PrivateData *privateData = (PrivateData*)malloc( sizeof(PrivateData) );
	RT_ASSERT( privateData != NULL, RT_ERROR_OUT_OF_MEMORY );

	privateData->prng = prng_descriptor[prngIndex];

	int err;
	if ( (err = privateData->prng.start( &privateData->state )) != CRYPT_OK )
		return ThrowCryptError(cx,err);

	if ((err = privateData->prng.ready( &privateData->state )) != CRYPT_OK )
		return ThrowCryptError(cx,err);

	JS_SetPrivate( cx, obj, privateData );
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool prng_addEntropy(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, &prng_class );
	PrivateData *privateData = (PrivateData *)JS_GetPrivate( cx, obj );
	RT_ASSERT( privateData != NULL, RT_ERROR_NOT_INITIALIZED );

	char *entropy;
	int entropyLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], entropy, entropyLength );

	int prngError;
	if ( (prngError = privateData->prng.add_entropy( (const unsigned char *)entropy, entropyLength, &privateData->state )) != CRYPT_OK )
		return ThrowCryptError(cx, prngError);

	if ((prngError = privateData->prng.ready( &privateData->state )) != CRYPT_OK )
		return ThrowCryptError(cx, prngError);

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool prng_autoEntropy(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, &prng_class );
	PrivateData *privateData = (PrivateData *)JS_GetPrivate( cx, obj );
	RT_ASSERT( privateData != NULL, RT_ERROR_NOT_INITIALIZED );

	int32 bits;
	RT_JSVAL_TO_INT32( argv[0], bits );
	rng_make_prng( bits, find_prng(privateData->prng.name), &privateData->state, NULL );

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JSBool prng_export(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//
//	return JS_TRUE;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec prng_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "AddEntropy"      , prng_addEntropy      , 0, 0, 0 },
 { "AutoEntropy"     , prng_autoEntropy     , 0, 0, 0 },
// { "Export"          , prng_export          , 0, 0, 0 },
 { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool prng_getter_name(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	RT_ASSERT_CLASS( obj, &prng_class );
	PrivateData *privateData = (PrivateData *)JS_GetPrivate( cx, obj );
	RT_ASSERT( privateData != NULL, RT_ERROR_NOT_INITIALIZED );
	
	*vp = STRING_TO_JSVAL( JS_NewStringCopyZ(cx,privateData->prng.name) );
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec prng_PropertySpec[] = { // *name, tinyid, flags, getter, setter
//	{ "name"  , 0, JSPROP_PERMANENT|JSPROP_READONLY, prng_getter_name       , NULL },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JSBool prng_nameByLength(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//
//	return JS_TRUE;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec prng_static_FunctionSpec[] = { // *name, call, nargs, flags, extra
// { "NameByLength"     , prng_nameByLength     , 0, 0, 0 },
 { 0 }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JSBool prng_static_getter_myStatic(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
//
//  return JS_TRUE;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec prng_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
//	{ "myStatic"                , 0, JSPROP_PERMANENT|JSPROP_READONLY, prng_static_getter_myStatic         , NULL },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *prngInitClass( JSContext *cx, JSObject *obj ) {

	register_prng(&yarrow_desc);
	register_prng(&fortuna_desc);
	register_prng(&rc4_desc);
	register_prng(&sprng_desc);
	register_prng(&sober128_desc);

	return JS_InitClass( cx, obj, NULL, &prng_class, prng_construct, 0, prng_PropertySpec, prng_FunctionSpec, prng_static_PropertySpec, prng_static_FunctionSpec );
}


/****************************************************************

*/