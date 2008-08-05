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

#include "../jslang/blobapi.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <cstring>
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
		J_CHK( JS_ValueToNumber(cx, value, &dval) );
		PTYPE val = (PTYPE)dval;
		for ( int i = 0; i < count; i++ )
			level[i] = val;
	} else if ( JSVAL_IS_OBJECT(value) && JS_IsArrayObject(cx, JSVAL_TO_OBJECT(value)) ) {

		FloatArrayToVector(cx, count, &value, level);
	} else if ( JSVAL_IS_STRING(value) ) {

		const char *color;
		size_t length;
		J_CHK( JsvalToStringAndLength(cx, &value, &color, &length) );
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

//	J_REPORT_ERROR("Invalid level data.");
	return JS_TRUE;
}


// curve: number | function | array
inline JSBool InitCurveData( JSContext* cx, jsval value, int length, float *curve ) { // length is the curve resolution
 
	if ( JSVAL_IS_NUMBER(value) ) {
		
		jsdouble dval;
		J_CHK( JS_ValueToNumber(cx, value, &dval) );
		PTYPE val = (PTYPE)dval;
		for ( int i = 0; i < length; i++ )
			curve[i] = val;
	} else 
	if ( JsvalIsFunction(cx, value) ) {

		jsval val[2], resultValue;
		jsdouble fval;
		for ( int i = 0; i < length; i++ ) {
			
			fval = i / (float)(length-1);
			J_CHK( JS_NewDoubleValue(cx, fval, val) );
			val[1] = INT_TO_JSVAL(i);
			J_CHK( JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), value, 2, val, &resultValue) );
			J_CHK( JS_ValueToNumber(cx, resultValue, &fval) );
			curve[i] = fval;
		}
	} else 
	if ( JsvalIsArray(cx, value) ) {

		int curveArrayLength;
		J_CHK( ArrayLength(cx, &curveArrayLength, value) );
		float *curveArray = (float*)malloc( curveArrayLength * sizeof(float) ); // (TBD) free the curveArray ???
		J_CHK( FloatArrayToVector(cx, curveArrayLength, &value, curveArray) );

		for ( int i = 0; i < length; i++ )
			curve[i] = curveArray[ i * curveArrayLength / length ];
	} else
	if ( JsvalIsBlob(cx, value) ) { // (TBD) test it. Replace JsvalIsBlob with something more generic like JsvalIsData that will match for string, blob, NIBufferGet, ...

		JSObject *bstrObj = JSVAL_TO_OBJECT(value);
		size_t bstrLen;
		const u_int8_t *bstrData;

//		BlobGetBufferAndLength( cx, bstrObj, (void**)&bstrData, &bstrLen );
		J_CHK( JsvalToStringAndLength( cx, &value, (const char **)&bstrData, &bstrLen ) );

		for ( int i = 0; i < length; i++ )
			curve[i] = bstrData[ i * bstrLen / length ] / 256;
	}	
	else {
      // (TBD) throws an error ?
		for ( int i = 0; i < length; i++ )
			curve[i] = PMAX;
	}
	return JS_TRUE;
}

/**doc
$SET pval pixel component intensity value
**/

/**doc
$CLASS_HEADER
**/

BEGIN_CLASS( Texture )

JSBool NativeInterfaceBufferGet( JSContext *cx, JSObject *obj, const char **buf, size_t *size ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( tex );
	*buf = (char*)tex->cbuffer;
	*size = tex->width * tex->height * tex->channels * sizeof(PTYPE);
	return JS_TRUE;
}


inline bool IsTexture( JSContext *cx, jsval value ) {
	
	return ( JSVAL_IS_OBJECT( value ) && JS_GET_CLASS(cx, JSVAL_TO_OBJECT( value )) == classTexture );
}


