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

#include "../common/jsConversionHelper.h"


extern "C" void init_genrand(unsigned long s);
extern "C" long genrand_int31(void);
extern "C" unsigned long genrand_int32(void);
extern "C" double genrand_real1(void);

enum BorderMode { borderClamp, borderWrap, borderMirror, borderValue };

/*
inline Point PixelByBorderMode( Point *out, const Point in, const Point rect, BorderMode borderMode ) {
	
	int sx, sy;
	switch ( borderMode ) { // Note: it is faster to do the switch outside the inner loop

		case borderClamp:
			break;
		case borderWrap:
			out->x = in.x % rect.x;
			out->y = in.y % rect.y;
			break;
		case borderMirror:
			if ( in.x < 0 ) in.x = -in.x;
			if ( in.x >= rect.x ) in.x = rect.x-in.x;
			if ( in.y < 0 ) in.y = -in.y;
			if ( in.y >= rect.y ) in.y = rect.y-in.y;

			break;
	}
}
*/

unsigned long int NoiseInt(unsigned long int n) {

	n = (n << 13) ^ n;
	return (n * (n * n * 60493 + 19990303) + 1376312589);
}

float NoiseReal(unsigned long int n) { // return: 0 <= val <= 1

	return (float)NoiseInt(n) / 4294967296.f;
}




BEGIN_CLASS( Texture )

DEFINE_FINALIZE() {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	if ( tex != NULL ) {

		if ( tex->cbackBuffer != NULL )
			free(tex->cbackBuffer);
		if ( tex->cbuffer != NULL )
			free(tex->cbuffer);
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

		size_t tsize = srcTex->width * srcTex->height * srcTex->channels;
		tex->cbuffer = (PTYPE*)malloc( tsize * sizeof(PTYPE) ); // (TBD) try with js_malloc
		RT_ASSERT_ALLOC( tex->buffer );

		memcpy( tex->cbuffer, srcTex->cbuffer, tsize * sizeof(PTYPE) );
		tex->width = srcTex->width;
		tex->height = srcTex->height;
		tex->channels = srcTex->channels;
	} else {

		RT_ASSERT_ARGC( 3 );

		size_t width, height, channels;
		RT_JSVAL_TO_UINT32( argv[0], width );
		RT_JSVAL_TO_UINT32( argv[1], height );
		RT_JSVAL_TO_UINT32( argv[2], channels );

		tex->cbuffer = (PTYPE*)malloc( width * height * channels * sizeof(PTYPE) ); // (TBD) try with js_malloc
		RT_ASSERT_ALLOC( tex->cbuffer );

		tex->width = width;
		tex->height = height;
		tex->channels = channels;
	}
	JS_SetPrivate(cx, obj, tex);
	return JS_TRUE;
}


DEFINE_FUNCTION( Decompose ) {
	return JS_TRUE;
}

DEFINE_FUNCTION( Compose ) {
	return JS_TRUE;
}

