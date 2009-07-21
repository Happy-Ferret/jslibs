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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C void init_genrand(unsigned long s);
EXTERN_C long genrand_int31(void);
EXTERN_C unsigned long genrand_int32(void);
EXTERN_C double genrand_real1(void);

void InitNoise();
float Noise1DPerlin( float x, float alpha, float beta, int n );
float Noise2DPerlin( float x, float y, float alpha, float beta, int n );
float Noise3DPerlin( float x, float y, float z, float alpha, float beta, int n );



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( seed )
  Resets a random sequence of the Mersenne Twister random number generator.
  $H example
  {{{
  Texture.RandSeed(1234);
  Print( Texture.RandReal(), '\n' );
  Print( Texture.RandReal(), '\n' );
  Print( Texture.RandReal(), '\n' );
  }}}
  will always prints:
  {{{
  0.19151945020806033
  0.4976636663772314
  0.6221087664882906
  }}}
**/
DEFINE_FUNCTION_FAST( RandSeed ) {

	JL_S_ASSERT_ARG_MIN(1);
	unsigned int seed;
	JL_CHK( JsvalToUInt(cx, JL_FARG(1), &seed) );
	init_genrand(seed);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Generates a random number on `[ 0 , 0x7fffffff ]` interval.
**/
DEFINE_FUNCTION_FAST( RandInt ) {

	int i = genrand_int31();
	*JL_FRVAL = INT_TO_JSVAL(i);
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $REAL $INAME
  Generates a random number on `[ 0 , 1 ]` real interval.
**/
DEFINE_FUNCTION_FAST( RandReal ) {

	jsdouble d = genrand_real1();
	JL_CHK( JS_NewNumberValue(cx, d, JL_FRVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( n, a, b, x [,y [,z] ] )
**/
DEFINE_FUNCTION_FAST( PerlinNoise ) {

	JL_S_ASSERT_ARG_RANGE(4,6);
	int n;
	double a, b, x, y, z;
	JL_CHK( JsvalToDouble(cx, JL_FARG(1), &a) );
	JL_CHK( JsvalToDouble(cx, JL_FARG(2), &b) );
	JL_CHK( JsvalToInt(cx, JL_FARG(3), &n) );

	JL_CHK( JsvalToDouble(cx, JL_FARG(4), &x) );
	if ( argc == 4 )
		return DoubleToJsval(cx, Noise1DPerlin(x, a, b, n), JL_FRVAL);

	JL_CHK( JsvalToDouble(cx, JL_FARG(5), &y) );
	if ( argc == 5 )
		return DoubleToJsval(cx, Noise2DPerlin(x, y, a, b, n), JL_FRVAL);

	JL_CHK( JsvalToDouble(cx, JL_FARG(6), &z) );
	if ( argc == 6 )
		return DoubleToJsval(cx, Noise3DPerlin(x, y, z, a, b, n), JL_FRVAL);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Reinitialize the perlin noise state with the current random state. 
**/
DEFINE_FUNCTION_FAST( PerlinNoiseReinit ) {

	JL_S_ASSERT_ARG(0);
	InitNoise();
	return JS_TRUE;
	JL_BAD;
}

DEFINE_INIT() {

	InitNoise();
	return JS_TRUE;
}


CONFIGURE_STATIC

	HAS_INIT

	REVISION(JL_SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST( RandSeed )
		FUNCTION_FAST( RandInt )
		FUNCTION_FAST( RandReal )
		FUNCTION_FAST( PerlinNoise )
		FUNCTION_FAST( PerlinNoiseReinit )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

END_STATIC
