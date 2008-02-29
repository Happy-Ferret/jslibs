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
#include "misc.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static JSBool misc_base64Encode(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_STRING(argv[0]);
	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], in, inLength );

	unsigned long outLength = 4 * ((inLength + 2) / 3) +1;
	char *out = (char *)JS_malloc( cx, outLength );
	RT_ASSERT( out != NULL, RT_ERROR_OUT_OF_MEMORY );

	int err;
	if ( (err=base64_encode( (const unsigned char *)in, inLength, (unsigned char *)out, &outLength )) != CRYPT_OK )
		return ThrowCryptError(cx, err);

	JSString *jssOutData = JS_NewString( cx, out, outLength );
	RT_ASSERT( jssOutData != NULL, "unable to create the base64 string." );
	*rval = STRING_TO_JSVAL(jssOutData);

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static JSBool misc_base64Decode(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_STRING(argv[0]);
	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], in, inLength );

	unsigned long outLength = 3 * (inLength-2) / 4 +1;
	char *out = (char *)JS_malloc( cx, outLength );
	RT_ASSERT( out != NULL, RT_ERROR_OUT_OF_MEMORY );

	int err;
	if ( (err=base64_decode( (const unsigned char *)in, inLength, (unsigned char *)out, &outLength )) != CRYPT_OK )
		return ThrowCryptError(cx, err);

	JSString *jssOutData = JS_NewString( cx, out, outLength );
	RT_ASSERT( jssOutData != NULL, "unable to create the plaintext string." );
	*rval = STRING_TO_JSVAL(jssOutData);

	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static JSBool misc_hex64Encode(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	static const char hex[] = "0123456789ABCDEF";

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_STRING(argv[0]);

	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], in, inLength );

	unsigned long outLength = inLength * 2;
	char *out = (char *)JS_malloc( cx, outLength );
	RT_ASSERT( out != NULL, RT_ERROR_OUT_OF_MEMORY );

	unsigned char c;
	for ( int i=0; i<inLength; ++i ) {

		c = in[i];
		out[i*2+0] = hex[ c >> 4 ];
		out[i*2+1] = hex[ c & 0xF ];
	}

	JSString *jssOutData = JS_NewString( cx, out, outLength );
	RT_ASSERT( jssOutData != NULL, "unable to create the hex string." );
	*rval = STRING_TO_JSVAL(jssOutData);

	return JS_TRUE;
}

#define XX 0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static JSBool misc_hex64Decode(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	static const unsigned char unhex[] = {

		XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
		XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
		XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, XX, XX, XX, XX, XX, XX,
		XX, 10, 11, 12, 13, 14, 15, XX, XX, XX, XX, XX, XX, XX, XX, XX,
		XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
		XX, 10, 11, 12, 13, 14, 15, XX, XX, XX, XX, XX, XX, XX, XX, XX
	};

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_STRING(argv[0]);
	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], in, inLength );

	unsigned long outLength = inLength / 2;
	char *out = (char *)JS_malloc( cx, outLength );
	RT_ASSERT( out != NULL, RT_ERROR_OUT_OF_MEMORY );

	for ( unsigned long i=0; i<outLength; ++i )
		out[i] = unhex[ (unsigned char)in[i*2] ] << 4 | unhex[ (unsigned char)in[i*2+1] ];

	JSString *jssOutData = JS_NewString( cx, out, outLength );
	RT_ASSERT( jssOutData != NULL, "unable to create the plaintext string." );
	*rval = STRING_TO_JSVAL(jssOutData);

	return JS_TRUE;
}



JSFunctionSpec Misc_FunctionSpec[] = { // *name, call, nargs, flags, extra
	{ "Base64Encode"    , misc_base64Encode    , 0, 0, 0 },
	{ "Base64Decode"    , misc_base64Decode    , 0, 0, 0 },
	{ "HexEncode"       , misc_hex64Encode     , 0, 0, 0 },
	{ "HexDecode"       , misc_hex64Decode     , 0, 0, 0 },
//
	{ 0 }
};




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *miscInitClass( JSContext *cx, JSObject *obj ) {

	JS_DefineFunctions( cx, obj, Misc_FunctionSpec );
//	JS_DefineProperties( cx, globalObject, Global_PropertySpec );
	return NULL;
}


/****************************************************************

*/
