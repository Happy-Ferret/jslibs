#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>

#include "cryptError.h"

#include "cipher.h"

#include <tomcrypt.h>

#include "../common/jshelper.h"

#define SLOT_CIPHERNAME 0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void cipher_Finalize(JSContext *cx, JSObject *obj) {

	//free( pskey )
	//done()
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool cipher_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ... [TBD]: check JS_InstanceOf( cx, thisObj, &NativeProc, NULL )

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass cipher_class = { "Cipher", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, cipher_Finalize,
   0,0, cipher_call
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool cipher_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT( JS_IsConstructing(cx), RT_ERROR_NEED_CONSTRUCTION );
	RT_ASSERT_ARGC( 2 );
	RT_ASSERT_CLASS( obj, &cipher_class );
	
	char *cipherName;
	RT_JSVAL_TO_STRING( argv[0], cipherName );

	JS_SetReservedSlot( cx, obj, SLOT_CIPHERNAME, argv[0] );


	int cipherIdx = find_hash(cipherName);
	RT_ASSERT_1( cipherIdx != -1, "cipher %s is not registred", cipherName )

	char *key;
	int keyLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[1], key, keyLength );

	symmetric_key *pskey = (symmetric_key*)malloc(sizeof(symmetric_key));
//	symmetric_key skey;

	JS_SetPrivate( cx, obj, pskey );

	int setupError = cipher_descriptor[cipherIdx].setup((const unsigned char *)key, keyLength, 0, pskey);
	if ( setupError != CRYPT_OK)
		return ThrowCryptError(cx, setupError);

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool cipher_encrypt(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	

	RT_ASSERT_ARGC( 1 );

	jsval jsvalCipherName;
	JS_GetReservedSlot( cx, obj, SLOT_CIPHERNAME, &jsvalCipherName );

	char *cipherName;
	RT_JSVAL_TO_STRING( jsvalCipherName, cipherName );

	int cipherIdx = find_cipher(cipherName);
	RT_ASSERT_1( cipherIdx != -1, "cipher %s is not registred", cipherName );

	ltc_cipher_descriptor cipher = cipher_descriptor[cipherIdx];

	symmetric_key *pskey = (symmetric_key*)JS_GetPrivate(cx,obj); // malloc(sizeof(symmetric_key));
	RT_ASSERT( pskey != NULL, "scheduled key not initialized" );

	char *pt;
	int ptLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], pt, ptLength );

//	int ptBlocks = 1 + ptLength / cipher.block_length;

	int ctLength = (ptLength/8)*8;

	char *ct = (char*)JS_malloc( cx, ctLength +1 ); // +1 is useless [TBD]
	RT_ASSERT( ct != NULL, RT_ERROR_OUT_OF_MEMORY );

	int setupError = cipher.accel_ecb_encrypt((const unsigned char *)pt, (unsigned char *)ct, ctLength, pskey);
	if ( setupError != CRYPT_OK )
		return ThrowCryptError(cx, setupError);

	ct[ctLength] = 0; // useless because ct will contain 0x00 in its data [TBD]

	JSString *jssCt = JS_NewString( cx, ct, ctLength );

	*rval = STRING_TO_JSVAL(jssCt);

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool cipher_decrypt(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec cipher_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "encrypt"          , cipher_encrypt          , 0, 0, 0 },
 { "decrypt"          , cipher_decrypt          , 0, 0, 0 },
 { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool cipher_getter_myProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec cipher_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "myProperty"            , 0, JSPROP_PERMANENT|JSPROP_READONLY, cipher_getter_myProperty       , NULL },
  { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool cipher_static_getter_myStatic(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec cipher_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "myStatic"                , 0, JSPROP_PERMANENT|JSPROP_READONLY, cipher_static_getter_myStatic         , NULL },
  { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *cipherInitClass( JSContext *cx, JSObject *obj ) {

// if register_cipher failed but the cipher will not be used, it is acceptable because further check is done on find_cipher call

  return JS_InitClass( cx, obj, NULL, &cipher_class, cipher_construct, 1, cipher_PropertySpec, cipher_FunctionSpec, cipher_static_PropertySpec, NULL );
}


/****************************************************************

CTR ( Counter Mode )
	http://en.wikipedia.org/wiki/Counter_mode (fr: http://fr.wikipedia.org/wiki/Mode_d%27op%C3%A9ration_%28cryptographie%29 )


*/