DEFINE_FUNCTION( SetChannel ) { // src texture, src channel, dest channel

	RT_ASSERT_ARGC( 3 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_OBJECT(argv[1]);
	JSObject *tex1Obj = JSVAL_TO_OBJECT(argv[1]);
	RT_ASSERT_CLASS( tex1Obj, _class );
	Texture *tex1 = (Texture *)JS_GetPrivate(cx, tex1Obj);
	RT_ASSERT_RESOURCE(tex1);

	int srcChannel, dstChannel;
	RT_JSVAL_TO_INT32( argv[0], dstChannel );
	RT_JSVAL_TO_INT32( argv[2], srcChannel );

	int texChannel = tex->channels;
	int tex1Channel = tex1->channels;

	if ( tex->width != tex1->width || tex->height != tex1->height )
		REPORT_ERROR("Images must have the same size.");

	if ( srcChannel >= tex1Channel )
		REPORT_ERROR("Invalid source channel.");

	if ( dstChannel >= texChannel )
		REPORT_ERROR("Invalid destination channel.");

	size_t size = tex->width * tex->height;
	for ( size_t i = 0; i < size; i++ )
		tex->cbuffer[texChannel * i + dstChannel] = tex1->cbuffer[tex1Channel * i + srcChannel];

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


/*
DEFINE_FUNCTION( Rect ) { // use Paste instead

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

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}
*/


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

DEFINE_FUNCTION( SetNoise ) { // seed

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int seed;
	if ( argc >= 1 )
		RT_JSVAL_TO_INT32( argv[0], seed )
	else 
		seed = 0;

	init_genrand(seed);
	size_t tsize = tex->width * tex->height * tex->channels;
	for ( size_t i = 0; i < tsize; i++ )
		tex->cbuffer[i] = genrand_real1(); // no common noize chunk between two different seed
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Pixels ) {

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int seed, count;
	RT_JSVAL_TO_UINT32( argv[0], count );
	RT_JSVAL_TO_INT32( argv[1], seed );

	size_t size = tex->width * tex->height;

	init_genrand(seed);

	size_t pos;
	int c;
	size_t rand;
	for ( size_t i = 0; i < count; i++ ) {

		pos = genrand_int32() % size * tex->channels;
		for ( c = pos; c < tex->channels; c++ )
			tex->cbuffer[pos+c] = PMAX;
	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Aliasing ) {

	RT_ASSERT_ARGC( 1 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	float count;
	RT_JSVAL_TO_UINT32( argv[0], count );

	size_t tsize = tex->width * tex->height * tex->channels;
	for ( size_t i = 0; i < tsize; i++ )
		tex->cbuffer[i] = floor( count * tex->cbuffer[i] ) / count;
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Normalize ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	float min = PMAXLIMIT;
	float max = PMINLIMIT;

	size_t tsize = tex->width * tex->height * tex->channels;
	for ( size_t i = 0; i < tsize; i++ ) {

		if ( tex->cbuffer[i] > max )
			max = tex->cbuffer[i];
		if ( tex->cbuffer[i] < min )
			min = tex->cbuffer[i];
	}

	float tmp, amp = max - min;
	for ( size_t i = 0; i < tsize; i++ )
		tex->cbuffer[i] = PMIN + PAMP * ( tex->cbuffer[i] - min ) / amp; // value is normalized to PMIN...PMAX

	*rval = OBJECT_TO_JSVAL(obj);
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

	float lmin, lmax, tmp, ratio;
	int c;
	size_t channels = tex->channels;
	size_t size = tex->width * tex->height;
	size_t i, pos;
	for ( i = 0; i < size; i++ ) {

		lmax = max;
		lmin = min;

		pos = i * channels;
		for ( c = 0; c < channels; c++ ) {
			
			tmp = tex->cbuffer[pos+c];
			if ( tmp > lmax )
				lmax = tmp;
			if ( tmp < lmin )
				lmin = tmp;
		}
		
		if ( lmax > max ) {

			ratio = max / lmax;
			for ( c = 0; c < channels; c++ ) {

				tmp = tex->cbuffer[pos+c];
				tex->cbuffer[pos+c] = ( tmp - PMIN ) / ratio + PMIN;
			}
		}

		if ( lmin < min ) {

			ratio = min / lmin;
			for ( c = 0; c < channels; c++ ) {

				tmp = tex->cbuffer[pos+c];
				tex->cbuffer[pos+c] = ( tmp - PMIN ) / ratio + PMIN;
			}
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Mix ) { // or lerp (Linear interpolation)

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_OBJECT(argv[0]);
	JSObject *tex1Obj = JSVAL_TO_OBJECT(argv[0]);
	RT_ASSERT_CLASS( tex1Obj, _class );
	Texture *tex1 = (Texture *)JS_GetPrivate(cx, tex1Obj);
	RT_ASSERT_RESOURCE(tex1);

	RT_ASSERT_OBJECT(argv[1]);
	JSObject *tex2Obj = JSVAL_TO_OBJECT(argv[1]);
	RT_ASSERT_CLASS( tex2Obj, _class );
	Texture *tex2 = (Texture *)JS_GetPrivate(cx, tex2Obj);
	RT_ASSERT_RESOURCE(tex2);

	// (TBD) check textures sized

	float blend;
	size_t tsize = tex->width * tex->height * tex->channels;
	for ( size_t i = 0; i < tsize; i++ ) {

		blend = PNORM(tex->cbuffer[i]); // 0..1
		tex->cbuffer[i] = ( (tex1->cbuffer[i]-PMIN) * blend + (tex2->cbuffer[i]-PMIN) * (1-blend) ) + PMIN;
	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}



DEFINE_FUNCTION( AddTexture ) {

	RT_ASSERT_ARGC( 1 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_OBJECT(argv[0]);
	JSObject *tex1Obj = JSVAL_TO_OBJECT(argv[0]);
	RT_ASSERT_CLASS( tex1Obj, _class );
	Texture *tex1 = (Texture *)JS_GetPrivate(cx, tex1Obj);
	RT_ASSERT_RESOURCE(tex1);

	if ( tex->width != tex1->width || tex->height != tex1->height )
		REPORT_ERROR("Images must have the same size.");

	size_t tsize = tex->width * tex->height * tex->channels;
	for ( size_t i = 0; i < tsize; i++ )
		tex->cbuffer[i] = (tex->cbuffer[i]-PMIN) + (tex1->cbuffer[i]-PMIN) + PMIN;

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Invert ) { // black -> white / white -> black

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	size_t tsize = tex->width * tex->height * tex->channels;
	for ( size_t i = 0; i < tsize; i++ )
		tex->cbuffer[i] = PMID - (tex->cbuffer[i] - PMID);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Absolute ) { //

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	float zero;
	RT_JSVAL_TO_REAL(argv[0], zero);

	size_t tsize = tex->width * tex->height * tex->channels;
	for ( size_t i = 0; i < tsize; i++ )
		tex->cbuffer[i] = abs(tex->cbuffer[i] - zero) + zero;
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Wrap ) { //

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	float wrap;
	RT_JSVAL_TO_REAL(argv[0], wrap);

	float div;
	size_t tsize = tex->width * tex->height * tex->channels;
	for ( size_t i = 0; i < tsize; i++ ) {

		div = tex->cbuffer[i] / wrap;
		tex->cbuffer[i] = div - floor( div );
	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Desaturate ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	
	RT_ASSERT( tex->channels >= 3, "Texture must be RGB or RGBA." );

	float average;
	size_t pos, i;
	size_t channels = tex->channels;
	size_t size = tex->width * tex->height;
	for ( i = 0; i < size; i++ ) {

		pos = i * channels;
		average = ( tex->cbuffer[pos+0] + tex->cbuffer[pos+1] + tex->cbuffer[pos+2] ) / 3.f;
		tex->cbuffer[pos+0] = average;
		tex->cbuffer[pos+1] = average;
		tex->cbuffer[pos+2] = average;
	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

DEFINE_FUNCTION( SetPixel ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_ARGC( 3 );

	int x, y;
	RT_JSVAL_TO_INT32( argv[0], x );
	RT_JSVAL_TO_INT32( argv[1], y );

	size_t count;
	ArrayLength(cx, &count, argv[2]);

	RT_ASSERT( count == tex->channels, "Invalid number of channels." );
	RT_ASSERT( x >= 0 && x < tex->width, "Invalid x position." );
	RT_ASSERT( y >= 0 && y < tex->height, "Invalid y position." );

	PTYPE pixel[PMAXCHANNELS];
	FloatArrayToVector(cx, count, &argv[2], (float*)&pixel);

	size_t channels = tex->channels;
	size_t pos = (x + y * tex->width) * channels;
	for ( int c = 0; c < channels; c++ )
		tex->cbuffer[pos+c] = pixel[c];

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}



DEFINE_FUNCTION( SetValue ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_ARGC( 1 );

	PTYPE pixel[PMAXCHANNELS];

	size_t count;
	ArrayLength(cx, &count, argv[0]);

	RT_ASSERT( count == tex->channels, "Invalid number of channels." );

	FloatArrayToVector(cx, count, &argv[0], (float*)&pixel);

	size_t channels = tex->channels;
	size_t size = tex->width * tex->height;
	int i, c;
	size_t pos;
	for ( i = 0; i < size; i++ ) {

		pos = i * channels;
		for ( c = 0; c < channels; c++ )
			tex->cbuffer[pos+c] = pixel[c];
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( AddValue ) {
	
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_ARGC( 1 );

	PTYPE pixel[PMAXCHANNELS];

	size_t count;
	ArrayLength(cx, &count, argv[0]);

	RT_ASSERT( count == tex->channels, "Invalid number of channels." );

	FloatArrayToVector(cx, count, &argv[0], (float*)&pixel);

	size_t channels = tex->channels;
	size_t size = tex->width * tex->height;
	int i, c;
	size_t pos;
	for ( i = 0; i < size; i++ ) {

		pos = i * channels;
		for ( c = 0; c < channels; c++ )
			tex->cbuffer[pos+c] += pixel[c];
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( MultValue ) {
	
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_ARGC( 1 );

	float pixel[PMAXCHANNELS];

	size_t count;
	ArrayLength(cx, &count, argv[0]);

	RT_ASSERT( count == tex->channels, "Invalid number of channels." );

	FloatArrayToVector(cx, count, &argv[0], (float*)&pixel);

	size_t channels = tex->channels;
	size_t size = tex->width * tex->height;
	int i, c;
	size_t pos;
	for ( i = 0; i < size; i++ ) {

		pos = i * channels;
		for ( c = 0; c < channels; c++ )
			tex->cbuffer[pos+c] = (tex->cbuffer[pos+c] - PMIN) * pixel[c] + PMIN;
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

/* 

DEFINE_FUNCTION( Contrast ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	
	float contrast;
	RT_JSVAL_TO_REAL( argv[0], contrast );

	size_t tsize = tex->width * tex->height * tex->channels;
	for ( size_t i = 0; i < tsize; i++ )
		tex->cbuffer[i] = PZNORM(tex->cbuffer[i]) * contrast + PMID;

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

*/


DEFINE_FUNCTION( Rotate90 ) { // (TBD) test it

	RT_ASSERT_ARGC( 1 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int turn;
	RT_JSVAL_TO_INT32(argv[0], turn);
	turn = abs( turn % (360/90) );
	size_t channels = tex->channels;
	size_t width = tex->width;
	size_t height = tex->height;

	int x, y;

	TextureSetupBackBuffer(tex);

	size_t i, pos, pos1;
	int c;

	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			pos = (x + y * width) * channels;
			switch ( turn ) {
				case 0:
					pos1 = pos;
					break;
				case 1:
					pos1 = ((width-1-y) + (x) * width) * channels;
					break;
				case 2:
					pos1 = ((width-1-x) + (height-1-y) * width) * channels;
					break;
				case 3:
					pos1 = ((y) + (height-1-x) * width) * channels;
					break;
			}
			for ( c = 0; c < channels; c++ )
				tex->cbackBuffer[pos+c] = tex->cbuffer[pos1+c];
		}

	switch ( turn ) {
		case 1:
		case 3:
			tex->height = width;
			tex->width = height;
		case 0:
		case 2:
			break;
	}

	TextureSwapBuffers(tex);

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}



DEFINE_FUNCTION( Resize ) {

	RT_ASSERT_ARGC( 2 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	size_t newWidth, newHeight;
	bool interpolate;
	RT_JSVAL_TO_REAL( argv[0], newWidth );
	RT_JSVAL_TO_REAL( argv[1], newHeight );
	RT_JSVAL_TO_REAL( argv[2], interpolate );

	BorderMode borderMode = borderWrap; // (TBD) from function arg

	size_t width = tex->width;
	size_t height = tex->height;
	size_t channels = tex->channels;

	PTYPE *newBuffer = (PTYPE*)malloc( newWidth * newHeight * channels * sizeof(PTYPE) );
	RT_ASSERT_ALLOC( newBuffer );

	PTYPE pixel[PMAXCHANNELS];
	int spx, spy; // position in the source
	float prx, pry; // pixel ratio
	float rx = (float)width / (float)newWidth; // texture ratio x
	float ry = (float)height / (float)newHeight; // texture ratio y
	float tmp;
	int x, y, c;

	size_t pos, pos1, pos2, pos3, pos4; // offset in the buffer
	float ratio1, ratio2, ratio3, ratio4; // pixel value ratio

	for ( y = 0; y < newHeight; y++ )
		for ( x = 0; x < newWidth; x++ ) {	

			if ( interpolate ) {

				prx = modf( (float)x*rx, &tmp );
				spx = (int)tmp;
				pry = modf( (float)y*ry, &tmp );
				spy = (int)tmp;

				// (TBD) for radio1,..4, try to use the distance ?
				switch ( borderMode ) { // Note: it is faster to do the switch outside the inner loop

					case borderClamp:
						break;
					case borderWrap:
						pos1 = (  ((spx + 0)        ) + ((spy + 0)         ) * width  ) * channels;
						pos2 = (  ((spx + 1) % width) + ((spy + 0)         ) * width  ) * channels;
						pos3 = (  ((spx + 0)        ) + ((spy + 1) % height) * width  ) * channels;
						pos4 = (  ((spx + 1) % width) + ((spy + 1) % height) * width  ) * channels;
						break;
					case borderMirror:
						break;
					case borderValue:
						break;
				}

				ratio1 = (1.f - prx) * (1.f - pry);
				ratio2 = (prx) * (1.f - pry);
				ratio3 = (1.f - prx) * (pry);
				ratio4 = (prx) * (pry);

				pos = (x + y * newWidth) * channels;
				for ( c = 0; c < channels; c++ )
					newBuffer[pos+c] = tex->cbuffer[pos1+c] * ratio1 + tex->cbuffer[pos2+c] * ratio2 + tex->cbuffer[pos3+c] * ratio3 + tex->cbuffer[pos4+c] * ratio4;
			} else {

				spx = (int)((float)x*rx);
				spy = (int)((float)y*ry);
				for ( c = 0; c < channels; c++ )
					newBuffer[ (x + y * newWidth) * channels + c ] = tex->cbuffer[ (spx + spy * width) * channels + c ];
			}
		}

	free( tex->cbuffer );
	if ( tex->backBuffer != NULL ) {

		free( tex->backBuffer );
		tex->backBuffer = NULL;
	}
	
	tex->cbuffer = newBuffer;
	tex->width = newWidth;
	tex->height = newHeight;
	// channels don't change

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}




DEFINE_FUNCTION( Convolution ) {

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	size_t width = tex->width;
	size_t height = tex->height;

	size_t count;
	RT_CHECK_CALL( ArrayLength(cx, &count, argv[0]) );
	float *vector = (float*)malloc(sizeof(float) * count);
	RT_ASSERT_ALLOC( vector );
	RT_CHECK_CALL( FloatArrayToVector(cx, count, &argv[0], vector) );

	int size = (int)sqrtf(count);

	RT_ASSERT( size * size == count, "Invalid convolution kernel size.");

	BorderMode borderMode = borderClamp; // (TBD) from function arg

	TextureSetupBackBuffer(tex);

	int offset = size / 2;
	float sizeWeight = count;
	float ratio;

//	float r, g, b, a;
	PTYPE pixel[PMAXCHANNELS];
	size_t channels = tex->channels;
	int c;

	size_t pos;
	int x, y, vx, vy, sx, sy;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

//			r = g = b = a = 0;
			for ( c = 0; c < channels; c++ )
				pixel[c] = 0;

			switch ( borderMode ) { // Note: it is faster to do the switch outside the inner loop

				case borderWrap:
					for ( vy = 0; vy < size; vy++ )
						for ( vx = 0; vx < size; vx++ ) {
								
							sx = (x + vx-offset)%width;
							sy = (y + vy-offset)%height;
						
							pos = (sx + sy * width)*channels;
							ratio =  vector[vx + vy * size];
							for ( c = 0; c < channels; c++ )
								pixel[c] += tex->cbuffer[pos+c] * ratio;
						}
					break;

				case borderMirror:
					for ( vy = 0; vy < size; vy++ )
						for ( vx = 0; vx < size; vx++ ) {

							sx = (x + vx-offset);
							sy = (y + vy-offset);

							if ( sx < 0 )
								sx = -sx % width;
							else if ( sx >= width )
								sx = (width-sx) % width;

							if ( sy < 0 )
								sy = -sy % height;
							else if ( sy >= height )
								sy = (height-sy) % height;

							pos = (sx + sy * width)*channels;
							ratio =  vector[vx + vy * size];
							for ( c = 0; c < channels; c++ )
								pixel[c] += tex->cbuffer[pos+c] * ratio;
						}
					break;

				case borderClamp:
				case borderValue:
					for ( vy = 0; vy < size; vy++ )
						for ( vx = 0; vx < size; vx++ ) {

							sx = (x + vx-offset);
							sy = (y + vy-offset);

							if ( sx < 0 )  sx = 0;
							if ( sx >= width )  sx = width-1;
							if ( sy < 0 )  sy = 0;
							if ( sy >= height )  sy = height-1;
							
							pos = (sx + sy * width)*channels;
							ratio =  vector[vx + vy * size];
							for ( c = 0; c < channels; c++ )
								pixel[c] += tex->cbuffer[pos+c] * ratio;
						}
					break;
			}

			pos = (x + y * width) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbackBuffer[pos+c] = pixel[c] / sizeWeight;
		}

	free(vector);
	TextureSwapBuffers(tex);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}



/// <<<<< WORGING ON USING cbuffer instead of buffer  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




DEFINE_FUNCTION( Gradiant ) {

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

DEFINE_FUNCTION( Polarize ) {

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

DEFINE_FUNCTION( UnPolarize ) {

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}



DEFINE_FUNCTION( Trim ) {

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

	size_t newWidth = x1-x0;
	size_t newHeight = y1-y0;

	Pixel *newBuffer = (Pixel*)malloc( newWidth * newHeight * sizeof(Pixel) );

	int x, y;
	for ( y = 0; y < newHeight; y++ )
		for ( x = 0; x < newWidth; x++ )
			newBuffer[ x + y * newWidth ] = tex->buffer[ (x0+x) + (y0+y) * width ];

	free( tex->buffer );
	if ( tex->backBuffer != NULL ) {

		free( tex->backBuffer );
		tex->backBuffer = NULL;
	}
	
	tex->buffer = newBuffer;
	tex->width = newWidth;
	tex->height = newHeight;
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Flip ) {

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	bool flipX, flipY;
	RT_JSVAL_TO_BOOL( argv[0], flipX );
	RT_JSVAL_TO_BOOL( argv[1], flipY );

	TextureSetupBackBuffer(tex);

	size_t width = tex->width;
	size_t height = tex->height;

	int x, y;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ )
			tex->backBuffer[ x + y * width ] = tex->buffer[ (flipX?width-x:x) + (flipY?height-y:y) * width ];
	TextureSwapBuffers(tex);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Paste ) { // (Texture)texture, (int)x, (int)y, (bool)wrap

	RT_ASSERT_ARGC( 4 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int px, py;
	bool wrap;

	RT_ASSERT_OBJECT(argv[0]);
	JSObject *tex1Obj = JSVAL_TO_OBJECT(argv[0]);
	RT_ASSERT_CLASS( tex1Obj, _class );
	Texture *tex1 = (Texture *)JS_GetPrivate(cx, tex1Obj);
	RT_ASSERT_RESOURCE(tex1);
	RT_JSVAL_TO_INT32( argv[1], px );
	RT_JSVAL_TO_INT32( argv[2], py );
	RT_JSVAL_TO_BOOL( argv[3], wrap );

	BorderMode mode = borderClamp;

	size_t texWidth = tex->width;
	size_t texHeight = tex->height;

	size_t tex1Width = tex1->width;
	size_t tex1Height = tex1->height;

	int x, y, tx, ty;
	size_t ptex, ptex1;
	if ( wrap ) {
		for ( y = 0; y < tex1Height; y++ )
			for ( x = 0; x < tex1Width; x++ ) {

				tx = px + x;
				ty = py + y;
				switch (mode) {
					case borderWrap:				
						ptex = (tx % texWidth) + (ty % texHeight) * texWidth;
						ptex1 = y * tex1Width + x;
						tex->buffer[ptex] = tex1->buffer[ptex1];
						break;
				case borderClamp:
					if ( tx >= 0 && tx < texWidth && ty >= 0 && ty < texHeight ) {

						ptex = tx + ty * texWidth;
						ptex1 = y * tex1Width + x;
						tex->buffer[ptex] = tex1->buffer[ptex1];
					}
					break;
				}
			}
	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

DEFINE_FUNCTION( Shift ) {
	// (TBD) I think it is possible to do the Shift operation without using a second buffer.

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int offsetX, offsetY;
	RT_JSVAL_TO_INT32( argv[0], offsetX );
	RT_JSVAL_TO_INT32( argv[1], offsetY );

	BorderMode mode = borderMirror;

	size_t width = tex->width;
	size_t height = tex->height;

	TextureSetupBackBuffer(tex);


	size_t ptex, ptex1;
	int x, y; // destination image x, y
	int sx, sy; // source image x, y
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			sx = x + offsetX;
			sy = y + offsetY;

			switch (mode) {
				case borderWrap:
					sx %= width;
					sy %= height;
					tex->backBuffer[x + y * width] = tex->buffer[sx + sy * width];
					break;

				case borderClamp:
					if ( sx >= 0 && sx < width && sy >= 0 && sy < height )
						tex->backBuffer[x + y * width] = tex->buffer[sx + sy * width];
					break;

				case borderMirror:

					if ( sx < 0 )  sx =  -sx;
					if ( sx >= width )  sx = width - sx;
					if ( sy < 0 )  sy =  -sy;
					if ( sy >= height )  sy = height - sy;
					tex->backBuffer[x + y * width] = tex->buffer[sx + sy * width];
					break;

				case borderValue:

					if ( sx < 0 )  sx = 0;
					if ( sx >= width )  sx = width - 1;
					if ( sy < 0 )  sy = 0;
					if ( sy >= height )  sy = height - 1;
					tex->backBuffer[x + y * width] = tex->buffer[sx + sy * width];
					break;
			}
		}
	TextureSwapBuffers(tex);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Displace ) {

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_OBJECT(argv[0]);
	JSObject *tex1Obj = JSVAL_TO_OBJECT(argv[0]);
	RT_ASSERT_CLASS( tex1Obj, _class );
	Texture *tex1 = (Texture *)JS_GetPrivate(cx, tex1Obj);
	RT_ASSERT_RESOURCE(tex1);

	float factor;
	RT_JSVAL_TO_INT32( argv[1], factor );

	size_t width = tex->width;
	size_t height = tex->height;

	RT_ASSERT( width == tex1->width && height == tex1->height, "Textures must have the same size." );

	TextureSetupBackBuffer(tex);

	BorderMode mode = borderClamp;

	int x, y, tx, ty;
	float ox, oy, oz, norm;
	size_t pos, pos1;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			pos = x + y * width;
			// normalize to (-1,+1)
			ox = PNORM(tex1->buffer[pos].r - PMID);
			oy = PNORM(tex1->buffer[pos].g - PMID);
			oz = PNORM(tex1->buffer[pos].b - PMID);

			norm = sqrtf(ox*ox+oy*oy+oz*oz);
			if ( norm != 0 ) {
				ox /= norm;
				oy /= norm;
//				oz /= norm; // not used
			}
			
			tx = long(x+ox*factor);
			ty = long(y+oy*factor);
			switch (mode) {
				case borderWrap:
					pos1 = tx % width + (ty % height) * width;
					tex->backBuffer[pos] = tex->buffer[pos1];
					break;
				case borderClamp:
					if ( tx >= 0 && tx < width && ty >= 0 && ty < height ) {

						pos1 = tx + ty * width;
						tex->backBuffer[pos] = tex->buffer[pos1];
					}
					break;
				case borderValue:
					if ( tx < 0 ) tx = 0;
					if ( tx >= width ) tx = width - 1;
					if ( ty < 0 ) ty = 0;
					if ( ty >= height ) ty = height - 1;

					pos1 = tx + ty * width;
					tex->backBuffer[pos] = tex->buffer[pos1];
					break;
				case borderMirror:
					if ( tx < 0 )  tx = 0 - tx;
					if ( ty < 0 )  tx = 0 - ty;
					if ( tx >= width )  tx = width - tx;
					if ( ty >= height )  ty = width - ty;
//					if ( tx >= 0 && tx < width && ty >= 0 && ty < height ) { // (TBD) do not manage infinite mirror ?

						pos1 = tx + ty * width;
						tex->backBuffer[pos] = tex->buffer[pos1];
//					}
					break;
			}
		}
	TextureSwapBuffers(tex);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}



DEFINE_FUNCTION( Cells ) { // source: FxGen

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

	*rval = OBJECT_TO_JSVAL(obj);
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

DEFINE_PROPERTY( channels ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	jsdouble d = tex->channels;
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
/*
	BEGIN_CONST_DOUBLE_SPEC
		CONST_DOUBLE( borderClamp, borderClamp );
		CONST_DOUBLE( borderWrap, borderWrap );
		CONST_DOUBLE( borderMirror, borderMirror );
	END_CONST_DOUBLE_SPEC
*/
//DEFINE_FUNCTION( Call ) {
//	return JS_TRUE;
//}


//DEFINE_FUNCTION( Func ) {
//	return JS_TRUE;
//}

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( SetChannel, 3 )
		FUNCTION_ARGC( SetPixel, 3 )
		FUNCTION_ARGC( SetValue, 4 )
		FUNCTION_ARGC( SetNoise, 1 )
		FUNCTION_ARGC( MultValue, 4 )
//		FUNCTION_ARGC( Contrast, 1 )
//		FUNCTION_ARGC( Rect, 8 )
		FUNCTION_ARGC( Resize, 3 )
		FUNCTION_ARGC( Trim, 4 )
		FUNCTION_ARGC( Paste, 4 )
		FUNCTION_ARGC( Flip, 2 )
		FUNCTION_ARGC( Shift, 2 )
		FUNCTION_ARGC( Pixels, 2 )
		FUNCTION_ARGC( Convolution, 1 )
		FUNCTION_ARGC( Cells, 3 )
		FUNCTION_ARGC( Clamp, 3 )
		FUNCTION_ARGC( Displace, 2 )
		FUNCTION_ARGC( Mix, 2 )
		FUNCTION_ARGC( Normalize, 0 )
		FUNCTION_ARGC( Invert, 0 )
		FUNCTION_ARGC( Absolute, 1 )
		FUNCTION_ARGC( Wrap, 1 )
		FUNCTION_ARGC( Aliasing, 0 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(width)
		PROPERTY_READ(height)
		PROPERTY_READ(channels)
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

Convolution & gaussian blur
	http://www.gamedev.net/reference/programming/features/imageproc/page2.asp


Image Processing Algorithms
	http://www.efg2.com/Lab/Library/ImageProcessing/Algorithms.htm


*/
