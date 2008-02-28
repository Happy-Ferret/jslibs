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
#include "prng.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void prng_Finalize(JSContext *cx, JSObject *obj) {

	PrngPrivate *privateData = (PrngPrivate *)JS_GetPrivate( cx, obj );
	if ( privateData != NULL ) {

		privateData->prng.done( &privateData->state );
		free( privateData );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool prng_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ...
	// (TBD) check JS_InstanceOf( cx, thisObj, &NativeProc, NULL )

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( thisObj, &prng_class );
	PrngPrivate *privateData = (PrngPrivate *)JS_GetPrivate( cx, thisObj );
	RT_ASSERT( privateData, RT_ERROR_NOT_INITIALIZED );

	unsigned long readCount;
	RT_JSVAL_TO_INT32( argv[0], readCount );

	char *pr = (char*)JS_malloc( cx, readCount );
	RT_ASSERT( pr != NULL, RT_ERROR_OUT_OF_MEMORY );
	unsigned long hasRead = privateData->prng.read( (unsigned char*)pr, readCount, &privateData->state );
	RT_ASSERT( hasRead == readCount, "unable to read prng." );

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

	PrngPrivate *privateData = (PrngPrivate*)malloc( sizeof(PrngPrivate) );
	RT_ASSERT( privateData != NULL, RT_ERROR_OUT_OF_MEMORY );

	privateData->prng = prng_descriptor[prngIndex];

	RT_ASSERT_1( privateData->prng.test() == CRYPT_OK, "%s prng test failed.", prngName );

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
	PrngPrivate *privateData = (PrngPrivate *)JS_GetPrivate( cx, obj );
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
	PrngPrivate *privateData = (PrngPrivate *)JS_GetPrivate( cx, obj );
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
	PrngPrivate *privateData = (PrngPrivate *)JS_GetPrivate( cx, obj );
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
JSClass prng_class = { "Prng", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, prng_Finalize,
	0,0, prng_call
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *prngInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &prng_class, prng_construct, 0, prng_PropertySpec, prng_FunctionSpec, prng_static_PropertySpec, prng_static_FunctionSpec );
}


/****************************************************************

*/
