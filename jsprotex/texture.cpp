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
extern "C" long genrand_int31(void);
extern "C" unsigned long genrand_int32(void);
extern "C" double genrand_real1(void);



unsigned long int NoiseInt(unsigned long int n) {
	
	n = (n << 13) ^ n;
	return (n * (n * n * 60493 + 19990303) + 1376312589);
}

float NoiseReal(unsigned long int n) { // return: 0 <= val <= 1

	return (float)NoiseInt(n) / 4294967296.f;
}



void TextureSwapBuffers( Texture *tex ) {

	Pixel *tmp = tex->buffer;
	tex->buffer = tex->backBuffer;
	tex->backBuffer = tmp;
}

BEGIN_CLASS( Texture )

DEFINE_FINALIZE() {
	
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	if ( tex != NULL ) {
		
		if ( tex->backBuffer != NULL )
			free(tex->backBuffer);
		if ( tex->buffer != NULL )
			free(tex->buffer);
		free(tex);
		JS_SetPrivate(cx, obj, NULL);
	}
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
	RT_JSVAL_TO_REAL( argv[4], r );
	RT_JSVAL_TO_REAL( argv[5], g );
	RT_JSVAL_TO_REAL( argv[6], b );
	RT_JSVAL_TO_REAL( argv[7], a );

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



double CoherentNoise (double x) {

  int intX = (int)(floor (x));
  double n0 = RealNoise (intX);
  double n1 = RealNoise (intX + 1);
  double weight = x - floor (x);
  double noise = n0 * (1-weight) + n1 * weight;
  return noise;
}
*/

DEFINE_FUNCTION( SetNoise ) { // coloredNoise, seed

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
		
		tex->buffer[i].r = floor( count * tex->buffer[i].r ) / count;
		tex->buffer[i].g = floor( count * tex->buffer[i].g ) / count;
		tex->buffer[i].b = floor( count * tex->buffer[i].b ) / count;
//		tex->buffer[i].a = floor( count * tex->buffer[i].a ) / count;
	}
	return JS_TRUE;
}

DEFINE_FUNCTION( Normalize ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	float min = PMAXLIMIT;
	float max = PMINLIMIT;

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
			min = tex->buffer[i].r;
		if ( tex->buffer[i].g < min )
			min = tex->buffer[i].g;
		if ( tex->buffer[i].b < min )
			min = tex->buffer[i].b;
	}

	float ratio = 1 / ( max - min );
	for ( i = 0; i < size; i++ ) {

		tex->buffer[i].r = ( tex->buffer[i].r - min ) * ratio;
		tex->buffer[i].g = ( tex->buffer[i].g - min ) * ratio;
		tex->buffer[i].b = ( tex->buffer[i].b - min ) * ratio;
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

	float local;
	size_t i, size = tex->width * tex->height;
	for ( i = 0; i < size; i++ ) {
		
		local = max;
		if ( tex->buffer[i].r > local ) local = tex->buffer[i].r;
		if ( tex->buffer[i].g > local ) local = tex->buffer[i].g;
		if ( tex->buffer[i].b > local ) local = tex->buffer[i].b;
		if ( local > max ) {

			float ratio = max / local;
			tex->buffer[i].r *= ratio;
			tex->buffer[i].g *= ratio;
			tex->buffer[i].b *= ratio;
		}

		local = min;
		if ( tex->buffer[i].r < local ) local = tex->buffer[i].r;
		if ( tex->buffer[i].g < local ) local = tex->buffer[i].g;
		if ( tex->buffer[i].b < local ) local = tex->buffer[i].b;
		if ( local < min ) {

			float ratio = min / local;
			tex->buffer[i].r *= ratio;
			tex->buffer[i].g *= ratio;
			tex->buffer[i].b *= ratio;
		}
	}

/*
	if ( keep )
		for ( i = 0; i < size; i++ ) {

			tex->buffer[i].r = MINMAX( tex->buffer[i].r, min, max );
			tex->buffer[i].g = MINMAX( tex->buffer[i].g, min, max );
			tex->buffer[i].b = MINMAX( tex->buffer[i].b, min, max );
		}
	else
		for ( i = 0; i < size; i++ ) {
			
			if ( tex->buffer[i].r > max ) tex->buffer[i].r = PMAX;
			if ( tex->buffer[i].r < min ) tex->buffer[i].r = PMIN;

			if ( tex->buffer[i].g > max ) tex->buffer[i].g = PMAX;
			if ( tex->buffer[i].g < min ) tex->buffer[i].g = PMIN;

			if ( tex->buffer[i].b > max ) tex->buffer[i].b = PMAX;
			if ( tex->buffer[i].b < min ) tex->buffer[i].b = PMIN;
		}
*/
	return JS_TRUE;
}

