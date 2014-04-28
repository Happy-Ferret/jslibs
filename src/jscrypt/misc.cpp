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


/**doc fileIndex:topmost **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
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
DEFINE_FUNCTION( base64Encode ) {

	JL_DEFINE_ARGS;

	JLData in;

		JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_STRING(1);

//	const char *in;
//	size_t inLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &in, &inLength) ); // warning: GC on the returned buffer !
	JL_CHK( jl::getValue(cx, JL_ARG(1), &in) );

	unsigned long outLength;
	outLength = 4 * (((unsigned long)in.Length() + 2) / 3) +1;
	char *out;
	out = (char *)JS_malloc( cx, outLength +1 );
	JL_CHK( out );
	out[outLength] = '\0';

	int err;
	err = base64_encode( (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length(), (unsigned char *)out, &outLength );
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JL_CHK( JLData(out, true, outLength).GetJSString(cx, JL_RVAL) ); // "unable to create the base64 string."

	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME( string )
  Encode the given _string_ using base64 encoding.
**/
DEFINE_FUNCTION( base64Decode ) {

	JL_DEFINE_ARGS;

	jl::AutoBuffer buffer;
	JLData in;

	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_STRING(1);

//	const char *in;
//	size_t inLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &in, &inLength) ); // warning: GC on the returned buffer !
	JL_CHK( jl::getValue(cx, JL_ARG(1), &in) );

	unsigned long outLength;
	outLength = 3 * ((unsigned long)in.Length()-2) / 4 +1; // max outLength
	//uint8_t *out;
	//out = JL_NewBuffer(cx, outLength, JL_RVAL);
	//JL_CHK( out );

	buffer.alloc(outLength);
	JL_ASSERT_ALLOC(buffer);


	int err;
	err = base64_decode( (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length(), buffer.data(), &outLength );
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	//out[outLength] = '\0';
	//JL_CHK( JL_NewBlob( cx, out, outLength, JL_RVAL ) );

	buffer.setSize(outLength);
	JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( string )
  Encode the given _string_ using hexadecimal encoding.
**/
DEFINE_FUNCTION( hexEncode ) {

	static const char hex[] = "0123456789ABCDEF";

	JL_DEFINE_ARGS;

	JLData data;

	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_STRING(1);

	const char *in;
	size_t inLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &in, &inLength) ); // warning: GC on the returned buffer !
	JL_CHK( jl::getValue(cx, JL_ARG(1), &data) );
	in = data.GetConstStr();
	inLength = data.Length();

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

	JL_CHK( JLData(out, true, outLength).GetJSString(cx, JL_RVAL) ); // "unable to create the hex string."

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( string )
  Decode the given _string_ using hexadecimal encoding.
**/
DEFINE_FUNCTION( hexDecode ) {

	static const unsigned char unhex[] = {
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0,
		 0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0
	};

	JL_DEFINE_ARGS;

	jl::AutoBuffer buffer;
	JLData data;

	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_STRING(1);

	const char *in;
	size_t inLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &in, &inLength) ); // warning: GC on the returned buffer !
	JL_CHK( jl::getValue(cx, JL_ARG(1), &data) );
	in = data.GetConstStr();
	inLength = data.Length();

	size_t outLength;
	outLength = inLength / 2;
	
	//uint8_t *out;
	//out = JL_NewBuffer(cx, outLength, JL_RVAL);
	//JL_CHK( out );
	buffer.alloc(outLength);
	JL_ASSERT_ALLOC(buffer);

	for ( unsigned long i=0; i<outLength; ++i )
		buffer.data()[i] = unhex[ (unsigned char)in[i*2] ] << 4 | unhex[ (unsigned char)in[i*2+1] ];

	//out[outLength] = '\0';
	//JL_CHK( JL_NewBlob( cx, out, outLength, JL_RVAL ) );
	JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );

	return true;
	JL_BAD;
}


CONFIGURE_STATIC

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( base64Encode )
		FUNCTION( base64Decode )
		FUNCTION( hexEncode )
		FUNCTION( hexDecode )
	END_STATIC_FUNCTION_SPEC

END_STATIC
