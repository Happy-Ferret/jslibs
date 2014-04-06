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

double PerlinNoise2(double x, double y, double z);

BEGIN_STATIC

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( seed )
  Resets a random sequence of the Mersenne Twister random number generator.
  $H example
  {{{
  Texture.randSeed(1234);
  print( Texture.randReal(), '\n' );
  print( Texture.randReal(), '\n' );
  print( Texture.randReal(), '\n' );
  ...
  }}}
  will always prints:
  {{{
  0.19151945020806033
  0.4976636663772314
  0.6221087664882906
  ...
  }}}
**/
DEFINE_FUNCTION( randSeed ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);

	unsigned int seed;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &seed) );
	init_genrand(seed);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Generates a random number on `[ 0 , 0x7fffffff ]` interval.
**/
DEFINE_FUNCTION( randInt ) {

	JL_DEFINE_ARGS;

	return JL_NativeToJsval(cx, genrand_int32(), *JL_RVAL);
}


/**doc
$TOC_MEMBER $INAME
 $REAL $INAME
  Generates a random number on `[ 0 , 1 ]` real interval.
**/
DEFINE_FUNCTION( randReal ) {

	JL_DEFINE_ARGS;

	return JL_NativeToJsval(cx, genrand_real1(), *JL_RVAL);
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( n, a, b, x [,y [,z] ] )
**/
DEFINE_FUNCTION( perlinNoise ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_RANGE(4,6);

	int n;
	float a, b, x, y, z;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &a) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &b) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &n) );

	JL_CHK( jl::getValue(cx, JL_ARG(4), &x) );
	if ( argc == 4 )
		return JL_NativeToJsval(cx, Noise1DPerlin(x, a, b, n), *JL_RVAL);

	JL_CHK( jl::getValue(cx, JL_ARG(5), &y) );
	if ( argc == 5 )
		return JL_NativeToJsval(cx, Noise2DPerlin(x, y, a, b, n), *JL_RVAL);

	JL_CHK( jl::getValue(cx, JL_ARG(6), &z) );
	if ( argc == 6 )
		return JL_NativeToJsval(cx, Noise3DPerlin(x, y, z, a, b, n), *JL_RVAL);

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Reinitialize the perlin noise state with the current random state.
**/
DEFINE_FUNCTION( perlinNoiseReinit ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MAX(0);

	InitNoise();

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL $INAME(x, y, z)
**/
DEFINE_FUNCTION( perlinNoise2 ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(3);

	double x, y, z;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &x) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &y) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &z) );
	return JL_NativeToJsval(cx, PerlinNoise2(x,y,z), *JL_RVAL);
	JL_BAD;
}


DEFINE_INIT() {

	JL_IGNORE( obj, proto, sc, cx );

	InitNoise();
	return true;
}


CONFIGURE_STATIC

	HAS_INIT

	REVISION(jl::SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( randSeed )
		FUNCTION( randInt )
		FUNCTION( randReal )
		FUNCTION( perlinNoise )
		FUNCTION( perlinNoiseReinit )

		FUNCTION( perlinNoise2 )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

END_STATIC



//unsigned long int NoiseInt(unsigned long int n) {
//
//	n = (n << 13) ^ n;
//	return (n * (n * n * 60493 + 19990303) + 1376312589);
//}
//
//float NoiseReal(unsigned long int n) { // return: 0 <= val <= 1
//
//	return (float)NoiseInt(n) / 4294967296.f;
//}