JSBool ValueToTexture( JSContext* cx, jsval value, Texture **tex ) {
	
	J_S_ASSERT_OBJECT( value );
	JSObject *texObj = JSVAL_TO_OBJECT( value );
	J_S_ASSERT_CLASS( texObj, classTexture );
	*tex = (Texture *)JS_GetPrivate(cx, texObj);
	J_S_ASSERT_RESOURCE(tex);
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

/**doc
 * $INAME( width, height, channels )
 * $INAME( sourceTexture )
 * $INAME( image )
  Creates a new Texture object.
  $H arguments
   $ARG integer height: height of texture.
   $ARG integer width: width of texture.
   $ARG integer channels: number of channels of the texture (current limit is 4). Channel has a meaning only in a few part of the API like ToHLS(), ToRGB(), ...
   $ARG Texture sourceTexture: an existing Texture object (acs like a copy constructor).
   $ARG ImageObject image: an existing Image object (From a jpeg image for example)
  $H note
   jsprotex uses single precision values per channel. The visibles values are in range [0,1].
   The darker value is 0.0 and the brighter value is 1.0.
  $H example
  {{{
  LoadModule('jsstd');
  LoadModule('jsimage');
  
  var image = DecodePngImage( new File('picture.png').Open("r") );
  var tex = new Texture(image);
  tex.NormalizeLevels();
  }}}
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN( 1 );
	Texture *tex = (Texture *)JS_malloc(cx, sizeof(Texture));
	tex->cbackBuffer = NULL;

	J_CHK( SetBufferGetInterface(cx, obj, NativeInterfaceBufferGet) );

	if ( J_ARGC >= 3 ) {
	
		int width, height, channels;
		J_JSVAL_TO_INT32( J_ARG(1), width );
		J_JSVAL_TO_INT32( J_ARG(2), height );
		J_JSVAL_TO_INT32( J_ARG(3), channels );

		J_S_ASSERT( width > 0, "Invalid width." );
		J_S_ASSERT( height > 0, "Invalid height." );
		J_S_ASSERT( channels <= PMAXCHANNELS, "Too many channels." );

		tex->cbuffer = (PTYPE*)JS_malloc(cx, width * height * channels * sizeof(PTYPE) );
		J_S_ASSERT_ALLOC( tex->cbuffer );

		tex->width = width;
		tex->height = height;
		tex->channels = channels;
		JS_SetPrivate(cx, obj, tex);
		return JS_TRUE;
	}

	if ( IsTexture(cx, J_ARG(1)) ) { // copy constructor

		Texture *srcTex = (Texture *)JS_GetPrivate(cx, JSVAL_TO_OBJECT(J_ARG(1)));
		J_S_ASSERT_RESOURCE(srcTex);

		int tsize = srcTex->width * srcTex->height * srcTex->channels;
		tex->cbuffer = (PTYPE*)JS_malloc(cx, tsize * sizeof(PTYPE) );
		J_S_ASSERT_ALLOC( tex->cbuffer );

		memcpy( tex->cbuffer, srcTex->cbuffer, tsize * sizeof(PTYPE) );
		tex->width = srcTex->width;
		tex->height = srcTex->height;
		tex->channels = srcTex->channels;
		JS_SetPrivate(cx, obj, tex);
		return JS_TRUE;
	}

//	if ( JsvalIsDataBuffer( cx, J_ARG(1) ) ) {
	if ( JSVAL_IS_STRING(J_ARG(1)) || JSVAL_IS_OBJECT(J_ARG(1)) && BufferGetInterface(cx, JSVAL_TO_OBJECT(J_ARG(1))) != NULL ) {

		JSObject *bstrObj;
//		bstrObj = JSVAL_TO_OBJECT( J_ARG(1) );
		J_CHK( JS_ValueToObject(cx, J_ARG(1), &bstrObj) );

		int dWidth = tex->width;
		int dHeight = tex->height;
		int dChannels = tex->channels;
		
		int sWidth, sHeight, sChannels;
		J_CHK( GetPropertyInt(cx, bstrObj, "width", &sWidth) );
		J_CHK( GetPropertyInt(cx, bstrObj, "height", &sHeight) );
		J_CHK( GetPropertyInt(cx, bstrObj, "channels", &sChannels) );

		const char *buffer;
//		u_int8_t *buffer = (u_int8_t*)
//		J_CHK( BlobBuffer(cx, bstr, (const void **)&buffer) );
		J_CHK( JsvalToString(cx, &J_ARG(1), (const char **)&buffer) ); // warning: GC on the returned buffer !

		tex->width = sWidth;
		tex->height = sHeight;
		tex->channels = sChannels;


		int tsize = sWidth * sHeight * sChannels;

		tex->cbuffer = (PTYPE*)JS_malloc(cx, tsize * sizeof(PTYPE) );
		J_S_ASSERT_ALLOC( tex->cbuffer );

		for ( int i=0; i<tsize; i++ )
			tex->cbuffer[i] = buffer[i] / (PTYPE)256;

		JS_SetPrivate(cx, obj, tex);

		return JS_TRUE;
	}

	J_REPORT_ERROR( "Invalid arguments" );
}

/**doc
=== Methods ===
**/

/**doc
 * $VOID $INAME()
  Free the memory allocated by the current texture.
  This operation is not mendatory but can be usefull to free memory.
**/
DEFINE_FUNCTION( Free ) {

	J_S_ASSERT_CLASS(obj, _class);
	Finalize(cx, obj);
	return JS_TRUE;
}

/**doc
 * $THIS $INAME( otherTexture )
  Swaps the content of two textures the current one and _otherTexture_.
  $H arguments
   $ARG Texture otherTexture: texture object against witch the exchange is done.
  $H example
  {{{
  function AddAlphaChannel( tex ) {
    
    if ( tex.channels == 1 )
      new Texture(tex.width, tex.height, 2).SetChannel(0, tex, 0).Swap(tex);
    else if ( tex.channels == 3 )
      new Texture(tex.width, tex.height, 4).SetChannel(0, tex, 0).SetChannel(1, tex, 1).SetChannel(2, tex, 2).Swap(tex);
  }
  }}}
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( Swap ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_CLASS( J_FOBJ, _class );
	J_S_ASSERT_OBJECT( J_FARG(1) );
	JSObject *texObj = JSVAL_TO_OBJECT( J_FARG(1) );
	J_S_ASSERT_CLASS( texObj, _class );
	void *tmp = JS_GetPrivate(cx, J_FOBJ);
	JS_SetPrivate(cx, J_FOBJ, JS_GetPrivate(cx, texObj));
	J_CHK( JS_SetPrivate(cx, texObj, tmp) );
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

/**doc
 * $THIS $INAME( [channel] )
  Clears (set to 0) the given _channel_ or all channels if the method is called without argument.
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( ClearChannel ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	if ( argc == 0 ) { // clear all channels

		memset(tex->cbuffer, 0, tex->width * tex->height * tex->channels * sizeof(PTYPE));
	} else
	if ( argc >= 1 ) {

		int channel;
		J_JSVAL_TO_INT32( J_FARG(1), channel );
		J_S_ASSERT( channel < tex->channels, "Invalid channel." );

		PTYPE *ptr = tex->cbuffer;
		ptr += channel;

		int tsize = tex->width * tex->height;
		int channels = tex->channels;
		for ( int i = 0; i < tsize; i++ ) {

			*ptr = 0;
			ptr += channels;
		}
	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

/**doc
 * $THIS $INAME( destinationChannel, otherTexture, sourceChannel )
  Replace the _destinationChannel_ channel of the current texture with the _sourceChannel_ channel of the _otherTexture_.
  $H arguments
   $ARG integer destinationChannel: a channel of the current texture.
   $ARG Texture otherTexture: the texture from witch a channel will be imported.
   $ARG integer destinationChannel: the channel of the _otherTexture_ to be imported.
  $H example
  {{{
  function NoiseChannel( tex, channel ) {

    var tmp = new Texture(tex.width, tex.height, 1);
    tmp.AddNoise();
    tex.SetChannel(channel, tmp, 0);
    tmp.Free();
  }
  }}}
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( SetChannel ) {

	J_S_ASSERT_ARG_MIN( 3 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	int dstChannel;
	J_JSVAL_TO_INT32( J_FARG(1), dstChannel );

	Texture *tex1;
	J_CHK( ValueToTexture(cx, J_FARG(2), &tex1) );

	int srcChannel;
	J_JSVAL_TO_INT32( J_FARG(3), srcChannel );

	int texChannel = tex->channels;
	int tex1Channel = tex1->channels;

	J_S_ASSERT( tex->width == tex1->width && tex->height == tex1->height, "Images must have the same size.");
	J_S_ASSERT( srcChannel < tex1Channel && srcChannel <= PMAXCHANNELS, "Invalid source channel.");
	J_S_ASSERT( dstChannel < texChannel && dstChannel <= PMAXCHANNELS, "Invalid destination channel.");

	int size = tex->width * tex->height;
	for ( int i = 0; i < size; i++ )
		tex->cbuffer[texChannel * i + dstChannel] = tex1->cbuffer[tex1Channel * i + srcChannel];

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


/**doc
 * $THIS $INAME()
  Does a conversion from RGB (Red, Green, Blue) to HSV (Hue, Saturation, Value) colorspace.
**/
// (TBD) PTYPE
DEFINE_FUNCTION_FAST( ToHLS ) { // (TBD) test it

	// see http://svn.gnome.org/viewcvs/gimp/trunk/libgimpcolor/gimpcolorspace.c?view=markup

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int channels = tex->channels;
	J_S_ASSERT( channels >= 3, "Invalid pixel format (need RGB).");

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

/**doc
 * $THIS $INAME()
  Does a conversion from HSV (Hue, Saturation, Value) to RGB (Red, Green, Blue) colorspace.
**/
// (TBD) PTYPE
DEFINE_FUNCTION_FAST( ToRGB ) { // (TBD) test it
	// see http://svn.gnome.org/viewcvs/gimp/trunk/libgimpcolor/gimpcolorspace.c?view=markup

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int channels = tex->channels;
	
	J_S_ASSERT( channels >= 3, "Invalid pixel format (need HLS).");

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

/**doc
 * $THIS $INAME( count [, curve] )
  Reduce the number of values used for each channel.
  $H arguments
   $ARG integer count: the number of different $pval in the resulting texture.
   $ARG curveInfo curve: the transformation curve used for each value. For further information about ,,curveInfo,, , see lower.
  $H note
   If _curveInfo_ is not provided, each channel is processed in a linear manner using the following formula:
    floor( count `*` colorValue ) / count
  $H note
   Each channel are processed independently.
  $H example 1
  {{{
  var t = Cloud(size, 0.5);
  t.Aliasing(2);
  }}}
  $H example 2
  {{{
  const curveLinear = function(v) { return v }
  var t = Cloud(size, 0.5);
  t.Aliasing(8, curveLinear);
  t.BoxBlur(3, 3)
  }}}
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( Aliasing ) {

	J_S_ASSERT_ARG_MIN( 1 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	long count;
	J_JSVAL_TO_UINT32( J_FARG(1), count );

	bool useCurve;

	float *curve;
	if ( argc >= 2 ) {

		useCurve = true;
		curve = (float*)malloc( count * sizeof(float) ); // (TBD) free curve ???
		J_CHK( InitCurveData( cx, J_FARG(2), count, curve ) );
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

/**doc
 * $THIS $INAME( fromColorInfo, toColorInfo )
  $H arguments
   $ARG colorInfo fromColorInfo: The color to be changed.
   $ARG colorInfo toColorInfo: The substitute color. For further information about ,,colorInfo,, see below.
  $H example
  {{{
  const BLUE = [0,0,1,1];
  const WHITE = [1,1,1,1];
  
  var texture = new Texture( 100, 100, 3 );
  ...
  texture.Colorize( WHITE, BLUE, 0 );
  }}}
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( Colorize ) {
	// GIMP color to alpha: http://www.google.com/codesearch?hl=en&q=+gimp+%22color+to+alpha%22
	// color exchange algo. : http://www.koders.com/c/fidB39DAC5A8DB8B6073D78FB23363C5E0541208B02.aspx

	J_S_ASSERT_ARG_MAX(2);

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	int channels = tex->channels;

	PTYPE colorSrc[PMAXCHANNELS];
	J_CHK( InitLevelData(cx, J_FARG(1), channels, colorSrc) );

	PTYPE colorDst[PMAXCHANNELS];
	J_CHK( InitLevelData(cx, J_FARG(2), channels, colorDst) );

	float power;
	if ( argc >= 3 )
		J_JSVAL_TO_REAL(J_FARG(3), power);
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


/**doc
 * $THIS $INAME( sourceTexture, sourceColorInfo [, strength = 1] )
  Fill the current texture with a given color from _sourceTexture_.
  $H arguments
   $ARG Texture sourceTexture: the texture from witch the color will be extracted.
   $ARG colorInfo sourceColorInfo: The color to extract.
   $ARG real strength: The strength of the exraction.
  $H note
   The current texture must have only one channel because the method only extracts one color.
  $H example
  {{{
  t1.ExtractColor(t, RED, 10);
  }}}
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( ExtractColor ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	J_S_ASSERT( tex->channels == 1, "Destination texture must have only 1 channel.");

	Texture *texSrc;
	J_CHK( ValueToTexture(cx, J_FARG(1), &texSrc) );
	J_S_ASSERT( tex->width == texSrc->width && tex->height == texSrc->height, "Images must have the same width and height." );

	int srcChannels = texSrc->channels;

	PTYPE color[PMAXCHANNELS];
	J_CHK( InitLevelData(cx, J_FARG(2), srcChannels, color) );

	float power;
	if ( argc >= 3 )
		J_JSVAL_TO_REAL(J_FARG(3), power);
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

/**doc
 * $THIS $INAME()
  Changes the range of each $pval of the texture. The resulting range of each $pval will be [0,1]
  $H note
   Normalization is sometimes called contrast stretching.
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( NormalizeLevels ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

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


/**doc
 * $THIS $INAME( min, max )
  All $pval that are out of the [min,max] range are forced to [min,max] range.
  $H arguments
   $ARG real min: low value
   $ARG real max: high value
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( ClampLevels ) { // (TBD) check if this algo is right

	J_S_ASSERT_ARG_MIN( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	PTYPE min, max;
	J_JSVAL_TO_REAL( J_FARG(1), min );
	J_JSVAL_TO_REAL( J_FARG(2), max );

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


/**doc
 * $THIS $INAME( min, max )
  All $pval that are out of the [min,max] range are forced to [0,1] range.
  $H arguments
   $ARG real min: low value
   $ARG real max: high value
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( CutLevels ) { // (TBD) check if this algo is right

	J_S_ASSERT_ARG_MIN( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	PTYPE min, max;
	J_JSVAL_TO_REAL( J_FARG(1), min );
	J_JSVAL_TO_REAL( J_FARG(2), max );

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
	J_S_ASSERT_RESOURCE(tex);

	PTYPE min, max;

	if ( argc == 0 ) {

		min = 0;
		max = PMAX;
	} 
	
	if ( argc >= 1 ) {

		J_JSVAL_TO_REAL( J_FARG(1), min );
		max = min;
	}
	
	if ( argc >= 2 ) {

		J_JSVAL_TO_REAL( J_FARG(2), max );
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

/**doc
 * $THIS $INAME()
  Each $pval is mathematically inverted ( v = 1 / v ).
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( InvertLevels ) { // level = 1 / level

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] = 1 / tex->cbuffer[i];
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

/**doc
 * $THIS $INAME()
  Each $pval is set to its mathematical opposite ( v = -v ).
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( OppositeLevels ) { // level = -level

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] =  - tex->cbuffer[i];
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

/**doc
 * $THIS $INAME( power )
  Each $pval is powered by _power_ ( v = v ^ _power_ ).
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( PowLevels ) { // 

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	J_S_ASSERT_ARG_MIN( 1 );

	float power;
	J_JSVAL_TO_REAL( J_FARG(1), power );

	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] =  powf(tex->cbuffer[i], power);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


/**doc
 * $THIS $INAME( threshold, mirrorFromTop )
  Each $pval is mirrored toward the top or the bottom.
  $H arguments
   $ARG real threshold: the point from the values are reflected.
   $ARG boolean mirrorFromTop: if true, the values over _threshold_ are reflected toward the bottom, else values under _threshold_ are reflected toward the top.
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( MirrorLevels ) {

	J_S_ASSERT_ARG_MIN( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	PTYPE threshold;
	J_JSVAL_TO_REAL( J_FARG(1), threshold );
	
	bool mirrorTop;
	if ( argc >= 2 && !JSVAL_IS_VOID(J_FARG(2)) )
		J_JSVAL_TO_BOOL( J_FARG(2), mirrorTop );
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

/**doc
 * $THIS $INAME( modulo )
  Each $pval is moduloed by _modulo_ ( v = v % _modulo_ ).
  $H arguments
   $REAL modulo: the non-integer modulo
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( WrapLevels ) { // real modulo

	J_S_ASSERT_ARG_MIN( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	PTYPE wrap;
	J_JSVAL_TO_REAL(J_FARG(1), wrap);

	PTYPE div;
	int tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ ) {

		div = tex->cbuffer[i] / wrap;
		tex->cbuffer[i] = div - (long)div;
	}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

/**doc
 * $THIS $INAME( [color] )
  Adds a random noise to the current texture.
  $H arguments
   $ARG colorInfo color: noise color.
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( AddNoise ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int channels = tex->channels;

	bool fullLevel;

	PTYPE pixel[PMAXCHANNELS];
	if ( argc >= 1 ) {

		J_CHK( InitLevelData(cx, J_FARG(1), tex->channels, pixel) );
		fullLevel = false;
	} else {

		fullLevel = true;
	}

	int i, tsize = tex->width * tex->height * channels;
	if ( fullLevel )
		for ( i = 0; i < tsize; i++ )
			tex->cbuffer[i] += PRAND;
	else
		for ( i = 0; i < tsize; i++ )
			tex->cbuffer[i] += PRAND * pixel[i % channels] / PMAX; //(TBD) test if i%channels works fine, else use double for loop
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


/**doc
 * $THIS $INAME( sourceTexture [, mode] )
  Desaturates _sourceTexture_ texture an put the result in the current texture.
  $H arguments
   $ARG Texture sourceTexture: texture from witch the desaturation will be done.
   $ARG integer mode: is the type of desaturation, either Texture.desaturateLightness, Texture.desaturateSum or Texture.desaturateAverage
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( Desaturate ) {

	J_S_ASSERT_ARG_MIN(2);

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	J_S_ASSERT( tex->channels == 1, "Destination texture must have only one channel.");

	Texture *texSrc;
	J_CHK( ValueToTexture(cx, J_FARG(1), &texSrc) );
	J_S_ASSERT( tex->width == texSrc->width && tex->height == texSrc->height, "Images must have the same width and height." );

	int modeVal;
	J_JSVAL_TO_INT32( J_FARG(2), modeVal );
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


/**doc
 * $THIS $INAME( otherTexture )
 * $THIS $INAME( color )
  $H arguments
   $ARG Texture otherTexture: 
   $ARG colorInfo color: 
  Set a texture with another texture or a given _color_.
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( Set ) {

	J_S_ASSERT_ARG_MIN( 1 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int channels = tex->channels;
	int tsize = tex->width * tex->height * channels;
	if ( IsTexture(cx, J_FARG(1)) ) {
		
		Texture *tex1;
		J_CHK( ValueToTexture(cx, J_FARG(1), &tex1) );
		J_S_ASSERT( tex->width == tex1->width && tex->height == tex1->height && channels == tex1->channels, "Images must have the same size." );
		for ( int i = 0; i < tsize; i++ )
			tex->cbuffer[i] = tex1->cbuffer[i];
	} else {

		PTYPE pixel[PMAXCHANNELS];
		J_CHK( InitLevelData(cx, J_FARG(1), channels, pixel) );
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


/**doc
 * $THIS $INAME( textureObject )
 * $THIS $INAME( colorInfo )
  Mathematically adds a texture (_textureObject_) or a given color (_colorInfo_) to the current texture.
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( Add ) {

	J_S_ASSERT_ARG_MIN( 1 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int channels = tex->channels;
	int tsize = tex->width * tex->height * channels;
	if ( IsTexture(cx, J_FARG(1)) ) {
		
		Texture *tex1;
		J_CHK( ValueToTexture(cx, J_FARG(1), &tex1) );
		J_S_ASSERT( tex->width == tex1->width && tex->height == tex1->height && channels == tex1->channels, "Images must have the same size." );
		for ( int i = 0; i < tsize; i++ )
			tex->cbuffer[i] += tex1->cbuffer[i];
	} else {

		PTYPE pixel[PMAXCHANNELS];
		J_CHK( InitLevelData(cx, J_FARG(1), channels, pixel) );
		int i, c, size = tex->width * tex->height;
		for ( i = 0; i < size; i++ )
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[i*channels+c] += pixel[c];
	}
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


/**doc
 * $THIS $INAME( textureObject )
 * $THIS $INAME( colorInfo )
  Mathematically multiply a texture (_textureObject_) or a given color (_colorInfo_) to the current texture.
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( Mult ) {

	J_S_ASSERT_ARG_MIN( 1 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int channels = tex->channels;
	int tsize = tex->width * tex->height * channels;

	if ( IsTexture(cx, J_FARG(1)) ) {
		
		Texture *tex1;
		J_CHK( ValueToTexture(cx, J_FARG(1), &tex1) );
		J_S_ASSERT( tex->width == tex1->width && tex->height == tex1->height && channels == tex1->channels, "Images must have the same size." );
		for ( int i = 0; i < tsize; i++ )
			tex->cbuffer[i] *= tex1->cbuffer[i];
/* using Aliasing() seems more useful than Mult()
	} else
	if ( JsvalIsFunction(cx, J_FARG(1)) ) {
		
		J_S_ASSERT_ARG_MIN( 2 );
		int aliasing;
		J_JSVAL_TO_INT32( J_FARG(2), aliasing );

		PTYPE *curve = (PTYPE*)malloc( sizeof(PTYPE) * aliasing );
		J_CHK( InitCurveData(cx, J_FARG(1), aliasing, curve) );

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
		J_CHK( InitLevelData(cx, J_FARG(1), channels, pixel) );
		int i, c, size = tex->width * tex->height;
		for ( i = 0; i < size; i++ )
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[i*channels+c] *= pixel[c];
	}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


/**doc
 * $THIS $INAME( otherTexture, blendTexture )
 * $THIS $INAME( otherTexture, color )
  Mathematically blends a texture (_textureObject_) or a given color (_colorInfo_) to the current texture.
  $H arguments
   $ARG Texture otherTexture: the texture to blend with the current one.
   $ARG Texture blendTexture: the texture that contains the blending coefficients for each pixel.
   $ARG colorInfo color: the color (ratio) ised to blend the current texture with _otherTexture_
  $H note
   The blend fornula is: this pixel = blend `*` _textureObject1_ + (1-blend) `*` _textureObject2_ pixel or _colorInfo_
  $H example
  {{{
  var tmp = new Texture(size, size, 1);
  tmp.ClearChannel();
  tmp.SetRectangle(10,10,size-10,size-10,1);
  t.Blend(tmp, 0.7);
  }}}
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( Blend ) { // texture1, blenderTexture|blenderColor

	J_S_ASSERT_ARG_MIN( 2 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int channels = tex->channels;
	int tsize = tex->width * tex->height * channels;

	Texture *tex1;
	J_CHK( ValueToTexture(cx, J_FARG(1), &tex1) );
	J_S_ASSERT( tex->width == tex1->width && tex->height == tex1->height && channels == tex1->channels, "Images must have the same size." );

	float blend;

	if ( IsTexture(cx, J_FARG(2)) ) {
		
		Texture *blenderTex;
		J_CHK( ValueToTexture(cx, J_FARG(1), &blenderTex) );
		J_S_ASSERT( tex->width == blenderTex->width && tex->height == blenderTex->height && channels == blenderTex->channels, "Images must have the same size." );
		for ( int i = 0; i < tsize; i++ ) {
			
			blend = blenderTex->cbuffer[i];
			tex->cbuffer[i] = (blend * tex->cbuffer[i] + (PMAX - blend) * tex1->cbuffer[i] ) / PMAX;
		}
	} else {

		PTYPE pixel[PMAXCHANNELS];
		J_CHK( InitLevelData(cx, J_FARG(2), channels, pixel) );

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


/**doc
 * $THIS *SetPixel*( x, y, colorInfo )
  Sets the color of the given pixel.
  $H note
   If x and y are wrapped to the image width and height.
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( SetPixel ) { // x, y, levels

	J_S_ASSERT_ARG_MIN( 3 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	int x, y;
	J_JSVAL_TO_INT32( J_FARG(1), x );
	J_JSVAL_TO_INT32( J_FARG(2), y );
	
	x = Wrap(x, tex->width);
	y = Wrap(y, tex->height);

	PTYPE pixel[PMAXCHANNELS];
	J_CHK( InitLevelData(cx, J_FARG(3), tex->channels, pixel) );

	int c, channels = tex->channels;
	int pos = (x + y * tex->width) * channels;
	for ( c = 0; c < channels; c++ )
		tex->cbuffer[pos+c] = pixel[c];

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

/**doc
 * $THIS $INAME( x0, y0, x1, y1, color )
  Draws a rectangle of the _colorInfo_ color over the current texture.
  $H arguments
   $ARG integer x0:
   $ARG integer y0:
   $ARG integer x1:
   $ARG integer y1:
   $ARG colorInfo color:
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( SetRectangle ) {

	J_S_ASSERT_ARG_MIN( 5 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	int x0, y0, x1, y1;
	J_JSVAL_TO_INT32( J_FARG(1), x0 );
	J_JSVAL_TO_INT32( J_FARG(2), y0 );
	J_JSVAL_TO_INT32( J_FARG(3), x1 );
	J_JSVAL_TO_INT32( J_FARG(4), y1 );

	PTYPE pixel[PMAXCHANNELS];
	J_CHK( InitLevelData(cx, J_FARG(5), tex->channels, pixel) );
	
//	PTYPE alpha;
//	if ( argc >= 6 )
//		J_JSVAL_TO_REAL( J_FARG(6), alpha )
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


/**doc
 * $THIS $INAME( count )
  Make _count_ 90 degres rotations.
  $H arguments
   $ARG integer count: the number of integer rotation to perform with the current texture. _count_ may be negative.
  $H note
   For non-integer rotations, see RotoZoom() function.
**/
DEFINE_FUNCTION_FAST( Rotate90 ) { // (TBD) test it

	J_S_ASSERT_ARG_MIN( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	int turn;
	J_JSVAL_TO_INT32(J_FARG(1), turn);

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


/**doc
 * $THIS $INAME( horizontally, vertically )
  Flips the current texture horizontally, vertically or both.
  $H arguments
   $ARG boolean horizontally: flips the texture horizontally (against x axis).
   $ARG boolean vertically: flips the texture vertically (against y axis).
**/
DEFINE_FUNCTION_FAST( Flip ) {

	J_S_ASSERT_ARG_MIN( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	bool flipX, flipY;
	J_JSVAL_TO_BOOL( J_FARG(1), flipX );
	J_JSVAL_TO_BOOL( J_FARG(2), flipY );

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


/**doc
 * $THIS $INAME( centerX, centerY, zoomX, zoomY, rotations )
  Make a zoom and/or a rotation of the current texture.
  $H arguments
   $ARG real centerX: coordinate of the center of the zoom or rotation.
   $ARG real centerY: 
   $ARG real zoomX: the zoom factor (use 1 for none).
   $ARG real zoomY: 
   $ARG real rotations: the number of totations to perform. 0.25 is 90 degres (use 0 for none).
**/
DEFINE_FUNCTION_FAST( RotoZoom ) { // source: FxGen

	J_S_ASSERT_ARG_MIN( 5 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	int newWidth = width;
	int newHeight = height;

	float centerX, centerY;
	J_JSVAL_TO_REAL( J_FARG(1), centerX );
	J_JSVAL_TO_REAL( J_FARG(2), centerY );

	float zoomX, zoomY;
	J_JSVAL_TO_REAL( J_FARG(3), zoomX );
	J_JSVAL_TO_REAL( J_FARG(4), zoomY );

//	zoomX = 0.5 - ( zoomX / 2 );
//	zoomX = exp( zoomX * 6 );

//	zoomY = 0.5 - ( zoomY / 2 );
//	zoomY = exp( zoomY * 6 );

	float rotate;
	J_JSVAL_TO_REAL( J_FARG(5), rotate ); // 1 for 1 turn

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


/**doc
 * $THIS $INAME( newWidth, newHeight, [interpolate = false [, borderMode = Texture.borderWrap ]] )
  Resize the current texture.
  $H arguments
   $ARG integer newWidth:
   $ARG integer newHeight:
   $ARG boolean interpolate: uses a linear interpolation.
   $ARG enum borderMode: how to manage the border. either Texture.borderClamp, Texture.borderWrap, Texture.borderMirror or Texture.borderValue.
**/
// PTYPE ok
DEFINE_FUNCTION_FAST( Resize ) {

	J_S_ASSERT_ARG_MIN( 2 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	int newWidth, newHeight;
	J_JSVAL_TO_REAL( J_FARG(1), newWidth );
	J_JSVAL_TO_REAL( J_FARG(2), newHeight );

	bool interpolate;
	if ( J_FARG_ISDEF(3) )
		J_JSVAL_TO_BOOL( J_FARG(3), interpolate );
	else
		interpolate = false;

	BorderMode borderMode; // (TBD) from function arg
	if ( J_FARG_ISDEF(4) ) {
		
		int tmp;
		J_JSVAL_TO_INT32( J_FARG(4), tmp );
		borderMode = (BorderMode)tmp;
	} else
		borderMode = borderWrap;


	if ( newWidth != width || newHeight != height ) { // nothing to do
		

		PTYPE *newBuffer = (PTYPE*)JS_malloc(cx, newWidth * newHeight * channels * sizeof(PTYPE) );
		J_S_ASSERT_ALLOC( newBuffer );

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
						default:
							J_REPORT_ERROR( "Invalid border mode." );
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


/**doc
 * $THIS $INAME( kernel, borderMode [, autoGain = Texture.borderWrap], [ autoGain = true ] )
  Apply a convolution to the current texture using _kernel_ factors.
  $H arguments
   $ARG Array kernel: kernel is a square matrix
   $ARG enum borderMode: how to manage the border. either Texture.borderClamp, Texture.borderWrap, Texture.borderMirror or Texture.borderValue.
   $ARG boolean autoGain: automatically ajust the weight of the _kernel_ 
  $H note
   The convolution is a complex transformation that could be very slow with big kernels.
  $H example
  {{{
  const kernelGaussian = [0,3,10,3,0, 3,16,26,16,3, 10,26,26,26,10, 3,16,26,16,3, 0,3,10,3,0 ];
  const kernelGaussian2 = [2,4,5,4,2, 4,9,12,9,4, 5,12,15,12,5, 4,9,12,9,4, 2,4,5,4,2]; // G(r) = pow(E,-r*r/(2*o*o))/sqrt(2*PI*o);
  const kernelEmboss = [-1,0,0, 0,0,0 ,0,0,1];
  const kernelLaplacian = [-1,-1,-1, -1,8,-1, -1,-1,-1];
  const kernelLaplacian4 = [0,-1,0, -1,4,-1 ,0,-1,0];
  const kernelShift = [0,0,0, 0,0,0 ,0,0,1];
  const kernelEmboss = [-1,0,0, 0,0,0 ,0,0,1];
  const kernelCrystals = [0,-1,0, -1,5,-1, 0,-1,0];
  ...
  texture.Convolution(kernelGaussian);
  }}}
**/
// (TBD) PTYPE
DEFINE_FUNCTION_FAST( Convolution ) {

	// (TBD) accumulate precalculated pixels ? ( like BoxBlur )

	J_S_ASSERT_ARG_MIN( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;

	int count;
	J_CHK( ArrayLength(cx, &count, J_FARG(1)) );
	float *kernel = (float*)malloc(sizeof(float) * count);
	J_S_ASSERT_ALLOC( kernel );
	J_CHK( FloatArrayToVector(cx, count, &J_FARG(1), kernel) );

	int size = (int)sqrtf(count);

	J_S_ASSERT( size * size == count, "Invalid convolution kernel size.");

	BorderMode borderMode;
	if ( J_FARG_ISDEF(2) ) {

		int tmp;
		J_JSVAL_TO_INT32( J_FARG(2), tmp );
		borderMode = (BorderMode)tmp;
	} else
		borderMode = borderWrap;

	float gain;
	bool autoGain;

	if ( J_FARG_ISDEF(3) )
		J_JSVAL_TO_BOOL( J_FARG(3), autoGain );
	else
		autoGain = true;

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
				default:
					J_REPORT_ERROR( "Invalid border mode." );
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


/**doc
 * $THIS $INAME( blurWidth, blurHeight )
  Apply a box blur of the given width and height.
  $H note
   BoxBlur is very fast but the result is not very smooth.
  $H example
  {{{
  function AddPixels(t, count) {
   while ( count-- > 0 )
    t.SetPixel(Texture.RandInt(), Texture.RandInt(), 1);
  }
  
  var texture = new Texture(128, 128, 3);
  texture.Set(0);
  AddPixels(texture, 100);
  texture.BoxBlur(20,20);
  texture.NormalizeLevels();
  }}}
**/
DEFINE_FUNCTION_FAST( BoxBlur ) {

	J_S_ASSERT_ARG_MIN( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;

	int bw, bh; // blur width & height
	J_JSVAL_TO_INT32( J_FARG(1), bw );
	J_JSVAL_TO_INT32( J_FARG(2), bh );

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


/**doc
 * $THIS $INAME()
  Converts each pixel into a vector, normalize this vector, then store the vector as a pixel.
**/
DEFINE_FUNCTION_FAST( NormalizeVectors ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

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


/**doc
 * $THIS $INAME( [ amplify = 1 ] )
  Converts the texture to a normals map using the Sobel filter.
**/
// (TBD) PTYPE
DEFINE_FUNCTION_FAST( Normals ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	if ( tex->channels != 3 && tex->cbackBuffer != NULL ) { // back buffer do not have the right format

		JS_free(cx, tex->cbackBuffer );
		tex->cbackBuffer = NULL;
	}

	if ( tex->cbackBuffer == NULL )
		tex->cbackBuffer = (PTYPE*)JS_malloc(cx, tex->width * tex->height * 3 * sizeof(PTYPE) );

	// from here, tex->cbackBuffer is a 3 channels buffer

	float amp;
	if ( argc >= 1 )
		J_JSVAL_TO_REAL( J_FARG(1), amp );
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


/**doc
 * $THIS $INAME( normalsTexture, lightPosition, ambiantColor, diffuseColor, specularColor, bumpPower, specularPower )
  Floodlight the current texture using the _normalsTexture_ as bump map.
  $H arguments
   $ARG Texture normalsTexture: the bump map where each pixel is a 3D vector.
   $ARG Array lightPosition: is the position of the light in a 3D space ( [x, y, z] )
   $ARG colorInfo ambiantColor: 
   $ARG colorInfo diffuseColor:
   $ARG colorInfo specularColor:
   $ARG real bumpPower:
   $ARG real specularPower:
  $H example
  {{{
  var bump = new Texture(size, size, 3).Cells(8, 0).Add( new Texture(size, size, 3).Cells(8, 1).OppositeLevels() ); // broken floor
  bump.Normals();
  texture.Set(1);
  texture.Light( bump, [-1, -1, 1], 0, [0.1, 0.3, 0.4], 0.2, 0.5, 10 );
  }}}
**/
// (TBD) PTYPE
DEFINE_FUNCTION_FAST( Light ) {
	// Simple Lighting: http://www.gamasutra.com/features/19990416/intel_simd_04.htm

	J_S_ASSERT_ARG_MIN( 5 );

	Texture *normals, *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	J_CHK( ValueToTexture(cx, J_FARG(1), &normals) );
	J_S_ASSERT( normals->channels == 3, "Invalid normals texture channel count." );

	J_S_ASSERT( tex->channels >= 3, "normals applys on a RGB or RGBA texture." );
	
	J_S_ASSERT( normals->width == tex->width && normals->height == tex->height, "Invalid normals texture size." );

	Vector3 lightPos;
	J_CHK( FloatArrayToVector(cx, 3, &J_FARG(2), lightPos.raw ) );

	PTYPE ambient[3];
	J_CHK( InitLevelData(cx, J_FARG(3), 3, ambient) );
	bool ambiantTexture = false;

	PTYPE diffuse[3];
	J_CHK( InitLevelData(cx, J_FARG(4), 3, diffuse) );

	PTYPE specular[3];
	J_CHK( InitLevelData(cx, J_FARG(5), 3, specular) );
	bool specularTexture = false;
	
	float bumpPower; // (TBD) default value
	if ( argc >= 6 )
		J_JSVAL_TO_REAL( J_FARG(6), bumpPower );
	else
		bumpPower = 1;

	float specularPower; // (TBD) default value
	if ( argc >= 7 )
		J_JSVAL_TO_REAL( J_FARG(7), specularPower );
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


/**doc
 * $THIS $INAME( x0, y0, x1, y1 )
  Remove the part of the texture that is outside the rectangle (x1,y1)-(x2,y2).
**/
DEFINE_FUNCTION_FAST( Trim ) { // (TBD) test this new version that use memcpy

	J_S_ASSERT_ARG_MIN( 4 );
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	int x0, y0, x1, y1;
	J_JSVAL_TO_INT32( J_FARG(1), x0 );
	J_JSVAL_TO_INT32( J_FARG(2), y0 );
	J_JSVAL_TO_INT32( J_FARG(3), x1 );
	J_JSVAL_TO_INT32( J_FARG(4), y1 );

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


/**doc
 * $THIS $INAME( sourceTexture, x, y [, borderMode = Texture.borderClamp] )
  Copy _sourceTexture_ in the current texture at the position (_x_, _y_).
  $H arguments
   $ARG Texture sourceTexture:
   $ARG integer x:
   $ARG integer y:
   $ARG enum borderMode: one of Texture.borderWrap or Texture.borderClamp.
**/
DEFINE_FUNCTION_FAST( Copy ) {

	J_S_ASSERT_ARG_MIN( 3 );

	Texture *srcTex, *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	J_CHK( ValueToTexture(cx, J_FARG(1), &srcTex) );

	J_S_ASSERT( tex->channels == srcTex->channels, "Invalid channel count." );

	int px, py; // position
	J_JSVAL_TO_INT32( J_FARG(2), px );
	J_JSVAL_TO_INT32( J_FARG(3), py );

	BorderMode borderMode; // (TBD) from function arg
	if ( J_FARG_ISDEF(4) ) {
		
		int tmp;
		J_JSVAL_TO_INT32( J_FARG(4), tmp );
		borderMode = (BorderMode)tmp;
	} else
		borderMode = borderClamp;

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
			default:
				J_REPORT_ERROR( "Invalid border mode." );
			}

			posDst = ( x + y * texWidth ) * channels;
			posSrc = ( sx + sy * srcTexWidth ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[posDst+c] = srcTex->cbuffer[posSrc+c];
		}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


/**doc
 * $THIS $INAME( texture, x, y, borderMode )
  Paste _sourceTexture_ in the current texture at the position (_x_, _y_).
  $H arguments
   $ARG Texture sourceTexture:
   $ARG integer x:
   $ARG integer y:
   $ARG enum borderMode: one of Texture.borderWrap or Texture.borderClamp.
**/
DEFINE_FUNCTION_FAST( Paste ) { // (Texture)texture, (int)x, (int)y, (bool)borderMode

	J_S_ASSERT_ARG_MIN( 4 );

	Texture *tex1, *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	J_CHK( ValueToTexture(cx, J_FARG(1), &tex1) );
	J_S_ASSERT( tex->channels == tex1->channels, "Invalid channel count." );

	int px, py; // position
	J_JSVAL_TO_INT32( J_FARG(2), px );
	J_JSVAL_TO_INT32( J_FARG(3), py );

	BorderMode borderMode;
	if ( J_FARG_ISDEF(4) ) {
		
		int tmp;
		J_JSVAL_TO_INT32( J_FARG(4), tmp );
		borderMode = (BorderMode)tmp;
	} else
		borderMode = borderClamp;

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
			default:
				J_REPORT_ERROR( "Invalid border mode." );
			}

			posDst = ( dx + dy * texWidth ) * channels;
			posSrc = ( x + y * tex1Width ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[posDst+c] = tex1->cbuffer[posSrc+c];
		}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


/**doc
 * $TYPE ImageObject $INAME( [x, y, width, height] )
  Creates an image object from the whole or a part of current texture.
  $H arguments
   $ARG integer x:
   $ARG integer y:
   $ARG integer width:
   $ARG integer height:
  $H return value
   returns an image object.
  $H example
  {{{
  var f = new Font('arial.ttf');
  f.size = 100;
  f.verticalPadding = -16;
  var img = f.DrawString('Hello world', true);
  
  var t = new Texture(img);
  var t1 = new Texture(t);

  t.BoxBlur(10,10);
  t1.OppositeLevels();
  t.Add(t1);
  t.OppositeLevels();
  t.Add(1);
  
  new File('text.png').content = EncodePngImage(t.Export());  
  }}}
**/
DEFINE_FUNCTION_FAST( Export ) { // (int)x, (int)y, (int)width, (int)height. Returns a Blob

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
				
				buffer[posDst+c] = (u_int8_t)(MINMAX(tex->cbuffer[posSrc+c] * 256.f, 0, 255));

				//buffer[posDst+c] = (u_int8_t)(PNORM(tex->cbuffer[posSrc+c]) * 256);
			}
		}

	jsval bstr;
	J_CHK( J_NewBlob(cx, buffer, bufferLength, &bstr ) );
	JSObject *bstrObj;
	J_CHK( JS_ValueToObject(cx, bstr, &bstrObj) );
	*J_FRVAL = OBJECT_TO_JSVAL( bstrObj );
	
	SetPropertyInt(cx, bstrObj, "width", dWidth);
	SetPropertyInt(cx, bstrObj, "height", dHeight);
	SetPropertyInt(cx, bstrObj, "channels", sChannels);

	return JS_TRUE;
}



/**doc
 * $THIS $INAME( sourceImage, x, y [, borderMode] )
  Draws the _image_ over the current texture at position (_x_, _y_).
  $H arguments
   $ARG ImageObject sourceImage:
   $ARG integer x: X position of the image in the current texture.
   $ARG integer y: Y position of the image in the current texture.
   $ARG enum borderMode: one of Texture.borderWrap, Texture.borderClamp.
  $H example
  {{{
  var file = new File('myImage.png').Open('r'); // note: Open() returns the file object.
  var image = DecodePngImage( file );
  file.Close();
  texture.Import( image, 0, 0 );
  
  Ogl.MatrixMode(MODELVIEW);
  Ogl.DefineTextureImage(TEXTURE_2D, undefined, texture);
  Ogl.LoadIdentity();
  ...
  }}}
**/
DEFINE_FUNCTION_FAST( Import ) { // (Blob)image, (int)x, (int)y

	J_S_ASSERT_ARG_MIN(1);

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	
	J_S_ASSERT_OBJECT( J_FARG(1) );
	JSObject *bstr = JSVAL_TO_OBJECT( J_FARG(1) );
	J_S_ASSERT_CLASS( bstr, BlobJSClass(cx) );

	int px, py;
	J_JSVAL_TO_INT32( J_FARG(2), px );
	J_JSVAL_TO_INT32( J_FARG(3), py );

	BorderMode borderMode;
	if ( J_FARG_ISDEF(4) ) {
		
		int tmp;
		J_JSVAL_TO_INT32( J_FARG(4), tmp );
		borderMode = (BorderMode)tmp;
	} else
		borderMode = borderClamp;


	int dWidth = tex->width;
	int dHeight = tex->height;
	int dChannels = tex->channels;
	
	int sWidth, sHeight, sChannels;
	GetPropertyInt(cx, bstr, "width", &sWidth);
	GetPropertyInt(cx, bstr, "height", &sHeight);
	GetPropertyInt(cx, bstr, "channels", &sChannels);

	//u_int8_t *buffer = (u_int8_t*)BlobData(cx, bstr);

	const u_int8_t *buffer;
//	J_CHK( BlobBuffer(cx, bstr, (const void **)&buffer) );

	J_CHK( JsvalToString(cx, &J_FARG(1), (const char **)&buffer) );

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

			default:
				J_REPORT_ERROR( "Invalid border mode." );
			}

			posDst = ( dx + dy * dWidth ) * dChannels;
			posSrc = ( x + y * sWidth ) * sChannels;
			for ( c = 0; c < sChannels; c++ )
				tex->cbuffer[posDst+c] = buffer[posSrc+c] / (PTYPE)256.f;
		}

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


/**doc
 * $THIS $INAME( offsetX, offsetY [, borderMode] )
  Shift the current image.
  $H arguments
   $ARG integer offsetX: 
   $ARG integer offsetY:
   $ARG enum borderMode: one of Texture.borderWrap, Texture.borderClamp, Texture.borderMirror or Texture.borderValue.
**/
DEFINE_FUNCTION_FAST( Shift ) {
	// (TBD) I think it is possible to do the Shift operation without using a second buffer.

	J_S_ASSERT_ARG_MIN( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	int offsetX, offsetY;
	J_JSVAL_TO_INT32( J_FARG(1), offsetX );
	J_JSVAL_TO_INT32( J_FARG(2), offsetY );

	BorderMode borderMode;
	if ( J_FARG_ISDEF(3) ) {
		
		int tmp;
		J_JSVAL_TO_INT32( J_FARG(3), tmp );
		borderMode = (BorderMode)tmp;
	} else
		borderMode = borderClamp;

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

			switch (borderMode) {
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

				default:
					J_REPORT_ERROR( "Invalid border mode." );
			}
			for ( c = 0; c < channels; c++ ) 
				tex->cbackBuffer[(x + y * width) * channels + c] = tex->cbuffer[(sx + sy * width) * channels + c];
		}
	TextureSwapBuffers(tex);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

/**doc
 * $THIS $INAME( displaceTexture, factor [, borderMode] )
  Move each pixel of the texture according to the 
  $H arguments
   $ARG Texture displaceTexture: is a texture that contains displacement vectors.
   $ARG real factor: displacement factor of each pixel.
   $ARG enum borderMode: one of Texture.borderWrap, Texture.borderClamp, Texture.borderMirror or Texture.borderValue.
**/
// (TBD) PTYPE
DEFINE_FUNCTION_FAST( Displace ) {

	J_S_ASSERT_ARG_MIN( 1 );

	Texture *tex1, *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	J_CHK( ValueToTexture(cx, J_FARG(1), &tex1) );

	int displaceChannels = tex1->channels;

	J_S_ASSERT( width == tex1->width && height == tex1->height, "Textures must have the same size." );
	J_S_ASSERT( tex1->channels >= 2, "Displacement texture must have 2 or more channels." );

	float factor;
	if ( argc >= 2 )
		J_JSVAL_TO_REAL( J_FARG(2), factor );
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
				default:
					J_REPORT_ERROR( "Invalid border mode." );
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


/**doc
 * $THIS $INAME( density, regularity )
  Draws cells in the current texture.
  $H arguments
   $ARG integer density: 
   $ARG real regularity: 
**/
DEFINE_FUNCTION_FAST( Cells ) { // source: FxGen

	J_S_ASSERT_ARG_MIN( 2 );

	int density;
	float regularity;
	J_JSVAL_TO_INT32( J_FARG(1), density );
	J_JSVAL_TO_REAL( J_FARG(2), regularity );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

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


/**doc
 * $THIS $INAME( topLeft, topRight, bottomLeft, bottomRight )
  Add a quad radiant to the current texture
  $H arguments
   $ARG colorInfo topLeft: color of the top-left corner.
   $ARG colorInfo topRight: color of the top-right corner.
   $ARG colorInfo bottomLeft: color of the bottom-left corner.
   $ARG colorInfo bottomRight: color of the bottom-right corner.
  $H example
  {{{
  const RED = [1,0,0,1];
  const BLUE = [0,0,1,1];
  const BLACK = [0,0,0,1];
  
  texture.Set(0); // clears the texture
  texture.AddGradiantQuad(BLACK, RED, BLUE, BLACK);
  }}}
**/
DEFINE_FUNCTION_FAST( AddGradiantQuad ) {

	J_S_ASSERT_ARG_MIN( 4 );
	
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;


	PTYPE pixel1[PMAXCHANNELS];
	J_CHK( InitLevelData(cx, J_FARG(1), channels, pixel1) );

	PTYPE pixel2[PMAXCHANNELS];
	J_CHK( InitLevelData(cx, J_FARG(2), channels, pixel2) );

	PTYPE pixel3[PMAXCHANNELS];
	J_CHK( InitLevelData(cx, J_FARG(3), channels, pixel3) );

	PTYPE pixel4[PMAXCHANNELS];
	J_CHK( InitLevelData(cx, J_FARG(4), channels, pixel4) );

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

/**doc
 * $THIS $INAME( curveInfoX, curveInfoY )
  Add a linear radiant using a curve for X and Y. Each point of the curve is the light intensity of a pixel.
  $H arguments
   $ARG curveInfo curveInfoX:
   $ARG curveInfo curveInfoY:
  $H example 1
  {{{
  const curveHalfSine = function(v) Math.cos(v*Math.PI/2);
  const curveOne = function() 1;
  
  texture.Set(0); // clears the texture
  texture.AddGradiantLinear(curveHalfSine, curveOne);
  }}}
  $H example 2
  {{{
  texture.AddGradiantLinear([0,1,0], [0,1,0]);
  }}}
**/
DEFINE_FUNCTION_FAST( AddGradiantLinear ) {

	J_S_ASSERT_ARG_MIN( 2 );
	
	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	float *curvex = (float*)malloc( width * sizeof(float) ); // (TBD) free curvex
	J_CHK( InitCurveData( cx, J_FARG(1), width, curvex ) );

	float *curvey = (float*)malloc( height * sizeof(float) ); // (TBD) free curvey
	J_CHK( InitCurveData( cx, J_FARG(2), height, curvey ) );

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


/**doc
 * $THIS $INAME( curveInfo [, drawToCorner = false] )
  Add a radial radiant using a curve from the center to the outside. Each point of the curve is the light intensity of a pixel.
  $H arguments
   $ARG curveInfo curveInfo:
   $ARG boolean drawToCorner: If true, the curve goes from the center to the corner, else from the center to the edge.
  $H example 1
  {{{
  const curveHalfSine = function(v) Math.cos(v*Math.PI/2);
  
  texture.AddGradiantRadial(curveHalfSine);
  }}}
  $H example 2
  {{{
  texture.AddGradiantRadial([0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1], true);
  }}}
**/
DEFINE_FUNCTION_FAST( AddGradiantRadial ) {

	J_S_ASSERT_ARG_MIN( 1 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);
	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	bool drawToCorner;
	if ( J_FARG_ISDEF(2) )
		J_JSVAL_TO_BOOL( J_FARG(2), drawToCorner );
	else
		drawToCorner = false;

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

	J_S_ASSERT_ARG_MIN( 3 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(tex);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	int ox, oy;
	J_JSVAL_TO_INT32( J_FARG(1), ox );
	J_JSVAL_TO_INT32( J_FARG(2), oy );

	int radius;
	J_JSVAL_TO_INT32( J_FARG(3), radius );

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


/**doc
 * $THIS $INAME( count, crackLength, wayVariation [, color = 1] [, curve = 1] )
  Adds cracks to the current texture.
  $H arguments
   $ARG integer count: number of cracks to draw.
   $ARG integer crackLength: length of each crack.
   $ARG real wayVariation: the variation of each crack (eg. 0 is straight and 1 is randomly curved).
   $ARG colorInfo color: the color of the crack.
   $ARG curveInfo curve: the curve that defines the intensity of each point of the crack.
  $H note
   The curve is computed before each crack is drawn.
  $H examples
  {{{
  texture.AddCracks( 1000, 10, 0, RED, 1 );
  
  texture.AddCracks( 100, 100, 0, 1, [1,0,1,0,1,0,1,0,1] );

  texture.AddCracks( 10, 10000, 0.1, 1, curveLinear );

  texture.AddCracks( 10, 10000, 0.1, 1, function(v) Texture.RandReal() );
  }}}
**/
DEFINE_FUNCTION_FAST( AddCracks ) { // source: FxGen

	J_S_ASSERT_ARG_MIN( 2 );

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	int count;
	J_JSVAL_TO_INT32( J_FARG(1), count );

	int crackMaxLength;
	J_JSVAL_TO_INT32( J_FARG(2), crackMaxLength );
	
	float variation;
	J_JSVAL_TO_REAL( J_FARG(3), variation );

	PTYPE pixel[PMAXCHANNELS];
	if ( J_FARG_ISDEF(4) ) {
		
		J_CHK( InitLevelData(cx, J_FARG(4), channels, pixel) );
	} else {
		for ( int i=0; i<PMAXCHANNELS; i++ )
			pixel[i] = PMAX;
	}


	float *curve = (float*)malloc( crackMaxLength * sizeof(float) );
	if ( J_FARG_ISDEF(5) ) {

		J_CHK( InitCurveData( cx, J_FARG(5), crackMaxLength, curve ) );
	} else {
		for ( int i=0; i<crackMaxLength; i++ )
			curve[i] = PMAX;
	}

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
	free(curve);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}



/**doc
 * $TYPE Array $INAME( x, y )
  Read the value of a pixel in the current texture.
  $H arguments
   $ARG integer x
   $ARG integer y
  $H return value
   returns the pixel value at position (x, y) in the current texture. If the texture is RGB, an array of 3 values is returned.
  $H example
  {{{
  var texture = new Texture(20,20,3);
  texture.Set([0.1, 0.2, 0.3]);
  var pixel = texture.PixelAt(10,10);
  Print( 'Red: '+pixel[0], 'Green: '+pixel[1], 'Blue: '+pixel[2] );
  }}}
**/
DEFINE_FUNCTION_FAST( PixelAt ) {

	J_S_ASSERT_ARG_MIN(2);
	J_S_ASSERT_INT(J_FARG(1));
	J_S_ASSERT_INT(J_FARG(2));

	int x = JSVAL_TO_INT(J_FARG(1));
	int y = JSVAL_TO_INT(J_FARG(2));

	Texture *tex = (Texture *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tex);

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


/**doc
=== Properties ===
**/


/**doc
 * $REAL $INAME
  Is the higher $pval. Higher values are not brighter.
  = =
  See Normalize() function.
**/
DEFINE_PROPERTY( vmax ) {

	J_CHK( JS_NewNumberValue(cx, PMAX, vp) );
	return JS_TRUE;
}


/**doc
 * $INT $INAME
  Width of the texture in pixel.
**/
DEFINE_PROPERTY( width ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(tex);
	jsdouble d = tex->width;
	J_CHK( JS_NewNumberValue(cx, d, vp) );
	return JS_TRUE;
}


/**doc
 * $INT $INAME
  Height of the texture in pixel.
**/
DEFINE_PROPERTY( height ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(tex);
	jsdouble d = tex->height;
	J_CHK( JS_NewNumberValue(cx, d, vp) );
	return JS_TRUE;
}


/**doc
 * $INT $INAME
  Number of channels of the texture.
**/
DEFINE_PROPERTY( channels ) {

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(tex);
	jsdouble d = tex->channels;
	J_CHK( JS_NewNumberValue(cx, d, vp) );
	return JS_TRUE;
}


/**doc
=== Static functions ===
**/

/**doc
 * $VOID $INAME( seed )
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

	J_S_ASSERT_ARG_MIN(1);
	unsigned long seed;
	J_JSVAL_TO_UINT32(J_FARG(1), seed);
	init_genrand(seed);
	return JS_TRUE;
}


/**doc
 * $INT $INAME
  Generates a random number on [0,0x7fffffff]-interval
**/
DEFINE_FUNCTION_FAST( RandInt ) {

	int i = genrand_int31();
	*J_FRVAL = INT_TO_JSVAL(i);
	return JS_TRUE;
}


/**doc
 * $REAL $INAME
  Generates a random number on [0,1]-real-interval
**/
DEFINE_FUNCTION_FAST( RandReal ) {

	jsdouble d = genrand_real1();
	J_CHK( JS_NewNumberValue(cx, d, J_FRVAL) );
	return JS_TRUE;
}


//DEFINE_FUNCTION( Noise ) {
//
//	J_S_ASSERT_ARG_MIN(1);
//	unsigned long seed;
//	J_JSVAL_TO_UINT32(J_FARG(1), seed);
//	jsdouble d = NoiseInt(seed);
//	J_CHK( JS_NewNumberValue(cx, d, rval) );
//	return JS_TRUE;
//}



#ifdef _DEBUG
static JSBool _Test(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_FALSE;
}
#endif // _DEBUG


/**doc
=== Note ===
 Nearly all methods returns _this_ object. This allows to easily chain texture operations.
 $H example
 {{{
 var texture = new Texture(64,64,3);
 texture.Set(0).AddNoise().AddCracks(10, 100, 0, BLUE, 1);
 }}}

=== Definitions ===
 * *$pval*
  The $pval is the value of a channel of a pixel.
  In a RGB texture, each pixel has 3 components (Red, Green and Blue) and each one represents a light intensity.

=== Special values and data types ===

 * *borderMode*
  * Texture.borderWrap: get the pixel from the the other side (opposite edge).
  * Texture.borderClamp: do not take any other pixel.
  * Texture.borderMirror: take the pixel from this edge but in a reverse way (like a mirror).
  * Texture.borderValue: use the pixel of the current border.

 * *colorInfo*
  a _colorInfo_ can be one of the following type:
  * _real_: in this case, the same value is used for each channel (eg. 0.5 gor gray).
  * _Array_: that contains the $pval of each channel (eg. [1,1,1] is white in RGB mode).
  * _string_: the string represents an HTML color like #1100AA or #8800AAFF (depending the number of channels)
  $H examples
  {{{
  const RED = [1,0,0,1];
  const GREEN = [0,1,0,1];
  const BLUE = [0,0,1,1];
  const MAGENTA = [1,0,1,1];
  const CYAN = [0,1,1,1];
  const YELLOW = [1,1,0,1];
  const GRAY = [.5,.5,.5,1];
  const BLACK = [0,0,0,1];
  const WHITE = [1,1,1,1];
  }}}

 * *curveInfo*
  _curveInfo_ describes a curve and can be one of the following type:
   * _real_: this describes a constant curve.
   * _function($REAL posX, �INT indexX)_: the function is called and must returns values for each curve point.
   * _Array_: an Array that describes the curve (no interpolation is done between values).
   * _buffer_: a Blob or a string that contains the curve data.
  $H examples
  {{{
  const curveLinear = function(v) { return v }
  const curveHalfSine = function(v) { return Math.cos(v*Math.PI/2) }
  const curveSine = function(v) { return Math.sin(v*Math.PI) }
  const curveInverse = function(v) { return 1/v }
  const curveSquare = function(v) { return v*v }
  const curveDot = function(v,i) { return i%2 }
  const curveZero = function(v,i) { return 0 }
  const curveOne = function(v,i) { return 1 }
  function GaussianCurveGenerator(c) { return function(x) { return Math.exp( -(x*x)/(2*c*c) ) } }
  
  texture.AddGradiantRadial( GaussianCurveGenerator( 0.5 ) );
  }}}

 * *ImageObject*
  An image object is nothing else that a buffer of data with a width, a height and a channels properties.
**/

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
		FUNCTION_FAST( SetPixel )
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

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER( borderClamp, borderClamp )
		CONST_INTEGER( borderWrap, borderWrap )
		CONST_INTEGER( borderMirror, borderMirror )
		CONST_INTEGER( borderValue, borderValue )

		CONST_INTEGER( desaturateLightness, desaturateLightness )
		CONST_INTEGER( desaturateSum, desaturateSum )
		CONST_INTEGER( desaturateAverage, desaturateAverage )
		
	END_CONST_INTEGER_SPEC

	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

END_CLASS

/**doc
=== Examples ===
$H example 1
 Some utility functions.
{{{
function Cloud( size, amp ) {

   var octaves = Math.log(size) / Math.LN2;
   var a = 1, s = 1;
   var cloud = new Texture(s, s, 1);
   cloud.ClearChannel();
   while ( octaves-- > 0 ) {
      
      cloud.AddNoise(a);
      a *= amp;
      s *= 2;
      cloud.Resize(s, s, false);
      cloud.BoxBlur(5, 5);
   }
   cloud.NormalizeLevels();
   return cloud;
}

function DesaturateLuminosity( tex ) {
   
   tex.Mult([0.2126, 0.7152, 0.0722]);
   var tmp = new Texture(tex.width, tex.height, 1).Desaturate(tex, Texture.desaturateSum);
   tex.Swap(tmp);
   tmp.Free();
}

function AddAlphaChannel( tex ) {

   if ( tex.channels == 1 )
      new Texture(tex.width, tex.height, 2).SetChannel(0, tex, 0).Swap(tex);
   else if ( tex.channels == 3 )
      new Texture(tex.width, tex.height, 4).SetChannel(0, tex, 0).SetChannel(1, tex, 1).SetChannel(2, tex, 2).Swap(tex);
}
}}}

$H example 3
 This example shows how to save a texture to the disk.
{{{
LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsprotex');
LoadModule('jsimage');

var bacteria = new Texture(256,256,3);
bacteria.Set(0);
bacteria.AddCracks(100, 100, 2, undefined, function(v) v);
bacteria.BoxBlur(5,5);
bacteria.MirrorLevels(0.5, false);
bacteria.BoxBlur(2,2);

new File('test.png').content = EncodePngImage(bacteria.Export());
}}}

$H example 3
 This is a complete example that displays a texture in real-time in a OpenGL environment.
{{{
LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsgraphics');
LoadModule('jsprotex');
LoadModule('jsimage');

with (Ogl) {
   
   var texture = new Texture(128, 128, 3);
   texture.Set(0);
   // play here for static textures

   function UpdateTexture(imageIndex) {

      // play here for dynamic textures
      texture.Set(0);
      texture.AddNoise(1);
   }

   var win = new Window();
   win.Open();
   win.CreateOpenGLContext();
   win.rect = [200,200,800,800];

   function ResizeWindow(w, h) {

      Ogl.Viewport(0,0,w,h);
      Ogl.MatrixMode(PROJECTION);
      Ogl.LoadIdentity();
      Ogl.Ortho(0,0,10,10, -1, 1);
      Render();
   }

   ShadeModel(FLAT);
   FrontFace(CCW);
   ClearColor(0, 0, 0, 0);
   Enable(TEXTURE_2D);
   tid = GenTexture();
   BindTexture(TEXTURE_2D, tid);
   TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST); // GL_LINEAR
   TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
   Clear( COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT );

   function Render(imageIndex) {

      UpdateTexture();
      DefineTextureImage(TEXTURE_2D, undefined, texture);
      MatrixMode(MODELVIEW);
      LoadIdentity();
      Scale(1, -1, 1);
      Color(1,1,1);
      Begin(QUADS);
      TexCoord( 0, 0 );
      Vertex( -1, -1, 0 );
      TexCoord( 1, 0, 0 );
      Vertex( 1, -1 );
      TexCoord( 1, 1 );
      Vertex( 1, 1 );
      TexCoord( 0, 1 );
      Vertex( -1, 1 );
      End();
      win.SwapBuffers();
      MaybeCollectGarbage();
   }

   win.onsize = ResizeWindow;
   var end = false;
   win.onkeydown = function( key, l ) { end = ( key == 0x1B ) }
   while (!end) {

      win.ProcessEvents();
      Render();
   }
   win.Close();
}
}}}
**/



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
