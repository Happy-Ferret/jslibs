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

//DECLARE_CLASS( Texture )

#include "texture.h"

#include "../jslang/bstringapi.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>


#include <stdlib.h>
#include <limits.h>

#include "../common/vector3.h"
#include "../common/jsConversionHelper.h"

extern "C" void init_genrand(unsigned long s);
extern "C" long genrand_int31(void);
extern "C" unsigned long genrand_int32(void);
extern "C" double genrand_real1(void);

#define ABS(val) ( (val) < 0 ? -(val) : (val) )

#define MIN(val1, val2) ( (val1) < (val2) ? (val1) : (val2) )

#define MAX(val1, val2) ( (val1) > (val2) ? (val1) : (val2) )

#define MINMAX(val, min, max) ((val) > (max) ? (max) : (val) < (min) ? (min) : (val) )

#include <float.h>

// min 'invisible' value
#define PMINLIMIT FLT_MIN

// max 'invisible' value
#define PMAXLIMIT FLT_MAX

// min 'visible' value
// #define PMIN (0.f) // IMPORTANT: the lowest visible value is always 0, then this macro is no more used

// max 'visible' value
// with bytes, the range should be [0,255]
// BUT with real, the range should be [0,1] or [0,1) ( use 1.f - FLT_EPSILON )
#define PMAX (1.f)

// full amplitude
//#define PAMP (PMAX-PMIN)

// middle pixel value (gray)
#define PMID ( PMAX / 2 )

// normalize the pixel value to range 0..1
#define PNORM(p) ((p) / PMAX)

// un-normalize the pixel value from range 0..1
#define PUNNORM(p) ((p) * PMAX)

// normalize the pixel value to range -1..1
#define PZNORM(p) (PNORM(p) * 2 - 1)

// un-normalize the pixel value from range -1..1
#define PUNZNORM(p) ( (PUNNORM(p) + 1 ) / 2)

#define PRAND (genrand_real1())





inline void TextureSetupBackBuffer( JSContext *cx, Texture *tex ) {

	if ( tex->cbackBuffer == NULL )
		tex->cbackBuffer = (PTYPE*)JS_malloc(cx, tex->width * tex->height * tex->channels * sizeof(PTYPE) );
}

inline void TextureSwapBuffers( Texture *tex ) {

	PTYPE *tmp = tex->cbuffer;
	tex->cbuffer = tex->cbackBuffer;
	tex->cbackBuffer = tmp;
}


enum BorderMode { borderClamp, borderWrap, borderMirror, borderValue };

enum DesaturateMode { desaturateLightness, desaturateSum, desaturateAverage };



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


inline unsigned int Wrap( int value, int limit ) {

	if ( value >= limit )
		return value % limit;
	else if ( value < 0 )
		return limit - (-value) % limit;
	else
		return value;
}

inline unsigned int Mirror( int value, unsigned int limit ) {

	// (TBD)
	return value;
}

inline float Length2D( float a, float b ) {

	return sqrtf(a*a + b*b);
}


inline PTYPE* PosByMode( const Texture *tex, int x, int y, BorderMode mode ) {

	switch ( mode ) {
		case borderWrap:
			x = Wrap(x, tex->width);
			y = Wrap(y, tex->height);
			break;
		case borderMirror:
			if ( x < 0 ) x = - x;
			else if ( x >= tex->width ) x = tex->width - 1 - x;
			if ( y < 0 ) y = - y;
			else if ( y >= tex->height ) y = tex->height - 1 - y;
			break;
		case borderClamp:
			if ( x < 0 || x >= tex->width || y < 0 || y >= tex->height )
				return NULL; // skip
			break;
		case borderValue:
			if ( x < 0 ) x = 0;
			else if ( x >= tex->width ) x = tex->width - 1;
			if ( y < 0 ) y = 0;
			else if ( y >= tex->height ) y = tex->height - 1;
			break;
	}
	return &tex->cbuffer[ ( x + y * tex->width ) * tex->channels ];
}



// levels: number | array | string ('#8800AAFF')
inline JSBool InitLevelData( JSContext* cx, jsval value, int count, PTYPE *level ) {

	if ( JSVAL_IS_NUMBER(value) ) {
		
		jsdouble dval;
		RT_CHECK_CALL( JS_ValueToNumber(cx, value, &dval) );
		PTYPE val = (PTYPE)dval;
		for ( int i = 0; i < count; i++ )
			level[i] = val;
	} else if ( JSVAL_IS_OBJECT(value) && JS_IsArrayObject(cx, JSVAL_TO_OBJECT(value)) ) {

		FloatArrayToVector(cx, count, &value, level);
	} else if ( JSVAL_IS_STRING(value) ) {

		const char *color;
		size_t length;
		RT_JSVAL_TO_STRING_AND_LENGTH( value, color, length );
		if ( *color++ == '#' && ((int)length-1) / 2 >= count ) {
				
			unsigned char val;
			for ( int i = 0; i < count; i++ ) {
			
				if ( *color >= '0' && *color <= '9' ) val = *color - '0';
				else if ( *color >= 'A' && *color <= 'F' ) val = *color - 'A' + 10;
				else if ( *color >= 'a' && *color <= 'f' ) val = *color - 'a' + 10;
				color++;
				val <<= 4;
				if ( *color >= '0' && *color <= '9' ) val |= *color - '0';
				else if ( *color >= 'A' && *color <= 'F' ) val |= *color - 'A' + 10;
				else if ( *color >= 'a' && *color <= 'f' ) val |= *color - 'a' + 10;
				color++;
				level[i] = PMAX * val / 255;
			}			
		}
	}

//	REPORT_ERROR("Invalid level data.");
	return JS_TRUE;
}


// curve: number | function | array
inline JSBool InitCurveData( JSContext* cx, jsval value, int length, float *curve ) { // length is the curve resolution
 
	if ( JSVAL_IS_NUMBER(value) ) {
		
		jsdouble dval;
		RT_CHECK_CALL( JS_ValueToNumber(cx, value, &dval) );
		PTYPE val = (PTYPE)dval;
		for ( int i = 0; i < length; i++ )
			curve[i] = val;
	} else 
	if ( JsvalIsFunction(cx, value) ) {

		jsval val[2], resultValue;
		jsdouble fval;
		for ( int i = 0; i < length; i++ ) {
			
			fval = i / (float)(length-1);
			RT_CHECK_CALL( JS_NewDoubleValue(cx, fval, val) );
			val[1] = INT_TO_JSVAL(i);
			RT_CHECK_CALL( JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), value, 2, val, &resultValue) );
			RT_CHECK_CALL( JS_ValueToNumber(cx, resultValue, &fval) );
			curve[i] = fval;
		}
	} else 
	if ( JsvalIsArray(cx, value) ) {

		int curveArrayLength;
		RT_CHECK_CALL( ArrayLength(cx, &curveArrayLength, value) );
		float *curveArray = (float*)malloc( curveArrayLength * sizeof(float) ); // (TBD) free the curveArray ???
		RT_CHECK_CALL( FloatArrayToVector(cx, curveArrayLength, &value, curveArray) );

		for ( int i = 0; i < length; i++ )
			curve[i] = curveArray[ i * curveArrayLength / length ];
	} else
	if ( JsvalIsBString(cx, value) ) { // (TBD) test it

		JSObject *bstrObj = JSVAL_TO_OBJECT(value);
		size_t bstrLen;
		u_int8_t *bstrData;
		BStringGetDataAndLength( cx, bstrObj, (void**)&bstrData, &bstrLen );

		for ( int i = 0; i < length; i++ )
			curve[i] = bstrData[ i * bstrLen / length ] / 256;
	}	
	else {

		for ( int i = 0; i < length; i++ )
			curve[i] = PMAX;
	}
	return JS_TRUE;
}



BEGIN_CLASS( Texture )

JSBool NativeInterfaceBufferGet( JSContext *cx, JSObject *obj, const char **buf, size_t *size ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( tex );
	*buf = (char*)tex->cbuffer;
	*size = tex->width * tex->height * tex->channels * sizeof(PTYPE);
	return JS_TRUE;
}


inline bool IsTexture( JSContext *cx, jsval value ) {
	
	return ( JSVAL_IS_OBJECT( value ) && JS_GET_CLASS(cx, JSVAL_TO_OBJECT( value )) == &classTexture );
}


JSBool ValueToTexture( JSContext* cx, jsval value, Texture **tex ) {
	
	RT_ASSERT_OBJECT( value );
	JSObject *texObj = JSVAL_TO_OBJECT( value );
	RT_ASSERT_CLASS( texObj, &classTexture );
	*tex = (Texture *)JS_GetPrivate(cx, texObj);
	RT_ASSERT_RESOURCE(tex);
	return JS_TRUE;
}


DEFINE_FINALIZE() {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	if ( tex != NULL ) {

		if ( tex->cbackBuffer != NULL )
			JS_free(cx, tex->cbackBuffer);
		if ( tex->cbuffer != NULL )
			JS_free(cx, tex->cbuffer);
		JS_free(cx, tex);
		JS_SetPrivate(cx, obj, NULL);
	}
}


DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	RT_ASSERT_ARGC( 1 );
	Texture *tex = (Texture *)JS_malloc(cx, sizeof(Texture));
	tex->cbackBuffer = NULL;

	J_CHECK_CALL( SetBufferGetInterface(cx, obj, NativeInterfaceBufferGet) );

	if ( J_ARGC >= 3 ) {
	
		int width, height, channels;
		RT_JSVAL_TO_INT32( J_ARG(1), width );
		RT_JSVAL_TO_INT32( J_ARG(2), height );
		RT_JSVAL_TO_INT32( J_ARG(3), channels );

		RT_ASSERT( width > 0, "Invalid width." );
		RT_ASSERT( height > 0, "Invalid height." );
		RT_ASSERT( channels <= PMAXCHANNELS, "Too many channels." );

		tex->cbuffer = (PTYPE*)JS_malloc(cx, width * height * channels * sizeof(PTYPE) );
		RT_ASSERT_ALLOC( tex->cbuffer );

		tex->width = width;
		tex->height = height;
		tex->channels = channels;
		JS_SetPrivate(cx, obj, tex);
		return JS_TRUE;
	}

	if ( IsTexture(cx, J_ARG(1)) ) { // copy constructor

		Texture *srcTex = (Texture *)JS_GetPrivate(cx, JSVAL_TO_OBJECT(J_ARG(1)));
		RT_ASSERT_RESOURCE(srcTex);

		int tsize = srcTex->width * srcTex->height * srcTex->channels;
		tex->cbuffer = (PTYPE*)JS_malloc(cx, tsize * sizeof(PTYPE) );
		RT_ASSERT_ALLOC( tex->cbuffer );

		memcpy( tex->cbuffer, srcTex->cbuffer, tsize * sizeof(PTYPE) );
		tex->width = srcTex->width;
		tex->height = srcTex->height;
		tex->channels = srcTex->channels;
		JS_SetPrivate(cx, obj, tex);
		return JS_TRUE;
	}

	if ( JsvalIsBString( cx, J_ARG(1) ) ) {

		JSObject *bstr = JSVAL_TO_OBJECT( J_ARG(1) );

		int dWidth = tex->width;
		int dHeight = tex->height;
		int dChannels = tex->channels;
		
		int sWidth, sHeight, sChannels;
		GetPropertyInt(cx, bstr, "width", &sWidth);
		GetPropertyInt(cx, bstr, "height", &sHeight);
		GetPropertyInt(cx, bstr, "channels", &sChannels);
		u_int8_t *buffer = (u_int8_t*)BStringData(cx, bstr);

		tex->width = sWidth;
		tex->height = sHeight;
		tex->channels = sChannels;


		int tsize = sWidth * sHeight * sChannels;

		tex->cbuffer = (PTYPE*)JS_malloc(cx, tsize * sizeof(PTYPE) );
		RT_ASSERT_ALLOC( tex->cbuffer );

		for ( int i=0; i<tsize; i++ )
			tex->cbuffer[i] = buffer[i] / (PTYPE)256;

		JS_SetPrivate(cx, obj, tex);

		return JS_TRUE;
	}

	J_REPORT_ERROR( "Invalid arguments" );
}


