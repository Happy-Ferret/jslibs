#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>

#include "crypt.h"

#include <tomcrypt.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void crypt_Finalize(JSContext *cx, JSObject *obj) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass crypt_class = { "crypt", JSCLASS_HAS_PRIVATE,
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, crypt_Finalize
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool crypt_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	if ( !JS_IsConstructing(cx) ) {

		JS_ReportError( cx, "construction is needed" );
		return JS_FALSE;
	}



prng_state prng;
unsigned char buf[10];
int err;
/* start it */
if ((err = yarrow_start(&prng)) != CRYPT_OK) {
printf("Start error: %s\n", error_to_string(err));
}
/* add entropy */
if ((err = yarrow_add_entropy((unsigned char*)"hello world", 11, &prng)) != CRYPT_OK) {
printf("Add_entropy error: %s\n", error_to_string(err));
}
/* ready and read */
if ((err = yarrow_ready(&prng)) != CRYPT_OK) {
printf("Ready error: %s\n", error_to_string(err));
}
printf("Read %lu bytes from yarrow\n", yarrow_read(buf, 10, &prng));


   register_cipher(&rijndael_enc_desc);
   register_hash(&sha256_desc);


	if (register_prng(&yarrow_desc) == -1) {
      printf("Error registering sprng PRNG\n");
      exit(-1);
   }


	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool crypt_myFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec crypt_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "myFunction"          , crypt_myFunction          , 0, 0, 0 },
 { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool crypt_getter_myProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec crypt_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "myProperty"            , 0, JSPROP_PERMANENT|JSPROP_READONLY, crypt_getter_myProperty       , NULL },
  { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool crypt_static_getter_myStatic(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec crypt_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "myStatic"                , 0, JSPROP_PERMANENT|JSPROP_READONLY, crypt_static_getter_myStatic         , NULL },
  { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *cryptInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &crypt_class, crypt_construct, 1, crypt_PropertySpec, crypt_FunctionSpec, crypt_static_PropertySpec, NULL );
}


/****************************************************************

*/