DEFINE_FUNCTION( PasteAt ) { // (Texture)texture, (int)x, (int)y, (bool)wrap

	RT_ASSERT_ARGC( 4 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	
	int px, py;
	bool wrap;

	JSObject *tex1Obj = JSVAL_TO_OBJECT(argv[0]);
	RT_ASSERT_CLASS( tex1Obj, _class );
	Texture *tex1 = (Texture *)JS_GetPrivate(cx, tex1Obj);
	RT_ASSERT_RESOURCE(tex1);
	RT_JSVAL_TO_INT32( argv[1], px );
	RT_JSVAL_TO_INT32( argv[2], py );
	RT_JSVAL_TO_BOOL( argv[3], wrap );

	size_t texWidth = tex->width;
	size_t texHeight = tex->height;

	size_t tex1Width = tex1->width;
	size_t tex1Height = tex1->height;

	int x, y;
	size_t ptex, ptex1;
	if ( wrap ) {
		for ( y = 0; y < tex1Height; y++ )
			for ( x = 0; x < tex1Width; x++ ) {
				
				ptex = ((py+y)%texHeight) * texWidth + ((px+x)%texWidth);
				ptex1 = y * tex1Width + x;
				tex->buffer[ptex].r = tex1->buffer[ptex1].r;
				tex->buffer[ptex].g = tex1->buffer[ptex1].g;
				tex->buffer[ptex].b = tex1->buffer[ptex1].b;
				tex->buffer[ptex].a = tex1->buffer[ptex1].a;
			}
	} else {
		// other method: calc. the two rect intersection and use this to draw the safe tex1
		for ( y = 0; y < tex1Height; y++ )
			for ( x = 0; x < tex1Width; x++ )
				if ( (px+x) >= 0 && (px+x) < texWidth && (py+y) >= 0 && (py+y) < texHeight ) {

					ptex = (py+y) * texWidth + (px+x);
					ptex1 = y * tex1Width + x;
					tex->buffer[ptex].r = tex1->buffer[ptex1].r;
					tex->buffer[ptex].g = tex1->buffer[ptex1].g;
					tex->buffer[ptex].b = tex1->buffer[ptex1].b;
					tex->buffer[ptex].a = tex1->buffer[ptex1].a;
				}
	}
	return JS_TRUE;
}

DEFINE_FUNCTION( Shift ) {
	// (TBD) I think it is possible to do the Shift operation without using a second buffer. 

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int px, py;
	RT_JSVAL_TO_INT32( argv[0], px );
	RT_JSVAL_TO_INT32( argv[1], py );

	size_t texWidth = tex->width;
	size_t texHeight = tex->height;

	if ( tex->backBuffer == NULL )
		tex->backBuffer = (Pixel*)malloc( tex->width * tex->height * sizeof(Pixel) );
	
	size_t ptex, ptex1;
	int x,y;
	for ( y = 0; y < texHeight; y++ )
		for ( x = 0; x < texWidth; x++ ) {
			
			ptex = ((py+y)%texHeight) * texWidth + ((px+x)%texWidth);
			ptex1 = y * texWidth + x;

			tex->backBuffer[ptex].r = tex->buffer[ptex1].r;
			tex->backBuffer[ptex].g = tex->buffer[ptex1].g;
			tex->backBuffer[ptex].b = tex->buffer[ptex1].b;
			tex->backBuffer[ptex].a = tex->buffer[ptex1].a;
		}
	TextureSwapBuffers(tex);
	return JS_TRUE;
}


DEFINE_FUNCTION( Mix ) { // or lerp (Linear interpolation)

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	JSObject *tex1Obj = JSVAL_TO_OBJECT(argv[0]);
	RT_ASSERT_CLASS( tex1Obj, _class );
	Texture *tex1 = (Texture *)JS_GetPrivate(cx, tex1Obj);
	RT_ASSERT_RESOURCE(tex1);

	JSObject *tex2Obj = JSVAL_TO_OBJECT(argv[1]);
	RT_ASSERT_CLASS( tex2Obj, _class );
	Texture *tex2 = (Texture *)JS_GetPrivate(cx, tex2Obj);
	RT_ASSERT_RESOURCE(tex2);

	size_t i, size = tex->width * tex->height;
	for ( i = 0; i < size; i++ ) {

		float blend;
		blend = tex->buffer[i].r;
		tex->buffer[i].r = ( tex1->buffer[i].r * (PMAX-blend) + tex2->buffer[i].r * blend ) / PMAX;
		blend = tex->buffer[i].g;
		tex->buffer[i].g = ( tex1->buffer[i].g * (PMAX-blend) + tex2->buffer[i].g * blend ) / PMAX;
		blend = tex->buffer[i].b;
		tex->buffer[i].r = ( tex1->buffer[i].b * (PMAX-blend) + tex2->buffer[i].b * blend ) / PMAX;
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( Invert ) { // black -> white / white -> black

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	size_t i, size = tex->width * tex->height;
	for ( i = 0; i < size; i++ ) {
		
		tex->buffer[i].r = PMID - (tex->buffer[i].r - PMID);
		tex->buffer[i].g = PMID - (tex->buffer[i].g - PMID);
		tex->buffer[i].b = PMID - (tex->buffer[i].b - PMID);
		tex->buffer[i].a = PMID - (tex->buffer[i].a - PMID);
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( Absolute ) { // 

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	float zero;
	RT_JSVAL_TO_REAL(argv[0], zero);

	size_t i, size = tex->width * tex->height;
	for ( i = 0; i < size; i++ ) {
		
		tex->buffer[i].r = abs(tex->buffer[i].r - zero) + zero;
		tex->buffer[i].g = abs(tex->buffer[i].g - zero) + zero;
		tex->buffer[i].b = abs(tex->buffer[i].b - zero) + zero;
		tex->buffer[i].a = abs(tex->buffer[i].a - zero) + zero;
	}
	return JS_TRUE;
}

DEFINE_FUNCTION( Desaturate ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	float average;
	size_t i, size = tex->width * tex->height;
	for ( i = 0; i < size; i++ ) {
		
		average = ( tex->buffer[i].r + tex->buffer[i].g + tex->buffer[i].b ) / 3.f;
		tex->buffer[i].r = average;
		tex->buffer[i].g = average;
		tex->buffer[i].b = average;
	}
	return JS_TRUE;
}


/*

float TextureFx::InterpolateLinear( float nx, float ny ) { // 0..1

	float fx = (_width -1) * nx;
	float fy = (_height-1) * ny;

	size_t x = fx;
	size_t y = fy;
	size_t x1 = (x+1)<_width  ? x+1 : x;
	size_t y1 = (y+1)<_height ? y+1 : y;

	float dx = fx - x;
	float dy = fy - y;

	float e = Data( x, y  ) * (1 - dx) + Data( x1, y  ) * dx;
	float f = Data( x, y1 ) * (1 - dx) + Data( x1, y1 ) * dx;
	return e * (1 - dy) + f * dy;
}


DEFINE_FUNCTION( Resample ) {

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int sx, sy;
	RT_JSVAL_TO_INT32( argv[0], sx );
	RT_JSVAL_TO_INT32( argv[1], sy );

	size_t texWidth = tex->width;
	size_t texHeight = tex->height;

//	if ( tex->backBuffer == NULL )
//		tex->backBuffer = (Pixel*)malloc( tex->width * tex->height * sizeof(Pixel) );
	
	size_t ptex, ptex1;
	int x,y;
	for ( y = 0; y < sy; y++ )
		for ( x = 0; x < sx; x++ )
			tex->buffer[ x + y * sx ] = InterpolateLinear( x / (float)sx, y / (float)sy );

	return JS_TRUE;
}

*/



DEFINE_FUNCTION( Polarize ) {
	return JS_TRUE;
}

DEFINE_FUNCTION( UnPolarize ) {
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
	for ( size_t i = 0; i < size; i++ ) {
		tex->buffer[i].r += tex1->buffer[i].r;
		tex->buffer[i].g += tex1->buffer[i].g;
		tex->buffer[i].b += tex1->buffer[i].b;
//		tex->buffer[i].a += tex1->buffer[i].a;
	}
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

DEFINE_PROPERTY( width ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	jsdouble d = tex->width;
	RT_CHECK_CALL( JS_NewNumberValue(cx, d, vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY( height ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	jsdouble d = tex->height;
	RT_CHECK_CALL( JS_NewNumberValue(cx, d, vp) );
	return JS_TRUE;
}

DEFINE_FUNCTION( RandSeed ) {

	RT_ASSERT_ARGC(1);
	unsigned long seed;
	RT_JSVAL_TO_UINT32(argv[0], seed);
	init_genrand(seed);
	return JS_TRUE;
}

DEFINE_FUNCTION( Rand ) {

	jsdouble d = genrand_int32();
	RT_CHECK_CALL( JS_NewNumberValue(cx, d, rval) );
	return JS_TRUE;
}

DEFINE_FUNCTION( Noise ) {

	RT_ASSERT_ARGC(1);
	unsigned long seed;
	RT_JSVAL_TO_UINT32(argv[0], seed);
	jsdouble d = NoiseInt(seed);
	RT_CHECK_CALL( JS_NewNumberValue(cx, d, rval) );
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

//DEFINE_FUNCTION( Call ) {
//	return JS_TRUE;
//}


//DEFINE_FUNCTION( Func ) {
//	return JS_TRUE;
//}

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( SetValue, 4 )
		FUNCTION_ARGC( SetNoise, 2 )
		FUNCTION_ARGC( MultValue, 4 )
		FUNCTION_ARGC( Rect, 8 )
		FUNCTION_ARGC( PasteAt, 4 )
		FUNCTION_ARGC( Shift, 2 )
		FUNCTION_ARGC( Pixels, 2 )
		FUNCTION_ARGC( Cells, 3 )
		FUNCTION_ARGC( Clamp, 3 )
		FUNCTION_ARGC( Mix, 2 )
		FUNCTION_ARGC( Normalize, 0 )
		FUNCTION_ARGC( Invert, 0 )
		FUNCTION_ARGC( Absolute, 0 )
		FUNCTION_ARGC( Aliasing, 0 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(width)
		PROPERTY_READ(height)
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_ARGC( RandSeed, 1 )
		FUNCTION_ARGC( Rand, 0 )
		FUNCTION_ARGC( Noise, 1 )
	END_STATIC_FUNCTION_SPEC


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

RGB to HSV color space conversion
	http://en.literateprograms.org/RGB_to_HSV_color_space_conversion_(C)#chunk%20def:compute%20hue

	http://en.wikipedia.org/wiki/Hue#Computing_hue_from_RGB
	
*/