DEFINE_FUNCTION( Free ) {

	RT_ASSERT_CLASS(obj, _class);
	Finalize(cx, obj);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( Swap ) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( J_FOBJ, _class );
	RT_ASSERT_OBJECT( J_FARG(1) );
	JSObject *texObj = JSVAL_TO_OBJECT( J_FARG(1) );
	RT_ASSERT_CLASS( texObj, _class );
	void *tmp = JS_GetPrivate(cx, J_FOBJ);
	JS_SetPrivate(cx, J_FOBJ, JS_GetPrivate(cx, texObj));
	RT_CHECK_CALL( JS_SetPrivate(cx, texObj, tmp) );
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( ClearChannel ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	if ( argc == 0 ) { // clear all channels

		memset(tex->cbuffer, 0, tex->width * tex->height * tex->channels * sizeof(PTYPE));
		*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	} else
	if ( argc >= 1 ) {

		int channel;
		RT_JSVAL_TO_INT32( J_FARG(1), channel );
		RT_ASSERT( channel < tex->channels, "Invalid channel." );

		PTYPE *ptr = tex->cbuffer;
		ptr += channel;

		int tsize = tex->width * tex->height;
		int channels = tex->channels;
		for ( int i = 0; i < tsize; i++ ) {

			*ptr = 0;
			ptr += channels;
		}
	}
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( SetChannel ) { // dstChannel,   tex1, srcChannel

	RT_ASSERT_ARGC( 3 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	int dstChannel;
	RT_JSVAL_TO_INT32( J_FARG(1), dstChannel );

	Texture *tex1;
	RT_CHECK_CALL( ValueToTexture(cx, J_FARG(2), &tex1) );

	int srcChannel;
	RT_JSVAL_TO_INT32( J_FARG(3), srcChannel );

	int texChannel = tex->channels;
	int tex1Channel = tex1->channels;

	RT_ASSERT( tex->width == tex1->width && tex->height == tex1->height, "Images must have the same size.");
	RT_ASSERT( srcChannel < tex1Channel && srcChannel <= PMAXCHANNELS, "Invalid source channel.");
	RT_ASSERT( dstChannel < texChannel && dstChannel <= PMAXCHANNELS, "Invalid destination channel.");

	int size = tex->width * tex->height;
	for ( int i = 0; i < size; i++ )
		tex->cbuffer[texChannel * i + dstChannel] = tex1->cbuffer[tex1Channel * i + srcChannel];

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


// (TBD) PTYPE
DEFINE_FUNCTION_FAST( ToHLS ) { // (TBD) test it

	// see http://svn.gnome.org/viewcvs/gimp/trunk/libgimpcolor/gimpcolorspace.c?view=markup

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int channels = tex->channels;
	RT_ASSERT( channels >= 3, "Invalid pixel format (need RGB).");

	int size = tex->width * tex->height;
	int pos;
	float R, G, B, H, L, S, min, max;

	for ( int i = 0; i < size; i++ ) {
		
		pos = i * channels;
		R = tex->cbuffer[pos+0];
		G = tex->cbuffer[pos+1];
		B = tex->cbuffer[pos+2];

		min = MIN( R, MIN( G, B ) );
		max = MAX( R, MAX( G, B ) );

		L = (max + min) / 2.f;

      if (max == min) {

         S = 0;
         H = 0;
		} else {

			if (L < 0.5f)
				S = (max-min)/(max+min);
			else
				S = (max-min)/(2.f-max-min);

			if (R == max)
				H = (G - B) / (max-min);
			else if (G == max)
				H = 2.f + (B - R) / (max-min);
			else
				H = 4.f + (R - G) / (max-min);

			H /= 6;
		}

		tex->cbuffer[pos+0] = H;
		tex->cbuffer[pos+1] = L;
		tex->cbuffer[pos+2] = S;
	}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// (TBD) PTYPE
inline float HLSToRGB_hue( float n1, float n2, float hue) { // helper function for the HLS->RGB conversion

	if (hue > 1) hue -= 1;
	if (hue < 0) hue += 1;
	if (hue < 1.f/6.f) return( n1 + (n2 - n1)*hue/(1.f/6.f) );
	if (hue < (1.f/2.f)) return( n2 );
	if (hue < (4.f/6.f)) return( n1 + (n2 - n1)*((4.f/6.f) - hue)/(1.f/6.f) );
	return( n1 );
}

// (TBD) PTYPE
DEFINE_FUNCTION_FAST( ToRGB ) { // (TBD) test it
	// see http://svn.gnome.org/viewcvs/gimp/trunk/libgimpcolor/gimpcolorspace.c?view=markup

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int channels = tex->channels;
	
	RT_ASSERT( channels >= 3, "Invalid pixel format (need HLS).");

	int size = tex->width * tex->height;
	int pos;
	float R, G, B, H, L, S;
	float m1, m2;
	for ( int i = 0; i < size; i++ ) {
		
		pos = i * channels;
		H = tex->cbuffer[pos+0];
		L = tex->cbuffer[pos+1];
		S = tex->cbuffer[pos+2];

		if (L <= 0.5)
				m2 = L * ( 1.0 + S );
		else 
				m2 = L + S - L * S;
		m1 = 2.0 * L - m2;
		if (S == 0.0) { // achromatic cast

			R = G = B = 1;
		} else { // chromatic case

			R = HLSToRGB_hue(m1, m2, H + (1.f / 3.f));
			G = HLSToRGB_hue(m1, m2, H);
			B = HLSToRGB_hue(m1, m2, H - (1.f / 3.f));
		}

		tex->cbuffer[pos+0] = R;
		tex->cbuffer[pos+1] = G;
		tex->cbuffer[pos+2] = B;
	}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( Aliasing ) {

	RT_ASSERT_ARGC( 1 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	long count;
	RT_JSVAL_TO_UINT32( J_FARG(1), count );

	bool useCurve;

	float *curve;
	if ( argc >= 2 ) {

		useCurve = true;
		curve = (float*)malloc( count * sizeof(float) ); // (TBD) free curve ???
		RT_CHECK_CALL( InitCurveData( cx, J_FARG(2), count, curve ) );
	} else
		useCurve = false;

	int tsize = tex->width * tex->height * tex->channels;
	
	if ( useCurve )
		for ( int i = 0; i < tsize; i++ ) {
			
			long curveIndex = (long)(count * tex->cbuffer[i] / PMAX);
			tex->cbuffer[i] = curve[MINMAX(curveIndex, 0, count-1)];
		}
	else
		for ( int i = 0; i < tsize; i++ )
			tex->cbuffer[i] = floor( count * tex->cbuffer[i] ) / count;

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( Colorize ) {
	// GIMP color to alpha: http://www.google.com/codesearch?hl=en&q=+gimp+%22color+to+alpha%22
	// color exchange algo. : http://www.koders.com/c/fidB39DAC5A8DB8B6073D78FB23363C5E0541208B02.aspx

	J_S_ASSERT_ARG_MAX(2);

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	int channels = tex->channels;

	PTYPE colorSrc[PMAXCHANNELS];
	RT_CHECK_CALL( InitLevelData(cx, J_FARG(1), channels, colorSrc) );

	PTYPE colorDst[PMAXCHANNELS];
	RT_CHECK_CALL( InitLevelData(cx, J_FARG(2), channels, colorDst) );

	float power;
	if ( argc >= 3 )
		RT_JSVAL_TO_REAL(J_FARG(3), power);
	else
		power = 1;
	
	float ratio;
	int pos, size = tex->width * tex->height;
	int c;
	for ( int i = 0; i < size; i++ ) {

		pos = i * channels;

		ratio = 1;
		for ( c = 0; c  < channels; c++ )
			ratio *= (PMAX - abs(tex->cbuffer[pos+c] - colorSrc[c])) / PMAX;

		if ( power == 0 ) {
			if ( ratio == 1 )
				for ( c = 0; c < channels; c++ )
					tex->cbuffer[pos+c] = colorDst[c];
			continue;
		}

		if ( power != 1 )
			ratio = powf(ratio, 1 / power);

		for ( c = 0; c < channels; c++ )
			tex->cbuffer[pos+c] = (tex->cbuffer[pos+c] * (1-ratio) + colorDst[c] * ratio);
	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


// PTYPE ok
DEFINE_FUNCTION_FAST( ExtractColor ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT( tex->channels == 1, "Destination texture must have only 1 channel.");

	Texture *texSrc;
	RT_CHECK_CALL( ValueToTexture(cx, J_FARG(1), &texSrc) );
	RT_ASSERT( tex->width == texSrc->width && tex->height == texSrc->height, "Images must have the same width and height." );

	int srcChannels = texSrc->channels;

	PTYPE color[PMAXCHANNELS];
	RT_CHECK_CALL( InitLevelData(cx, J_FARG(2), srcChannels, color) );

	float power;
	if ( argc >= 3 )
		RT_JSVAL_TO_REAL(J_FARG(3), power);
	else
		power = 1;

	float ratio;
	int pos, size = tex->width * tex->height;
	int c;
	for ( int i = 0; i < size; i++ ) {

		pos = i * srcChannels;
		ratio = 1;
		for ( c = 0; c < srcChannels; c++ )
			ratio *= ( PMAX - abs(texSrc->cbuffer[pos+c] - color[c]) ) / PMAX;
		if ( power != 1 )
			ratio = powf(ratio, 1 / power);
		tex->cbuffer[i] = ratio;
	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( NormalizeLevels ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	PTYPE min = PMAXLIMIT;
	PTYPE max = PMINLIMIT;
	PTYPE tmp;

	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ ) {

		tmp = tex->cbuffer[i]; 
		if ( tmp > max )
			max = tmp;
		else if ( tmp < min )
			min = tmp;
	}
	PTYPE amp = max - min;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] = PMAX * ( tex->cbuffer[i] - min ) / amp; // value is normalized to 0...PMAX
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( ClampLevels ) { // (TBD) check if this algo is right

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	PTYPE min, max;
	RT_JSVAL_TO_REAL( J_FARG(1), min );
	RT_JSVAL_TO_REAL( J_FARG(2), max );

	PTYPE tmp;
	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ ) {
		
		tmp = tex->cbuffer[i];
		if ( tmp > max )
			tex->cbuffer[i] = max;
		else if ( tmp < min )
			tex->cbuffer[i] = min;
	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


// PTYPE ok
DEFINE_FUNCTION_FAST( CutLevels ) { // (TBD) check if this algo is right

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	PTYPE min, max;
	RT_JSVAL_TO_REAL( J_FARG(1), min );
	RT_JSVAL_TO_REAL( J_FARG(2), max );

	PTYPE tmp;
	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ ) {
		
		tmp = tex->cbuffer[i];
		if ( tmp > max )
			tex->cbuffer[i] = PMAX;
		else if ( tmp < min )
			tex->cbuffer[i] = 0;
	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

/*
// PTYPE ok
DEFINE_FUNCTION_FAST( CutLevels ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	PTYPE min, max;

	if ( argc == 0 ) {

		min = 0;
		max = PMAX;
	} 
	
	if ( argc >= 1 ) {

		RT_JSVAL_TO_REAL( J_FARG(1), min );
		max = min;
	}
	
	if ( argc >= 2 ) {

		RT_JSVAL_TO_REAL( J_FARG(2), max );
	}

	PTYPE level, lmin, lmax; // local min & max
	int channels = tex->channels;
	int size = tex->width * tex->height;
	int i, c, pos;
	for ( i = 0; i < size; i++ ) {

		lmax = max;
		lmin = min;
		pos = i * channels;
		for ( c = 0; c < channels; c++ ) {
			
			level = tex->cbuffer[pos+c];
			if ( level > lmax )
				lmax = level;
			else if ( level < lmin )
				lmin = level;
		}
		level = ( lmin + lmax ) / 2;
		if ( level > max )
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[pos+c] = PMAX;
		else if ( level < min )
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[pos+c] = 0;
	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}
*/

// PTYPE ok
DEFINE_FUNCTION_FAST( InvertLevels ) { // level = 1 / level

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] = 1 / tex->cbuffer[i];
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( OppositeLevels ) { // level = -level

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] =  - tex->cbuffer[i];
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( PowLevels ) { // 

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	RT_ASSERT_ARGC( 1 );

	float power;
	RT_JSVAL_TO_REAL( J_FARG(1), power );

	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] =  powf(tex->cbuffer[i], power);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


// PTYPE ok
DEFINE_FUNCTION_FAST( MirrorLevels ) {

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	PTYPE threshold;
	RT_JSVAL_TO_REAL( J_FARG(1), threshold );
	
	bool mirrorTop;
	if ( argc >= 2 && !JSVAL_IS_VOID(J_FARG(2)) )
		RT_JSVAL_TO_BOOL( J_FARG(2), mirrorTop );
	else
		mirrorTop = true;

	PTYPE value;
	int i, tsize = tex->width * tex->height * tex->channels;

	if ( mirrorTop )
		for ( i = 0; i < tsize; i++ ) {
			
			value = tex->cbuffer[i];
			if ( value > threshold )
				tex->cbuffer[i] = 2 * threshold - value;
		}
	else
		for ( i = 0; i < tsize; i++ ) {
			
			value = tex->cbuffer[i];
			if ( value < threshold )
				tex->cbuffer[i] = 2 * threshold - value;
		}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( WrapLevels ) { // real modulo

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	PTYPE wrap;
	RT_JSVAL_TO_REAL(J_FARG(1), wrap);

	PTYPE div;
	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ ) {

		div = tex->cbuffer[i] / wrap;
		tex->cbuffer[i] = div - (long)div;
	}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( AddNoise ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int channels = tex->channels;

	bool fullLevel;

	PTYPE pixel[PMAXCHANNELS];
	if ( argc >= 1 ) {

		RT_CHECK_CALL( InitLevelData(cx, J_FARG(1), tex->channels, pixel) );
		fullLevel = false;
	} else {

		fullLevel = true;
	}

	int i, tsize = tex->width * tex->height * channels;
	if ( fullLevel )
		for ( i = 0; i < tsize; i++ )
			tex->cbuffer[i] = PRAND;
	else
		for ( i = 0; i < tsize; i++ )
			tex->cbuffer[i] += PRAND * pixel[i % channels] / PMAX; //(TBD) test if i%channels works fine, else use double for loop
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


// PTYPE ok
DEFINE_FUNCTION_FAST( Desaturate ) {

	RT_ASSERT_ARGC(2);

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	RT_ASSERT( tex->channels == 1, "Destination texture must have only one channel.");

	Texture *texSrc;
	RT_CHECK_CALL( ValueToTexture(cx, J_FARG(1), &texSrc) );
	RT_ASSERT( tex->width == texSrc->width && tex->height == texSrc->height, "Images must have the same width and height." );

	int modeVal;
	RT_JSVAL_TO_INT32( J_FARG(2), modeVal );
	DesaturateMode mode = (DesaturateMode)modeVal;
	
	int pos, i, c;
	int srcChannels = tex->channels;
	int size = tex->width * tex->height;
	PTYPE val, min, max, tmp;
	for ( i = 0; i < size; i++ ) {

		pos = i * srcChannels;
		switch( mode ) {
			case desaturateLightness:
				min = max = texSrc->cbuffer[pos+0];
				for ( c = 1; c < srcChannels; c++ ) {
					
					tmp = texSrc->cbuffer[pos+c];
					if ( tmp > max )
						max = tmp;
					else if ( tmp < min )
						min = tmp;
				}
				val = ( min + max ) / 2;
				break;
			case desaturateSum:
				val = 0;
				for ( c = 0; c < srcChannels; c++ )
					val += texSrc->cbuffer[pos+c];
				break;
			case desaturateAverage:
				val = 0;
				for ( c = 0; c < srcChannels; c++ )
					val += texSrc->cbuffer[pos+c];
				val /= srcChannels;
				break;
//			case desaturateLuminosity: // see http://svn.gnome.org/viewcvs/gimp/trunk/libgimpcolor/gimprgb.h?revision=19720&view=markup
//				break;
		}
		tex->cbuffer[pos] = val;
	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( Set ) {

	RT_ASSERT_ARGC( 1 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int channels = tex->channels;
	int tsize = tex->width * tex->height * channels;
	if ( IsTexture(cx, J_FARG(1)) ) {
		
		Texture *tex1;
		RT_CHECK_CALL( ValueToTexture(cx, J_FARG(1), &tex1) );
		RT_ASSERT( tex->width == tex1->width && tex->height == tex1->height && channels == tex1->channels, "Images must have the same size." );
		for ( int i = 0; i < tsize; i++ )
			tex->cbuffer[i] = tex1->cbuffer[i];
	} else {

		PTYPE pixel[PMAXCHANNELS];
		RT_CHECK_CALL( InitLevelData(cx, J_FARG(1), channels, pixel) );
/* slower
		int tsize = tex->width * tex->height * channels;
		for ( int i = 0; i < tsize; i++ )
			tex->cbuffer[i] = pixel[i % channels];
*/
		int i, c, size = tex->width * tex->height;
		for ( i = 0; i < size; i++ )
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[i*channels+c] = pixel[c];

	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( Add ) {

	RT_ASSERT_ARGC( 1 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int channels = tex->channels;
	int tsize = tex->width * tex->height * channels;
	if ( IsTexture(cx, J_FARG(1)) ) {
		
		Texture *tex1;
		RT_CHECK_CALL( ValueToTexture(cx, J_FARG(1), &tex1) );
		RT_ASSERT( tex->width == tex1->width && tex->height == tex1->height && channels == tex1->channels, "Images must have the same size." );
		for ( int i = 0; i < tsize; i++ )
			tex->cbuffer[i] += tex1->cbuffer[i];
	} else {

		PTYPE pixel[PMAXCHANNELS];
		RT_CHECK_CALL( InitLevelData(cx, J_FARG(1), channels, pixel) );
		int i, c, size = tex->width * tex->height;
		for ( i = 0; i < size; i++ )
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[i*channels+c] += pixel[c];
	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( Mult ) {

	RT_ASSERT_ARGC( 1 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int channels = tex->channels;
	int tsize = tex->width * tex->height * channels;

	if ( IsTexture(cx, J_FARG(1)) ) {
		
		Texture *tex1;
		RT_CHECK_CALL( ValueToTexture(cx, J_FARG(1), &tex1) );
		RT_ASSERT( tex->width == tex1->width && tex->height == tex1->height && channels == tex1->channels, "Images must have the same size." );
		for ( int i = 0; i < tsize; i++ )
			tex->cbuffer[i] *= tex1->cbuffer[i];
/* using Aliasing() seems more useful than Mult()
	} else
	if ( JsvalIsFunction(cx, J_FARG(1)) ) {
		
		J_S_ASSERT_ARG_MIN( 2 );
		int aliasing;
		J_JSVAL_TO_INT32( J_FARG(2), aliasing );

		PTYPE *curve = (PTYPE*)malloc( sizeof(PTYPE) * aliasing );
		RT_CHECK_CALL( InitCurveData(cx, J_FARG(1), aliasing, curve) );

		int i, c, size = tex->width * tex->height;
		for ( i = 0; i < size; i++ )
			for ( c = 0; c < channels; c++ ) {

				PTYPE *pval = &tex->cbuffer[i*channels+c];
				int curveIndex = (long)( (aliasing-1) * *pval / PMAX );
				*pval *= curve[MINMAX(curveIndex, 0, aliasing-1)];
			}

		free(curve);
*/
	} else {

		PTYPE pixel[PMAXCHANNELS];
		RT_CHECK_CALL( InitLevelData(cx, J_FARG(1), channels, pixel) );
		int i, c, size = tex->width * tex->height;
		for ( i = 0; i < size; i++ )
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[i*channels+c] *= pixel[c];
	}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( Blend ) { // texture1, blenderTexture|blenderColor

	RT_ASSERT_ARGC( 2 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int channels = tex->channels;
	int tsize = tex->width * tex->height * channels;

	Texture *tex1;
	RT_CHECK_CALL( ValueToTexture(cx, J_FARG(1), &tex1) );
	RT_ASSERT( tex->width == tex1->width && tex->height == tex1->height && channels == tex1->channels, "Images must have the same size." );

	float blend;

	if ( IsTexture(cx, J_FARG(2)) ) {
		
		Texture *blenderTex;
		RT_CHECK_CALL( ValueToTexture(cx, J_FARG(1), &blenderTex) );
		RT_ASSERT( tex->width == blenderTex->width && tex->height == blenderTex->height && channels == blenderTex->channels, "Images must have the same size." );
		for ( int i = 0; i < tsize; i++ ) {
			
			blend = blenderTex->cbuffer[i];
			tex->cbuffer[i] = (blend * tex->cbuffer[i] + (PMAX - blend) * tex1->cbuffer[i] ) / PMAX;
		}
	} else {

		PTYPE pixel[PMAXCHANNELS];
		RT_CHECK_CALL( InitLevelData(cx, J_FARG(2), channels, pixel) );

		int pos, c, size = tex->width * tex->height;
		for ( int i = 0; i < size; i++ ) {

			pos = i * channels;
			for ( c = 0; c < channels; c++ ) {

				blend = pixel[c];
				tex->cbuffer[i] = ( blend * tex->cbuffer[pos+c] + (PMAX - blend) * tex1->cbuffer[pos+c] ) / PMAX;
			}
		}
/* following is slower
		int tsize = tex->width * tex->height * channels;
		for ( int i = 0; i < tsize; i++ ) {

			blend = pixel[i%channels];
			tex->cbuffer[i] = blend * tex->cbuffer[i] + (PMAX - blend) * tex1->cbuffer[i];
		}
*/		
	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( _SetPixel ) { // x, y, levels

	RT_ASSERT_ARGC( 3 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	int x, y;
	RT_JSVAL_TO_INT32( J_FARG(1), x );
	RT_JSVAL_TO_INT32( J_FARG(2), y );
	
	x = Wrap(x, tex->width);
	y = Wrap(y, tex->height);

	PTYPE pixel[PMAXCHANNELS];
	RT_CHECK_CALL( InitLevelData(cx, J_FARG(3), tex->channels, pixel) );

	int c, channels = tex->channels;
	int pos = (x + y * tex->width) * channels;
	for ( c = 0; c < channels; c++ )
		tex->cbuffer[pos+c] = pixel[c];

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// PTYPE ok
DEFINE_FUNCTION_FAST( SetRectangle ) {

	RT_ASSERT_ARGC( 5 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	int x0, y0, x1, y1;
	RT_JSVAL_TO_INT32( J_FARG(1), x0 );
	RT_JSVAL_TO_INT32( J_FARG(2), y0 );
	RT_JSVAL_TO_INT32( J_FARG(3), x1 );
	RT_JSVAL_TO_INT32( J_FARG(4), y1 );

	PTYPE pixel[PMAXCHANNELS];
	RT_CHECK_CALL( InitLevelData(cx, J_FARG(5), tex->channels, pixel) );
	
//	PTYPE alpha;
//	if ( argc >= 6 )
//		RT_JSVAL_TO_REAL( J_FARG(6), alpha )
//	else
//		alpha = 1;

	int channels = tex->channels;
	int width = tex->width;
	int height = tex->height;

	int x, y;
	int c, pos;
	for ( y = y0; y < y1; y++ )
		for ( x = x0; x < x1; x++ ) {

			pos = ( Wrap(x, width) + Wrap(y, height) * width ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[pos+c] = pixel[c];
		}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Rotate90 ) { // (TBD) test it

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	int turn;
	RT_JSVAL_TO_INT32(J_FARG(1), turn);

	turn = Wrap(turn, turn);

	int channels = tex->channels;
	int width = tex->width;
	int height = tex->height;

	int x, y;

	TextureSetupBackBuffer(cx, tex);

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

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Flip ) {

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	bool flipX, flipY;
	RT_JSVAL_TO_BOOL( J_FARG(1), flipX );
	RT_JSVAL_TO_BOOL( J_FARG(2), flipY );

	TextureSetupBackBuffer(cx, tex);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	int x, y, c;
	int posDst, posSrc;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			posDst = (x + y * width) * channels;
			posSrc = ( (flipX?width-1-x:x) + (flipY?height-1-y:y) * width ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbackBuffer[posDst+c] = tex->cbuffer[posSrc+c];
		}
	TextureSwapBuffers(tex);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( RotoZoom ) { // source: FxGen

	RT_ASSERT_ARGC( 5 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	int newWidth = width;
	int newHeight = height;

	float centerX, centerY;
	RT_JSVAL_TO_REAL( J_FARG(1), centerX );
	RT_JSVAL_TO_REAL( J_FARG(2), centerY );

	float zoomX, zoomY;
	RT_JSVAL_TO_REAL( J_FARG(3), zoomX );
	RT_JSVAL_TO_REAL( J_FARG(4), zoomY );

//	zoomX = 0.5 - ( zoomX / 2 );
//	zoomX = exp( zoomX * 6 );

//	zoomY = 0.5 - ( zoomY / 2 );
//	zoomY = exp( zoomY * 6 );

	float rotate;
	RT_JSVAL_TO_REAL( J_FARG(5), rotate ); // 1 for 1 turn

	rotate = M_PI * 2 * rotate;

	float coefX = zoomX * width / newWidth;
	float coefY = zoomY * height / newHeight;

	float	cosVal = cos(rotate);
	float	sinVal = sin(rotate);

	float tw2 = newWidth / 2.0f;
	float th2 = newHeight / 2.0f;

	float	ys = sinVal * -th2;
	float	yc = cosVal * -th2;

	TextureSetupBackBuffer(cx, tex);

	unsigned int spx, spy; // position in the source
	float prx, pry; // pixel ratio
	int x, y, c;
	int pos, pos1, pos2, pos3, pos4; // offset in the buffer
	float ratio1, ratio2, ratio3, ratio4; // pixel value ratio
	float u, v;
	for ( y = 0; y < newHeight; y++ ) {

		u = (cosVal * -tw2 - ys) * coefX + centerX * width;		// x' = cos(x)-sin(y) + Center X;
		v = (sinVal * -tw2 + yc) * coefY + centerY * height;		// y' = sin(x)+cos(y) + Center Y;

		for ( x = 0; x < newWidth; x++ ) {

			prx = abs(u - (long)u);	//Fraction
			pry = abs(v - (long)v);	//Fraction
			spx = Wrap(u, width);
			spy = Wrap(v, height);

			ratio1 = (1.f - prx) * (1.f - pry);
			ratio2 = (prx) * (1.f - pry);
			ratio3 = (1.f - prx) * (pry);
			ratio4 = (prx) * (pry);

			pos1 = (  ((spx + 0)        ) + ((spy + 0)         ) * width  ) * channels;
			pos2 = (  ((spx + 1) % width) + ((spy + 0)         ) * width  ) * channels;
			pos3 = (  ((spx + 0)        ) + ((spy + 1) % height) * width  ) * channels;
			pos4 = (  ((spx + 1) % width) + ((spy + 1) % height) * width  ) * channels;


/*
			prx = abs(u - (long)u);	//Fraction
			pry = abs(v - (long)v);	//Fraction

			ratio1 = (1.f - prx) * (1.f - pry);
			ratio2 = (prx) * (1.f - pry);
			ratio3 = (1.f - prx) * (pry);
			ratio4 = (prx) * (pry);

			pos1 = (  Wrap(u  , width) + Wrap(v  , height) * width  ) * channels;
			pos2 = (  Wrap(u+1, width) + Wrap(v  , height) * width  ) * channels;
			pos3 = (  Wrap(u  , width) + Wrap(v+1, height) * width  ) * channels;
			pos4 = (  Wrap(u+1, width) + Wrap(v+1, height) * width  ) * channels;
*/

			pos = (x + y * newWidth) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbackBuffer[pos+c] = tex->cbuffer[pos1+c] * ratio1 + tex->cbuffer[pos2+c] * ratio2 + tex->cbuffer[pos3+c] * ratio3 + tex->cbuffer[pos4+c] * ratio4;

			//Vectors
			u += cosVal * coefX;
			v += sinVal * coefY;
		}

		//Vectors
		ys += sinVal;
		yc += cosVal;
	}

	TextureSwapBuffers(tex);

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


// PTYPE ok
DEFINE_FUNCTION_FAST( Resize ) {

	RT_ASSERT_ARGC( 2 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	int newWidth, newHeight;
	bool interpolate;
	RT_JSVAL_TO_REAL( J_FARG(1), newWidth );
	RT_JSVAL_TO_REAL( J_FARG(2), newHeight );

	if ( newWidth != width || newHeight != height ) { // nothing to do
		
		RT_JSVAL_TO_BOOL( J_FARG(3), interpolate );

		BorderMode borderMode = borderWrap; // (TBD) from function arg

		PTYPE *newBuffer = (PTYPE*)JS_malloc(cx, newWidth * newHeight * channels * sizeof(PTYPE) );
		RT_ASSERT_ALLOC( newBuffer );

		int spx, spy; // position in the source
		float prx, pry; // pixel ratio
		float rx = (float)width / newWidth; // texture ratio x
		float ry = (float)height / newHeight; // texture ratio y
		int x, y, c;

		int pos, pos1, pos2, pos3, pos4; // offset in the buffer
		float ratio1, ratio2, ratio3, ratio4; // pixel value ratio

		for ( y = 0; y < newHeight; y++ )
			for ( x = 0; x < newWidth; x++ ) {	

				if ( interpolate ) {
/* slower
					prx = modf( (float)x*rx, &tmp );
					spx = (int)tmp;
					pry = modf( (float)y*ry, &tmp );
					spy = (int)tmp;
*/
					float u = x*rx;
					float v = y*ry;

					prx = abs(u - (long)u);	//Fraction
					pry = abs(v - (long)v);	//Fraction
					spx = Wrap(u, width);
					spy = Wrap(v, height);



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

					spx = (int)(rx * x);
					spy = (int)(ry * y);
					pos = (x + y * newWidth) * channels;
					pos1 = (spx + spy * width) * channels;
					for ( c = 0; c < channels; c++ )
						newBuffer[pos+c] = tex->cbuffer[pos1+c];
				}
			}

		JS_free(cx, tex->cbuffer );
		if ( tex->cbackBuffer != NULL ) {

			JS_free(cx, tex->cbackBuffer );
			tex->cbackBuffer = NULL;
		}
		
		tex->cbuffer = newBuffer;
		tex->width = newWidth;
		tex->height = newHeight;
		// channels don't change
	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


// (TBD) PTYPE
DEFINE_FUNCTION_FAST( Convolution ) {

	// (TBD) accumulate precalculated pixels ? ( like BoxBlur )

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;

	int count;
	RT_CHECK_CALL( ArrayLength(cx, &count, J_FARG(1)) );
	float *kernel = (float*)malloc(sizeof(float) * count);
	RT_ASSERT_ALLOC( kernel );
	RT_CHECK_CALL( FloatArrayToVector(cx, count, &J_FARG(1), kernel) );

	int size = (int)sqrtf(count);

	RT_ASSERT( size * size == count, "Invalid convolution kernel size.");

	BorderMode borderMode;
	if ( argc >= 2 && !JSVAL_IS_VOID(J_FARG(2)) ) {

		int tmp;
		RT_JSVAL_TO_INT32( J_FARG(2), tmp );
		borderMode = (BorderMode)tmp;
	} else
		borderMode = borderWrap;

	float gain;
	bool autoGain;
	RT_JSVAL_TO_BOOL( J_FARG(3), autoGain );

	TextureSetupBackBuffer(cx, tex);

	int offset = size / 2;
	float sizeWeight = count;
	float ratio;

	PTYPE pixel[PMAXCHANNELS];
	int channels = tex->channels;

	int pos;
	int x, y, c, vx, vy, sx, sy;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			gain = 0;
			for ( c = 0; c < channels; c++ )
				pixel[c] = 0;

			switch ( borderMode ) { // Note: it is faster to do the switch outside the inner loop

				case borderWrap:
					for ( vy = 0; vy < size; vy++ )
						for ( vx = 0; vx < size; vx++ ) {
				
							pos = (Wrap(x + vx-offset, width) + Wrap(y + vy-offset, height) * width) * channels;
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
								sx = abs(-sx % width);
							else if ( sx >= width )
								sx = abs((width-sx) % width);

							if ( sy < 0 )
								sy = abs(-sy % height);
							else if ( sy >= height )
								sy = abs((height-sy) % height);

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

							if ( sx < 0 )
								sx = 0;
							if ( sx >= width )
								sx = width-1;
							if ( sy < 0 )
								sy = 0;
							if ( sy >= height )
								sy = height-1;
							
							pos = (sx + sy * width)*channels;
							ratio =  kernel[vx + vy * size];
							gain += ratio;
							for ( c = 0; c < channels; c++ )
								pixel[c] += tex->cbuffer[pos+c] * ratio;
						}
					break;
			}

//			if ( gain == 0 )
//				gain = 1;

			pos = (x + y * width) * channels;
			for ( c = 0; c < channels; c++ ) {

				// (TBD) manage gain
				tex->cbackBuffer[pos+c] = pixel[c];// / gain;
			}
		}

	free(kernel);
	TextureSwapBuffers(tex);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( BoxBlur ) {

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;

	int bw, bh; // blur width & height
	RT_JSVAL_TO_INT32( J_FARG(1), bw );
	RT_JSVAL_TO_INT32( J_FARG(2), bh );

	if ( bw > width )
		bw = width;
	if ( bh > height )
		bh = height;

	int channels = tex->channels;

	PTYPE *line = (PTYPE*)malloc( MAX( width, height ) * channels * sizeof(PTYPE) ); // line buffer
	PTYPE sum[PMAXCHANNELS];

	int x, y, c;
	for ( y = 0; y < height; y++ )
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

	for ( x = 0; x < width; x++ )
		for ( c = 0; c < channels; c++ ) {
			
			for ( y = 0; y < height; y++ )
				line[y * channels + c] = tex->cbuffer[(x + y * width) * channels + c];

			sum[c] = 0;
		
			for ( y = 0; y < bh; y++ )
				sum[c] += line[y * channels + c];

			for ( y = 0; y < height; y++ ) {
	
				tex->cbuffer[ ( x + ((y + bh/2) % height) * width ) * channels + c ] = sum[c] / bh;
				sum[c] = sum[c] - line[y * channels + c] + line[( (y + bh) % height ) * channels + c];
			}
		}

	free( line );
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( NormalizeVectors ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	int size = tex->width * tex->height;
	PTYPE val, sum;
	float length;
	int i, pos, c;
	for ( i = 0; i < size; i++ ) {
		
		pos = i * channels;
		sum = 0;
		for ( c = 0; c < channels; c++ ) {
			
			val = PZNORM( tex->cbuffer[pos+c] );
			sum += val * val;
		}
		length = sqrt(sum);
		for ( c = 0; c < channels; c++ )
			tex->cbuffer[pos+c] *= length;
	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


// (TBD) PTYPE
DEFINE_FUNCTION_FAST( Normals ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	if ( tex->channels != 3 && tex->cbackBuffer != NULL ) { // back buffer do not have the right format

		JS_free(cx, tex->cbackBuffer );
		tex->cbackBuffer = NULL;
	}

	if ( tex->cbackBuffer == NULL )
		tex->cbackBuffer = (PTYPE*)JS_malloc(cx, tex->width * tex->height * 3 * sizeof(PTYPE) );

	// from here, tex->cbackBuffer is a 3 channels buffer

	float amp;
	if ( argc >= 1 )
		RT_JSVAL_TO_REAL( J_FARG(1), amp );
	else
		amp = 1;

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	Vector3 normal;
	float dX, dY, fPix;
	int x, y;
	int pos;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			//Y Sobel filter
			fPix = tex->cbuffer[(  (x-1 + width) % width + (((y+1) % height)*width)  ) * channels];
			dY  = PNORM(fPix) * -1.f;

			fPix = (float)tex->cbuffer[(  x % width + (((y+1) % height)*width)  ) * channels];
			dY+= PNORM(fPix) * -2.f;

			fPix = (float)tex->cbuffer[(  (x+1) % width + (((y+1) % height)*width)  ) * channels];
			dY+= PNORM(fPix) * -1.f;

			fPix = (float)tex->cbuffer[(  (x-1+width) % width + (((y-1+height) % height)*width)  ) * channels];
			dY+= PNORM(fPix) * 1.f;

			fPix = (float)tex->cbuffer[(  x % width + (((y-1+height) % height)*width)  ) * channels];
			dY+= PNORM(fPix) * 2.f;

			fPix = (float)tex->cbuffer[(  (x+1) % width + (((y-1+height) % height)*width)  ) * channels];
			dY+= PNORM(fPix) * 1.f;

			//X Sobel filter
			fPix = (float)tex->cbuffer[(  (x-1+width) % width + (((y-1+height) % height)*width)  ) * channels];
			dX  = PNORM(fPix) * -1.f;

			fPix = (float)tex->cbuffer[(  (x-1+width) % width + ((y % height)*width)  ) * channels];
			dX+= PNORM(fPix) * -2.f;

			fPix = (float)tex->cbuffer[(  (x-1+width) % width + (((y+1) % height)*width)  ) * channels];
			dX+= PNORM(fPix) * -1.f;

			fPix = (float)tex->cbuffer[(  (x+1) % width + (((y-1+height) % height)*width)  ) * channels];
			dX+= PNORM(fPix) * 1.f;

			fPix = (float)tex->cbuffer[(  (x+1) % width + ((y % height)*width)  ) * channels];
			dX+= PNORM(fPix) * 2.f;

			fPix = (float)tex->cbuffer[(  (x+1) % width + (((y+1) % height)*width)  ) * channels];
			dX+= PNORM(fPix) * 1.f;
			
			Vector3Set(&normal, dX*amp, dY*amp, 1.f);
			Vector3Normalize(&normal);

			pos = (x + y * width) * 3; // 3 is the number of channels
			tex->cbackBuffer[pos+0] = PUNZNORM( normal.x );
			tex->cbackBuffer[pos+1] = PUNZNORM( normal.y );
			tex->cbackBuffer[pos+2] = PUNZNORM( normal.z );
		}

	TextureSwapBuffers(tex);

	if ( tex->channels != 3 ) {

		JS_free(cx, tex->cbackBuffer );
		tex->cbackBuffer = NULL;
	}

	tex->channels = 3;

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


// (TBD) PTYPE
DEFINE_FUNCTION_FAST( Light ) {
	// Simple Lighting: http://www.gamasutra.com/features/19990416/intel_simd_04.htm

	RT_ASSERT_ARGC( 5 );

	Texture *normals, *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	RT_CHECK_CALL( ValueToTexture(cx, J_FARG(1), &normals) );
	RT_ASSERT( normals->channels == 3, "Invalid normals texture channel count." );

	RT_ASSERT( tex->channels >= 3, "normals applys on a RGB or RGBA texture." );
	
	RT_ASSERT( normals->width == tex->width && normals->height == tex->height, "Invalid normals texture size." );

	Vector3 lightPos;
	FloatArrayToVector(cx, 3, &J_FARG(2), lightPos.raw );

	PTYPE ambient[3];
	RT_CHECK_CALL( InitLevelData(cx, J_FARG(3), 3, ambient) );
	bool ambiantTexture = false;

	PTYPE diffuse[3];
	RT_CHECK_CALL( InitLevelData(cx, J_FARG(4), 3, diffuse) );

	PTYPE specular[3];
	RT_CHECK_CALL( InitLevelData(cx, J_FARG(5), 3, specular) );
	bool specularTexture = false;
	
	float bumpPower; // (TBD) default value
	if ( argc >= 6 )
		RT_JSVAL_TO_REAL( J_FARG(6), bumpPower );
	else
		bumpPower = 1;

	float specularPower; // (TBD) default value
	if ( argc >= 7 )
		RT_JSVAL_TO_REAL( J_FARG(7), specularPower );
	else
		specularPower = 1;

	Vector3Normalize(&lightPos);
	Vector3 halfV = lightPos;
	halfV.z += 1;
	Vector3Normalize(&halfV);
	Vector3 n;

	int x, y, c;
	int pos;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {
			
			pos = (x + y * width) * 3; // normal texture is only 3 channels
			Vector3Set(&n, PZNORM(normals->cbuffer[pos+0]), PZNORM(normals->cbuffer[pos+1]), PZNORM(normals->cbuffer[pos+2]) / bumpPower );
			Vector3Normalize(&n);

			float fDiffDot = Vector3Dot(&n, &lightPos);
			if ( fDiffDot < 0 )
				fDiffDot = 0;
			
			float fSpecDot = Vector3Dot(&n, &halfV);
			if ( fSpecDot < 0 )
				fSpecDot = 0;
			fSpecDot = powf(fSpecDot, 1 / specularPower);

			PTYPE localAmbient[3];
			if ( ambiantTexture ) { // (TBD) manage ambiant texture
			} else {
				localAmbient[0] = ambient[0];
				localAmbient[1] = ambient[1];
				localAmbient[2] = ambient[2];
			}

			PTYPE localSpecular[3];
			if ( specularTexture ) { // (TBD) manage ambiant texture
			} else {
				localSpecular[0] = specular[0];
				localSpecular[1] = specular[1];
				localSpecular[2] = specular[2];
			}

			pos = (x + y * width) * channels;
			for ( c = 0; c < 3; c++ )
				tex->cbuffer[pos+c] = tex->cbuffer[pos+c] * ( localAmbient[c] + fDiffDot * diffuse[c] ) / PMAX + fSpecDot * localSpecular[c];  // sdword r	= (sdword) ((sdword(pPxSrc->r*(localAmbient.r + fDiffDot*Diffuse.r)) >> 8) + (fSpecDot*localSpecular.r));
		}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}



DEFINE_FUNCTION_FAST( Trim ) { // (TBD) test this new version that use memcpy

	RT_ASSERT_ARGC( 4 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	int x0, y0, x1, y1;
	RT_JSVAL_TO_INT32( J_FARG(1), x0 );
	RT_JSVAL_TO_INT32( J_FARG(2), y0 );
	RT_JSVAL_TO_INT32( J_FARG(3), x1 );
	RT_JSVAL_TO_INT32( J_FARG(4), y1 );

	int width = tex->width;
	int height = tex->height;

	int channels = tex->channels;

	int newWidth = x1 - x0;
	int newHeight = y1 - y0;

	if ( tex->cbackBuffer != NULL ) {

		JS_free(cx, tex->cbackBuffer );
		tex->cbackBuffer = NULL;
	}

	int srcLineLength = width * channels * sizeof(PTYPE);
	int dstLineLength = newWidth * channels * sizeof(PTYPE);

	tex->cbackBuffer = (PTYPE*)JS_malloc(cx, newHeight * dstLineLength );

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
	JS_free(cx, tex->cbackBuffer );
	tex->cbackBuffer = NULL;

	tex->width = newWidth;
	tex->height = newHeight;

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Copy ) { // (Texture)source, (int)x, (int)y

	RT_ASSERT_ARGC( 3 );

	Texture *srcTex, *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	RT_CHECK_CALL( ValueToTexture(cx, J_FARG(1), &srcTex) );

	RT_ASSERT( tex->channels == srcTex->channels, "Invalid channel count." );

	int px, py; // position
	RT_JSVAL_TO_INT32( J_FARG(2), px );
	RT_JSVAL_TO_INT32( J_FARG(3), py );

	BorderMode borderMode = borderClamp; //

	int channels = tex->channels;

	int texWidth = tex->width;
	int texHeight = tex->height;

	int srcTexWidth = srcTex->width;
	int srcTexHeight = srcTex->height;

	int x, y;
	int sx, sy; // position in source
	int posDst, posSrc;
	int c;

	for ( y = 0; y < texHeight; y++ )
		for ( x = 0; x < texWidth; x++ ) {

			sx = x + px;
			sy = y + py;
			switch (borderMode) {
				case borderWrap:
					sx = Wrap(sx, texWidth);
					sy = Wrap(sy, texHeight);
					break;
			case borderClamp:
				if ( !(sx >= 0 && sx < srcTexWidth && sy >= 0 && sy < srcTexHeight) )
					continue; // skip
				break;
			}

			posDst = ( x + y * texWidth ) * channels;
			posSrc = ( sx + sy * srcTexWidth ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[posDst+c] = srcTex->cbuffer[posSrc+c];
		}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Export ) { // (int)x, (int)y, (int)width, (int)height . Returns a BString

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	int sChannels = tex->channels;
	int sWidth = tex->width;
	int sHeight = tex->height;

	int dWidth, dHeight;

	u_int8_t *buffer;
	int bufferLength;

	int px, py;

	if ( J_ARGC == 0 ) { // copy the whole image

		px = 0;
		py = 0;
		dWidth = sWidth;
		dHeight = sHeight;
	} else {
		
		J_S_ASSERT_ARG_MIN( 4 );
		int px, py;
		J_JSVAL_TO_INT32( J_FARG(1), px );
		J_JSVAL_TO_INT32( J_FARG(2), py );
		J_JSVAL_TO_INT32( J_FARG(3), dWidth );
		J_JSVAL_TO_INT32( J_FARG(4), dHeight );
	}

	bufferLength = dWidth * dHeight * sChannels;
	buffer = (u_int8_t*)JS_malloc(cx, bufferLength);

	int posDst, posSrc;
	int c;
	int x, y;

	for ( y = 0; y < dHeight; y++ )
		for ( x = 0; x < dWidth; x++ ) {

			int sx, sy; // position in source
			
			sx = x + px;
			sy = y + py;

			posDst = ( x + y * dWidth ) * sChannels;
			posSrc = ( sx + sy * sWidth ) * sChannels;
			for ( c = 0; c < sChannels; c++ ) {
				
				buffer[posDst+c] = (u_int8_t)(MINMAX(tex->cbuffer[posSrc+c] * 256, 0, 255));

				//buffer[posDst+c] = (u_int8_t)(PNORM(tex->cbuffer[posSrc+c]) * 256);
			}
		}


	JSObject *bstr = NewBString(cx, buffer, bufferLength);
	*J_FRVAL = OBJECT_TO_JSVAL(bstr);
	SetPropertyInt(cx, bstr, "width", dWidth);
	SetPropertyInt(cx, bstr, "height", dHeight);
	SetPropertyInt(cx, bstr, "channels", sChannels);

	return JS_TRUE;
}



DEFINE_FUNCTION_FAST( Paste ) { // (Texture)texture, (int)x, (int)y, (bool)borderMode

	RT_ASSERT_ARGC( 4 );

	Texture *tex1, *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	RT_CHECK_CALL( ValueToTexture(cx, J_FARG(1), &tex1) );
	RT_ASSERT( tex->channels == tex1->channels, "Invalid channel count." );

	int px, py; // position
	RT_JSVAL_TO_INT32( J_FARG(2), px );
	RT_JSVAL_TO_INT32( J_FARG(3), py );
	bool borderMode;
	RT_JSVAL_TO_BOOL( J_FARG(4), borderMode );

	borderMode = borderClamp; //

	int channels = tex->channels;

	int texWidth = tex->width;
	int texHeight = tex->height;

	int tex1Width = tex1->width;
	int tex1Height = tex1->height;

	int x, y;
	int dx, dy; // destination
	int posDst, posSrc;
	int c;

	for ( y = 0; y < tex1Height; y++ )
		for ( x = 0; x < tex1Width; x++ ) {

			dx = x + px;
			dy = y + py;
			switch (borderMode) {
				case borderWrap:
					dx = Wrap(dx, texWidth);
					dy = Wrap(dy, texHeight);
					break;
			case borderClamp:
				if ( !(dx >= 0 && dx < texWidth && dy >= 0 && dy < texHeight) ) {
					continue; // skip
				}
				break;
			}

			posDst = ( dx + dy * texWidth ) * channels;
			posSrc = ( x + y * tex1Width ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[posDst+c] = tex1->cbuffer[posSrc+c];
		}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Import ) { // (BString)image, (int)x, (int)y

	J_S_ASSERT_ARG_MIN(1);

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	
	J_S_ASSERT_OBJECT( J_FARG(1) );
	JSObject *bstr = JSVAL_TO_OBJECT( J_FARG(1) );
	J_S_ASSERT_CLASS( bstr, BStringJSClass(cx) );

	int dWidth = tex->width;
	int dHeight = tex->height;
	int dChannels = tex->channels;
	
	int sWidth, sHeight, sChannels;
	GetPropertyInt(cx, bstr, "width", &sWidth);
	GetPropertyInt(cx, bstr, "height", &sHeight);
	GetPropertyInt(cx, bstr, "channels", &sChannels);
	u_int8_t *buffer = (u_int8_t*)BStringData(cx, bstr);

	int px, py;
	if ( J_ARGC >= 3 ) {

		J_JSVAL_TO_INT32( J_FARG(2), px );
		J_JSVAL_TO_INT32( J_FARG(3), py );
	} else {

		px = 0;
		py = 0;
	}

	BorderMode borderMode = borderWrap;

	int x, y;
	int dx, dy; // destination
	int posDst, posSrc;
	int c;

	for ( y = 0; y < sHeight; y++ )
		for ( x = 0; x < sWidth; x++ ) {

			dx = x + px;
			dy = y + py;
			switch (borderMode) {
				case borderWrap:
					dx = Wrap(dx, dWidth);
					dy = Wrap(dy, dHeight);
					break;
			case borderClamp:
				if ( !(dx >= 0 && dx < dWidth && dy >= 0 && dy < dHeight) ) {
					continue; // skip
				}
				break;
			}

			posDst = ( dx + dy * dWidth ) * dChannels;
			posSrc = ( x + y * sWidth ) * sChannels;
			for ( c = 0; c < sChannels; c++ )
				tex->cbuffer[posDst+c] = buffer[posSrc+c] / (PTYPE)256;
		}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}



DEFINE_FUNCTION_FAST( Shift ) {
	// (TBD) I think it is possible to do the Shift operation without using a second buffer.

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	int offsetX, offsetY;
	RT_JSVAL_TO_INT32( J_FARG(1), offsetX );
	RT_JSVAL_TO_INT32( J_FARG(2), offsetY );

	BorderMode mode = borderWrap;

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	TextureSetupBackBuffer(cx, tex);

	int x, y; // destination image x, y
	int sx, sy; // source image x, y
	int c;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			sx = x + offsetX;
			sy = y + offsetY;

			switch (mode) {
				case borderWrap:
					sx = Wrap(sx, width);
					sy = Wrap(sy, height);
					break;

				case borderClamp:
					if ( !(sx >= 0 && sx < width && sy >= 0 && sy < height) )
						continue; // skip
					break;

				case borderMirror:
					if ( sx < 0 )  sx =  -sx;
					if ( sx >= width )  sx = width - sx;
					if ( sy < 0 )  sy =  -sy;
					if ( sy >= height )  sy = height - sy;
					break;

				case borderValue:
					if ( sx < 0 )  sx = 0;
					if ( sx >= width )  sx = width - 1;
					if ( sy < 0 )  sy = 0;
					if ( sy >= height )  sy = height - 1;
					break;
			}
			for ( c = 0; c < channels; c++ ) 
				tex->cbackBuffer[(x + y * width) * channels + c] = tex->cbuffer[(sx + sy * width) * channels + c];
		}
	TextureSwapBuffers(tex);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

// (TBD) PTYPE
DEFINE_FUNCTION_FAST( Displace ) {

	RT_ASSERT_ARGC( 1 );

	Texture *tex1, *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	RT_CHECK_CALL( ValueToTexture(cx, J_FARG(1), &tex1) );

	int displaceChannels = tex1->channels;

	RT_ASSERT( width == tex1->width && height == tex1->height, "Textures must have the same size." );
	RT_ASSERT( tex1->channels >= 2, "Displacement texture must have 2 or more channels." );

	float factor;
	if ( argc >= 2 )
		RT_JSVAL_TO_REAL( J_FARG(2), factor );
	else
		factor = 1;

	TextureSetupBackBuffer(cx, tex);

	BorderMode mode = borderWrap;

	int x, y;
	int sx, sy; // source position
	int pos, pos1;
	int c;
//	Vector3 o;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			pos = (x + y * width) * displaceChannels;

//			Vector3Set(&o, PZNORM( tex1->cbuffer[pos+0] ), PZNORM( tex1->cbuffer[pos+1] ), PZNORM( tex1->cbuffer[pos+2] ) );
//			Vector3Normalize(&o);
//			sx = (int)( x + o.x * factor );
//			sy = (int)( y + o.y * factor );

			// (TBD) test !
			
			sx = x + PZNORM( tex1->cbuffer[pos+0] ) * factor;
			sy = y + PZNORM( tex1->cbuffer[pos+1] ) * factor;

			switch (mode) {
				case borderWrap:
					sx = Wrap(sx, width);
					sy = Wrap(sy, height);
					break;
				case borderClamp:
					if ( !(sx >= 0 && sx < width && sy >= 0 && sy < height) )
						continue;
					break;
				case borderValue:
					if ( sx < 0 ) sx = 0;
					if ( sx >= width ) sx = width - 1;
					if ( sy < 0 ) sy = 0;
					if ( sy >= height ) sy = height - 1;
					break;
				case borderMirror:
					if ( sx < 0 )  sx = 0 - sx;
					if ( sy < 0 )  sx = 0 - sy;
					if ( sx >= width )  sx = width - sx;
					if ( sy >= height )  sy = width - sy;
					break;
			}
			
			pos = (x + y * width) * channels;
			pos1 = (sx + sy * width) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbackBuffer[pos+c] = tex->cbuffer[pos1+c];
		}
	TextureSwapBuffers(tex);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Cells ) { // source: FxGen

	RT_ASSERT_ARGC( 2 );

	int density;
	float regularity;
	RT_JSVAL_TO_INT32( J_FARG(1), density );
	RT_JSVAL_TO_REAL( J_FARG(2), regularity );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
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

	for ( int i = 0; i < count; i++ ) {

		float rand1 = genrand_real1(); // [0,1]-real-interval
		float rand2 = genrand_real1(); // [0,1]-real-interval
		int x = i % density;
		int y = i / density;

		cellPoints[i].x = ( x + 0.5 + (rand1-0.5f) * (1.0f-regularity) ) / density;
		cellPoints[i].y = ( y + 0.5 + (rand2-0.5f) * (1.0f-regularity) ) / density;
	}

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {

			Point pixelPos;
			pixelPos.x = (float)x / (float)width,
			pixelPos.y = (float)y / (float)height;

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

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( AddGradiantQuad ) {

	RT_ASSERT_ARGC( 4 );
	
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;


	PTYPE pixel1[PMAXCHANNELS];
	RT_CHECK_CALL( InitLevelData(cx, J_FARG(1), channels, pixel1) );

	PTYPE pixel2[PMAXCHANNELS];
	RT_CHECK_CALL( InitLevelData(cx, J_FARG(2), channels, pixel2) );

	PTYPE pixel3[PMAXCHANNELS];
	RT_CHECK_CALL( InitLevelData(cx, J_FARG(3), channels, pixel3) );

	PTYPE pixel4[PMAXCHANNELS];
	RT_CHECK_CALL( InitLevelData(cx, J_FARG(4), channels, pixel4) );

	float aspectRatio = width * height;

	float r1, r2, r3, r4;
	int x, y, c;
	int pos;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			r1 = (float)((width-x) * (height-y)) / aspectRatio;
			r2 = (float)((x)       * (height-y)) / aspectRatio;
			r3 = (float)((width-x) * (y))        / aspectRatio;
			r4 = (float)((x)       * (y))        / aspectRatio;
			
			pos = ( x + y * width ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[pos+c] += pixel1[c]*r1 + pixel2[c]*r2 + pixel3[c]*r3 + pixel4[c]*r4;
		}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( AddGradiantLinear ) {

	RT_ASSERT_ARGC( 2 );
	
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	float *curvex = (float*)malloc( width * sizeof(float) ); // (TBD) free curvex
	RT_CHECK_CALL( InitCurveData( cx, J_FARG(1), width, curvex ) );

	float *curvey = (float*)malloc( height * sizeof(float) ); // (TBD) free curvey
	RT_CHECK_CALL( InitCurveData( cx, J_FARG(2), height, curvey ) );

	int x, y, c;
	int pos;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {
	
			pos = ( x + y * width ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[pos+c] += curvex[x] * curvey[y];
		}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( AddGradiantRadial ) {

	RT_ASSERT_ARGC( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	bool drawToCorner;
	if ( argc >= 2 )
		RT_JSVAL_TO_BOOL( J_FARG(2), drawToCorner );
	else
		drawToCorner = true;

	float radius;
	if ( drawToCorner )
		radius = Length2D( width, height ) / 2.0;
	else
		radius = MAX( width, height ) / 2.0;

	float *curve = (float*)malloc( (int)radius * sizeof(float) ); // (TBD) free curve
	InitCurveData(cx, J_FARG(1), (int)radius, curve);

	float aspectRatio = (float)width / (float)height;

	// draw
	float dist;
	int x, y, c;
	int pos, curvePos;
	float curveValue;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {
		
			dist = Length2D( (float)x / width - 0.5, (float)y / height - 0.5 ); // distance to the center ( 0..M_SQRT1_2 )

			if ( drawToCorner ) {

				pos = (x + y * width) * channels; // (TBD) use borderMode
				curvePos = dist * (radius-1) / M_SQRT1_2;
				curveValue = curve[curvePos];
				for ( c = 0; c < channels; c++ )
					tex->cbuffer[pos+c] += curveValue;
			} else
			if ( dist <= 0.5 ) { // if dist == 0.5, (int)(dist*radius) is out of the curve data

				pos = (x + y * width) * channels; // (TBD) use borderMode
				curvePos = dist * (radius-1) * 2;
				curveValue = curve[curvePos];
				for ( c = 0; c < channels; c++ )
					tex->cbuffer[pos+c] += curveValue;
			}
		}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}



/*
DEFINE_FUNCTION( AddGradiantRadial ) {

	RT_ASSERT_ARGC( 3 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	int ox, oy;
	RT_JSVAL_TO_INT32( J_FARG(1), ox );
	RT_JSVAL_TO_INT32( J_FARG(2), oy );

	int radius;
	RT_JSVAL_TO_INT32( J_FARG(3), radius );

	BorderMode borderMode = borderWrap;

	float *curve = (float*)malloc( radius * sizeof(float) ); // JS_malloc(cx,
	InitCurveData( cx, J_FARG(4), radius, curve );

	// draw
	Vector3 p;
	int dist;
	int x, y, c;
	int pos;
	float curveValue;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			switch (borderMode) {
				case borderWrap:
					break;
				case borderClamp:
					break;
			}

			// (TBD) manage border
			if ( x >= ox-radius && x <= ox+radius && y >= oy-radius && y <= oy+radius ) { // are we inside the box-radius ?
				
				Vector3Set(&p, ox - x , oy - y, 0);
				dist = (int)Vector3Len(&p);
				if ( dist < radius ) {

					pos = ((x%width) + (y%height) * width) * channels; // (TBD) use borderMode

//					curveValue = curve[dist * curveLength / radius];
					curveValue = curve[dist];
					for ( c = 0; c < channels; c++ )
						tex->cbuffer[pos+c] += curveValue;
				}
			}		
		}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}
*/



DEFINE_FUNCTION_FAST( AddCracks ) { // source: FxGen

	RT_ASSERT_ARGC( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	int count;
	RT_JSVAL_TO_INT32( J_FARG(1), count );

	int crackMaxLength;
	RT_JSVAL_TO_INT32( J_FARG(2), crackMaxLength );
	
	float variation;
	RT_JSVAL_TO_REAL( J_FARG(3), variation );

	PTYPE pixel[PMAXCHANNELS];
	RT_CHECK_CALL( InitLevelData(cx, J_FARG(4), channels, pixel) );

	float *curve = (float*)malloc( crackMaxLength * sizeof(float) ); // (TBD) free curve
	InitCurveData( cx, J_FARG(5), crackMaxLength, curve );

	// draw
	int pos, c;
	float curveValue;
	//Process operator
	int n = 0;
	while( n++ < count ) {
		float x = genrand_real1() * width;
		float y = genrand_real1() * height;
		float a = 2.f * M_PI * genrand_real1();
//		int crackLength = (int)(genrand_real1() * crackMaxLength);
		int crackLength = crackMaxLength;

		while( --crackLength >= 0 ) {

			int ix = int(x)%width;
			int iy = int(y)%height;
			
			pos = (ix + iy * width) * channels;
			for ( c = 0; c < channels; c++ )

//			curveValue = curve[crackLength * curveLength / length];
			curveValue = curve[crackLength];
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[pos+c] += curveValue * pixel[c];

			x = x + cos(a);
			y = y + sin(a);
			if( x < 0.0 ) x = x + width;
			if( y < 0.0 ) y = y + height;

			a = a + variation*(2 * genrand_real1() - 1);
		}
	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_PROPERTY( vmax ) {

	RT_CHECK_CALL( JS_NewNumberValue(cx, PMAX, vp) );
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


DEFINE_FUNCTION_FAST( RandSeed ) {

	RT_ASSERT_ARGC(1);
	unsigned long seed;
	RT_JSVAL_TO_UINT32(J_FARG(1), seed);
	init_genrand(seed);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( RandInt ) {

	int i = genrand_int31();
	*J_FRVAL = INT_TO_JSVAL(i);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( RandReal ) {

	jsdouble d = genrand_real1();
	RT_CHECK_CALL( JS_NewNumberValue(cx, d, J_FRVAL) );
	return JS_TRUE;
}


//DEFINE_FUNCTION( Noise ) {
//
//	RT_ASSERT_ARGC(1);
//	unsigned long seed;
//	RT_JSVAL_TO_UINT32(J_FARG(1), seed);
//	jsdouble d = NoiseInt(seed);
//	RT_CHECK_CALL( JS_NewNumberValue(cx, d, rval) );
//	return JS_TRUE;
//}


DEFINE_FUNCTION_FAST( PixelAt ) {

	RT_ASSERT_ARGC(2);
	RT_ASSERT_INT(J_FARG(1));
	RT_ASSERT_INT(J_FARG(2));

	int x = JSVAL_TO_INT(J_FARG(1));
	int y = JSVAL_TO_INT(J_FARG(2));

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tex);

	PTYPE *ptr = tex->cbuffer + tex->channels * ( x + tex->width * y );

	JSObject *jsArray = JS_NewArrayObject(cx, 0, NULL);
	*J_FRVAL = OBJECT_TO_JSVAL(jsArray);
	jsval value;
	for (int i=0; i<tex->channels; ++i) {

		JS_NewNumberValue(cx, *(ptr + i), &value); // JS_NewDoubleValue(cx, vector[i], &value);
		JS_SetElement(cx, jsArray, i, &value);
	}
	return JS_TRUE;
}


#ifdef _DEBUG

static JSBool Test(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_FALSE;
}

#endif // _DEBUG


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( Free )
		FUNCTION_FAST( Swap )
		FUNCTION_FAST( ClearChannel )
		FUNCTION_FAST( ToHLS )
		FUNCTION_FAST( ToRGB )
		FUNCTION_FAST( SetChannel )
		FUNCTION_FAST( Desaturate )
		FUNCTION_FAST( Colorize )
		FUNCTION_FAST( ExtractColor )
		FUNCTION2_FAST( SetPixel, _SetPixel )
		FUNCTION_FAST( SetRectangle )
		FUNCTION_FAST( AddNoise )
		FUNCTION_FAST( Set )
		FUNCTION_FAST( Add )
		FUNCTION_FAST( Mult )
		FUNCTION_FAST( Blend )
		FUNCTION_FAST( RotoZoom )
		FUNCTION_FAST( Resize )
		FUNCTION_FAST( Trim )
		FUNCTION_FAST( Copy )
		FUNCTION_FAST( Paste )

		FUNCTION_FAST( Export )
		FUNCTION_FAST( Import )

		FUNCTION_FAST( Flip )
		FUNCTION_FAST( Shift )
		FUNCTION_FAST( Convolution )
		FUNCTION_FAST( Normals )
		FUNCTION_FAST( NormalizeVectors )
		FUNCTION_FAST( Light )
		FUNCTION_FAST( BoxBlur )
		FUNCTION_FAST( ClampLevels )
		FUNCTION_FAST( CutLevels )
		FUNCTION_FAST( Displace )
		FUNCTION_FAST( NormalizeLevels )
		FUNCTION_FAST( InvertLevels )
		FUNCTION_FAST( OppositeLevels )
		FUNCTION_FAST( PowLevels )
		FUNCTION_FAST( MirrorLevels )
		FUNCTION_FAST( WrapLevels )
		FUNCTION_FAST( Aliasing )
		FUNCTION_FAST( Cells )
		FUNCTION_FAST( AddCracks )
		FUNCTION_FAST( AddGradiantQuad )
		FUNCTION_FAST( AddGradiantLinear )
		FUNCTION_FAST( AddGradiantRadial )
		FUNCTION_FAST( PixelAt )

#ifdef _DEBUG
		FUNCTION( Test )
#endif // _DEBUG

	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(vmax)
		PROPERTY_READ(width)
		PROPERTY_READ(height)
		PROPERTY_READ(channels)
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST( RandSeed )
		FUNCTION_FAST( RandInt )
		FUNCTION_FAST( RandReal )
//		FUNCTION_FAST( Noise )
	END_STATIC_FUNCTION_SPEC

	BEGIN_CONST_DOUBLE_SPEC
		CONST_DOUBLE( borderClamp, borderClamp )
		CONST_DOUBLE( borderWrap, borderWrap )
		CONST_DOUBLE( borderMirror, borderMirror )
		CONST_DOUBLE( borderValue, borderValue )

		CONST_DOUBLE( desaturateLightness, desaturateLightness )
		CONST_DOUBLE( desaturateSum, desaturateSum )
		CONST_DOUBLE( desaturateAverage, desaturateAverage )
		
	END_CONST_DOUBLE_SPEC

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



float a = 0;
int b = 2;
int c = 3;

a = b / c; // 0
a = (float)b / c; // 0.66666669
a = b / (float)c; // 0.66666669
a = (float)b / (float)c; // 0.66666669

a = 2 / 3; // 0
a = 2.f / 3; // 0.66666669
a = 2 / 3.f; // 0.66666669
a = 2.f / 3.f; // 0.66666669


2 / 3
0

2 / 3.0
0.66666666666666663

2.0 / 3
0.66666666666666663
*/