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


/**doc fileIndex:topmost **/

BEGIN_STATIC


/**doc
=== Static functions ===
**/

/**doc
 * $STR *Base64Encode*( string )
  Encode the given _string_ using base64 encoding.
**/
DEFINE_FUNCTION( Base64Encode ) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_STRING(argv[0]);
	const char *in;
	size_t inLength;
	J_CHK( JsvalToStringAndLength(cx, argv[0], &in, &inLength) );

	unsigned long outLength = 4 * ((inLength + 2) / 3) +1;
	char *out = (char *)JS_malloc( cx, outLength +1 );
	out[outLength] = '\0';
	RT_ASSERT_ALLOC( out );

	int err;
	err = base64_encode( (const unsigned char *)in, inLength, (unsigned char *)out, &outLength );
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JSString *jssOutData = JS_NewString( cx, out, outLength );
	RT_ASSERT( jssOutData != NULL, "unable to create the base64 string." );
	*rval = STRING_TO_JSVAL(jssOutData);

	return JS_TRUE;
}

/**doc
 * $STR *Base64Decode*( string )
  Encode the given _string_ using base64 encoding.
**/
DEFINE_FUNCTION( Base64Decode ) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_STRING(argv[0]);
	const char *in;
	size_t inLength;
	J_CHK( JsvalToStringAndLength(cx, argv[0], &in, &inLength) );

	unsigned long outLength = 3 * (inLength-2) / 4 +1;
	char *out = (char *)JS_malloc(cx, outLength +1);
	RT_ASSERT_ALLOC( out );
	out[outLength] = '\0';

	int err;
	err = base64_decode( (const unsigned char *)in, inLength, (unsigned char *)out, &outLength );
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JSObject *jssOutData = J_NewBinaryString( cx, out, outLength );
	RT_ASSERT( jssOutData != NULL, "unable to create the plaintext string." );
	*rval = OBJECT_TO_JSVAL(jssOutData);

	return JS_TRUE;
}


/**doc
 * $STR *HexEncode*( string )
  Encode the given _string_ using hexadecimal encoding.
**/
DEFINE_FUNCTION( HexEncode ) {

	static const char hex[] = "0123456789ABCDEF";

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_STRING(argv[0]);

	const char *in;
	size_t inLength;
	J_CHK( JsvalToStringAndLength(cx, argv[0], &in, &inLength) );

	unsigned long outLength = inLength * 2;
	char *out = (char *)JS_malloc(cx, outLength +1);
	RT_ASSERT_ALLOC( out );
	out[outLength] = '\0';

	unsigned char c;
	for ( size_t i=0; i<inLength; ++i ) {

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

/**doc
 * $STR *HexDecode*( string )
  Decode the given _string_ using hexadecimal encoding.
**/
DEFINE_FUNCTION( HexDecode ) {

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
	const char *in;
	size_t inLength;
	J_CHK( JsvalToStringAndLength(cx, argv[0], &in, &inLength) );

	unsigned long outLength = inLength / 2;
	char *out = (char *)JS_malloc(cx, outLength +1);
	RT_ASSERT_ALLOC( out );
	out[outLength] = '\0';

	for ( unsigned long i=0; i<outLength; ++i )
		out[i] = unhex[ (unsigned char)in[i*2] ] << 4 | unhex[ (unsigned char)in[i*2+1] ];

//	JSString *jssOutData = JS_NewString( cx, out, outLength );
	JSObject *jssOutData = J_NewBinaryString( cx, out, outLength );
	RT_ASSERT( jssOutData != NULL, "unable to create the plaintext string." );
//	*rval = STRING_TO_JSVAL(jssOutData);
	*rval = OBJECT_TO_JSVAL(jssOutData);

	return JS_TRUE;
}


CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Base64Encode )
		FUNCTION( Base64Decode )
		FUNCTION( HexEncode )
		FUNCTION( HexDecode )
	END_STATIC_FUNCTION_SPEC

END_STATIC
