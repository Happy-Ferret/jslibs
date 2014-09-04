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

	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_STRING(1);

	{

		JS::AutoCheckCannotGC nogc;
		jl::BufBase out;
		jl::BufString in;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &in) );

		unsigned long outLength;
		outLength = 4 * ((in.length() + 2) / 3) +1;
		out.alloc(outLength);
		JL_ASSERT_ALLOC( out );

		int err;
		err = base64_encode( in.toData<const uint8_t *>(), in.length(), out.dataAs<uint8_t*>(), &outLength );
		if (err != CRYPT_OK)
			return ThrowCryptError(cx, err);
	
		out.setUsed(outLength);
		JL_CHK( BlobCreate(cx, out, JL_RVAL) );
	}

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
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_STRING(1);

	{

		JS::AutoCheckCannotGC nogc;
		jl::BufBase buffer;
		jl::BufString in;

		JL_CHK( jl::getValue(cx, JL_ARG(1), &in) );

		unsigned long outLength;
		outLength = 3 * (in.length()-2) / 4 +1; // max outLength
		buffer.alloc(outLength);
		JL_ASSERT_ALLOC( buffer );

		int err;
		err = base64_decode( in.toData<const uint8_t *>(), in.length(), buffer.data(), &outLength );
		if (err != CRYPT_OK)
			return ThrowCryptError(cx, err);
	
		buffer.setUsed(outLength);
		JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );
	}


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
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_STRING(1);

	{

		JS::AutoCheckCannotGC nogc;
		jl::BufString data;
		jl::BufBase out;

		JL_CHK( jl::getValue(cx, JL_ARG(1), &data) );
	
		size_t outLength;
		outLength = data.length() * 2;
		out.alloc(outLength, true);
		JL_ASSERT_ALLOC( out );

		const uint8_t *inIt = data.toData<const uint8_t *>();
		const uint8_t *inEnd = inIt + data.length();
		uint8_t *outIt = out.data();

		uint8_t c;
		for ( ; inIt != inEnd; ++inIt, ++outIt ) {
		
			c = *inIt;
			*outIt = hex[ c >> 4 ];
			++outIt;
			*outIt = hex[ c & 0xF ];
		}
	
		JL_CHK( BlobCreate(cx, out, JL_RVAL) );
	
	}

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( string )
  Decode the given _string_ using hexadecimal encoding.
**/
DEFINE_FUNCTION( hexDecode ) {

	static const uint8_t unhex[] = {
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0,
		 0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0
	};

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_STRING(1);

	{

		JS::AutoCheckCannotGC nogc;
		jl::BufString in;
		jl::BufBase out;


		JL_CHK( jl::getValue(cx, JL_ARG(1), &in) );

		size_t outLength;
		outLength = in.length() / 2;
	
		out.alloc(outLength, true);
		JL_ASSERT_ALLOC( out );

		const uint8_t *inIt = in.toData<const uint8_t*>();
		const uint8_t *inEnd = inIt + in.length();
		uint8_t *outIt = out.data();

		uint8_t c;
		for ( ; inIt != inEnd; ++inIt, ++outIt ) {
		
			c = unhex[*inIt] << 4;
			++inIt;
			*outIt = c | unhex[*inIt];
		}

		JL_CHK( BlobCreate(cx, out, JL_RVAL) );
	
	}

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
