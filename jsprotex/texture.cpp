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
#include <math.h>

#include <stdlib.h>
#include "texture.h"

BEGIN_CLASS( Texture )

DEFINE_FINALIZE() {
}

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(_class);
	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)malloc(sizeof(Texture));

	tex->backBuffer = NULL;

	if ( JSVAL_IS_OBJECT(argv[1]) && JS_GET_CLASS(cx, JSVAL_TO_OBJECT(argv[1])) == _class ) { // copy constructor

	} else {
		
		unsigned int width, height;
		RT_JSVAL_TO_UINT32( argv[0], width );
		RT_JSVAL_TO_UINT32( argv[1], height );

		tex->buffer = (Pixel *)malloc(sizeof(Pixel) * width * height);

		RT_ASSERT_ARGC( 2 );
	}

	return JS_TRUE;
}

BEGIN_STATIC

DEFINE_FUNCTION( Flat ) {

	RT_ASSERT_ARGC( 4 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	float r,g,b,a;
	RT_JSVAL_TO_FLOAT( argv[0], r );
	RT_JSVAL_TO_FLOAT( argv[1], g );
	RT_JSVAL_TO_FLOAT( argv[2], b );
	RT_JSVAL_TO_FLOAT( argv[3], a );

	size_t size = tex->width * tex->height;
	for ( size_t i = 0; i < size; i++ ) {

		tex->buffer[i].r = r;
		tex->buffer[i].g = g;
		tex->buffer[i].b = b;
		tex->buffer[i].a = a;
	}
	
	return JS_TRUE;
}


DEFINE_FUNCTION( Rect ) {
	
	RT_ASSERT_ARGC( 4 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int x0, y0, x1, y1;
	RT_JSVAL_TO_INT32( argv[0], x0 );
	RT_JSVAL_TO_INT32( argv[1], y0 );
	RT_JSVAL_TO_INT32( argv[2], x1 );
	RT_JSVAL_TO_INT32( argv[3], y1 );

	size_t width = tex->width;
	size_t height = tex->height;

	RT_ASSERT( x0 >= 0 && x0 < x1 && x1 < width  &&  y0 >= 0 && y0 < y1 && y1 < height, "Invalid size" );

	float r,g,b,a;
	if ( !JSVAL_IS_VOID( argv[0] )
		RT_JSVAL_TO_FLOAT( argv[0], r );

	if ( !JSVAL_IS_VOID( argv[1] )
		RT_JSVAL_TO_FLOAT( argv[1], g );
	
	if ( !JSVAL_IS_VOID( argv[2] )
		RT_JSVAL_TO_FLOAT( argv[2], b );
	
	if ( !JSVAL_IS_VOID( argv[3] )
		RT_JSVAL_TO_FLOAT( argv[3], a );

	for ( size_t x = x0; x < x1; x++ )
		for ( size_t y = y0; y < y1; y++ ) {
			
			tex->buffer[x + width * y].r = r;
			tex->buffer[x + width * y].g = g;
			tex->buffer[x + width * y].b = b;
			tex->buffer[x + width * y].a = a;
		}
	return JS_TRUE;
}


float IntegerNoise (int n) {

  n = (n >> 13) ^ n;
  int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
  return 1.0 - ((float)nn / 1073741824.0);
}

double CoherentNoise (double x) {
  int intX = (int)(floor (x));
  double n0 = IntegerNoise (intX);
  double n1 = IntegerNoise (intX + 1);
  double weight = x - floor (x);
  double noise = n0 * (1-weight) + n1 * weight;
  return noise;
}


DEFINE_FUNCTION( Noise ) {
	
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int seed = JsvalToInt(cx, argv[1], 0);

	size_t size = tex->width * tex->height;

	for ( size_t i = 0; i < size; i++ ) {

		float noiseValue = IntegerNoise(seed * size + i); // no common noize chunk between two different seed
		
		tex->buffer[i].r = noiseValue;
		tex->buffer[i].g = noiseValue;
		tex->buffer[i].b = noiseValue;
	}
	return JS_TRUE;
}



CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

//DEFINE_FUNCTION( Call ) {
//	return JS_TRUE;
//}

//DEFINE_PROPERTY( prop ) {
//	return JS_TRUE;
//}

//DEFINE_FUNCTION( Func ) {
//	return JS_TRUE;
//}

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( Flat, 4 )
		FUNCTION_ARGC( Rect, 8 )
		FUNCTION_ARGC( Noise, 1 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY(prop)
	END_PROPERTY_SPEC

	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

END_CLASS
