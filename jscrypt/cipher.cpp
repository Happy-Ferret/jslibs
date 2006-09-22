#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>

#include "cipher.h"

#include <tomcrypt.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void cipher_Finalize(JSContext *cx, JSObject *obj) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool cipher_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ... [TBD]: check JS_InstanceOf( cx, thisObj, &NativeProc, NULL )




/*

unsigned char key[8];
symmetric_key skey;
int err;
// you must register a cipher before you use it 
if (register_cipher(&blowfish_desc)) == -1) {
printf("Unable to register Blowfish cipher.");
return -1;
}
// generic call to function (assuming the key in key[] was already setup) 
if ((err = cipher_descriptor[find_cipher("blowfish")].setup(key, 8, 0, &skey)) !=
CRYPT_OK) {
printf("Error setting up Blowfish: %s\n", error_to_string(err));
return -1;
}

*/

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass cipher_class = { "cipher", JSCLASS_HAS_PRIVATE,
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, cipher_Finalize,
   0,0, cipher_call
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool cipher_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	if ( !JS_IsConstructing(cx) ) {

		JS_ReportError( cx, "construction is needed" );
		return JS_FALSE;
	}

	if ( argc < 2 ) {

		JS_ReportError( cx, "argument is missing" );
		return JS_FALSE;
	}


	char *cipherName = JS_GetStringBytes( JSVAL_TO_STRING(argv[0]) );
	if ( cipherName == NULL ) {

		JS_ReportError( cx, "unable to create cipher string" );
		return JS_FALSE;
	}

	int cipherIdx = find_cipher(cipherName);
	if ( cipherIdx < 0 ) {

		JS_ReportError( cx, "cipher %s is not registred", cipherName );
		return JS_FALSE;
	}

	jsval jsvalKey = JSVAL_TO_STRING(argv[1]);
	char *key = JS_GetStringBytes( jsvalKey );
	if ( key == NULL ) {

		JS_ReportError( cx, "unable to create key string" );
		return JS_FALSE;
	}

	int keyLength = jsvalKey

unsigned char key[8];
symmetric_key skey;
int err;


	int err;
	err = cipher_descriptor[cipherIdx].setup(key, 8, 0, &skey);

	if ( err != CRYPT_OK)
		return ThrowCryptError(err);
	



	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool cipher_myFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec cipher_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "myFunction"          , cipher_myFunction          , 0, 0, 0 },
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

	return JS_InitClass( cx, obj, NULL, &cipher_class, cipher_construct, 1, cipher_PropertySpec, cipher_FunctionSpec, cipher_static_PropertySpec, NULL );
}


/****************************************************************

*/