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

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_STATIC


/**doc
=== Static functions ===
**/

/**doc
$TOC_MEMBER $INAME
 $STR $INAME( string )
  Encode the given _string_ using base64 encoding.
**/
DEFINE_FUNCTION( Base64Encode ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_MIN( 1 );
	JL_S_ASSERT_STRING(JL_ARG(1));

	const char *in;
	size_t inLength;
	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &in, &inLength) ); // warning: GC on the returned buffer !

	unsigned long outLength;
	outLength = 4 * (((unsigned long)inLength + 2) / 3) +1;
	char *out;
	out = (char *)JS_malloc( cx, outLength +1 );
	JL_CHK( out );
	out[outLength] = '\0';

	int err;
	err = base64_encode( (const unsigned char *)in, (unsigned long)inLength, (unsigned char *)out, &outLength );
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JSString *jssOutData;
	jssOutData = JS_NewString( cx, out, outLength );
	JL_S_ASSERT( jssOutData != NULL, "unable to create the base64 string." );
	*JL_RVAL = STRING_TO_JSVAL(jssOutData);

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME( string )
  Encode the given _string_ using base64 encoding.
**/
DEFINE_FUNCTION( Base64Decode ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	JL_S_ASSERT_STRING(JL_ARG(1));
	const char *in;
	size_t inLength;
	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &in, &inLength) ); // warning: GC on the returned buffer !

	unsigned long outLength;
	outLength = 3 * ((unsigned long)inLength-2) / 4 +1;
	char *out;
	out = (char *)JS_malloc(cx, outLength +1);
	JL_CHK( out );
	out[outLength] = '\0';

	int err;
	err = base64_decode( (const unsigned char *)in, (unsigned long)inLength, (unsigned char *)out, &outLength );
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JL_CHK( JL_NewBlob( cx, out, outLength, JL_RVAL ) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( string )
  Encode the given _string_ using hexadecimal encoding.
**/
DEFINE_FUNCTION( HexEncode ) {

	static const char hex[] = "0123456789ABCDEF";

	JL_S_ASSERT_ARG_MIN( 1 );

	JL_S_ASSERT_STRING(JL_ARG(1));

	const char *in;
	size_t inLength;
	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &in, &inLength) ); // warning: GC on the returned buffer !

	size_t outLength;
	outLength = inLength * 2;
	char *out;
	out = (char *)JS_malloc(cx, outLength +1);
	JL_CHK( out );
	out[outLength] = '\0';

	unsigned char c;
	for ( size_t i=0; i<inLength; ++i ) {

		c = in[i];
		out[i*2+0] = hex[ c >> 4 ];
		out[i*2+1] = hex[ c & 0xF ];
	}

	JSString *jssOutData;
	jssOutData = JS_NewString( cx, out, outLength );
	JL_S_ASSERT( jssOutData != NULL, "unable to create the hex string." );
	*JL_RVAL = STRING_TO_JSVAL(jssOutData);

	return JS_TRUE;
	JL_BAD;
}

#define XX 0

/**doc
$TOC_MEMBER $INAME
 $STR $INAME( string )
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

	JL_S_ASSERT_ARG_MIN( 1 );
	JL_S_ASSERT_STRING(JL_ARG(1));
	const char *in;
	size_t inLength;
	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &in, &inLength) ); // warning: GC on the returned buffer !

	size_t outLength;
	outLength = inLength / 2;
	char *out;
	out = (char *)JS_malloc(cx, outLength +1);
	JL_CHK( out );
	out[outLength] = '\0';

	for ( unsigned long i=0; i<outLength; ++i )
		out[i] = unhex[ (unsigned char)in[i*2] ] << 4 | unhex[ (unsigned char)in[i*2+1] ];

	JL_CHK( JL_NewBlob( cx, out, outLength, JL_RVAL ) );

	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Base64Encode )
		FUNCTION( Base64Decode )
		FUNCTION( HexEncode )
		FUNCTION( HexDecode )
	END_STATIC_FUNCTION_SPEC

END_STATIC
