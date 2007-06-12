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

extern "C" void init_genrand(unsigned long s);
extern "C" unsigned long genrand_int32(void);
extern "C" double genrand_real1(void);


BEGIN_CLASS( Texture )

DEFINE_FINALIZE() {
}

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(_class);
	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)malloc(sizeof(Texture));

	tex->backBuffer = NULL;

	if ( JSVAL_IS_OBJECT(argv[0]) && JS_GET_CLASS(cx, JSVAL_TO_OBJECT(argv[0])) == _class ) { // copy constructor

		Texture *srcTex = (Texture *)JS_GetPrivate(cx, JSVAL_TO_OBJECT(argv[0]));
		RT_ASSERT_RESOURCE(srcTex);

		size_t size = srcTex->width * srcTex->height;
		tex->buffer = (Pixel*)malloc( size * sizeof(PTYPE) ); // (TBD) try with js_malloc
		RT_ASSERT_ALLOC( tex->buffer );

		memcpy( tex->buffer, srcTex->buffer, size * sizeof(PTYPE) );
		tex->width = srcTex->width;
		tex->height = srcTex->height;
		JS_SetPrivate(cx, obj, tex);
	} else {

		RT_ASSERT_ARGC( 2 );

		size_t width, height;
		RT_JSVAL_TO_UINT32( argv[0], width );
		RT_JSVAL_TO_UINT32( argv[1], height );

		size_t size = width * height;
		tex->buffer = (Pixel*)malloc( size * sizeof(Pixel) ); // (TBD) try with js_malloc
		RT_ASSERT_ALLOC( tex->buffer );

		tex->width = width;
		tex->height = height;

		JS_SetPrivate(cx, obj, tex);
	}

	return JS_TRUE;
}

BEGIN_STATIC


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
	RT_JSVAL_TO_REAL( argv[0], r );
	RT_JSVAL_TO_REAL( argv[1], g );
	RT_JSVAL_TO_REAL( argv[2], b );
	RT_JSVAL_TO_REAL( argv[3], a );

	for ( size_t x = x0; x < x1; x++ )
		for ( size_t y = y0; y < y1; y++ ) {
			
			tex->buffer[x + width * y].r = r;
			tex->buffer[x + width * y].g = g;
			tex->buffer[x + width * y].b = b;
			tex->buffer[x + width * y].a = a;
		}
	return JS_TRUE;
}

/* old rand & noise function
inline long sqrt(long i) {

	long r,rnew=1,rold=r;
	do {
		rold=r;
		r=rnew;
		rnew = (r+(i/r));
		rnew >>= 1;
	} while(rold != rnew);
	return rnew;
}

inline int isprime(long i) {
	
	long si,j;
	si = sqrt(i);
	for (j=2; (j<=si); j++)
		if (i%j == 0)
			return 0;
	return 1;
}


unsigned long int IntNoise(unsigned long int a) { // 0 -> 1376312589
	
//	return ( (a * 19990303) % 1376312589 ) ^ ( (a * 60493) % 349997189 );
//	return ( (a * 349997189) % 1376312589 );
//	return (65539*a)%2147483648;

	return 1664525 * a + 1013904223;
}

float RealNoise(unsigned long int n) { // return: 0 <= val <= 1

	n = (n << 13) ^ n;
	n = (n * (n * n * 60493 + 19990303) + 1376312589);
	return (float)n / 4294967296.f;
}


double CoherentNoise (double x) {

  int intX = (int)(floor (x));
  double n0 = RealNoise (intX);
  double n1 = RealNoise (intX + 1);
  double weight = x - floor (x);
  double noise = n0 * (1-weight) + n1 * weight;
  return noise;
}
*/

