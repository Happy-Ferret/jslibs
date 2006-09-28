#include "stdafx.h"
#include "hash.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void hash_Finalize(JSContext *cx, JSObject *obj) {

	HashPrivate *privateData = (HashPrivate *)JS_GetPrivate( cx, obj );
	if ( privateData != NULL ) {

		free(privateData);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool hash_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ... [TBD]: check JS_InstanceOf( cx, thisObj, &NativeProc, NULL )
	RT_ASSERT_CLASS( thisObj, &hash_class );
	RT_ASSERT_ARGC( 1 );
	HashPrivate *privateData = (HashPrivate *)JS_GetPrivate( cx, thisObj );
	RT_ASSERT( privateData != NULL, RT_ERROR_NOT_INITIALIZED );

	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], in, inLength );

	int err;
	if ((err=privateData->hash.process(&privateData->state, (const unsigned char *)in, inLength)) != CRYPT_OK )
		return ThrowCryptError(cx, err);

	if ( argc == 1 ) { // do not continue

		unsigned long outLength = privateData->hash.hashsize;
		char *out = (char *)JS_malloc( cx, outLength );
		RT_ASSERT( out != NULL, RT_ERROR_OUT_OF_MEMORY );

		if ((err=privateData->hash.done(&privateData->state, (unsigned char*)out)) != CRYPT_OK )
			return ThrowCryptError(cx, err);

		JSString *jssHashData = JS_NewString( cx, out, outLength );
		RT_ASSERT( jssHashData != NULL, "unable to create the hash string." );
		*rval = STRING_TO_JSVAL(jssHashData);
	}

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool hash_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT( JS_IsConstructing(cx), RT_ERROR_NEED_CONSTRUCTION );
	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, &hash_class );

	char *hashName;
	RT_JSVAL_TO_STRING( argv[0], hashName );

	int hashIndex = find_hash(hashName);
	RT_ASSERT_1( hashIndex != -1, "hash %s is not registred", hashName );

	HashPrivate *privateData = (HashPrivate*)malloc( sizeof(HashPrivate) );
	RT_ASSERT( privateData != NULL, RT_ERROR_OUT_OF_MEMORY );

	privateData->hash = hash_descriptor[hashIndex];

	RT_ASSERT_1( privateData->hash.test() == CRYPT_OK, "%s hash test failed.", hashName );

	int err;
	if ( (err = privateData->hash.init(&privateData->state)) != CRYPT_OK )
		return ThrowCryptError(cx, err);

	JS_SetPrivate( cx, obj, privateData );

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JSBool hash_hash(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//
//	return JS_TRUE;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec hash_FunctionSpec[] = { // *name, call, nargs, flags, extra
// { "hash"          , hash_hash          , 0, 0, 0 },
 { 0 }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JSBool hash_getter_myProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
//
//  return JS_TRUE;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec hash_PropertySpec[] = { // *name, tinyid, flags, getter, setter
//	{ "myProperty"            , 0, JSPROP_PERMANENT|JSPROP_READONLY, hash_getter_myProperty       , NULL },
  { 0 }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool hash_static_cipherHash(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC(1);

	char *cipher;
	RT_JSVAL_TO_STRING( argv[0], cipher );

	int err;
	if ((err = chc_register(find_cipher(cipher))) != CRYPT_OK)
		return ThrowCryptError(cx, err);

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec hash_static_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "CipherHash"          , hash_static_cipherHash          , 0, 0, 0 },
 { 0 }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool hash_static_getter_list(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JSObject *list = JS_NewObject( cx, NULL, NULL, NULL );
	RT_ASSERT( list != NULL, "unable to create hash list." );

	const char *hashName;
	int x;
	LTC_MUTEX_LOCK(&ltc_hash_mutex);
	for (x = 0; x < TAB_SIZE; x++)
		if ( (hashName = hash_descriptor[x].name) != NULL) {
		  //JS_DefineProperty(cx, list, hashName, JSVAL_TRUE, NULL, NULL, 0);
			jsval value = INT_TO_JSVAL( hash_descriptor[x].hashsize );
			JS_SetProperty( cx, list, hashName, &value );
		}
	LTC_MUTEX_UNLOCK(&ltc_hash_mutex);

	*vp = OBJECT_TO_JSVAL(list);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec hash_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "list"                , 0, JSPROP_PERMANENT|JSPROP_READONLY, hash_static_getter_list         , NULL },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass hash_class = { "Hash", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, hash_Finalize,
	0,0, hash_call
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *hashInitClass( JSContext *cx, JSObject *obj ) {

	register_hash(&whirlpool_desc);
	register_hash(&sha512_desc);
	register_hash(&sha384_desc);
	register_hash(&sha256_desc);
	register_hash(&sha224_desc);
	register_hash(&sha1_desc);
	register_hash(&md5_desc);
	register_hash(&md4_desc);
	register_hash(&md2_desc);
	register_hash(&tiger_desc);
	register_hash(&rmd128_desc);
	register_hash(&rmd160_desc);
	
	register_hash(&chc_desc); // allow to use chc_register for a 'Cipher Hash Construction'

	return JS_InitClass( cx, obj, NULL, &hash_class, hash_construct, 0, hash_PropertySpec, hash_FunctionSpec, hash_static_PropertySpec, hash_static_FunctionSpec );
}


/****************************************************************

*/