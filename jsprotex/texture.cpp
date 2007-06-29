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

inline PTYPE* PosByMode( const Texture *tex, int x, int y, BorderMode mode ) {

	switch ( mode ) {
		case borderWrap:
			x = ABS( x % tex->width );
			y = ABS( y % tex->height );
			break;
		case borderMirror:
			if ( x < 0 ) x = - x;
			else if ( x >= tex->width ) x = tex->width - 1 - x;
			if ( y < 0 ) y = - y;
			else if ( y >= tex->height ) y = tex->height - 1 - y;
			break;
		case borderClamp:
		case borderValue:
			if ( x < 0 ) x = 0;
			else if ( x >= tex->width ) x = tex->width - 1;
			if ( y < 0 ) y = 0;
			else if ( y >= tex->height ) y = tex->height - 1;
			break;
	}
	return &tex->cbuffer[ ( x + y * tex->width ) * tex->channels ];
}


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
	tex->cbackBuffer = NULL;
	if ( JSVAL_IS_OBJECT(argv[0]) && JS_GET_CLASS(cx, JSVAL_TO_OBJECT(argv[0])) == _class ) { // copy constructor

		Texture *srcTex = (Texture *)JS_GetPrivate(cx, JSVAL_TO_OBJECT(argv[0]));
		RT_ASSERT_RESOURCE(srcTex);

		int tsize = srcTex->width * srcTex->height * srcTex->channels;
		tex->cbuffer = (PTYPE*)malloc( tsize * sizeof(PTYPE) ); // (TBD) try with js_malloc
		RT_ASSERT_ALLOC( tex->buffer );

		memcpy( tex->cbuffer, srcTex->cbuffer, tsize * sizeof(PTYPE) );
		tex->width = srcTex->width;
		tex->height = srcTex->height;
		tex->channels = srcTex->channels;
	} else {

		RT_ASSERT_ARGC( 3 );

		int width, height, channels;
		RT_JSVAL_TO_UINT32( argv[0], width );
		RT_JSVAL_TO_UINT32( argv[1], height );
		RT_JSVAL_TO_UINT32( argv[2], channels );

		if ( channels > PMAXCHANNELS )
			REPORT_ERROR("Too many channels.");


		tex->cbuffer = (PTYPE*)malloc( width * height * channels * sizeof(PTYPE) ); // (TBD) try with js_malloc
		RT_ASSERT_ALLOC( tex->cbuffer );

		tex->width = width;
		tex->height = height;
		tex->channels = channels;
	}
	JS_SetPrivate(cx, obj, tex);
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

	if ( srcChannel >= tex1Channel || srcChannel > PMAXCHANNELS )
		REPORT_ERROR("Invalid source channel.");

	if ( dstChannel >= texChannel || dstChannel > PMAXCHANNELS )
		REPORT_ERROR("Invalid destination channel.");

	int size = tex->width * tex->height;
	for ( int i = 0; i < size; i++ )
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

	int width = tex->width;
	int height = tex->height;

	RT_ASSERT( x0 >= 0 && x0 < x1 && x1 < width  &&  y0 >= 0 && y0 < y1 && y1 < height, "Invalid size" );

	float r,g,b,a;
	RT_JSVAL_TO_REAL( argv[4], r );
	RT_JSVAL_TO_REAL( argv[5], g );
	RT_JSVAL_TO_REAL( argv[6], b );
	RT_JSVAL_TO_REAL( argv[7], a );

	for ( int x = x0; x < x1; x++ )
		for ( int y = y0; y < y1; y++ ) {

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
		if (i % j == 0)
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
	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] = genrand_real1(); // no common noize chunk between two different seed
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( SetPixels ) {

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int seed;
	int count;
	RT_JSVAL_TO_UINT32( argv[0], count );

	if ( argc >= 2 )
		RT_JSVAL_TO_INT32( argv[1], seed )
	else
		seed = 0;

	int size = tex->width * tex->height;
	int channels = tex->channels;

	init_genrand(seed);

	int pos;
	int c;
	for ( int i = 0; i < count; i++ ) {

		pos = genrand_int32() % size * channels;
		for ( c = 0; c < channels; c++ )
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

	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] = floor( count * (tex->cbuffer[i]-PMIN) ) / count + PMIN;
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( NormalizeLevels ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	float min = PMAXLIMIT;
	float max = PMINLIMIT;

	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ ) {

		if ( tex->cbuffer[i] > max )
			max = tex->cbuffer[i];
		if ( tex->cbuffer[i] < min )
			min = tex->cbuffer[i];
	}

	float amp = max - min;
	for ( int i = 0; i < tsize; i++ )
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
	int c, channels = tex->channels;
	int size = tex->width * tex->height;
	int i, pos;
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
	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ ) {

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

	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] = (tex->cbuffer[i]-PMIN) + (tex1->cbuffer[i]-PMIN) + PMIN;

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Invert ) { // black -> white / white -> black

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] = PMID - (tex->cbuffer[i] - PMID);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( MirrorLevels ) {

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	float boundary;
	RT_JSVAL_TO_REAL(argv[0], boundary);
	
	bool bottom;
	if ( argc >= 2 && !JSVAL_IS_VOID(argv[1]) )
		RT_JSVAL_TO_BOOL(argv[1], bottom)
	else
		bottom = true;

	int tsize = tex->width * tex->height * tex->channels;
	
	if ( bottom )
		for ( int i = 0; i < tsize; i++ )
			tex->cbuffer[i] = ABS(tex->cbuffer[i] - boundary) + boundary;
	else
		for ( int i = 0; i < tsize; i++ )
			tex->cbuffer[i] = -( ABS(-tex->cbuffer[i] + boundary) - boundary);

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( WrapLevels ) { //

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	float wrap;
	RT_JSVAL_TO_REAL(argv[0], wrap);

	float div;
	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ ) {

		div = tex->cbuffer[i] / wrap;
		tex->cbuffer[i] = div - floor( div );
	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

DEFINE_FUNCTION( RGBToHLS ) { // (TBD) test it
	// source: http://support.microsoft.com/kb/q29240/

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT( tex->channels == 3, "Invalid pixel format (need RGB).");

	int channels = tex->channels;

	int size = tex->width * tex->height;
	char c;
	int pos;
	float R, G, B, H, L, S, min, max, Rdelta, Gdelta, Bdelta;

	for ( int i = 0; i < size; i++ ) {
		
		pos = i * channels;
		R = tex->cbuffer[pos+0];
		G = tex->cbuffer[pos+1];
		B = tex->cbuffer[pos+2];

		min = MIN( R, MIN( G, B ) );
		max = MAX( R, MAX( G, B ) );

		L = ( ((max+min)*PMAX) + PMAX )/(2.f*PMAX);

      if (max == min) {

         S = 0;
         H = (PMAX*2.f/3.f); // undefined
		} else {

			if (L <= (PMAX/2.f))
				S = ( ((max-min)*PMAX) + ((max+min)/2.f) ) / (max+min);
			else
				S = ( ((max-min)*PMAX) + ((2.f*PMAX-max-min)/2.f) ) / (2.f*PMAX-max-min);

			Rdelta = ( ((max-R)*(PMAX/6.f)) + ((max-min)/2.f) ) / (max-min);
			Gdelta = ( ((max-G)*(PMAX/6.f)) + ((max-min)/2.f) ) / (max-min);
			Bdelta = ( ((max-B)*(PMAX/6.f)) + ((max-min)/2.f) ) / (max-min);

			if (R == max)
				H = Bdelta - Gdelta;
			else if (G == max)
				H = (PMAX/3.f) + Rdelta - Bdelta;
			else
				H = ((2.f*PMAX)/3.f) + Gdelta - Rdelta;

			if (H < 0)
				H += PMAX;
			if (H > PMAX)
				H -= PMAX;
		}

		tex->cbuffer[pos+0] = H;
		tex->cbuffer[pos+1] = L;
		tex->cbuffer[pos+2] = S;
	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

inline float HueToRGB(float n1, float n2, float hue) {

	if (hue < 0)
		hue += PMAX;

	if (hue > PMAX)
		hue -= PMAX;

	if (hue < (PMAX/6.f))
		 return ( n1 + (((n2-n1)*hue+(PMAX/12.f))/(PMAX/6.f)) );
	if (hue < (PMAX/2.f))
		return ( n2 );
	if (hue < ((PMAX*2.f)/3.f))
		return ( n1 + (((n2-n1)*(((PMAX*2.f)/3.f)-hue)+(PMAX/12.f))/(PMAX/6.f)) );
	else
		return ( n1 );
}


DEFINE_FUNCTION( HLSToRGB ) { // (TBD) test it
	// source: http://support.microsoft.com/kb/q29240/

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT( tex->channels == 3, "Invalid pixel format (need HLS).");

	int channels = tex->channels;

	int size = tex->width * tex->height;
	char c;
	int pos;
	float R, G, B, H, L, S, magic1, magic2;

	for ( int i = 0; i < size; i++ ) {
		
		pos = i * channels;
		H = tex->cbuffer[pos+0];
		L = tex->cbuffer[pos+1];
		S = tex->cbuffer[pos+2];

      if (S == 0) {

         R = G = B = (L*PMAX)/PMAX;
         if (H != (PMAX*2.f/3.f) ) {
				// (TBD) manage this error
			}
       } else {

         if (L <= (PMAX/2.f))
            magic2 = (L*(PMAX + S) + (PMAX/2.f))/PMAX;
         else
            magic2 = L + S - ((L*S) + (PMAX/2.f))/PMAX;
         magic1 = 2.f*L-magic2;

         /* get RGB, change units from HLSMAX to RGBMAX */ 
         R = (HueToRGB(magic1,magic2,H+(PMAX/3.f))*PMAX + (PMAX/2.f))/PMAX;
         G = (HueToRGB(magic1,magic2,H)*PMAX + (PMAX/2.f)) / PMAX;
         B = (HueToRGB(magic1,magic2,H-(PMAX/3.f))*PMAX + (PMAX/2.f))/PMAX;
      }

		tex->cbuffer[pos+0] = R;
		tex->cbuffer[pos+1] = G;
		tex->cbuffer[pos+2] = B;
	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Desaturate ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	
	RT_ASSERT( tex->channels >= 3, "Texture must be RGB or RGBA." );

	float average;
	int pos, i;
	int channels = tex->channels;
	int size = tex->width * tex->height;
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

	int count;
	ArrayLength(cx, &count, argv[2]);

	RT_ASSERT( count == tex->channels, "Invalid color format." );

	x = ABS( x % tex->width );
	y = ABS( y % tex->height );

	PTYPE pixel[PMAXCHANNELS];
	FloatArrayToVector(cx, count, &argv[2], (float*)&pixel);

	int c, channels = tex->channels;
	int pos = (x + y * tex->width) * channels;
	for ( c = 0; c < channels; c++ )
		tex->cbuffer[pos+c] = pixel[c];

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( SetRectangle ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_ARGC( 3 );

	int x0, y0, x1, y1;
	RT_JSVAL_TO_INT32( argv[0], x0 );
	RT_JSVAL_TO_INT32( argv[1], y0 );
	RT_JSVAL_TO_INT32( argv[2], x1 );
	RT_JSVAL_TO_INT32( argv[3], y1 );

	int count;
	ArrayLength(cx, &count, argv[4]);
	RT_ASSERT( count == tex->channels, "Invalid color." );
	PTYPE pixel[PMAXCHANNELS];
	FloatArrayToVector(cx, count, &argv[4], (float*)&pixel);

	int channels = tex->channels;
	int width = tex->width;
	int height = tex->height;

	int x, y;
	int c, pos;
	for ( y = y0; y < y1; y++ )
		for ( x = x0; x < x1; x++ ) {

			pos = ( ABS(x % width) + ABS(y % height) * width ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[pos+c] = pixel[c];
		}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( SetLevels ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_ARGC( 1 );
	PTYPE pixel[PMAXCHANNELS];

	int count;
	ArrayLength(cx, &count, argv[0]);

	RT_ASSERT( count == tex->channels, "Invalid number of channels." );

	FloatArrayToVector(cx, count, &argv[0], (float*)&pixel);

	int channels = tex->channels;
	int tsize = tex->width * tex->height * channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] = pixel[i % channels];

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( AddLevels ) { // (TBD) test it
	
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_ARGC( 1 );

	PTYPE pixel[PMAXCHANNELS];

	int count;
	ArrayLength(cx, &count, argv[0]);

	RT_ASSERT( count == tex->channels, "Invalid color format." );

	FloatArrayToVector(cx, count, &argv[0], (float*)&pixel);

	int channels = tex->channels;
	int tsize = tex->width * tex->height * channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] += pixel[i % channels];

//	int size = tex->width * tex->height;
//	int i, c;
//	int pos;
//	for ( i = 0; i < size; i++ ) {
//
//		pos = i * channels;
//		for ( c = 0; c < channels; c++ )
//			tex->cbuffer[pos+c] += pixel[c];
//	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( MultLevels ) {
	
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_ARGC( 1 );
	PTYPE pixel[PMAXCHANNELS];
	int count;
	ArrayLength(cx, &count, argv[0]);
	RT_ASSERT( count == tex->channels, "Invalid color format." );
	FloatArrayToVector(cx, count, &argv[0], (float*)&pixel);

	int channels = tex->channels;
	int tsize = tex->width * tex->height * channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] += (tex->cbuffer[i] - PMIN) * pixel[i % channels] + PMIN;

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

/* 

DEFINE_FUNCTION( Contrast ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	
	float contrast;
	RT_JSVAL_TO_REAL( argv[0], contrast );

	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ )
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
	int channels = tex->channels;
	int width = tex->width;
	int height = tex->height;

	int x, y;

	TextureSetupBackBuffer(tex);

	int pos, pos1;
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


DEFINE_FUNCTION( Flip ) {

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	bool flipX, flipY;
	RT_JSVAL_TO_BOOL( argv[0], flipX );
	RT_JSVAL_TO_BOOL( argv[1], flipY );

	TextureSetupBackBuffer(tex);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	int x, y;
	int c;
	int posDst, posSrc;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			posDst = (x + y * width) * channels;
			posSrc = ( (flipX?width-1-x:x) + (flipY?height-1-y:y) * width ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbackBuffer[posDst+c] = tex->cbuffer[posSrc+c];
		}

	TextureSwapBuffers(tex);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Resize ) {

	RT_ASSERT_ARGC( 2 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int newWidth, newHeight;
	bool interpolate;
	RT_JSVAL_TO_REAL( argv[0], newWidth );
	RT_JSVAL_TO_REAL( argv[1], newHeight );
	RT_JSVAL_TO_BOOL( argv[2], interpolate );

	BorderMode borderMode = borderWrap; // (TBD) from function arg

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	PTYPE *newBuffer = (PTYPE*)malloc( newWidth * newHeight * channels * sizeof(PTYPE) );
	RT_ASSERT_ALLOC( newBuffer );

	int spx, spy; // position in the source
	float prx, pry; // pixel ratio
	float rx = (float)width / (float)newWidth; // texture ratio x
	float ry = (float)height / (float)newHeight; // texture ratio y
	float tmp;
	int x, y, c;

	int pos, pos1, pos2, pos3, pos4; // offset in the buffer
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
						// (TBD)
						break;
					case borderWrap:
						pos1 = (  ((spx + 0)        ) + ((spy + 0)         ) * width  ) * channels;
						pos2 = (  ((spx + 1) % width) + ((spy + 0)         ) * width  ) * channels;
						pos3 = (  ((spx + 0)        ) + ((spy + 1) % height) * width  ) * channels;
						pos4 = (  ((spx + 1) % width) + ((spy + 1) % height) * width  ) * channels;
						break;
					case borderMirror:
						// (TBD)
						break;
					case borderValue:
						// (TBD)
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
	if ( tex->cbackBuffer != NULL ) {

		free( tex->cbackBuffer );
		tex->cbackBuffer = NULL;
	}
	
	tex->cbuffer = newBuffer;
	tex->width = newWidth;
	tex->height = newHeight;
	// channels don't change

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Convolution ) {

	// (TBD) accumulate precalculated pixels ? ( like BoxBlur )

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;

	int count;
	RT_CHECK_CALL( ArrayLength(cx, &count, argv[0]) );
	float *kernel = (float*)malloc(sizeof(float) * count);
	RT_ASSERT_ALLOC( kernel );
	RT_CHECK_CALL( FloatArrayToVector(cx, count, &argv[0], kernel) );

	int size = (int)sqrtf(count);

	RT_ASSERT( size * size == count, "Invalid convolution kernel size.");

	BorderMode borderMode;
	if ( argc >= 2 && !JSVAL_IS_VOID(argv[1]) ) {

		int tmp;
		RT_JSVAL_TO_INT32( argv[1], tmp );
		borderMode = (BorderMode)tmp;
	} else
		borderMode = borderWrap;

	float gain;
	bool autoGain;
	RT_JSVAL_TO_BOOL( argv[2], autoGain );

	TextureSetupBackBuffer(tex);

	int offset = size / 2;
	float sizeWeight = count;
	float ratio;

	PTYPE pixel[PMAXCHANNELS];
	int channels = tex->channels;

	int pos;
	int x, y, vx, vy, sx, sy;
	int c;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			gain = 0;

			for ( c = 0; c < channels; c++ )
				pixel[c] = 0;

			switch ( borderMode ) { // Note: it is faster to do the switch outside the inner loop

				case borderWrap:
					for ( vy = 0; vy < size; vy++ )
						for ( vx = 0; vx < size; vx++ ) {
								
							sx = ABS((x + vx-offset) % width);
							sy = ABS((y + vy-offset) % height);
						
							pos = (sx + sy * width) * channels;
							ratio =  kernel[vx + vy * size];
							gain += ratio;
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
								sx = ABS(-sx % width);
							else if ( sx >= width )
								sx = ABS((width-sx) % width);

							if ( sy < 0 )
								sy = ABS(-sy % height);
							else if ( sy >= height )
								sy = ABS((height-sy) % height);

							pos = (sx + sy * width)*channels;
							ratio =  kernel[vx + vy * size];
							gain += ratio;
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
							ratio =  kernel[vx + vy * size];
							gain += ratio;
							for ( c = 0; c < channels; c++ )
								pixel[c] += tex->cbuffer[pos+c] * ratio;
						}
					break;
			}

			pos = (x + y * width) * channels;
			for ( c = 0; c < channels; c++ ) {

				// (TBD) manage gain

				tex->cbackBuffer[pos+c] = pixel[c];
			}
		}

	free(kernel);
	TextureSwapBuffers(tex);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( BoxBlur ) {

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;

	int bw, bh;
	RT_JSVAL_TO_INT32( argv[0], bw );
	RT_JSVAL_TO_INT32( argv[1], bh );

	RT_ASSERT( bw >= 0 && bw < width, "Invalid width." );
	RT_ASSERT( bh >= 0 && bh < height, "Invalid height." );

	int channels = tex->channels;

	PTYPE *line = (PTYPE*)malloc( MAX(width,height) * channels * sizeof(PTYPE) ); // line buffer
	PTYPE sum[PMAXCHANNELS];

	int x, y, c;
	for ( y = 0; y < height; y++ ) {

		for ( c = 0; c < channels; c++ ) {
			
			for ( x = 0; x < width; x++ )
				line[x * channels + c] = tex->cbuffer[(x + y * width) * channels + c];

			sum[c] = 0;
		
			for ( x = 0; x < bw; x++ )
				sum[c] += line[x * channels + c];

			for ( x = 0; x < width; x++ ) {
	
				tex->cbuffer[ ( (x + bw/2) % width + y * width ) * channels + c ] = sum[c] / bw;
				sum[c] = sum[c] - line[x * channels + c] + line[( (x + bw) % width ) * channels + c];
			}
		}
	}

	for ( x = 0; x < width; x++ ) {

		for ( c = 0; c < channels; c++ ) {
			
			for ( y = 0; y < height; y++ )
				line[y * channels + c] = tex->cbuffer[(x + y * width) * channels + c];

			sum[c] = 0;
		
			for ( y = 0; y < bh; y++ )
				sum[c] += line[y * channels + c];

			for ( y = 0; y < height; y++ ) {
	
				tex->cbuffer[ ( x + ((y+bh/2) % height) * width ) * channels + c ] = sum[c] / bh;
				sum[c] = sum[c] - line[y * channels + c] + line[( (y + bh) % height ) * channels + c];
			}
		}
	}

	free( line );

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Gradiant ) {

	RT_ASSERT_ARGC( 4 );
	
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int count;

	PTYPE pixel1[PMAXCHANNELS];
	ArrayLength(cx, &count, argv[0]);
	RT_ASSERT( count == tex->channels, "Invalid number of channels." );
	FloatArrayToVector(cx, count, &argv[0], (float*)&pixel1);

	PTYPE pixel2[PMAXCHANNELS];
	ArrayLength(cx, &count, argv[0]);
	RT_ASSERT( count == tex->channels, "Invalid number of channels." );
	FloatArrayToVector(cx, count, &argv[1], (float*)&pixel2);

	PTYPE pixel3[PMAXCHANNELS];
	ArrayLength(cx, &count, argv[0]);
	RT_ASSERT( count == tex->channels, "Invalid number of channels." );
	FloatArrayToVector(cx, count, &argv[2], (float*)&pixel3);

	PTYPE pixel4[PMAXCHANNELS];
	ArrayLength(cx, &count, argv[0]);
	RT_ASSERT( count == tex->channels, "Invalid number of channels." );
	FloatArrayToVector(cx, count, &argv[3], (float*)&pixel4);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	float aspectRatio = 1.f / (float)(width * height);

	float r1, r2, r3, r4;
	int x, y, c;
	int pos;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			r1 = (float)((width-x) * (height-y)) * aspectRatio;
			r2 = (float)((x)       * (height-y)) * aspectRatio;
			r3 = (float)((width-x) * (y))        * aspectRatio;
			r4 = (float)((x)       * (y))        * aspectRatio;
			
			pos = ( x + y * width ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[pos+c] = pixel1[c]*r1 + pixel2[c]*r2 + pixel3[c]*r3 + pixel4[c]*r4;
		}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( NormalizeVectors ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;
	int c;

	int size = tex->width * tex->height;
	float val, sum, ratio;
	int pos;
	for ( int i = 0; i < size; i++ ) {
		
		pos = i * channels;
		sum = 0;
		for ( c = 0; c < channels; c++ ) {
			
			val = PZNORM( tex->cbuffer[pos+c] );
			sum += val*val;
		}

		ratio = sum == 0.f ? 0.f : (1.f / sqrt(sum));

		for ( c = 0; c < channels; c++ ) {

			val = PZNORM( tex->cbuffer[pos+c] );
			val *= ratio;
			tex->cbuffer[pos+c] = PUNZNORM(val);
		}
	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Normals ) {

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	if ( tex->channels != 3 && tex->cbackBuffer != NULL ) { // back buffer do not have the right format

		free( tex->cbackBuffer );
		tex->cbackBuffer = NULL;
	}

	if ( tex->cbackBuffer == NULL )
		tex->cbackBuffer = (PTYPE*)malloc( tex->width * tex->height * 3 * sizeof(PTYPE) );

	// from here, tex->cbackBuffer is a 3 channels buffer

	float amp;
	RT_JSVAL_TO_REAL( argv[0], amp );

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	float dX, dY, fPix;
	int x, y;
	int pos;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			//Y Sobel filter
			fPix = tex->cbuffer[(  (x-1 + width) % width + (((y+1) % height)*width)  ) * channels];
			dY  = PNORM(fPix) * -1.0f;

			fPix = (float)tex->cbuffer[(  x % width + (((y+1) % height)*width)  ) * channels];
			dY+= PNORM(fPix) * -2.0f;

			fPix = (float)tex->cbuffer[(  (x+1) % width + (((y+1) % height)*width)  ) * channels];
			dY+= PNORM(fPix) * -1.0f;

			fPix = (float)tex->cbuffer[(  (x-1+width) % width + (((y-1+height) % height)*width)  ) * channels];
			dY+= PNORM(fPix) * 1.0f;

			fPix = (float)tex->cbuffer[(  x % width + (((y-1+height) % height)*width)  ) * channels];
			dY+= PNORM(fPix) * 2.0f;

			fPix = (float)tex->cbuffer[(  (x+1) % width + (((y-1+height) % height)*width)  ) * channels];
			dY+= PNORM(fPix) * 1.0f;

			//X Sobel filter
			fPix = (float)tex->cbuffer[(  (x-1+width) % width + (((y-1+height) % height)*width)  ) * channels];
			dX  = PNORM(fPix) * -1.0f;

			fPix = (float)tex->cbuffer[(  (x-1+width) % width + ((y % height)*width)  ) * channels];
			dX+= PNORM(fPix) * -2.0f;

			fPix = (float)tex->cbuffer[(  (x-1+width) % width + (((y+1) % height)*width)  ) * channels];
			dX+= PNORM(fPix) * -1.0f;

			fPix = (float)tex->cbuffer[(  (x+1) % width + (((y-1+height) % height)*width)  ) * channels];
			dX+= PNORM(fPix) * 1.0f;

			fPix = (float)tex->cbuffer[(  (x+1) % width + ((y % height)*width)  ) * channels];
			dX+= PNORM(fPix) * 2.0f;

			fPix = (float)tex->cbuffer[(  (x+1) % width + (((y+1) % height)*width)  ) * channels];
			dX+= PNORM(fPix) * 1.0f;

			dX *= amp;
			dY *= amp;

			float n = sqrt( dX*dX + dY*dY + 1.f ); // sqrt( x^2 + y^2 + z^2 )

			pos = (x + y * width) * 3; // 3 is the number of channels
			tex->cbackBuffer[pos+0] = PUNZNORM( dX  / n );
			tex->cbackBuffer[pos+1] = PUNZNORM( dY  / n );
			tex->cbackBuffer[pos+2] = PUNZNORM( 1.f / n );
		}

	TextureSwapBuffers(tex);

	if ( tex->channels != 3 ) {

		free( tex->cbackBuffer );
		tex->cbackBuffer = NULL;
	}

	tex->channels = 3;

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}



DEFINE_FUNCTION( Light ) {

//	RT_ASSERT_ARGC( 4 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_OBJECT(argv[0]);
	JSObject *normalsObj = JSVAL_TO_OBJECT(argv[0]);
	RT_ASSERT_CLASS( normalsObj, _class );
	Texture *normals = (Texture*)JS_GetPrivate(cx, normalsObj);
	RT_ASSERT_RESOURCE(normals);
	RT_ASSERT( normals->channels == 3, "Invalid normals texture." );

	int count;
	PTYPE ambient[3];
	ArrayLength(cx, &count, argv[1]);
	RT_ASSERT( count == 3, "Invalid ambient color." );
	FloatArrayToVector(cx, 3, &argv[1], (float*)&ambient);

	PTYPE diffuse[3];
	ArrayLength(cx, &count, argv[2]);
	RT_ASSERT( count == 3, "Invalid diffuse color." );
	FloatArrayToVector(cx, 3, &argv[1], (float*)&diffuse);

	PTYPE specular[3];
	ArrayLength(cx, &count, argv[3]);
	RT_ASSERT( count == 3, "Invalid specular color." );
	FloatArrayToVector(cx, 3, &argv[1], (float*)&specular);


	// (TBD)


	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}



DEFINE_FUNCTION( Trim ) { // (TBD) test this new version that use memcpy

	RT_ASSERT_ARGC( 4 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int x0, y0, x1, y1;
	RT_JSVAL_TO_INT32( argv[0], x0 );
	RT_JSVAL_TO_INT32( argv[1], y0 );
	RT_JSVAL_TO_INT32( argv[2], x1 );
	RT_JSVAL_TO_INT32( argv[3], y1 );

	int width = tex->width;
	int height = tex->height;

	int channels = tex->channels;

	int newWidth = x1-x0;
	int newHeight = y1-y0;

	if ( tex->cbackBuffer != NULL ) {

		free( tex->cbackBuffer );
		tex->cbackBuffer = NULL;
	}

	int srcLineLength = width * channels * sizeof(PTYPE);
	int dstLineLength = newWidth * channels * sizeof(PTYPE);

	tex->cbackBuffer = (PTYPE*)malloc( newHeight * dstLineLength );

	char *pSrc = (char*)tex->cbuffer + ( x0 + y0 * width ) * channels * sizeof(PTYPE);
	char *pDst = (char*)tex->cbackBuffer;

	for ( int y = 0; y < newHeight; y++ ) {

		memcpy( pDst, pSrc, dstLineLength);
		pDst += dstLineLength;
		pSrc += srcLineLength;
	}

/*
	int x, y;
	int c;
	int posDst, posSrc;
	for ( y = 0; y < newHeight; y++ )
		for ( x = 0; x < newWidth; x++ ) {
			
			posDst = (x + y * newWidth) * channels;
			posSrc = ((x0+x) + (y0+y) * width) * channels;
			for ( c = 0; c < channels; c++ )
				newBuffer[posDst+c] = tex->cbuffer[posSrc+c];
		}
*/

	TextureSwapBuffers(tex);
	free( tex->cbackBuffer );
	tex->cbackBuffer = NULL;

	tex->width = newWidth;
	tex->height = newHeight;

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


/// <<<<< WORGING ON USING cbuffer instead of buffer  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





DEFINE_FUNCTION( Paste ) { // (Texture)texture, (int)x, (int)y, (bool)wrap

	RT_ASSERT_ARGC( 4 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_OBJECT(argv[0]);
	JSObject *tex1Obj = JSVAL_TO_OBJECT(argv[0]);
	RT_ASSERT_CLASS( tex1Obj, _class );
	Texture *tex1 = (Texture *)JS_GetPrivate(cx, tex1Obj);
	RT_ASSERT_RESOURCE(tex1);

	RT_ASSERT( tex->channels == tex1->channels, "Invalid channel count." );

	int px, py;
	RT_JSVAL_TO_INT32( argv[1], px );
	RT_JSVAL_TO_INT32( argv[2], py );
	bool wrap;
	RT_JSVAL_TO_BOOL( argv[3], wrap );


	BorderMode mode = borderClamp;

	int channels = tex->channels;

	int texWidth = tex->width;
	int texHeight = tex->height;

	int tex1Width = tex1->width;
	int tex1Height = tex1->height;

	int x, y, tx, ty;
	int posDst, posSrc;
	int c;

	for ( y = 0; y < tex1Height; y++ )
		for ( x = 0; x < tex1Width; x++ ) {

			tx = px + x;
			ty = py + y;
			switch (mode) {
				case borderWrap:				
					posDst = ( ABS(tx % texWidth) + ABS(ty % texHeight) * texWidth ) * channels;
					posSrc = ( y * tex1Width + x ) * channels;
					for ( c = 0; c < channels; c++ )
						tex->cbuffer[posDst+c] = tex1->cbuffer[posSrc+c];
					break;

			case borderClamp:
				if ( tx >= 0 && tx < texWidth && ty >= 0 && ty < texHeight ) {

					posDst = ( tx + ty * texWidth ) * channels;
					posSrc = ( y * tex1Width + x ) * channels;
					for ( c = 0; c < channels; c++ )
						tex->cbuffer[posDst+c] = tex1->cbuffer[posSrc+c];
				}
				break;
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

	int width = tex->width;
	int height = tex->height;

	TextureSetupBackBuffer(tex);


	int x, y; // destination image x, y
	int sx, sy; // source image x, y
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			sx = x + offsetX;
			sy = y + offsetY;

			switch (mode) {
				case borderWrap:
					sx = ABS( sx % width);
					sy = ABS( sy % height);
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

	int width = tex->width;
	int height = tex->height;

	RT_ASSERT( width == tex1->width && height == tex1->height, "Textures must have the same size." );

	TextureSetupBackBuffer(tex);

	BorderMode mode = borderClamp;

	int x, y, tx, ty;
	float ox, oy, oz, norm;
	int pos, pos1;
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
					pos1 = ABS(tx % width) + ABS(ty % height) * width;
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

	int width = tex->width;
	int height = tex->height;
	int c, channels = tex->channels;

	int size = width * height;
	PTYPE* pPxDst = tex->cbuffer;
	int count = density * density;
	Point *cellPoints = (Point*)malloc(sizeof(Point)*count);
	init_genrand(seed);

	for ( int i = 0; i < count; i++ ) {

		float rand1 = genrand_real1();
		float rand2 = genrand_real1();
		int x = i % density;
		int y = i / density;

		cellPoints[i].x = (x+0.5f+(rand1-0.5f)*(1.f-regularity))/density;
		cellPoints[i].y = (y+0.5f+(rand2-0.5f)*(1.f-regularity))/density;
	}

	for (int y=0; y<height; y++) {
		for (int x=0; x<width; x++) {

			Point pixelPos;
			pixelPos.x = (float)x/(float)width,
			pixelPos.y = (float)y/(float)height;

			float minDist = 10.f;
			float nextMinDist = minDist;
			int xo = x*density/width;
			int yo = y*density/height;
			for ( int v=-1; v<2; ++v ) {

				int vo = ((yo+density+v) % density)*density;
				for ( int u=-1; u<2; ++u ) {

					Point cellPos = cellPoints[((xo+density+u) % density) + vo];
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
			for ( c = 0; c < channels; c++ )
				pPxDst[c] = minDist;
			pPxDst += channels;
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
		FUNCTION_ARGC( SetPixels, 2 )
		FUNCTION_ARGC( SetLevels, 4 )
		FUNCTION_ARGC( SetRectangle, 5 )
		FUNCTION_ARGC( SetNoise, 1 )
		FUNCTION_ARGC( AddLevels, 4 )
		FUNCTION_ARGC( MultLevels, 4 )
//		FUNCTION_ARGC( Contrast, 1 )
//		FUNCTION_ARGC( Rect, 8 )
		FUNCTION_ARGC( Resize, 3 )
		FUNCTION_ARGC( Trim, 4 )
		FUNCTION_ARGC( Paste, 4 )
		FUNCTION_ARGC( Flip, 2 )
		FUNCTION_ARGC( Shift, 2 )
		FUNCTION_ARGC( Gradiant, 4 )
		FUNCTION_ARGC( Convolution, 1 )
		FUNCTION_ARGC( Normals, 1 )
		FUNCTION_ARGC( NormalizeVectors, 0 )
		FUNCTION( Light )
		FUNCTION_ARGC( BoxBlur, 2 )
		FUNCTION_ARGC( Cells, 3 )
		FUNCTION_ARGC( Clamp, 3 )
		FUNCTION_ARGC( Displace, 2 )
		FUNCTION_ARGC( Mix, 2 )
		FUNCTION_ARGC( NormalizeLevels, 0 )
		FUNCTION_ARGC( Invert, 0 )
		FUNCTION_ARGC( MirrorLevels, 2 )
		FUNCTION_ARGC( WrapLevels, 1 )
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