DEFINE_FUNCTION( Noise ) { // coloredNoise, seed

	RT_ASSERT_ARGC( 1 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	bool color;
	RT_JSVAL_TO_BOOL( argv[0], color );
	int seed;
	RT_JSVAL_TO_INT32( argv[1], seed );
	init_genrand(seed);
	size_t size = tex->width * tex->height;
	for ( size_t i = 0; i < size; i++ ) {
		
		float r,g,b;
		r = genrand_real1(); // no common noize chunk between two different seed
		if ( color ) {

			g = genrand_real1();
			b = genrand_real1();
		} else {

			g = r;
			b = r;
		}
		tex->buffer[i].r = r;
		tex->buffer[i].g = g;
		tex->buffer[i].b = b;
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( Pixels ) {

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int seed, count;
	RT_JSVAL_TO_UINT32( argv[0], count );
	RT_JSVAL_TO_INT32( argv[1], seed );

	size_t width = tex->width;
	size_t height = tex->height;
	size_t size = width * height;

	init_genrand(seed);

	size_t rand = seed * 2 + 1;
	for ( size_t i = 0; i < count; i++ ) {
		
		rand = genrand_int32();
		size_t pos = rand % size;
		tex->buffer[pos].r = PMAX;
		tex->buffer[pos].g = PMAX;
		tex->buffer[pos].b = PMAX;
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( Aliasing ) {

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	float count;
	RT_JSVAL_TO_UINT32( argv[0], count );

	size_t size = tex->width * tex->height;
	for ( size_t i = 0; i < size; i++ ) {
		
		tex->buffer[i].r = count * floor( tex->buffer[i].r / count );
		tex->buffer[i].g = count * floor( tex->buffer[i].g / count );
		tex->buffer[i].b = count * floor( tex->buffer[i].b / count );
		tex->buffer[i].a = count * floor( tex->buffer[i].a / count );
	}

	return JS_TRUE;
}

DEFINE_FUNCTION( Normalize ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	float min = 1.f;
	float max = 0.f;

	size_t size = tex->width * tex->height;
	size_t i;
	for ( i = 0; i < size; i++ ) {
		
		if ( tex->buffer[i].r > max )
			max = tex->buffer[i].r;
		if ( tex->buffer[i].g > max )
			max = tex->buffer[i].g;
		if ( tex->buffer[i].b > max )
			max = tex->buffer[i].b;

		if ( tex->buffer[i].r < min )
			max = tex->buffer[i].r;
		if ( tex->buffer[i].g < min )
			max = tex->buffer[i].g;
		if ( tex->buffer[i].b < min )
			max = tex->buffer[i].b;
	}

	float ratio = 1 / ( max - min );

	for ( i = 0; i < size; i++ ) {

		tex->buffer[i].r = ( tex->buffer[i].r - min ) * ratio;
		tex->buffer[i].r = ( tex->buffer[i].g - min ) * ratio;
		tex->buffer[i].r = ( tex->buffer[i].b - min ) * ratio;
	}

	return JS_TRUE;
}

DEFINE_FUNCTION( Clamp ) { // min, max, keepClampedColor

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	
	RT_ASSERT_ARGC( 3 );

	float min, max;
	bool keep;
	RT_JSVAL_TO_REAL( argv[0], min );
	RT_JSVAL_TO_REAL( argv[1], max );
	RT_JSVAL_TO_BOOL( argv[2], keep );

	size_t size = tex->width * tex->height;
	size_t i;
	if ( keep )
		for ( i = 0; i < size; i++ ) {

			tex->buffer[i].r = MINMAX( tex->buffer[i].r, min, max );
			tex->buffer[i].g = MINMAX( tex->buffer[i].g, min, max );
			tex->buffer[i].b = MINMAX( tex->buffer[i].b, min, max );
//			tex->buffer[i].a = MINMAX( tex->buffer[i].r, min, max );
		}
	else
		for ( i = 0; i < size; i++ ) {

		}

	
	return JS_TRUE;
}

DEFINE_FUNCTION( PasteAt ) {
	return JS_TRUE;
}

DEFINE_FUNCTION( Polarize ) {
	return JS_TRUE;
}

DEFINE_FUNCTION( UnPolarize ) {
	return JS_TRUE;
}

DEFINE_FUNCTION( Lerp ) {
	return JS_TRUE;
}

DEFINE_FUNCTION( Invert ) {
	return JS_TRUE;
}

DEFINE_FUNCTION( Reverse ) {
	return JS_TRUE;
}

DEFINE_FUNCTION( Resample ) {
	return JS_TRUE;
}



DEFINE_FUNCTION( SetValue ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	
	RT_ASSERT_ARGC( 4 );

	float r,g,b,a;
	RT_JSVAL_TO_REAL( argv[0], r );
	RT_JSVAL_TO_REAL( argv[1], g );
	RT_JSVAL_TO_REAL( argv[2], b );
	RT_JSVAL_TO_REAL( argv[3], a );

	size_t size = tex->width * tex->height;
	for ( size_t i = 0; i < size; i++ ) {

		tex->buffer[i].r = r;
		tex->buffer[i].g = g;
		tex->buffer[i].b = b;
		tex->buffer[i].a = a;
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( AddValue ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	RT_ASSERT_ARGC( 4 );
	float r,g,b,a;
	RT_JSVAL_TO_REAL( argv[0], r );
	RT_JSVAL_TO_REAL( argv[1], g );
	RT_JSVAL_TO_REAL( argv[2], b );
	RT_JSVAL_TO_REAL( argv[3], a );
	size_t size = tex->width * tex->height;
	for ( size_t i = 0; i < size; i++ ) {

		tex->buffer[i].r += r;
		tex->buffer[i].g += g;
		tex->buffer[i].b += b;
		tex->buffer[i].a += a;
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( MultValue ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	RT_ASSERT_ARGC( 4 );
	float r,g,b,a;
	RT_JSVAL_TO_REAL( argv[0], r );
	RT_JSVAL_TO_REAL( argv[1], g );
	RT_JSVAL_TO_REAL( argv[2], b );
	RT_JSVAL_TO_REAL( argv[3], a );
	size_t size = tex->width * tex->height;
	for ( size_t i = 0; i < size; i++ ) {

		tex->buffer[i].r *= r;
		tex->buffer[i].g *= g;
		tex->buffer[i].b *= b;
		tex->buffer[i].a *= a;
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( AddTexture ) {

	RT_ASSERT_ARGC( 1 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	JSObject *tex1Obj = JSVAL_TO_OBJECT(argv[1]);
	RT_ASSERT_CLASS( tex1Obj, _class );
	Texture *tex1 = (Texture*)JS_GetPrivate(cx, JSVAL_TO_OBJECT(argv[1]));
	RT_ASSERT_RESOURCE(tex1);

	if ( tex->width != tex1->width || tex->height != tex1->height )
		REPORT_ERROR("Images must have the same size.");

	size_t size = tex->width * tex->height;
	for ( size_t i = 0; i < size; i++ )
		tex->buffer[i] += tex1->buffer[i];
	return JS_TRUE;
}



DEFINE_FUNCTION( Convolution ) {

	return JS_TRUE;
}

DEFINE_FUNCTION( Cells ) { // source: FxGen

	typedef struct {
		float x,y;
	} Point;

	RT_ASSERT_ARGC( 3 );
	
	int seed, density;
	float regularity;
	RT_JSVAL_TO_INT32( argv[0], density );
	RT_JSVAL_TO_REAL( argv[1], regularity );
	RT_JSVAL_TO_INT32( argv[2], seed );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	density = MINMAX( density, 1, 128 );
	regularity = MINMAX( regularity, 0, 1 );

	size_t width = tex->width;
	size_t height = tex->height;
	size_t size = width * height;

	Pixel* pPxDst = tex->buffer;

	size_t count = density * density;

	Point *cellPoints = (Point*)malloc(sizeof(Point)*count);

	init_genrand(seed);
	//Render
	for ( size_t i = 0; i < count; i++ ) {
	
		float rand1 = genrand_real1();
		float rand2 = genrand_real1();
		size_t x = i%density;
		size_t y = i/density;

		cellPoints[i].x = (x+0.5f+(rand1-0.5f)*(1.f-regularity))/density;
		cellPoints[i].y = (y+0.5f+(rand2-0.5f)*(1.f-regularity))/density;
	}

	for (size_t y=0; y<height; y++) {
		for (size_t x=0; x<width; x++) {

			Point pixelPos;
			pixelPos.x = (float)x/(float)width,
			pixelPos.y = (float)y/(float)height;

			float minDist = 10.f;
			float nextMinDist = minDist;
			int xo = x*density/width;
			int yo = y*density/height;
			for ( int v=-1; v<2; ++v ) {
				
				int vo = ((yo+density+v)%density)*density;
				for ( int u=-1; u<2; ++u ) {

					Point cellPos = cellPoints[((xo+density+u)%density) + vo];
					if (u == -1 && x*density < width)
						cellPos.x -= 1;
					if (v == -1 && y*density < height)
						cellPos.y -= 1;
					if (u == 1 && x*density >= width*(density-1))
						cellPos.x += 1;
					if (v == 1 && y*density >= height*(density-1))
						cellPos.y += 1;
					
					float tx = pixelPos.x - cellPos.x;
					float ty = pixelPos.y - cellPos.y;
					float dist = sqrtf( tx*tx + ty*ty );
					
					if (dist < minDist) {

						nextMinDist = minDist;
						minDist = dist;
					} else if (dist < nextMinDist) {
						
						nextMinDist = dist;
					}
				}
			}

			minDist = (nextMinDist-minDist)*density;
			minDist = MINMAX( minDist, 0, 1 );
			pPxDst->r = minDist;
			pPxDst->g = minDist;
			pPxDst->b = minDist;
			pPxDst->a = PMAX;

			pPxDst++;
		}
	}

	free(cellPoints);

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
		FUNCTION_ARGC( Noise, 2 )
		FUNCTION_ARGC( Pixels, 2 )
		FUNCTION_ARGC( Cells, 3 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY(prop)
	END_PROPERTY_SPEC

	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

END_CLASS

/*

TextureMathematique:
	http://raphaello.univ-fcomte.fr/Ig/Textures/ExemplesGLUt/TextureMathematique.htm

Mersenne Twister Random Number Generator
	http://www-personal.engin.umich.edu/~wagnerr/MersenneTwister.h

Mersenne Twist Pseudorandom Number Generator Package
	http://www.cs.hmc.edu/~geoff/mtwist.html#downloading

SFMT (sse2) version of Mersenne Twist Pseudorandom Number Generator
	http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT/index.html

ISAAC: a fast cryptographic random number generator
	http://www.burtleburtle.net/bob/rand/isaacafa.html
	periodicity: 2^40
*/
