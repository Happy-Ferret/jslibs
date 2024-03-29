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

#include "texture.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <stdlib.h>
#include <limits.h>
#include <float.h>

#include "vector3.h"
#include "vector4.h"
#include "matrix44.h"


DECLARE_CLASS( Texture )


EXTERN_C double genrand_real1(void); // mt19937ar-cok.c
#define PRAND (PTYPE(genrand_real1()))

double PerlinNoise2(double x, double y, double z);


#define ABS(val) ( (val) < 0 ? -(val) : (val) )

#define MIN(val1, val2) ( (val1) < (val2) ? (val1) : (val2) )

#define MAX(val1, val2) ( (val1) > (val2) ? (val1) : (val2) )

#define MINMAX(val, min, max) ((val) > (max) ? (max) : (val) < (min) ? (min) : (val) )


// min 'invisible' value
#define PMINLIMIT FLT_MIN

// max 'invisible' value
#define PMAXLIMIT FLT_MAX

// min 'visible' value
// #define PMIN (0.f) // IMPORTANT: the lowest visible value is always 0, then this macro is no more used

// max 'visible' value
// with bytes, the range should be [0,255]
// BUT with real, the range should be [0,1] or [0,1) ( use 1.f - FLT_EPSILON )
#define PMAX (PTYPE(1))

// full amplitude
//#define PAMP (PMAX-PMIN)

// middle pixel value (gray)
#define PMID ( PMAX / PTYPE(2) )

// normalize the pixel value to range 0..1
#define PNORM(p) ((p) / PMAX)

// un-normalize the pixel value from range 0..1
#define PUNNORM(p) ((p) * PMAX)

// normalize the pixel value to range -1..1
#define PZNORM(p) (PNORM(p) * PTYPE(2) - PTYPE(1))

// un-normalize the pixel value from range -1..1
#define PUNZNORM(p) ( (PUNNORM(p) + PTYPE(1) ) / PTYPE(2))


#define COLOR_NAME_COMPONENT_COUNT (4)
#define COLOR_NAME_COMPONENT_VALUE_MAX (255)

struct ColorName {
	const char *name;
	uint8_t component[COLOR_NAME_COMPONENT_COUNT];
};

const ColorName colorNameList[] = {
#define DEF(name, r, g, b) {name, r, g, b, 255},
#include "colors.h"
{0}
#undef DEF
};


ALWAYS_INLINE bool FASTCALL
TextureInit( TextureStruct *tex, unsigned int width, unsigned int height, unsigned int channels ) {

	tex->cbufferSize = width * height * channels * sizeof(PTYPE);
	tex->cbuffer = (PTYPE*)jl_malloc(tex->cbufferSize);
	tex->cbackBuffer = NULL; // see TextureSetupBackBuffer()
	tex->width = width;
	tex->height = height;
	tex->channels = channels;
	return true;
}

ALWAYS_INLINE bool FASTCALL
TextureResizeBackBuffer( TextureStruct *tex, size_t newSize ) {
	
	if ( tex->cbackBuffer != NULL && tex->cbackBufferSize == newSize )
		return true;
	if ( tex->cbackBuffer == NULL )
		tex->cbackBuffer = (PTYPE*)jl_malloc(newSize);
	else
		tex->cbackBuffer = (PTYPE*)jl_realloc(tex->cbackBuffer, newSize);
	tex->cbackBufferSize = newSize;
	return true;
}

ALWAYS_INLINE bool FASTCALL
TextureSetupBackBuffer( TextureStruct *tex ) {

	return TextureResizeBackBuffer(tex, tex->cbufferSize);
}

ALWAYS_INLINE void FASTCALL
TextureSwapBuffers( TextureStruct *tex ) {

	PTYPE *tmpBuffer = tex->cbuffer;
	tex->cbuffer = tex->cbackBuffer;
	tex->cbackBuffer = tmpBuffer;

	size_t tmpBufferSize = tex->cbufferSize;
	tex->cbufferSize = tex->cbackBufferSize;
	tex->cbackBufferSize = tmpBufferSize;
}

ALWAYS_INLINE void FASTCALL
TextureFreeBuffers( TextureStruct *tex ) {

	if ( tex->cbackBuffer != NULL ) {

		jl_free(tex->cbackBuffer);
		tex->cbackBuffer = NULL;
	}

	if ( tex->cbuffer != NULL ) {

		jl_free(tex->cbuffer);
		tex->cbuffer = NULL;
	}
}

ALWAYS_INLINE bool FASTCALL
TextureSameFormat( TextureStruct *t1, TextureStruct *t2 ) {

	return t1->width == t2->width && t1->height == t2->height && t1->channels == t2->channels;
}

ALWAYS_INLINE bool FASTCALL
IsTexture( JSContext *cx, jsval val ) {
	
	JL_IGNORE(cx);
	return val.isObject() && JL_GetClass(&val.toObject()) == JL_CLASS(Texture);
}


ALWAYS_INLINE bool FASTCALL
ValueToTexture( JSContext* cx, jsval value, TextureStruct **tex ) {

	JL_ASSERT( IsTexture(cx, value), E_VALUE, E_TYPE, E_NAME(JL_CLASS_NAME(Texture)) );
	*tex = (TextureStruct*)JL_GetPrivate(&value.toObject());
	JL_ASSERT_OBJECT_STATE(tex, JL_GetClassName(&value.toObject()) );
	return true;
	JL_BAD;
}


ALWAYS_INLINE float FASTCALL
Length2D( float a, float b ) {

	return sqrtf(a*a + b*b);
}

enum DesaturateMode {
	desaturateLightness,
	desaturateSum,
	desaturateAverage
};

enum BorderMode {
	borderClamp,
	borderWrap,
	borderMirror,
	borderValue,
	borderZero
};

ALWAYS_INLINE unsigned int FASTCALL
Wrap( int value, int limit ) {

	if ( value >= limit )
		return value % limit;
	if ( value < 0 )
		return limit - (-value) % limit;
	return value;
}

ALWAYS_INLINE unsigned int FASTCALL
Mirror( int value, int limit ) { // (TBD) manage if value == 2*limit (use modulo)
	
	if ( value >= limit )
		return 2 * limit - value - 2;
	if ( value < 0 )
		return -value;
	return value;
}

ALWAYS_INLINE PTYPE* FASTCALL
PosByMode( const TextureStruct *tex, int x, int y, BorderMode mode ) {

	switch ( mode ) {
		case borderClamp:
			if ( x < 0 || x >= (int)tex->width || y < 0 || y >= (int)tex->height )
				return NULL; // skip
			break;
		case borderWrap:
			x = Wrap(x, tex->width);
			y = Wrap(y, tex->height);
			break;
		case borderMirror:
			x = Mirror(x, tex->width);
			y = Mirror(y, tex->height);
			break;
		case borderValue:
			if ( x < 0 )
				x = 0;
			else
			if ( x >= (int)tex->width )
				x = (int)tex->width - 1;
			
			if ( y < 0 )
				y = 0;
			else
			if ( y >= (int)tex->height )
				y = (int)tex->height - 1;
			break;
		default:
			return NULL; // skip
	}
	return &tex->cbuffer[ ( x + y * tex->width ) * tex->channels ];
}


ALWAYS_INLINE bool FASTCALL
JL_JsvalToBorderMode( JSContext* cx, jsval val, BorderMode *mode ) {
	
	if ( val != JSVAL_VOID )
		return jl::getValue(cx, val, (int*)mode);
	*mode = borderWrap;
	return true;
}


// levels: number | array | string ('#8800AAFF' | 'DarkMagenta')
bool FASTCALL
InitLevelData( JSContext* cx, jsval value, unsigned int levelMaxLength, PTYPE *level ) {
	
	unsigned int i;

	if ( value.isNumber() ) {

		PTYPE val;
		JL_CHK( jl::getValue(cx, value, &val) );
		for ( i = 0; i < levelMaxLength; i++ )
			level[i] = val;
		return true;
	}

	if ( value.isString() ) {

		JLData colorStr;
		const char *color;
		size_t length;
		JL_CHK( jl::getValue(cx, value, &colorStr) );
		length = colorStr.Length();
		color = colorStr.GetConstStr();

		if ( *color == '#' && (length-1) / 2 >= levelMaxLength ) {
			
			++color;
			unsigned char val;
			val = 0;
			for ( i = 0; i < levelMaxLength; i++ ) {

				if ( *color >= '0' && *color <= '9' ) val = *color - '0';
				else if ( *color >= 'A' && *color <= 'F' ) val = *color - 'A' + 10;
				else if ( *color >= 'a' && *color <= 'f' ) val = *color - 'a' + 10;
				color++;
				val <<= 4;
				if ( *color >= '0' && *color <= '9' ) val |= *color - '0';
				else if ( *color >= 'A' && *color <= 'F' ) val |= *color - 'A' + 10;
				else if ( *color >= 'a' && *color <= 'f' ) val |= *color - 'a' + 10;
				color++;
				level[i] = PMAX * val / 255.f;
			}
			return true;
		}

		// color names
		// (TBD) optimization through a hash table (or a js object using a lazy property definition)
		for ( const ColorName *n = colorNameList; n->name; ++n ) {

			if ( strnicmp(n->name, color, length) == 0 ) {

				for ( unsigned int i = 0; i < levelMaxLength; ++i ) {

					level[i] = i < COLOR_NAME_COMPONENT_COUNT ? n->component[i]/PTYPE(COLOR_NAME_COMPONENT_VALUE_MAX) : 0;
				}
//				int i = jl::min(levelMaxLength, COLOR_NAME_COMPONENT_COUNT);
//				while ( i-- )
//					level[i] = n->component[i]/(float)COLOR_NAME_COMPONENT_VALUE_MAX;
				return true;
			}
		}
	}

	if ( JL_ValueIsArrayLike(cx, value) ) {

		uint32_t length;
		JL_CHK( jl::getVector(cx, value, level, levelMaxLength, &length) );
		JL_ASSERT( length >= levelMaxLength, E_ARRAYLENGTH, E_MIN, E_NUM(levelMaxLength) );
		return true;
	}

	JL_ERR( E_ARG, E_TYPE, E_TY_NUMBER, E_OR, E_TY_ARRAY, E_OR, E_TY_STRING );
	JL_BAD;
}


// curve: number | function | array | blob
bool FASTCALL
InitCurveData( IN JSContext* cx, IN jsval value, IN size_t length, OUT float *curve ) { // length is the curve resolution
	
	size_t i;

	if ( JL_ValueIsCallable(cx, value) ) {

		double fval;
		jsval argv[3]; // argv[0] is the rval
		for ( i = 0; i < length; ++i ) {

			fval = double(i) / double(length-1);
			argv[1] = DOUBLE_TO_JSVAL(fval);
			argv[2] = INT_TO_JSVAL((int)i);
			JL_CHK( JS_CallFunctionValue(cx, JL_GetGlobal(cx), value, 2, argv+1, argv) );
			JL_CHK( jl::getValue(cx, argv[0], &fval) );
			curve[i] = PTYPE(fval);
		}
		return true;
	}

	if ( JL_ValueIsArray(cx, value) ) { // do not manage array-like here since string are handeled below.

		unsigned curveArrayLength;
		JL_CHK( JS_GetArrayLength(cx, &value.toObject(), &curveArrayLength) );
		JL_ASSERT( curveArrayLength >= 1, E_ARRAYLENGTH, E_MIN, E_NUM(1) );
		PTYPE *curveArray;
		curveArray = (PTYPE*)alloca(curveArrayLength * sizeof(PTYPE));
		uint32_t tmp;
		JL_CHK( jl::getVector(cx, value, curveArray, curveArrayLength, &tmp) );
		for ( i = 0; i < length; ++i )
			curve[i] = curveArray[i * curveArrayLength / length]; // no interpolation
		return true;
	}

	if ( value.isNumber() ) {

		double dval;
		JL_CHK( jl::getValue(cx, value, &dval) );
		PTYPE val;
		val = PTYPE(dval);
		for ( i = 0; i < length; ++i )
			curve[i] = val;
		return true;
	}

	if ( JL_ValueIsData(cx, value) ) {

		JLData curveData;

		JSObject *bstrObj;
		bstrObj = &value.toObject();
		size_t bstrLen;
		const uint8_t *bstrData;
//		JL_CHK( JL_JsvalToStringAndLength( cx, &value, (const char **)&bstrData, &bstrLen ) );
		JL_CHK( jl::getValue(cx, value, &curveData) );
		bstrLen = curveData.Length();
		bstrData = (const uint8_t *)curveData.GetConstStr();

		for ( i = 0; i < length; ++i )
			curve[i] = (PTYPE)bstrData[i * bstrLen / length] / 255.f; // (TBD) check
		return true;
	}

	//  // (TBD) throws an error ?
	//for ( int i = 0; i < length; i++ )
	//	curve[i] = PMAX;
	//return true;

	JL_ERR( E_ARG, E_TYPE, E_TY_FUNC, E_OR, E_TY_NUMBER, E_OR, E_TY_DATA );
	JL_BAD;
}




/**doc
$SET pval pixel component intensity value
**/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/

BEGIN_CLASS( Texture )

bool NativeInterfaceBufferGet( JSContext *cx, JSObject *obj, JLData *str ) {

	JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( tex );
//	*buf = (char*)tex->cbuffer;
//	*size = tex->width * tex->height * tex->channels * sizeof(PTYPE);
	*str = JLData((const void *)tex->cbuffer,  tex->width * tex->height * tex->channels * sizeof(PTYPE));
	return true;
	JL_BAD;
}


DEFINE_FINALIZE() {

//	if ( obj == JL_THIS_CLASS_PROTOTYPE )
//		return;

	if ( jl::Host::getJLHost(fop->runtime())->canSkipCleanup ) // do not cleanup in unsafe mode.  // see HostRuntime::skipCleanup()
		return;

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	if ( !tex )
		return;

	TextureFreeBuffers(tex);
	JL_freeop(fop, tex);
}


/**doc
$TOC_MEMBER $INAME
 $INAME( width, height, channels )
 $INAME( sourceTexture )
 $INAME( image )
  Creates a new Texture object.
  $H arguments
   $ARG $INT height: height of texture.
   $ARG $INT width: width of texture.
   $ARG $INT channels: number of channels of the texture (current limit is 4). Channel has a meaning only in a few part of the API like ToHLS(), ToRGB(), ...
   $ARG Texture sourceTexture: an existing Texture object (acs like a copy constructor).
   $ARG ImageObject image: an existing Image object (From a jpeg image for example)
  $H note
   jsprotex uses single precision values per channel. The visibles values are in range [ 0,1 ].
   The darker value is 0.0 and the brighter value is 1.0.
  $H example
  {{{
  loadModule('jsstd');
  loadModule('jsimage');

  var image = decodePngImage( new File('picture.png').open("r") );
  var tex = new Texture(image);
  tex.normalizeLevels();
  }}}
**/
DEFINE_CONSTRUCTOR() {

	JL_DEFINE_ARGS;

	TextureStruct *tex = NULL;

	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_RANGE(1,3);
	JL_ASSERT_CONSTRUCTING();

	tex = (TextureStruct*)JS_malloc(cx, sizeof(TextureStruct));
	JL_CHK(tex);
	tex->cbackBuffer = NULL;
	tex->cbuffer = NULL;

	JL_CHK( SetBufferGetInterface(cx, obj, NativeInterfaceBufferGet) );

	if ( JL_ARGC >= 3 ) {

		unsigned int width, height, channels;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &width) );
		JL_CHK( jl::getValue(cx, JL_ARG(2), &height) );
		JL_CHK( jl::getValue(cx, JL_ARG(3), &channels) );

		JL_ASSERT( width > 0, E_ARG, E_NUM(1), E_MIN, E_NUM(1) );
		JL_ASSERT( height > 0, E_ARG, E_NUM(2), E_MIN, E_NUM(1) );
		JL_ASSERT_ARG_VAL_RANGE(channels, 1, PMAXCHANNELS, 1);

		JL_CHK( TextureInit(tex, width, height, channels) );

		JL_SetPrivate(obj, tex);
		return true;
	}

	if ( IsTexture(cx, JL_ARG(1)) ) { // copy constructor

		TextureStruct *srcTex = (TextureStruct *)JL_GetPrivate(&JL_ARG(1).toObject());
		JL_ASSERT_OBJECT_STATE(srcTex, JL_GetClassName(&JL_ARG(1).toObject()) );

		JL_CHK( TextureInit(tex, srcTex->width, srcTex->height, srcTex->channels) );
		jl::memcpy( tex->cbuffer, srcTex->cbuffer, srcTex->width * srcTex->height * srcTex->channels * sizeof(PTYPE) );

		JL_SetPrivate(obj, tex);
		return true;
	}

	{
		int i, tsize, sWidth, sHeight, sChannels;
		ImageDataType dataType;
		JLData data = JL_GetImageObject(cx, JL_ARG(1), &sWidth, &sHeight, &sChannels, &dataType);
		if ( data.IsSet() ) {

			JL_ASSERT( dataType == TYPE_UINT8, E_ARG, E_NUM(1), E_DATATYPE, E_INVALID );
			tsize = data.Length();
			const uint8_t *buffer = (const uint8_t*)data.GetConstStr();
			JL_CHK( TextureInit(tex, sWidth, sHeight, sChannels) );
			for ( i = 0; i < tsize; ++i )
				tex->cbuffer[i] = (PTYPE)buffer[i] / (PTYPE)255.f; // map [0 -> 255] to [0.0 -> 1.0]

			JL_SetPrivate(obj, tex);
			return true;
		}
	}

	JL_ERR( E_ARG, E_INVALID );

bad:
	if ( tex ) {

		TextureFreeBuffers(tex);
		JS_free(cx, tex);
	}
	return false;
}

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Free the memory allocated by the current texture.
  This is not mandatory but may be useful to free memory before the GC did.
**/
DEFINE_FUNCTION( free ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex;
	tex = (TextureStruct*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	TextureFreeBuffers(tex);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( otherTexture )
  Swaps the content of two textures the current one and _otherTexture_.
  $H arguments
   $ARG Texture otherTexture: texture object against wich the exchange is done.
  $H example
  {{{
  function addAlphaChannel( tex ) {

    if ( tex.channels == 1 )
      new Texture(tex.width, tex.height, 2).setChannel(0, tex, 0).swap(tex);
    else if ( tex.channels == 3 )
      new Texture(tex.width, tex.height, 4).setChannel(0, tex, 0).setChannel(1, tex, 1).setChannel(2, tex, 2).swap(tex);
  }
  }}}
**/
// PTYPE ok
DEFINE_FUNCTION( swap ) {
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	JSObject *texObj;
	texObj = &JL_ARG(1).toObject();
	JL_ASSERT_INSTANCE( texObj, JL_THIS_CLASS );
	void *tmp;
	tmp = JL_GetPrivate(obj);
	JL_SetPrivate( obj, JL_GetPrivate(texObj));
	JL_SetPrivate( texObj, tmp);
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( [ channel ] )
  Clears (set to 0) the given _channel_ or all channels if the method is called without argument.
**/
// PTYPE ok
DEFINE_FUNCTION( clearChannel ) {
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	if ( argc == 0 ) { // clear all channels

		memset(tex->cbuffer, 0, tex->width * tex->height * tex->channels * sizeof(PTYPE));
	} else
	if ( argc >= 1 ) {

		unsigned int channel;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &channel) );
		JL_ASSERT( channel < tex->channels, E_ARG, E_NUM(1), E_MAX, E_NUM( tex->channels-1 ) );

		PTYPE *ptr;
		ptr = tex->cbuffer;
		ptr += channel;

		int tsize;
		tsize = tex->width * tex->height;
		int channels;
		channels = tex->channels;
		for ( int i = 0; i < tsize; i++ ) {

			*ptr = 0;
			ptr += channels;
		}
	}
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( destinationChannel, otherTexture, sourceChannel )
  Replace the _destinationChannel_ channel of the current texture with the _sourceChannel_ channel of the _otherTexture_.
  $H arguments
   $ARG $INT destinationChannel: a channel of the current texture.
   $ARG Texture otherTexture: the texture from wich a channel will be imported.
   $ARG $INT destinationChannel: the channel of the _otherTexture_ to be imported.
  $H example
  {{{
  function noiseChannel( tex, channel ) {

    var tmp = new Texture(tex.width, tex.height, 1);
    tmp.addNoise();
    tex.setChannel(channel, tmp, 0);
    tmp.free();
  }
  }}}
**/
// PTYPE ok
DEFINE_FUNCTION( setChannel ) {
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 3 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	int dstChannel;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &dstChannel) );

	TextureStruct *tex1;
	JL_CHK( ValueToTexture(cx, JL_ARG(2), &tex1) );

	int srcChannel;
	JL_CHK( jl::getValue(cx, JL_ARG(3), &srcChannel) );

	int texChannel, tex1Channel;
	texChannel = tex->channels;
	tex1Channel = tex1->channels;

	//JL_ASSERT( tex->width == tex1->width && tex->height == tex1->height, "Images must have the same size.");

	JL_ASSERT( tex->width == tex1->width && tex->height == tex1->height, E_ARG, E_NUM(2), E_FORMAT );
	JL_ASSERT( srcChannel < tex1Channel, E_ARG, E_NUM(3), E_MAX, E_NUM( tex1Channel-1 ) );
	JL_ASSERT( dstChannel < texChannel, E_ARG, E_NUM(1), E_MAX, E_NUM( texChannel-1 ) );

	unsigned int pos, pos1;
	pos = dstChannel;
	pos1 = srcChannel;

	int size;
	size = tex->width * tex->height;
	for ( int i = 0; i < size; i++ ) {
	
		tex->cbuffer[pos] = tex1->cbuffer[pos1];
		pos += texChannel;
		pos1 += tex1Channel;
	}

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  Does a conversion from RGB (Red, Green, Blue) to HSV (Hue, Saturation, Value) colorspace.
**/
// (TBD) PTYPE
DEFINE_FUNCTION( toHLS ) { // (TBD) test it

	// see http://svn.gnome.org/viewcvs/gimp/trunk/libgimpcolor/gimpcolorspace.c?view=markup

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	int channels;
	channels = tex->channels;
	
	JL_ASSERT( channels >= 3, E_THISOBJ, E_FORMAT );

	int size;
	size = tex->width * tex->height;
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

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}

// (TBD) PTYPE
ALWAYS_INLINE float FASTCALL
HLSToRGB_hue( float n1, float n2, float hue) { // helper function for the HLS->RGB conversion

	if (hue > 1) hue -= 1;
	if (hue < 0) hue += 1;
	if (hue < 1.f/6.f) return( n1 + (n2 - n1)*hue/(1.f/6.f) );
	if (hue < (1.f/2.f)) return( n2 );
	if (hue < (4.f/6.f)) return( n1 + (n2 - n1)*((4.f/6.f) - hue)/(1.f/6.f) );
	return( n1 );
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  Does a conversion from HSV (Hue, Saturation, Value) to RGB (Red, Green, Blue) colorspace.
**/
// (TBD) PTYPE
DEFINE_FUNCTION( toRGB ) { // (TBD) test it
	// see http://svn.gnome.org/viewcvs/gimp/trunk/libgimpcolor/gimpcolorspace.c?view=markup

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	int channels;
	channels = tex->channels;

	JL_ASSERT( channels >= 3, E_THISOBJ, E_FORMAT );

	int size;
	size = tex->width * tex->height;
	int pos;
	PTYPE R, G, B, H, L, S;
	PTYPE m1, m2;
	for ( int i = 0; i < size; i++ ) {

		pos = i * channels;
		H = tex->cbuffer[pos+0];
		L = tex->cbuffer[pos+1];
		S = tex->cbuffer[pos+2];

		if (L <= PTYPE(0.5))
				m2 = L * ( PTYPE(1) + S );
		else
				m2 = L + S - L * S;
		m1 = PTYPE(2) * L - m2;
		if (S == PTYPE(0)) { // achromatic cast

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

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( count [ , curve] )
  Reduce the number of values used for each channel.
  $H arguments
   $ARG $INT count: the number of different $pval in the resulting texture.
   $ARG curveInfo curve: the transformation curve used for each value. For further information about ,,curveInfo,, , see below.
  $H note
   If _curveInfo_ is not provided, each channel is processed in a linear manner using the following formula:
    floor( count `*` colorValue ) / count
  $H note
   Each channel is processed independently.
  $H example 1
  {{{
  var t = cloud(size, 0.5);
  t.aliasing(2);
  }}}
  $H example 2
  {{{
  const curveLinear = function(v) { return v }
  var t = cloud(size, 0.5);
  t.aliasing(8, curveLinear);
  t.boxBlur(3, 3)
  }}}
**/
// PTYPE ok
DEFINE_FUNCTION( aliasing ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	size_t count;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &count) );
	JL_ASSERT( count >= 1, E_ARG, E_NUM(1), E_MIN, E_NUM(1) );

	bool useCurve;

	float *curve;
	if ( argc >= 2 ) {

		useCurve = true;
		curve = (float*)alloca( count * sizeof(float) );
		JL_CHK( InitCurveData( cx, JL_ARG(2), count, curve ) );
	} else {

		IFDEBUG( curve = NULL ); // avoid "potentially uninitialized local variable" warning
		useCurve = false;
	}

	int tsize;
	tsize = tex->width * tex->height * tex->channels;

	if ( useCurve )
		for ( int i = 0; i < tsize; i++ ) {

			long curveIndex = (long)(count * tex->cbuffer[i] / PMAX);
			tex->cbuffer[i] = curve[MINMAX(curveIndex, 0, (long)count - 1)];
		}
	else
		for ( int i = 0; i < tsize; i++ )
			tex->cbuffer[i] = floor( count * tex->cbuffer[i] ) / count;

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( fromColorInfo, toColorInfo [ , power = 1 ] )
  $H arguments
   $ARG colorInfo fromColorInfo: The color to be changed.
   $ARG colorInfo toColorInfo: The substitute color. For further information about ,,colorInfo,, see below.
   $ARG $REAL power: strength of color replacement.
  $H example
  {{{
  const BLUE = [ 0,0,1,1 ];
  const WHITE = [ 1,1,1,1 ];

  var texture = new Texture( 100, 100, 3 );
  ...
  texture.colorize( WHITE, BLUE, 0 );
  }}}
**/
// PTYPE ok
DEFINE_FUNCTION( colorize ) {
	// GIMP color to alpha: http://www.google.com/codesearch?hl=en&q=+gimp+%22color+to+alpha%22
	// color exchange algo. : http://www.koders.com/c/fidB39DAC5A8DB8B6073D78FB23363C5E0541208B02.aspx

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(2,3);

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	int channels;
	channels = tex->channels;

	PTYPE colorSrc[PMAXCHANNELS], colorDst[PMAXCHANNELS];
	JL_CHK( InitLevelData(cx, JL_ARG(1), channels, colorSrc) );
	JL_CHK( InitLevelData(cx, JL_ARG(2), channels, colorDst) );

	PTYPE power;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( jl::getValue(cx, JL_ARG(3), &power) );
	else
		power = 1;

	PTYPE ratio;
	int pos, size;
	size = tex->width * tex->height;
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

		if ( power != PTYPE(1) )
			ratio = powf(ratio, PTYPE(1) / power);

		for ( c = 0; c < channels; c++ )
			tex->cbuffer[pos+c] = (tex->cbuffer[pos+c] * (PTYPE(1) - ratio) + colorDst[c] * ratio);
	}
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( sourceTexture, sourceColorInfo [ , strength = 1] )
  Fill the current texture with a given color from _sourceTexture_.
  $H arguments
   $ARG Texture sourceTexture: the texture from wich the color will be extracted.
   $ARG colorInfo sourceColorInfo: The color to extract.
   $ARG $REAL strength: The strength of the exraction.
  $H note
   The current texture must have only one channel because the method only extracts one color.
  $H example
  {{{
  t1.extractColor(t, RED, 10);
  }}}
**/
// PTYPE ok
DEFINE_FUNCTION( extractColor ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	TextureStruct *texSrc;
	JL_CHK( ValueToTexture(cx, JL_ARG(1), &texSrc) );

	JL_ASSERT( tex->channels == 1, E_THISOBJ, E_FORMAT, E_COMMENT("channels") ); // "Destination texture must have only 1 channel."
	JL_ASSERT( tex->width == texSrc->width && tex->height == texSrc->height, E_THISOBJ, E_FORMAT, E_COMMENT("size") ); // "Images must have the same width and height."

	int srcChannels;
	srcChannels = texSrc->channels;

	PTYPE color[PMAXCHANNELS];
	JL_CHK( InitLevelData(cx, JL_ARG(2), srcChannels, color) );

	PTYPE power;
	if ( argc >= 3 )
		JL_CHK( jl::getValue(cx, JL_ARG(3), &power) );
	else
		power = 1;

	PTYPE ratio;
	int pos, size;
	size = tex->width * tex->height;
	int c;
	for ( int i = 0; i < size; i++ ) {

		pos = i * srcChannels;
		ratio = 1;
		for ( c = 0; c < srcChannels; c++ )
			ratio *= ( PMAX - abs(texSrc->cbuffer[pos+c] - color[c]) ) / PMAX;
		if ( power != 1 )
			ratio = powf(ratio, PTYPE(1) / power);
		tex->cbuffer[i] = ratio;
	}
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  Changes the range of each $pval of the texture. The resulting range of each $pval will be [ 0,1 ]
  $H note
   Normalization is sometimes called contrast stretching.
**/
// PTYPE ok
DEFINE_FUNCTION( normalizeLevels ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	PTYPE min, max, tmp;
	min = PMAXLIMIT;
	max = PMINLIMIT;

	int tsize;
	tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ ) {

		tmp = tex->cbuffer[i];
		if ( tmp > max )
			max = tmp;
		else if ( tmp < min )
			min = tmp;
	}
	PTYPE amp;
	amp = (max - min) * PMAX;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] = ( tex->cbuffer[i] - min ) / amp; // value is normalized to 0...PMAX
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( min, max )
  All $pval that are out of the [ min,max ] range are forced to [ min,max ] range.
  $H arguments
   $ARG $REAL min: low value
   $ARG $REAL max: high value
**/
// PTYPE ok
DEFINE_FUNCTION( clampLevels ) { // (TBD) check if this algo is right

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 2 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	PTYPE min, max;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &min) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &max) );

	PTYPE tmp;
	int tsize;
	tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ ) {

		tmp = tex->cbuffer[i];
		if ( tmp > max )
			tex->cbuffer[i] = max;
		else if ( tmp < min )
			tex->cbuffer[i] = min;
	}
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( min, max )
  All $pval that are out of the [ min,max ] range are forced to [ 0,1 ] range.
  $H arguments
   $ARG $REAL min: low value
   $ARG $REAL max: high value
**/
// PTYPE ok
DEFINE_FUNCTION( cutLevels ) { // (TBD) check if this algo is right

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 2 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	double min, max;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &min) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &max) );

	PTYPE tmp;
	int tsize;
	tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ ) {

		tmp = tex->cbuffer[i];
		if ( tmp > max )
			tex->cbuffer[i] = PMAX;
		else if ( tmp < min )
			tex->cbuffer[i] = 0;
	}
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}

/*
// PTYPE ok
DEFINE_FUNCTION( cutLevels ) {

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	PTYPE min, max;

	if ( argc == 0 ) {

		min = 0;
		max = PMAX;
	}

	if ( argc >= 1 ) {

		JL_CHK( jl::getValue(cx, JL_ARG(1), &min) );
		max = min;
	}

	if ( argc >= 2 ) {

		JL_CHK( jl::getValue(cx, JL_ARG(2), &max) );
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
	*JL_RVAL = OBJECT_TO_JSVAL(JL_OBJ);
	return true;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  Each $pval is mathematically inverted ( v = 1 / v ).
**/
// PTYPE ok
DEFINE_FUNCTION( invertLevels ) { // level = 1 / level

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	unsigned int i, size;
	size = tex->width * tex->height * tex->channels;
	for ( i = 0; i < size; i++ )
		tex->cbuffer[i] = PTYPE(1) / tex->cbuffer[i];
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  Each $pval is set to its mathematical opposite ( v = -v ).
**/
// PTYPE ok
DEFINE_FUNCTION( oppositeLevels ) { // level = -level

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	int tsize;
	tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] =  - tex->cbuffer[i];
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( power )
  Each $pval is powered by _power_ ( v = v ^ _power_ ).
**/
// PTYPE ok
DEFINE_FUNCTION( powLevels ) { //
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	PTYPE power;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &power) );

	int tsize;
	tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ )
		tex->cbuffer[i] =  powf(tex->cbuffer[i], power);
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( threshold, mirrorFromTop )
  Each $pval is mirrored toward the top or the bottom.
  $H arguments
   $ARG $REAL threshold: the point from the values are reflected.
   $ARG $BOOL mirrorFromTop: if true, the values over _threshold_ are reflected toward the bottom, else values under _threshold_ are reflected toward the top.
**/
// PTYPE ok
DEFINE_FUNCTION( mirrorLevels ) {
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	PTYPE threshold;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &threshold) );

	bool mirrorTop;
	if ( argc >= 2 && !JL_ARG(2).isUndefined() )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &mirrorTop) );
	else
		mirrorTop = true;

	PTYPE value;
	int i, tsize;
	tsize = tex->width * tex->height * tex->channels;

	if ( mirrorTop )
		for ( i = 0; i < tsize; i++ ) {

			value = tex->cbuffer[i];
			if ( value > threshold )
				tex->cbuffer[i] = PTYPE(2) * threshold - value;
		}
	else
		for ( i = 0; i < tsize; i++ ) {

			value = tex->cbuffer[i];
			if ( value < threshold )
				tex->cbuffer[i] = PTYPE(2) * threshold - value;
		}
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( modulo )
  Each $pval is moduloed by _modulo_ ( v = v % _modulo_ ).
  $H arguments
   $REAL modulo: the non-integer modulo
**/
// PTYPE ok
DEFINE_FUNCTION( wrapLevels ) { // real modulo
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	PTYPE wrap;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &wrap) );

	PTYPE div;
	int tsize;
	tsize = tex->width * tex->height * tex->channels;
	for ( int i = 0; i < tsize; i++ ) {

		div = tex->cbuffer[i] / wrap;
		tex->cbuffer[i] = div - (long)div;
	}

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( [ color ] )
  Adds a random noise to the current texture.
  $H arguments
   $ARG colorInfo color: noise color.
**/
// PTYPE ok
DEFINE_FUNCTION( addNoise ) {
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	int channels;
	channels = tex->channels;

	bool fullLevel;

	PTYPE pixel[PMAXCHANNELS];
	if ( argc >= 1 ) {

		JL_CHK( InitLevelData(cx, JL_ARG(1), tex->channels, pixel) );
		fullLevel = false;
	} else {

		fullLevel = true;
	}

	int i, tsize;
	tsize = tex->width * tex->height * channels;
	if ( fullLevel )
		for ( i = 0; i < tsize; i++ )
			tex->cbuffer[i] += PRAND;
	else
		for ( i = 0; i < tsize; i++ )
			tex->cbuffer[i] += PRAND * pixel[i % channels] / PMAX; //(TBD) test if i%channels works fine, else use double for loop
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( [ , mode ] )
  Desaturates the texture.
  $H arguments
   $ARG $INT mode: is the type of desaturation, either Texture.desaturateLightness, Texture.desaturateSum or Texture.desaturateAverage
**/
// PTYPE ok
DEFINE_FUNCTION( desaturate ) {
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MAX(1);

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	DesaturateMode mode;
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( jl::getValue(cx, JL_ARG(1), (int*)&mode) );
	else
		mode = desaturateAverage;

	PTYPE val, min, max, tmp, *sPos, *dPos;
	unsigned int i, c, channels, size;
	channels = tex->channels;
	size = tex->width * tex->height;

	TextureResizeBackBuffer(tex, size * sizeof(PTYPE));

	dPos = tex->cbackBuffer;
	for ( i = 0; i < size; i++ ) {

		sPos = &tex->cbuffer[i * channels];
		switch( mode ) {
			case desaturateLightness:
				min = max = *sPos;
				for ( c = 1; c < channels; c++ ) {

					tmp = sPos[c];
					if ( tmp > max )
						max = tmp;
					else if ( tmp < min )
						min = tmp;
				}
				val = ( min + max ) / 2;
				break;
			case desaturateSum:
				val = 0;
				for ( c = 0; c < channels; c++ )
					val += sPos[c];
				break;
			case desaturateAverage:
				val = 0;
				for ( c = 0; c < channels; c++ )
					val += sPos[c];
				val /= channels;
				break;
//			case desaturateLuminosity: // see http://svn.gnome.org/viewcvs/gimp/trunk/libgimpcolor/gimprgb.h?revision=19720&view=markup
//				break;
			default:
				val = 0;
				JL_WARN( E_NOTIMPLEMENTED );
		}
		*dPos = val;
		dPos++;
	}

	TextureSwapBuffers(tex);
	tex->channels = 1;

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( texture )
 $THIS $INAME( image )
 $THIS $INAME( color )
  $H arguments
   $ARG Texture texture:
   $ARG Image image:
   $ARG colorInfo color:
  Set a texture with another texture, image or _color_.
**/
// PTYPE ok
DEFINE_FUNCTION( set ) {
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	unsigned int i, size, channels;
	channels = tex->channels;
	size = tex->width * tex->height * channels;
	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	if ( IsTexture(cx, JL_ARG(1)) ) {

		TextureStruct *tex1;
		JL_CHK( ValueToTexture(cx, JL_ARG(1), &tex1) );
		
		JL_ASSERT( TextureSameFormat(tex, tex1), E_STR("Texture"), E_FORMAT );
		jl::memcpy(tex->cbuffer, tex1->cbuffer, size * sizeof(PTYPE));
		return true;
	}

	{
		int i, tsize, sWidth, sHeight, sChannels;
		ImageDataType dataType;
		JLData data = JL_GetImageObject(cx, JL_ARG(1), &sWidth, &sHeight, &sChannels, &dataType);
		if ( data.IsSet() ) {

			JL_ASSERT( dataType == TYPE_UINT8, E_ARG, E_NUM(1), E_DATATYPE, E_INVALID );
			tsize = data.Length();
			const uint8_t *buffer = (const uint8_t*)data.GetConstStr();
			for ( i = 0; i < tsize; i++ )
				tex->cbuffer[i] = (PTYPE)buffer[i] / PTYPE(255); // map [0 -> 255] to [0.0 -> 1.0]
			return true;
		}
	}

	PTYPE pixel[PMAXCHANNELS];
	JL_CHK( InitLevelData(cx, JL_ARG(1), channels, pixel) );
	unsigned int c;
	size = tex->width * tex->height;
	for ( c = 0; c < channels; c++ )
		for ( i = 0; i < size; i++ )
			tex->cbuffer[i*channels+c] = pixel[c];

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( textureObject [, factor = 1] )
 $THIS $INAME( colorInfo [, factor = 1] )
  Mathematically adds a texture (_textureObject_) or a given color (_colorInfo_) to the current texture.$LF
  _factor_ is applied to the source (object or color) before the addition.
**/
// PTYPE ok
DEFINE_FUNCTION( add ) {
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(1, 2);
	
	TextureStruct *tex;
	tex = (TextureStruct*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	unsigned int i, c, channels, size;
	channels = tex->channels;

	float factor;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &factor) );
	else
		factor = 1.;

	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	if ( JL_ARG(1).isNumber() ) {

		PTYPE value;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &value) );
		size = tex->width * tex->height * tex->channels;
		value *= factor;
		for ( i = 0; i < size; i++ )
			tex->cbuffer[i] += value;
		return true;
	}

	if ( IsTexture(cx, JL_ARG(1)) ) {

		TextureStruct *tex1;
		JL_CHK( ValueToTexture(cx, JL_ARG(1), &tex1) );
		JL_ASSERT( tex1->width == tex->width && tex1->height == tex->height && ( tex1->channels == 1 || tex1->channels == channels), E_ARG, E_NUM(1), E_FORMAT );

		size = tex->width * tex->height * channels;
		if ( tex1->channels == 1 ) {
			
			if ( factor == 1. )  // optimization if factor == 1
				for ( i = 0; i < size; i++ )
					tex->cbuffer[i] += tex1->cbuffer[i / channels];
			else
				for ( i = 0; i < size; i++ )
					tex->cbuffer[i] += tex1->cbuffer[i / channels] * factor;
		} else {

			if ( factor == 1. ) // optimization if factor == 1
				for ( i = 0; i < size; i++ )
					tex->cbuffer[i] += tex1->cbuffer[i];
			else
				for ( i = 0; i < size; i++ )
					tex->cbuffer[i] += tex1->cbuffer[i] * factor;
		}
		return true;
	}

	if ( JL_ValueIsArrayLike(cx, JL_ARG(1)) ) {

		PTYPE *pos, level, pixel[PMAXCHANNELS];
		JL_CHK( InitLevelData(cx, JL_ARG(1), channels, pixel) );
		size = tex->width * tex->height;
		for ( c = 0; c < channels; c++ ) {

			pos = &tex->cbuffer[c];
			level = pixel[c] * factor;
			for ( i = 0; i < size; ++i ) {

				*pos += level;
				pos += channels;
			}
		}
		return true;
	}
	
	JL_ERR( E_ARG, E_INVALID );
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( textureObject )
 $THIS $INAME( colorInfo )
  Mathematically multiply a texture (_textureObject_) or a given color (_colorInfo_) to the current texture.
**/
// PTYPE ok
DEFINE_FUNCTION( mult ) {
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	unsigned int i, c, channels, size;
	PTYPE *dPos, *sPos;
	channels = tex->channels;

	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	if ( JL_ARG(1).isNumber() ) {

		PTYPE value;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &value) );
		size = tex->width * tex->height * tex->channels;
		for ( i = 0; i < size; i++ )
			tex->cbuffer[i] *= value;
		return true;
	}

	if ( IsTexture(cx, JL_ARG(1)) ) {

		TextureStruct *tex1;
		JL_CHK( ValueToTexture(cx, JL_ARG(1), &tex1) );
		JL_ASSERT( tex1->width == tex->width && tex1->height == tex->height && ( tex1->channels == 1 || tex1->channels == channels), E_ARG, E_NUM(1), E_FORMAT );

		if ( tex1->channels == 1 ) {
			
			dPos = tex->cbuffer;
			sPos = tex1->cbuffer;
			size = tex->width * tex->height;
			for ( c = 0; c < channels; ++c ) {

				for ( i = 0; i < size; ++i ) {

					*dPos *= *sPos;
					dPos++;
				}
				sPos++;
			}

		} else {

			size = tex->width * tex->height * channels;
			for ( i = 0; i < size; i++ )
				tex->cbuffer[i] *= tex1->cbuffer[i];
		}
		return true;
	}

	if ( JL_ValueIsArrayLike(cx, JL_ARG(1)) ) {

		PTYPE *pos, level, pixel[PMAXCHANNELS];
		JL_CHK( InitLevelData(cx, JL_ARG(1), channels, pixel) );
		size = tex->width * tex->height;
		for ( c = 0; c < channels; c++ ) {

			pos = &tex->cbuffer[c];
			level = pixel[c];
			for ( i = 0; i < size; ++i ) {

				*pos *= level;
				pos += channels;
			}
		}
		return true;
	}

	JL_ERR( E_ARG, E_INVALID );
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( otherTexture, blendTexture )
 $THIS $INAME( otherTexture, color )
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
  tmp.clearChannel();
  tmp.setRectangle(10, 10, size-10, size-10, 1);
  t.blend(tmp, 0.7);
  }}}
**/
// PTYPE ok
DEFINE_FUNCTION( blend ) { // texture1, blenderTexture|blenderColor
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 2 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	int channels;
	channels = tex->channels;
	int tsize;
	tsize = tex->width * tex->height * channels;

	TextureStruct *tex1;
	JL_CHK( ValueToTexture(cx, JL_ARG(1), &tex1) );
	JL_ASSERT( TextureSameFormat(tex, tex1),  E_ARG, E_NUM(1), E_FORMAT );

	float blend;

	if ( IsTexture(cx, JL_ARG(2)) ) {

		TextureStruct *blenderTex;
		JL_CHK( ValueToTexture(cx, JL_ARG(2), &blenderTex) );
		JL_ASSERT( TextureSameFormat(tex, blenderTex), E_ARG, E_NUM(2), E_FORMAT );

		for ( int i = 0; i < tsize; i++ ) {

			blend = blenderTex->cbuffer[i];
			tex->cbuffer[i] = (blend * tex->cbuffer[i] + (PMAX - blend) * tex1->cbuffer[i] ) / PMAX;
		}
	} else {

		PTYPE pixel[PMAXCHANNELS];
		JL_CHK( InitLevelData(cx, JL_ARG(2), channels, pixel) );

		int pos, c, size;
		size = tex->width * tex->height;
		for ( int i = 0; i < size; i++ ) {

			pos = i * channels;
			for ( c = 0; c < channels; c++ ) { // (TBD) optimization: iterates on channels first

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
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( count )
  Make _count_ 90 degres rotations.
  $H arguments
   $ARG $INT count: the number of integer rotation to perform with the current texture. _count_ may be negative.
  $H note
   For non-integer rotations, see RotoZoom() function.
**/
DEFINE_FUNCTION( rotate90 ) { // (TBD) test it
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	int turn;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &turn) );

	turn = Wrap(turn, turn);

	unsigned int width, height, channels, x, y;
	width = tex->width;
	height = tex->height;
	channels = tex->channels;

	TextureSetupBackBuffer(tex);

	int pos, pos1;
	unsigned int c;

	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			pos = (x + y * width) * channels;
			switch ( turn ) {
				case 1:
					pos1 = ((width-1-y) + (x) * width) * channels;
					break;
				case 2:
					pos1 = ((width-1-x) + (height-1-y) * width) * channels;
					break;
				case 3:
					pos1 = ((y) + (height-1-x) * width) * channels;
					break;
				case 0:
				default:
					pos1 = pos;
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

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( horizontally, vertically )
  Flips the current texture horizontally, vertically or both.
  $H arguments
   $ARG $BOOL horizontally: flips the texture horizontally (against x axis).
   $ARG $BOOL vertically: flips the texture vertically (against y axis).
**/
DEFINE_FUNCTION( flip ) {
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 2 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	bool flipX, flipY;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &flipX) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &flipY) );

	TextureSetupBackBuffer(tex);

	unsigned int width, height, channels, x, y, c;
	width = tex->width;
	height = tex->height;
	channels = tex->channels;

	PTYPE *posDst, *posSrc;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			posDst = &tex->cbackBuffer[(x + y * width) * channels];
			posSrc = &tex->cbuffer[( (flipX?width-1-x:x) + (flipY?height-1-y:y) * width ) * channels];
			for ( c = 0; c < channels; c++ )
				posDst[c] = posSrc[c];
		}
	TextureSwapBuffers(tex);
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( centerX, centerY, zoomX, zoomY, rotations )
  Make a zoom and/or a rotation of the current texture.
  $H arguments
   $ARG $REAL centerX: coordinate of the center of the zoom or rotation.
   $ARG $REAL centerY:
   $ARG $REAL zoomX: the zoom factor (use 1 for none).
   $ARG $REAL zoomY:
   $ARG $REAL rotations: the number of totations to perform. 0.25 is 90 degres (use 0 for none).
**/
DEFINE_FUNCTION( rotoZoom ) { // source: FxGen
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 5 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	int width;
	width = tex->width;
	int height;
	height = tex->height;
	int channels;
	channels = tex->channels;

	int newWidth;
	newWidth = width;
	int newHeight;
	newHeight = height;

	PTYPE centerX, centerY;
	double zoomX, zoomY;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &centerX) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &centerY) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &zoomX) );
	JL_CHK( jl::getValue(cx, JL_ARG(4), &zoomY) );

//	zoomX = 0.5 - ( zoomX / 2 );
//	zoomX = exp( zoomX * 6 );

//	zoomY = 0.5 - ( zoomY / 2 );
//	zoomY = exp( zoomY * 6 );

	double rotate;
	JL_CHK( jl::getValue(cx, JL_ARG(5), &rotate) ); // 1 for 1 turn

	rotate = M_PI * 2 * rotate;

	float coefX;
	coefX = PTYPE(zoomX * width / newWidth);
	float coefY;
	coefY = PTYPE(zoomY * height / newHeight);

	float	cosVal;
	cosVal = PTYPE(cos(rotate));
	float	sinVal;
	sinVal = PTYPE(sin(rotate));

	float tw2;
	tw2 = newWidth / 2.0f;
	float th2;
	th2 = newHeight / 2.0f;

	float	ys;
	ys = sinVal * -th2;
	float	yc;
	yc = cosVal * -th2;

	TextureSetupBackBuffer(tex);

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
			spx = Wrap(int(u), width);
			spy = Wrap(int(v), height);

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

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( newWidth, newHeight, [ interpolate = false [, borderMode = Texture.borderWrap ] ] )
  Resize the current texture.
  $H arguments
   $ARG $INT newWidth:
   $ARG $INT newHeight:
   $ARG $BOOL interpolate: uses a linear interpolation.
   $ARG $ENUM borderMode: how to manage the border. either Texture.borderClamp, Texture.borderWrap, Texture.borderMirror or Texture.borderValue.
**/
// PTYPE ok
DEFINE_FUNCTION( resize ) {
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 2 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	unsigned int width, height, channels;
	width = tex->width;
	height = tex->height;
	channels = tex->channels;

	unsigned int newWidth, newHeight;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &newWidth) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &newHeight) );

	bool interpolate;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( jl::getValue(cx, JL_ARG(3), &interpolate) );
	else
		interpolate = false;

	BorderMode borderMode;
	JL_CHK( JL_JsvalToBorderMode(cx, JL_SARG(4), &borderMode) );

	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	if ( newWidth == width && newHeight == height ) // optimization
		return true;

	JL_CHK( TextureResizeBackBuffer(tex, newWidth * newHeight * channels * sizeof(PTYPE)) );

	int spx, spy; // position in the source
	float prx, pry; // pixel ratio
	float rx;
	rx = (float)width / newWidth; // texture ratio x
	float ry;
	ry = (float)height / newHeight; // texture ratio y
	size_t x, y, c;

	size_t pos, pos1, pos2, pos3, pos4; // offset in the buffer
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
				spx = Wrap(int(u), width);
				spy = Wrap(int(v), height);



				// (TBD) for radio1,..4, try to use the distance ?
				switch ( borderMode ) { // Note: it is faster to do the switch outside the inner loop

					case borderClamp:
						// (TBD)
						pos1 = 0;
						pos2 = 0;
						pos3 = 0;
						pos4 = 0;
						break;
					case borderWrap:
						pos1 = (  ((spx + 0)        ) + ((spy + 0)         ) * width  ) * channels;
						pos2 = (  ((spx + 1) % width) + ((spy + 0)         ) * width  ) * channels;
						pos3 = (  ((spx + 0)        ) + ((spy + 1) % height) * width  ) * channels;
						pos4 = (  ((spx + 1) % width) + ((spy + 1) % height) * width  ) * channels;
						break;
					case borderMirror:
						// (TBD)
						pos1 = 0;
						pos2 = 0;
						pos3 = 0;
						pos4 = 0;
						break;
					case borderValue:
						// (TBD)
						pos1 = 0;
						pos2 = 0;
						pos3 = 0;
						pos4 = 0;
						break;
					default:
//						pos1 = 0;
//						pos2 = 0;
//						pos3 = 0;
//						pos4 = 0;
						JL_ERR( E_ARG, E_NUM(4), E_INVALID );
				}

				ratio1 = (1.f - prx) * (1.f - pry);
				ratio2 = (prx) * (1.f - pry);
				ratio3 = (1.f - prx) * (pry);
				ratio4 = (prx) * (pry);

				pos = (x + y * newWidth) * channels;
				for ( c = 0; c < channels; c++ )
					tex->cbackBuffer[pos+c] = tex->cbuffer[pos1+c] * ratio1 + tex->cbuffer[pos2+c] * ratio2 + tex->cbuffer[pos3+c] * ratio3 + tex->cbuffer[pos4+c] * ratio4;
			} else {

				spx = (int)(rx * x);
				spy = (int)(ry * y);
				pos = (x + y * newWidth) * channels;
				pos1 = (spx + spy * width) * channels;
				for ( c = 0; c < channels; c++ )
					tex->cbackBuffer[pos+c] = tex->cbuffer[pos1+c];
			}
		}

	TextureSwapBuffers(tex);

	tex->width = newWidth;
	tex->height = newHeight;
	// channels don't change


	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( kernel, [ borderMode = Texture.borderWrap ] )
  Apply a convolution to the current texture using _kernel_ factors.
  $H arguments
   $ARG $ARRAY kernel: kernel is a square matrix
   $ARG $ENUM borderMode: how to manage the border. either Texture.borderClamp, Texture.borderWrap, Texture.borderMirror or Texture.borderValue.
  $H note
   The convolution is a complex transformation that could be very slow with big kernels.
  $H note
   Because convolution apples a factor to each point of the texture,
   it is recomanded to normalize the levels of the texture using Mult() or NormalizeLevels()
  $H example
  {{{
  const kernelGaussian = [0,3,10,3,0, 3,16,26,16,3, 10,26,26,26,10, 3,16,26,16,3, 0,3,10,3,0 ];
  const kernelGaussian2 = [2,4,5,4,2, 4,9,12,9,4, 5,12,15,12,5, 4,9,12,9,4, 2,4,5,4,2]; // G(r) = pow(E,-r*r/(2*o*o))/sqrt(2*PI*o);
  const kernelEmboss = [-1,0,0, 0,0,0 ,0,0,1];
  const kernelLaplacian = [-1,-1,-1, -1,8,-1, -1,-1,-1];
  const kernelLaplacian4 = [0,-1,0, -1,4,-1 ,0,-1,0];
  const kernelShift = [0,0,0, 0,0,0 ,0,0,1];
  const kernelCrystals = [0,-1,0, -1,5,-1, 0,-1,0];
  ...
  texture.convolution(kernelGaussian);
  texture.normalizeLevels();
  }}}
**/
// (TBD) PTYPE
DEFINE_FUNCTION( convolution ) {
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();


	// (TBD) accumulate precalculated pixels ? ( like BoxBlur )
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_OBJECT(1);

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	int width, height;
	width = tex->width;
	height = tex->height;

	unsigned count;
	// JL_CHK( ArrayLength(cx, &count, JL_ARG(1)) );
	JL_CHK( JS_GetArrayLength( cx, &JL_ARG(1).toObject(), &count ) );
	float *kernel;
	kernel = (float*)alloca(sizeof(float) * count);
	JL_ASSERT_ALLOC( kernel );
	//JL_CHK( FloatArrayToVector(cx, count, &JL_ARG(1), kernel) );
	uint32_t length;
	JL_CHK( jl::getVector(cx, JL_ARG(1), kernel, count, &length) );

	int size;
	size = (int)sqrtf(float(count));

	JL_ASSERT( size * size == (int)count, E_ARG, E_NUM(1), E_SEP, E_ARRAYLENGTH, E_INVALID ); // "Invalid convolution kernel size."
	
	BorderMode borderMode;
	JL_CHK( JL_JsvalToBorderMode(cx, JL_SARG(2), &borderMode) );

	TextureSetupBackBuffer(tex);

	int offset;
	offset = size / 2;
	float sizeWeight;
	sizeWeight = float(count);
	float ratio;

	PTYPE pixel[PMAXCHANNELS];
	int channels;
	channels = tex->channels;

	int pos;
	int x, y, c, vx, vy, sx, sy;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			for ( c = 0; c < channels; c++ )
				pixel[c] = 0;

			switch ( borderMode ) { // Note: it is faster to do the switch outside the inner loop

				case borderWrap:
					for ( vy = 0; vy < size; vy++ )
						for ( vx = 0; vx < size; vx++ ) {

							pos = (Wrap(x + vx-offset, width) + Wrap(y + vy-offset, height) * width) * channels;
							ratio =  kernel[vx + vy * size];
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
							for ( c = 0; c < channels; c++ )
								pixel[c] += tex->cbuffer[pos+c] * ratio;
						}
					break;
				default:
					JL_ERR( E_ARG, E_NUM(2), E_INVALID );
			}

			pos = (x + y * width) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbackBuffer[pos+c] = pixel[c];
		}

	//jl_free(kernel); // alloca
	TextureSwapBuffers(tex);
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( iterations [ , radius = 1 ] [ , borderMode = Texture.borderWrap ] )
  (TBD)
**/
DEFINE_FUNCTION( dilate ) {
	
	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(1,3);

	int iterations, radius;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &iterations) );

	if ( JL_ARG_ISDEF(2) )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &radius) );
	else
		radius = 1;

	BorderMode borderMode;
	JL_CHK( JL_JsvalToBorderMode(cx, JL_SARG(3), &borderMode) );


	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	int width, height, channels;
	width = tex->width;
	height = tex->height;
	channels = tex->channels;

	TextureSetupBackBuffer(tex);

	int i, x, y, c, u, v;
	unsigned int pos;
	PTYPE sum, tmp;

	for (i = 0; i < iterations; ++i) {

		for (y = 0; y < height; ++y) {
			for (x = 0; x < width; ++x) {
				
				pos = (x + y * height) * channels;

				sum = 0;
				for ( c = 0; c < channels; ++c )
					tex->cbackBuffer[pos+c] = tex->cbuffer[pos+c];

				for (v = -radius; v <= radius; ++v) {
					for (u = -radius; u <= radius; ++u) {

						PTYPE *src = PosByMode(tex, x+u, y+v, borderMode);
						if ( !src ) // skip
							continue;

						tmp = 0;
						for ( c = 0; c < channels; ++c )
							tmp += src[c];

						if ( tmp > sum ) {

							sum = tmp;
							for ( c = 0; c < channels; ++c )
								tex->cbackBuffer[pos+c] = src[c];
						}
					}
				}
			}
		}
	
		TextureSwapBuffers(tex);
	}

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( function )
  Call _function_ for each pixel of the texture. the function is called with the arguments (x, y, pixel) where pixel is an array of levels.
  $H note
   Because a JS function is called for each pixel, the processing could be very slow.
  $H example
   draws a line from (0,0) to (100,100)
  {{{
  var texture = new Texture(100, 100, 3); // RGB texture
  texture.set(0); // clears the texture
  texture.forEachPixel(function(x, y, pixel) {
   if ( x == y ) {
    pixel[0] = 1; // Red
    pixel[1] = 1; // Green
    pixel[2] = 1; // Blue
    return pixel;
   }
  });
  }}}
**/
DEFINE_FUNCTION( forEachPixel ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_CALLABLE(1);

	jsval functionValue;
	functionValue = JL_ARG(1);

	TextureStruct *tex;
	tex = (TextureStruct*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	unsigned int x, y, c, width, height, channels;
	width = tex->width;
	height = tex->height;
	channels = tex->channels;

	TextureSetupBackBuffer(tex);

	jsval level, callArgv[4]; // callArgv[0] is the rval

	JSObject *cArrayObj;
	cArrayObj = JS_NewArrayObject(cx, channels);
	JL_CHK( cArrayObj );
	callArgv[1] = OBJECT_TO_JSVAL(cArrayObj);
	{

		for ( y = 0; y < height; y++ ) { // faster than (for 0 to (width*height))
			for ( x = 0; x < width; x++ ) {

				size_t pos = (x+y*width)*channels;
				callArgv[2] = INT_TO_JSVAL(x);
				callArgv[3] = INT_TO_JSVAL(y);
				for ( c = 0; c < channels; c++ ) {

					JL_CHK( JL_NativeToJsval(cx, tex->cbuffer[pos+c], level) );
					JL_CHK( JL_SetElement(cx, cArrayObj, c, level) );
				}

				JL_CHK( JS_CallFunctionValue(cx, obj, functionValue, COUNTOF(callArgv)-1, callArgv+1, callArgv) );

				if ( callArgv[0] != JSVAL_FALSE ) { // function's return value

					for ( c = 0; c < channels; c++ ) {

						JL_CHK( JL_GetElement(cx, cArrayObj, c, level) );
						JL_CHK( jl::getValue(cx, level, &tex->cbackBuffer[pos+c]) );
					}
				}
			}
		}
	}
	TextureSwapBuffers(tex);
	return true;
bad:
	return false;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( blurWidth, blurHeight [, iterations = 1 ] )
  Apply a box blur of the given width and height.
  $H note
   BoxBlur is very fast but the result is not very smooth.
  $H example
  {{{
  function addPixels(t, count) {
   while ( count-- > 0 )
    t.setPixel(Texture.randInt(), Texture.randInt(), 1);
  }

  var texture = new Texture(128, 128, 3);
  texture.set(0);
  addPixels(texture, 100);
  texture.boxBlur(20,20);
  texture.normalizeLevels();
  }}}
**/
DEFINE_FUNCTION( boxBlur ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(2,3);

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	int width, height;
	width = tex->width;
	height = tex->height;

	int bw, bh, iterations; // blur width & height
	JL_CHK( jl::getValue(cx, JL_ARG(1), &bw) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &bh) );


	if ( JL_ARG_ISDEF(3) )
		JL_CHK( jl::getValue(cx, JL_ARG(3), &iterations) );
	else
		iterations = 1;


	if ( bw > width )
		bw = width;
	if ( bh > height )
		bh = height;

	int channels;
	channels = tex->channels;

	PTYPE sum;

	TextureSetupBackBuffer(tex);

	int x, y, c;

	for ( int i = 0; i < iterations; i++ ) {

		if ( bw > 1 ) {
			for ( y = 0; y < height; y++ ) {

				for ( c = 0; c < channels; c++ ) {

					PTYPE *pos = &tex->cbuffer[y * width * channels + c];
					
					sum = 0;
					for ( x = 0; x < bw; x++ )
						sum += pos[x * channels];

					for ( x = 0; x < width; x++ ) {

						tex->cbackBuffer[ ( (x + bw/2)%width + y * width ) * channels + c ] = sum / bw;
						sum = sum - pos[x * channels] + pos[((x+bw)%width) * channels];
					}
				}
			}
			TextureSwapBuffers(tex);
		}
		
		if ( bh > 1 ) {

			int wc = width * channels;
			for ( x = 0; x < width; x++ ) {

				for ( c = 0; c < channels; c++ ) {

					PTYPE *pos = &tex->cbuffer[x * channels + c];
					
					sum = 0;
					for ( y = 0; y < bh; y++ )
						sum += pos[y * wc];

					for ( y = 0; y < height; y++ ) {

						tex->cbackBuffer[ ( x + ((y + bh/2)%height) * width ) * channels + c ] = sum / bh;
						sum = sum - pos[y * wc] + pos[((y+bh)%height) * wc];
					}
				}
			}
			TextureSwapBuffers(tex);
		}
	}

//	jl_free( line );
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  Converts each pixel into a vector, normalize this vector, then store the vector as a pixel.
**/
DEFINE_FUNCTION( normalizeVectors ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	int width;
	width = tex->width;
	int height;
	height = tex->height;
	int channels;
	channels = tex->channels;

	int size;
	size = tex->width * tex->height;
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
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( [ amplify = 1 ] )
  Converts the texture to a normals map using the Sobel filter.
**/
// (TBD) PTYPE
DEFINE_FUNCTION( normals ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	JL_CHK( TextureResizeBackBuffer(tex, tex->width * tex->height * 3 * sizeof(PTYPE)) ); // need a 3 channels back buffer

	PTYPE amp;
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( jl::getValue(cx, JL_ARG(1), &amp) );
	else
		amp = PTYPE(1);

	unsigned int width, height, channels, x, y, pos;
	width = tex->width;
	height= tex->height;
	channels = tex->channels;

	Vector3 normal;
	float dX, dY, fPix;

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
			Vector3Normalize(&normal, &normal);

			pos = (x + y * width) * 3; // 3 is the number of channels
			tex->cbackBuffer[pos+0] = PUNZNORM( normal.x );
			tex->cbackBuffer[pos+1] = PUNZNORM( normal.y );
			tex->cbackBuffer[pos+2] = PUNZNORM( normal.z );
		}

	TextureSwapBuffers(tex);
	tex->channels = 3;

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( normalsTexture, lightPosition, ambiantColor, diffuseColor, specularColor, bumpPower, specularPower )
  Floodlight the current texture using the _normalsTexture_ as bump map.
  $H arguments
   $ARG Texture normalsTexture: the bump map where each pixel is a 3D vector.
   $ARG $ARRAY lightPosition: is the position of the light in a 3D space ( [ x, y, z ] )
   $ARG colorInfo ambiantColor:
   $ARG colorInfo diffuseColor:
   $ARG colorInfo specularColor:
   $ARG $REAL bumpPower:
   $ARG $REAL specularPower:
  $H example
  {{{
  var bump = new Texture(size, size, 3).cells(8, 0).add( new Texture(size, size, 3).cells(8, 1).oppositeLevels() ); // broken floor
  bump.normals();
  var texture = new Texture(size, size, 3);
  texture.set(1);
  texture.light( bump, [-1, -1, 1], 0, [0.1, 0.3, 0.4], 0.2, 0.5, 10 );
  }}}
**/
// (TBD) PTYPE
DEFINE_FUNCTION( light ) {
	// Simple Lighting: http://www.gamasutra.com/features/19990416/intel_simd_04.htm

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(5);

	TextureStruct *normals, *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	int width;
	width = tex->width;
	int height;
	height = tex->height;
	int channels;
	channels = tex->channels;

	JL_ASSERT( tex->channels >= 3, E_THISOBJ, E_SEP, E_STR("channels"), E_MIN, E_NUM(3) );

	JL_CHK( ValueToTexture(cx, JL_ARG(1), &normals) );
	JL_ASSERT( normals->channels == 3, E_ARG, E_NUM(1), E_FORMAT, E_COMMENT("channels") );
	JL_ASSERT( normals->width == tex->width && normals->height == tex->height, E_ARG, E_NUM(1), E_FORMAT, E_COMMENT("width, height") );

	Vector3 lightPos;
//	JL_CHK( FloatArrayToVector(cx, 3, &JL_ARG(2), lightPos.raw ) );
	uint32_t length;
	JL_CHK( jl::getVector(cx, JL_ARG(2), lightPos.raw, 3, &length) );
	JL_ASSERT( length == 3, E_ARG, E_NUM(2), E_SEP, E_ARRAYLENGTH, E_EQUALS, E_NUM(3) );

	PTYPE ambient[3];
	JL_CHK( InitLevelData(cx, JL_ARG(3), 3, ambient) );
	bool ambiantTexture;
	ambiantTexture = false;

	PTYPE diffuse[3];
	JL_CHK( InitLevelData(cx, JL_ARG(4), 3, diffuse) );

	PTYPE specular[3];
	JL_CHK( InitLevelData(cx, JL_ARG(5), 3, specular) );
	bool specularTexture;
	specularTexture = false;

	PTYPE bumpPower; // (TBD) default value
	if ( argc >= 6 )
		JL_CHK( jl::getValue(cx, JL_ARG(6), &bumpPower) );
	else
		bumpPower = 1;

	PTYPE specularPower; // (TBD) default value
	if ( argc >= 7 )
		JL_CHK( jl::getValue(cx, JL_ARG(7), &specularPower) );
	else
		specularPower = 1;

	Vector3Normalize(&lightPos, &lightPos);
	Vector3 halfV;
	halfV.x = lightPos.x;
	halfV.y = lightPos.y;
	halfV.z = lightPos.z;

	halfV.z += 1;
	Vector3Normalize(&halfV, &halfV);
	Vector3 n;

	int x, y, c;
	int pos;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			pos = (x + y * width) * 3; // normal texture is only 3 channels
			Vector3Set(&n, PZNORM(normals->cbuffer[pos+0]), PZNORM(normals->cbuffer[pos+1]), PZNORM(normals->cbuffer[pos+2]) / bumpPower );
			Vector3Normalize(&n, &n);

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
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( t1, t2 )
  Among the three textures (this, t1, t2), process each $pval by making the average of both that are the nearest.

**/
DEFINE_FUNCTION( nr ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC( 2 );

	TextureStruct *tex, *t1, *t2;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	JL_CHK( ValueToTexture(cx, JL_ARG(1), &t1) );
	JL_CHK( ValueToTexture(cx, JL_ARG(2), &t2) );

	JL_ASSERT( TextureSameFormat(t1, tex), E_ARG, E_NUM(1), E_FORMAT );
	JL_ASSERT( TextureSameFormat(t2, tex), E_ARG, E_NUM(2), E_FORMAT );

	PTYPE *pos, *pos1, *pos2,  d1, d2, d3;
	unsigned int i, size;
	size = tex->width * tex->height * tex->channels;
	
	pos = tex->cbuffer;
	pos1 = t1->cbuffer;
	pos2 = t2->cbuffer;

	for ( i = 0; i < size; ++i ) {

/*
// method 1
		d1 = abs(*pos - *pos1);
		d2 = abs(*pos1 - *pos2);
		d3 = abs(*pos2 - *pos);

		if ( d2 > d1 && d3 > d1 )
			*pos = ( *pos + *pos1 ) / 2.; // *pos2 is the most different level
		else
			if ( d1 > d3 && d2 > d3 )
				*pos = ( *pos + *pos2 ) / 2.; // *pos1 is the most different level
			else
				*pos = ( *pos1 + *pos2 ) / 2.; // *pos is the most different level
*/

// method 2
		PTYPE m = ( *pos + *pos1 + *pos2 ) / PTYPE(3);

		d1 = abs(*pos - m);
		d2 = abs(*pos1 - m);
		d3 = abs(*pos2 - m);
		if ( d1 < d2 && d1 < d3 )
//			*pos = *pos;
			;
		else
			if ( d2 < d1 && d2 < d3 )
				*pos = *pos1;
			else
				*pos = *pos2;

		++pos;
		++pos1;
		++pos2;
	}

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( x0, y0, x1, y1 )
  Remove the part of the texture that is outside the rectangle (x1,y1)-(x2,y2).
**/
DEFINE_FUNCTION( trim ) { // (TBD) test this new version that use jl::memcpy

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 4 );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	int x0, y0, x1, y1;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &x0) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &y0) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &x1) );
	JL_CHK( jl::getValue(cx, JL_ARG(4), &y1) );

	unsigned int channels, width, height, newWidth, newHeight;
	channels = tex->channels;
	width = tex->width;
	height = tex->height;

	JL_ASSERT( x0 <= x1, E_ARG, E_NUM(1), E_MIN, E_ARG, E_NUM(3) );
	JL_ASSERT( x0 <= x1, E_ARG, E_NUM(2), E_MIN, E_ARG, E_NUM(4) );
	
	newWidth = x1 - x0;
	newHeight = y1 - y0;

	unsigned int srcLineLength, dstLineLength, y;
	srcLineLength = width * channels * sizeof(PTYPE);
	dstLineLength = newWidth * channels * sizeof(PTYPE);

	JL_CHK( TextureResizeBackBuffer(tex, newHeight * dstLineLength) );

	char *pSrc, *pDst;
	pSrc = (char*)tex->cbuffer + ( x0 + y0 * width ) * channels * sizeof(PTYPE);
	pDst = (char*)tex->cbackBuffer;

	for ( y = 0; y < newHeight; y++ ) {

		jl::memcpy( pDst, pSrc, dstLineLength);
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
	tex->width = newWidth;
	tex->height = newHeight;

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( sourceTexture, x, y [ , borderMode = Texture.borderClamp] )
  Copy _sourceTexture_ in the current texture at the position (_x_, _y_).
  $H arguments
   $ARG Texture sourceTexture:
   $ARG $INT x:
   $ARG $INT y:
   $ARG $ENUM borderMode: one of Texture.borderWrap or Texture.borderClamp.
**/
DEFINE_FUNCTION( copy ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 3 );

	TextureStruct *srcTex, *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	JL_CHK( ValueToTexture(cx, JL_ARG(1), &srcTex) );

	JL_ASSERT( tex->channels == srcTex->channels, E_ARG, E_NUM(1), E_FORMAT, E_COMMENT("channels") );

	int px, py; // position
	JL_CHK( jl::getValue(cx, JL_ARG(2), &px) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &py) );

	BorderMode borderMode;
	JL_CHK( JL_JsvalToBorderMode(cx, JL_SARG(4), &borderMode) );

	int channels;
	channels = tex->channels;

	int texWidth;
	texWidth = tex->width;
	int texHeight;
	texHeight = tex->height;

	int srcTexWidth;
	srcTexWidth = srcTex->width;
	int srcTexHeight;
	srcTexHeight = srcTex->height;

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
				JL_ERR( E_ARG, E_NUM(4), E_INVALID );
			}

			posDst = ( x + y * texWidth ) * channels;
			posSrc = ( sx + sy * srcTexWidth ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[posDst+c] = srcTex->cbuffer[posSrc+c];
		}

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( texture, x, y, borderMode )
  Paste _sourceTexture_ in the current texture at the position (_x_, _y_).
  $H arguments
   $ARG Texture sourceTexture:
   $ARG $INT x:
   $ARG $INT y:
   $ARG $ENUM borderMode: one of Texture.borderWrap or Texture.borderClamp.
**/
DEFINE_FUNCTION( paste ) { // (Texture)texture, (int)x, (int)y, (bool)borderMode

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 4 );

	TextureStruct *tex1, *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	JL_CHK( ValueToTexture(cx, JL_ARG(1), &tex1) );
	JL_ASSERT( tex->channels == tex1->channels, E_ARG, E_NUM(1), E_FORMAT, E_COMMENT("channels") );

	int px, py; // position
	JL_CHK( jl::getValue(cx, JL_ARG(2), &px) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &py) );

	BorderMode borderMode;
	JL_CHK( JL_JsvalToBorderMode(cx, JL_SARG(4), &borderMode) );

	int channels;
	channels = tex->channels;

	int texWidth;
	texWidth = tex->width;
	int texHeight;
	texHeight = tex->height;

	int tex1Width;
	tex1Width = tex1->width;
	int tex1Height;
	tex1Height = tex1->height;

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
				JL_ERR( E_ARG, E_NUM(4), E_INVALID );
			}

			posDst = ( dx + dy * texWidth ) * channels;
			posSrc = ( x + y * tex1Width ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[posDst+c] = tex1->cbuffer[posSrc+c];
		}

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE ImageObject $INAME( [ x, y, width, height] )
  Creates an image object from the whole or a part of current texture.
  $H arguments
   $ARG $INT x:
   $ARG $INT y:
   $ARG $INT width:
   $ARG $INT height:
  $H return value
   An image object.
  $H example
  {{{
  var f = new Font('arial.ttf');
  f.size = 100;
  f.verticalPadding = -16;
  var img = f.drawString('Hello world', true);

  var t = new Texture(img);
  var t1 = new Texture(t);

  t.boxBlur(10,10);
  t1.oppositeLevels();
  t.add(t1);
  t.oppositeLevels();
  t.add(1);

  new File('text.png').content = EncodePngImage(t.Export());
  }}}
**/
DEFINE_FUNCTION( export ) { // (int)x, (int)y, (int)width, (int)height. Returns a Blob

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	unsigned int sChannels, sWidth, sHeight;
	sChannels = tex->channels;
	sWidth = tex->width;
	sHeight = tex->height;

	unsigned int dWidth, dHeight;

	uint8_t *buffer;
	int bufferLength;

	unsigned int px, py;

	if ( JL_ARGC == 0 ) { // copy the whole image

		px = 0;
		py = 0;
		dWidth = sWidth;
		dHeight = sHeight;
	} else {

		JL_ASSERT_ARGC_MIN( 4 );
		JL_CHK( jl::getValue(cx, JL_ARG(1), &px) );
		JL_CHK( jl::getValue(cx, JL_ARG(2), &py) );
		JL_CHK( jl::getValue(cx, JL_ARG(3), &dWidth) );
		JL_CHK( jl::getValue(cx, JL_ARG(4), &dHeight) );
	}

	bufferLength = dWidth * dHeight * sChannels;
	//buffer = (uint8_t*)jl_malloc(bufferLength +1);
	buffer = JL_NewImageObject(cx, dWidth, dHeight, sChannels, TYPE_UINT8, *JL_RVAL);
	JL_CHK( buffer );

	unsigned int c, x, y, posDst, posSrc;
	int sx, sy; // position in source

	for ( y = 0; y < dHeight; y++ )
		for ( x = 0; x < dWidth; x++ ) {

			sx = x + px;
			sy = y + py;

			posDst = ( x + y * dWidth ) * sChannels;
			posSrc = ( sx + sy * sWidth ) * sChannels;
			for ( c = 0; c < sChannels; c++ )
				buffer[posDst+c] = (uint8_t)(MINMAX(tex->cbuffer[posSrc+c] * 255.f, 0, 255)); // map [0.0 -> 1.0] to [0 -> 255]
		}

	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( sourceImage, [x], [y] [ , borderMode] )
  Draws the _image_ over the current texture at position (_x_, _y_).
  $H arguments
   $ARG ImageObject sourceImage:
   $ARG $INT x: X position of the image in the current texture.
   $ARG $INT y: Y position of the image in the current texture.
   $ARG $ENUM borderMode: one of Texture.borderWrap, Texture.borderClamp.
  $H example
  {{{
  var file = new File('myImage.png').open('r'); // note: Open() returns the file object.
  var image = decodePngImage( file );
  file.close();
  texture.import(image, 0, 0);

  Ogl.matrixMode(MODELVIEW);
  Ogl.defineTextureImage(TEXTURE_2D, undefined, texture);
  Ogl.loadIdentity();
  ...
  }}}
**/
DEFINE_FUNCTION( import ) { // (Blob)image, (int)x, (int)y

	JLData data;

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(1,4);
	JL_ASSERT_ARG_IS_OBJECT(1);

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	JSObject *bstr;
	bstr = &JL_ARG(1).toObject();
	//JL_ASSERT_INSTANCE( bstr, JL_BlobJSClass(cx) ); // (TBD) String object should also work.

	int px, py;
	if ( JL_ARG_ISDEF(2) && JL_ARG_ISDEF(3) ) {
		
		JL_CHK( jl::getValue(cx, JL_ARG(2), &px) );
		JL_CHK( jl::getValue(cx, JL_ARG(3), &py) );
	} else {
		
		px = 0;
		py = 0;
	}

	int dWidth, dHeight, dChannels;
	int sWidth, sHeight, sChannels;
	ImageDataType dataType;

	dWidth = tex->width;
	dHeight = tex->height;
	dChannels = tex->channels;

	data = JL_GetImageObject(cx, JL_ARG(1), &sWidth, &sHeight, &sChannels, &dataType);
	JL_ASSERT( data.IsSet(), E_ARG, E_NUM(1), E_INVALID );
	JL_ASSERT( dataType == TYPE_UINT8, E_ARG, E_NUM(1), E_DATATYPE, E_INVALID );
	const uint8_t *buffer = (const uint8_t*)data.GetConstStr();

	if ( dWidth == sWidth && dHeight == sHeight && dChannels == sChannels && px == 0 && py == 0 ) { // optimization
		
		PTYPE *dPos = tex->cbuffer;
		const uint8_t *sPos = buffer;
		unsigned int count = data.Length();

		for ( unsigned int i = 0; i < count; i++ ) {
			
			*dPos = (PTYPE)*sPos / (PTYPE)255.f;
			dPos++;
			sPos++;
		}
		return true;			
	}

	BorderMode borderMode;
	JL_CHK( JL_JsvalToBorderMode(cx, JL_SARG(4), &borderMode) );

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
					if ( !(dx >= 0 && dx < dWidth && dy >= 0 && dy < dHeight) )
						continue; // skip
				break;
			default:
				JL_ERR( E_ARG, E_NUM(4), E_INVALID );
			}

			posDst = ( dx + dy * dWidth ) * dChannels;
			posSrc = ( x + y * sWidth ) * sChannels;
			for ( c = 0; c < sChannels; c++ )
				tex->cbuffer[posDst+c] = (PTYPE)buffer[posSrc+c] / (PTYPE)255.f;
		}

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( offsetX, offsetY [ , borderMode] )
  Shift the current image.
  $H arguments
   $ARG $INT offsetX:
   $ARG $INT offsetY:
   $ARG $ENUM borderMode: one of Texture.borderWrap, Texture.borderClamp, Texture.borderMirror or Texture.borderValue.
**/
DEFINE_FUNCTION( shift ) {
	// (TBD) I think it is possible to do the Shift operation without using a second buffer.

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(2, 3);

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	int offsetX, offsetY;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &offsetX) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &offsetY) );

	BorderMode borderMode;
	JL_CHK( JL_JsvalToBorderMode(cx, JL_SARG(3), &borderMode) );

	unsigned int width, height, channels;
	width = tex->width;
	height = tex->height;
	channels = tex->channels;

	TextureSetupBackBuffer(tex);

	PTYPE *sPos, *dPos;
	unsigned int x, y, c; // destination image x, y
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			sPos = PosByMode(tex, x + offsetX, y + offsetY, borderMode);
			dPos = &tex->cbackBuffer[(x + y * width) * channels];
			for ( c = 0; c < channels; c++ )
				dPos[c] = sPos[c];
		}
	TextureSwapBuffers(tex);
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( displaceTexture, factor [ , borderMode] )
  Move each pixel of the texture according to the color of _displaceTexture_ texture. only the two first channels are used as displacement source. 
  $H arguments
   $ARG Texture displaceTexture: is a texture that contains displacement vectors.
   $ARG $REAL factor: displacement factor of each pixel.
   $ARG $ENUM borderMode: one of Texture.borderWrap, Texture.borderClamp, Texture.borderMirror or Texture.borderValue.
**/
// (TBD) PTYPE
DEFINE_FUNCTION( displace ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(2, 3);

	TextureStruct *tex1, *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	unsigned int width, height, channels;
	width = tex->width;
	height = tex->height;
	channels = tex->channels;

	JL_CHK( ValueToTexture(cx, JL_ARG(1), &tex1) );

	int displaceChannels;
	displaceChannels = tex1->channels;

	JL_ASSERT( width == tex1->width && height == tex1->height, E_ARG, E_NUM(1), E_FORMAT, E_COMMENT("width, height") );

//	JL_ASSERT( displaceChannels >= 2, "Displacement texture must have 2 or more channels." );

	PTYPE factor;
	if ( argc >= 2 )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &factor) );
	else
		factor = PTYPE(1);

	TextureSetupBackBuffer(tex);

	BorderMode borderMode;
	JL_CHK( JL_JsvalToBorderMode(cx, JL_SARG(3), &borderMode) );

	unsigned int x, y, c;
	int sx, sy; // source position
	int pos;
	PTYPE *sPos, *dPos;

//	Vector3 o;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			pos = (x + y * width) * displaceChannels;

//			Vector3Set(&o, PZNORM( tex1->cbuffer[pos+0] ), PZNORM( tex1->cbuffer[pos+1] ), PZNORM( tex1->cbuffer[pos+2] ) );
//			Vector3Normalize(&o);
//			sx = (int)( x + o.x * factor );
//			sy = (int)( y + o.y * factor );

			// (TBD) test !

			sx = x + int(PZNORM( tex1->cbuffer[pos+0] ) * factor);
			sy = y + int(PZNORM( tex1->cbuffer[pos+1] ) * factor);

			sPos = PosByMode(tex, sx, sy, borderMode);
			dPos = &tex->cbackBuffer[(x + y * width) * channels];
			if ( sPos != NULL )
				for ( c = 0; c < channels; c++ )
					dPos[c] = sPos[c];
		}
	TextureSwapBuffers(tex);
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( density, regularity )
  Draws cells (Worley noise) in the current texture.
  $H arguments
   $ARG $INT density:
   $ARG $REAL regularity:
**/
DEFINE_FUNCTION( cells ) { // source: FxGen

	struct Point {
		float x, y;
	};

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 2 );

	// http://petewarden.com/notes/archives/2005/05/testing.html


	int density;
	PTYPE regularity;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &density) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &regularity) );

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	density = MINMAX( density, 1, 128 );
	regularity = MINMAX( regularity, 0, 1 );

	int width;
	width = tex->width;
	int height;
	height = tex->height;
	int c, channels;
	channels = tex->channels;

	int size;
	size = width * height;
	PTYPE* pPxDst;
	pPxDst = tex->cbuffer;
	int count;
	count = density * density;
	Point *cellPoints;
	cellPoints = (Point*)alloca(sizeof(Point)*count);

	for ( int i = 0; i < count; i++ ) {

		float rand1 = PRAND; // [0,1]-real-interval
		float rand2 = PRAND; // [0,1]-real-interval
		int x = i % density;
		int y = i / density;

		cellPoints[i].x = ( x + PTYPE(0.5) + (rand1 - PTYPE(0.5)) * (PTYPE(1) - regularity) ) / density;
		cellPoints[i].y = ( y + PTYPE(0.5) + (rand2 - PTYPE(0.5)) * (PTYPE(1) - regularity) ) / density;
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

	//jl_free(cellPoints); // alloca

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( topLeft, topRight, bottomLeft, bottomRight )
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

  texture.set(0); // clears the texture
  texture.addGradiantQuad(BLACK, RED, BLUE, BLACK);
  }}}
**/
DEFINE_FUNCTION( addGradiantQuad ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(4);

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	unsigned int width, height, channels;
	width = tex->width;
	height = tex->height;
	channels = tex->channels;

	PTYPE pixel1[PMAXCHANNELS], pixel2[PMAXCHANNELS], pixel3[PMAXCHANNELS], pixel4[PMAXCHANNELS];
	JL_CHK( InitLevelData(cx, JL_ARG(1), channels, pixel1) );
	JL_CHK( InitLevelData(cx, JL_ARG(2), channels, pixel2) );
	JL_CHK( InitLevelData(cx, JL_ARG(3), channels, pixel3) );
	JL_CHK( InitLevelData(cx, JL_ARG(4), channels, pixel4) );

	PTYPE aspectRatio;
	aspectRatio = PTYPE(width * height);

	float r1, r2, r3, r4;
	unsigned int x, y, c, pos;
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
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( curveInfoX, curveInfoY )
  Add a linear radiant using a curve for X and Y. Each point of the curve is the light intensity of a pixel.
  $H arguments
   $ARG curveInfo curveInfoX:
   $ARG curveInfo curveInfoY:
  $H example 1
  {{{
  const curveHalfSine = function(v) Math.cos(v*Math.PI/2);
  const curveOne = function() 1;

  texture.set(0); // clears the texture
  texture.addGradiantLinear(curveHalfSine, curveOne);
  }}}
  $H example 2
  {{{
  texture.addGradiantLinear([0,1,0], [0,1,0]);
  }}}
**/
DEFINE_FUNCTION( addGradiantLinear ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(2);

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	unsigned int width, height, channels;
	width = tex->width;
	height = tex->height;
	channels = tex->channels;

	float *curvex, *curvey;
	curvex = (float*)alloca( width * sizeof(float) );
	JL_CHK( InitCurveData( cx, JL_ARG(1), width, curvex ) );
	curvey = (float*)alloca( height * sizeof(float) );
	JL_CHK( InitCurveData( cx, JL_ARG(2), height, curvey ) );

	unsigned int x, y, c, pos;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			pos = ( x + y * width ) * channels;
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[pos+c] += curvex[x] * curvey[y];
		}

	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	//jl_free(curvex); // alloca
	//jl_free(curvey); // alloca
	return true;

bad:
	return false;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( curveInfo [ , drawToCorner = false] )
  Add a radial radiant using a curve from the center to the outside. Each point of the curve is the light intensity of a pixel.
  $H arguments
   $ARG curveInfo curveInfo:
   $ARG $BOOL drawToCorner: If true, the curve goes from the center to the corner, else from the center to the edge.
  $H example 1
  {{{
  const curveHalfSine = function(v) Math.cos(v*Math.PI/2);

  texture.addGradiantRadial(curveHalfSine);
  }}}
  $H example 2
  {{{
  texture.addGradiantRadial([0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1], true);
  }}}
**/
DEFINE_FUNCTION( addGradiantRadial ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(1);

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	int width, height, channels;
	width = tex->width;
	height = tex->height;
	channels = tex->channels;

	bool drawToCorner;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &drawToCorner) );
	else
		drawToCorner = false;

	PTYPE radius;
	if ( drawToCorner )
		radius = Length2D( PTYPE(width), PTYPE(height) ) / PTYPE(2);
	else
		radius = MAX( width, height ) / PTYPE(2);

	float *curve;
	curve = (float*)alloca( (int)radius * sizeof(float) );
	JL_CHK( InitCurveData(cx, JL_ARG(1), (int)radius, curve) );

	float aspectRatio;
	aspectRatio = (float)width / (float)height;

	// draw
	float dist;
	int x, y, c;
	int pos, curvePos;
	float curveValue;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ ) {

			dist = Length2D( (float)x / width - 0.5f, (float)y / height - 0.5f ); // distance to the center ( 0..M_SQRT1_2 )

			if ( drawToCorner ) {

				pos = (x + y * width) * channels; // (TBD) use borderMode
				curvePos = int(dist * (radius - PTYPE(1)) / PTYPE(M_SQRT1_2));
				curveValue = curve[curvePos];
				for ( c = 0; c < channels; c++ )
					tex->cbuffer[pos+c] += curveValue;
			} else
			if ( dist <= 0.5 ) { // if dist == 0.5, (int)(dist*radius) is out of the curve data

				pos = (x + y * width) * channels; // (TBD) use borderMode
				curvePos = int(dist * (radius - PTYPE(1)) * PTYPE(2));
				curveValue = curve[curvePos];
				for ( c = 0; c < channels; c++ )
					tex->cbuffer[pos+c] += curveValue;
			}
		}

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}



/*
DEFINE_FUNCTION( addGradiantRadial ) {

	JL_ASSERT_ARGC_MIN( 3 );

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	int width = tex->width;
	int height = tex->height;
	int channels = tex->channels;

	int ox, oy;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &ox) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &oy) );

	int radius;
	JL_CHK( jl::getValue(cx, JL_ARG(3), &radius) );

	BorderMode borderMode = borderWrap;

	float *curve = (float*)jl_malloc( radius * sizeof(float) ); // JS_malloc(cx,
	InitCurveData( cx, JL_ARG(4), radius, curve );

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

	*JL_RVAL = OBJECT_TO_JSVAL(JL_OBJ);
	return true;
	JL_BAD;
}
*/


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( count, crackLength, wayVariation [ , color = 1] [ , curve = 1] )
  Adds cracks to the current texture.
  $H arguments
   $ARG $INT count: number of cracks to draw.
   $ARG $INT crackLength: length of each crack.
   $ARG $REAL wayVariation: the variation of each crack (eg. 0 is straight and 1 is randomly curved).
   $ARG colorInfo color: the color of the crack.
   $ARG curveInfo curve: the curve that defines the intensity of each point of the crack.
  $H note
   The curve is computed before each crack is drawn.
  $H beware
   Adding cracks on a white texture does nothing.
  $H examples
  {{{
  texture.addCracks( 1000, 10, 0, RED, 1 );

  texture.addCracks( 100, 100, 0, 1, [1,0,1,0,1,0,1,0,1] );

  texture.addCracks( 10, 10000, 0.1, 1, curveLinear );

  texture.addCracks( 10, 10000, 0.1, 1, function(v) Texture.randReal() );
  }}}
**/
DEFINE_FUNCTION( addCracks ) { // source: FxGen

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(2);

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	int width;
	width = tex->width;
	int height;
	height = tex->height;
	int channels;
	channels = tex->channels;

	int count;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &count) );

	int crackMaxLength;
	JL_CHK( jl::getValue(cx, JL_ARG(2), &crackMaxLength) );

	double variation;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( jl::getValue(cx, JL_ARG(3), &variation) );
	else
		variation = 0;

	PTYPE pixel[PMAXCHANNELS];
	if ( JL_ARG_ISDEF(4) ) {

		JL_CHK( InitLevelData(cx, JL_ARG(4), channels, pixel) );
	} else {
		for ( int i=0; i<PMAXCHANNELS; i++ )
			pixel[i] = PMAX;
	}


	float *curve;
	curve = (float*)alloca( crackMaxLength * sizeof(float) );
	if ( JL_ARG_ISDEF(5) ) {

		JL_CHK( InitCurveData( cx, JL_ARG(5), crackMaxLength, curve ) );
	} else {
		for ( int i=0; i<crackMaxLength; i++ )
			curve[i] = PMAX;
	}

	// draw
	int pos, c;
	float curveValue;
	//Process operator
	int n;
	n = 0;
	while( n++ < count ) {

		float x = PRAND * width;
		float y = PRAND * height;
		float a = 2.f * float(M_PI) * PRAND;
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

			a = a + float(variation*(2 * genrand_real1() - 1));
		}
	}
	//jl_free(curve); // alloca
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( $TYPE matrix )
**/
DEFINE_FUNCTION( applyColorMatrix ) {

	// 4x4 matrix for linear transformations, 5x5 matrix for non-linear transformations
	// Color Transformations and the Color Matrix: http://www.c-sharpcorner.com/UploadFile/mahesh/Transformations0512192005050129AM/Transformations05.aspx
	// Matrix Operations for Image Processing: http://www.graficaobscura.com/matrix/index.html
	// Fun with the colormatrix: http://hirntier.blogspot.com/2008/09/fun-with-colormatrix.html

	JL_DEFINE_ARGS;
		JL_ASSERT_ARGC(1);
	JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	JL_ASSERT( tex->channels == 4, E_THISOBJ, E_FORMAT, E_COMMENT("channels") );
	
	Matrix44 colorMatrixTmp, *colorMatrix;
	colorMatrix = &colorMatrixTmp;
	JL_CHK( jl::getMatrix44(cx, JL_ARG(1), (float**)&colorMatrix) );

	Vector4 tmp;
	PTYPE *end;
	end = tex->cbuffer + tex->width * tex->height * 4;
	for ( float *pos = tex->cbuffer; pos < end; pos += 4 ) {

		Vector4LoadFromPtr(&tmp, pos);
		Matrix44MultVector4(colorMatrix, &tmp, &tmp);
		Vector4LoadToPtr(&tmp, pos);
	}
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( pos, dirX, dirY [ , alpha = 1 ] )
  $H arguments
   $ARG vec3 pos: position vector [x,y,z].
   $ARG vec3 dirX: direction vector [x,y,z] for X axis.
   $ARG vec3 dirY: direction vector [x,y,z] for Y axis.
  $H example
{{{
texture.addPerlin2([0,0,0], [1,0,0], [0,1,0]);
}}}
**/
DEFINE_FUNCTION( addPerlin2 ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(3,4);

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
//	JL_ASSERT( tex->channels == 4, "Invalid channel count." );

	double x,y,z, offset[3], dirX[3], dirY[3], alpha;

	uint32_t len;
	JL_CHK( jl::getVector(cx, JL_ARG(1), offset, 3, &len) );
	JL_ASSERT( len == 3, E_ARG, E_NUM(1), E_SEP, E_ARRAYLENGTH, E_EQUALS, E_NUM(3) );

	JL_CHK( jl::getVector(cx, JL_ARG(2), dirX, 3, &len) );
	JL_ASSERT( len == 3, E_ARG, E_NUM(2), E_SEP, E_ARRAYLENGTH, E_EQUALS, E_NUM(3) );

	JL_CHK( jl::getVector(cx, JL_ARG(3), dirY, 3, &len) );
	JL_ASSERT( len == 3, E_ARG, E_NUM(3), E_SEP, E_ARRAYLENGTH, E_EQUALS, E_NUM(3) );

	if ( JL_ARG_ISDEF(4) )
		JL_CHK( jl::getValue(cx, JL_ARG(4), &alpha) );
	else
		alpha = 1;

	dirX[0] /= tex->width;
	dirX[1] /= tex->width;
	dirX[2] /= tex->width;

	dirY[0] /= tex->height;
	dirY[1] /= tex->height;
	dirY[2] /= tex->height;

	unsigned int pos, tx, ty;
	pos = 0;
	for ( ty = 0; ty < tex->height; ++ty )
		for ( tx = 0; tx < tex->width; ++tx ) {

			x = offset[0] + dirX[0]*tx + dirY[0]*ty;
			y = offset[1] + dirX[1]*tx + dirY[1]*ty;
			z = offset[2] + dirX[2]*tx + dirY[2]*ty;

			tex->cbuffer[pos] += PTYPE(PerlinNoise2(x, y, z) * alpha);
			pos += tex->channels;
		}

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( x0, y0, x1, y1, color )
  Draws a rectangle of the _colorInfo_ color over the current texture.
  $H arguments
   $ARG $INT x0:
   $ARG $INT y0:
   $ARG $INT x1:
   $ARG $INT y1:
   $ARG colorInfo color:
**/
// PTYPE ok
DEFINE_FUNCTION( setRectangle ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(5);

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	int x0, y0, x1, y1;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &x0) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &y0) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &x1) );
	JL_CHK( jl::getValue(cx, JL_ARG(4), &y1) );

	PTYPE pixel[PMAXCHANNELS];
	JL_CHK( InitLevelData(cx, JL_ARG(5), tex->channels, pixel) );

	unsigned int width, height, channels, c, pos;
	int x, y;
	channels = tex->channels;
	width = tex->width;
	height = tex->height;

	for ( y = y0; y < y1; y++ )
		for ( x = x0; x < x1; x++ ) {

			pos = ( Wrap(x, width) + Wrap(y, height) * width ) * channels;
//			sPos = PosByMode(tex, x, y, mode);
			for ( c = 0; c < channels; c++ )
				tex->cbuffer[pos+c] = pixel[c];
		}

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( x, y, colorInfo [ , borderMode ] )
  Sets the color of the given pixel.
  $H note
   If x and y are wrapped to the image width and height.
**/
// PTYPE ok
DEFINE_FUNCTION( setPixel ) { // x, y, levels

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(3, 4);

	TextureStruct *tex;
	tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	int x, y;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &x) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &y) );

	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	BorderMode borderMode;
	JL_CHK( JL_JsvalToBorderMode(cx, JL_SARG(4), &borderMode) );

	PTYPE *pos;
	pos = PosByMode(tex, x, y, borderMode);
	if (likely( pos != NULL ))
		return InitLevelData(cx, JL_ARG(3), tex->channels, pos);

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Array $INAME( x, y [ , borderMode] )
  Read the value of a pixel in the current texture.
  $H arguments
   $ARG $INT x
   $ARG $INT y
  $H return value
   returns the pixel value at position (x, y) in the current texture. If the texture is RGB, an array of 3 values is returned.
  $H example
  {{{
  var texture = new Texture(20,20,3);
  texture.set([0.1, 0.2, 0.3]);
  var pixel = texture.pixelAt(10,10);
  print( 'Red: '+pixel[0], 'Green: '+pixel[1], 'Blue: '+pixel[2] );
  }}}
**/
DEFINE_FUNCTION( getPixelAt ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(2, 3);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	int sx, sy;
	sx = JL_ARG(1).toInt32();
	sy = JL_ARG(2).toInt32();

	TextureStruct *tex;
	tex = (TextureStruct*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	BorderMode borderMode;
	JL_CHK( JL_JsvalToBorderMode(cx, JL_SARG(3), &borderMode) );

	PTYPE *pos;
	pos = PosByMode(tex, sx, sy, borderMode);
	if (likely( pos != NULL ))
		return jl::setVector(cx, JL_RVAL, pos, tex->channels);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( [ channel ] )
  Returns the average $pval. If _channel_ argument is given, the average is done on that channel only.
**/
DEFINE_FUNCTION( getGlobalLevel ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	unsigned int i, size;
	PTYPE sum;
	sum = 0;

	if (likely( !JL_ARG_ISDEF(1) )) {

		size = tex->width * tex->height * tex->channels;
		for ( i = 0; i < size; ++i )
			sum += tex->cbuffer[i];
		JL_CHK( JL_NativeToJsval(cx, sum / (PTYPE)size, *JL_RVAL) );
		return true;
	}

	int onlyChannel;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &onlyChannel) );
	PTYPE *pos;
	pos = &tex->cbuffer[onlyChannel];

	size = tex->width * tex->height;
	for ( i = 0; i < size; ++i ) {
		
		sum += *pos;
		pos += tex->channels;
	}
	JL_CHK( JL_NativeToJsval(cx, sum / (PTYPE)size, *JL_RVAL) );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( [ channel ] )
  Returns the [ lowest,highest ] level value of the texture. If _channel_ argument is given, the average is done on that channel only.
**/
// PTYPE ok
DEFINE_FUNCTION( getLevelRange ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	unsigned int size, i;
	PTYPE min, max, tmp, *pos;
	min = PMAXLIMIT;
	max = PMINLIMIT;

	if (likely( !JL_ARG_ISDEF(1) )) {

		pos = tex->cbuffer;
		size = tex->width * tex->height * tex->channels;
		for ( i = 0; i < size; ++i ) {

			tmp = *pos;
			if ( tmp > max )
				max = tmp;
			else if ( tmp < min )
				min = tmp;
			pos++;
		}
	} else {

		int onlyChannel;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &onlyChannel) );
		
		pos = &tex->cbuffer[onlyChannel];
		size = tex->width * tex->height;
		for ( i = 0; i < size; ++i ) {

			tmp = *pos;
			if ( tmp > max )
				max = tmp;
			else if ( tmp < min )
				min = tmp;
			pos += tex->channels;
		}
	}

	double vector[2]; // = { min, max };
	vector[0] = min;
	vector[1] = max;

	return jl::setVector(cx, JL_RVAL, vector, 2);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME()
  Returns the [ lowest, highest ] level value of the border of the texture.
**/
// PTYPE ok
DEFINE_FUNCTION( getBorderLevelRange ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(tex);

	PTYPE min, max, tmp;
	min = PMAXLIMIT;
	max = PMINLIMIT;

	unsigned int i, c, pos, lineSize, imageSize;
	lineSize = tex->width * tex->channels;
	imageSize = lineSize * tex->height;

	for ( i = 0; i < lineSize; i++ ) {

		tmp = tex->cbuffer[i];
		if ( tmp > max )
			max = tmp;
		else if ( tmp < min )
			min = tmp;
	}
	
	for ( i = imageSize - lineSize - 1; i < imageSize; i++ ) {

		tmp = tex->cbuffer[i];
		if ( tmp > max )
			max = tmp;
		else if ( tmp < min )
			min = tmp;
	}

	for ( c = 0; c < tex->channels; c++ ) {
		
		pos = c;
		for ( i = 0; i < tex->height; i++ ) {
		
			tmp = tex->cbuffer[pos];
			if ( tmp > max )
				max = tmp;
			else if ( tmp < min )
				min = tmp;
			pos += lineSize;
		}
	}

	for ( c = 0; c < tex->channels; c++ ) {
		
		pos = lineSize - c - 1;
		for ( i = 0; i < tex->height; i++ ) {
		
			tmp = tex->cbuffer[pos];
			if ( tmp > max )
				max = tmp;
			else if ( tmp < min )
				min = tmp;
			pos += lineSize;
		}
	}

	double vector[2]; // = { min, max };
	vector[0] = min;
	vector[1] = max;

	return jl::setVector(cx, JL_RVAL, vector, 2);
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME
  Is the higher $pval. Higher values are not brighter than this one.
  $LF
  See Normalize() function.
**/
DEFINE_PROPERTY_GETTER( vmax ) {
	
	vp.setNumber(PMAX);
	return jl::StoreProperty(cx, obj, id, vp, true);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Width of the texture in pixel.
**/
DEFINE_PROPERTY_GETTER( width ) {

	JL_IGNORE(id);

	JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	return JL_NativeToJsval(cx, tex->width, vp);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Height of the texture in pixel.
**/
DEFINE_PROPERTY_GETTER( height ) {

	JL_IGNORE(id);

	JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	return JL_NativeToJsval(cx, tex->height, vp);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Number of channels of the texture.
**/
DEFINE_PROPERTY_GETTER( channels ) {

	JL_IGNORE(id);

	JL_ASSERT_THIS_INSTANCE();

	TextureStruct *tex = (TextureStruct *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(tex);
	return JL_NativeToJsval(cx, tex->channels, vp);
	JL_BAD;
}


/**doc
=== Static functions ===
**/


//DEFINE_FUNCTION( noise ) {
//
//	JL_ASSERT_ARGC_MIN(1);
//	unsigned long seed;
//	JL_CHK( jl::getValue(cx, JL_ARG(1), &seed) );
//	double d = NoiseInt(seed);
//	JL_CHK( JL_NewNumberValue(cx, d, rval) );
//	return true;
//}


#ifdef _DEBUG
//static bool _Test(JSContext *cx, JSObject *obj, unsigned argc, jsval *argv, jsval *rval) {
DEFINE_FUNCTION( test ) {

	JL_DEFINE_ARGS;

	JL_RVAL.setUndefined();
	return true;
}
#endif // _DEBUG


/**doc
=== Note ===
 Nearly all methods returns _this_ object. This allows to easily chain texture operations.
 $H example
 {{{
 var texture = new Texture(64,64,3);
 texture.set(0).addNoise().addCracks(10, 100, 0, BLUE, 1);
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
  * _Array_: that contains the $pval of each channel (eg. `[ 1, 1, 1 ]` is white in RGB mode).
  * _string_: the string represents an HTML color like $F #1100AA or $F #8800AAFF (depending the number of channels)
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
   * _function( $REAL posX, INT indexX )_: the function is called and must returns values for each curve point.
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
  function gaussianCurveGenerator(c) { return function(x) { return Math.exp( -(x*x)/(2*c*c) ) } }

  texture.addGradiantRadial( gaussianCurveGenerator( 0.5 ) );
  }}}

 * *ImageObject*
  An image object is nothing else that a buffer of data with a width, a height and a channels properties.
**/


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( free )
		FUNCTION( swap )
		FUNCTION( clearChannel )
		FUNCTION( toHLS )
		FUNCTION( toRGB )
		FUNCTION( setChannel )
		FUNCTION( desaturate )
		FUNCTION( colorize )
		FUNCTION( extractColor )
		FUNCTION( setPixel )
		FUNCTION( setRectangle )
		FUNCTION( addNoise )
		FUNCTION( set )
		FUNCTION( add )
		FUNCTION( mult )
		FUNCTION( blend )
		FUNCTION( rotate90 )
		FUNCTION( rotoZoom )
		FUNCTION( resize )
		FUNCTION( trim )
		FUNCTION( copy )
		FUNCTION( paste )
		FUNCTION( export )
		FUNCTION( import )
		FUNCTION( flip )
		FUNCTION( shift )
		FUNCTION( convolution )
		FUNCTION( dilate )
		FUNCTION( forEachPixel )
		FUNCTION( normals )
		FUNCTION( normalizeVectors )
		FUNCTION( light )
		FUNCTION( boxBlur )
		FUNCTION( clampLevels )
		FUNCTION( cutLevels )
		FUNCTION( displace )
		FUNCTION( normalizeLevels )
		FUNCTION( invertLevels )
		FUNCTION( oppositeLevels )
		FUNCTION( powLevels )
		FUNCTION( mirrorLevels )
		FUNCTION( wrapLevels )
		FUNCTION( aliasing )
		FUNCTION( cells )
		FUNCTION( addCracks )
		FUNCTION( addGradiantQuad )
		FUNCTION( addGradiantLinear )
		FUNCTION( addGradiantRadial )
		FUNCTION( getPixelAt )
		FUNCTION( getLevelRange )
		FUNCTION( getBorderLevelRange )
		FUNCTION( getGlobalLevel )
		FUNCTION( nr )
		FUNCTION( applyColorMatrix )
		FUNCTION( addPerlin2 )

		#ifdef _DEBUG
		FUNCTION( test )
		#endif // _DEBUG

	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( vmax )
		PROPERTY_GETTER( width )
		PROPERTY_GETTER( height )
		PROPERTY_GETTER( channels )
	END_PROPERTY_SPEC

	BEGIN_CONST
		CONST_INTEGER_SINGLE( borderClamp )
		CONST_INTEGER_SINGLE( borderWrap )
		CONST_INTEGER_SINGLE( borderMirror )
		CONST_INTEGER_SINGLE( borderValue )

		CONST_INTEGER_SINGLE( desaturateLightness )
		CONST_INTEGER_SINGLE( desaturateSum )
		CONST_INTEGER_SINGLE( desaturateAverage )

	END_CONST

END_CLASS

/**doc
=== Examples ===
$H example 1
 Some utility functions.
{{{
function cloud( size, amp ) {

   var octaves = Math.log(size) / Math.LN2;
   var a = 1, s = 1;
   var cloud = new Texture(s, s, 1);
   cloud.clearChannel();
   while ( octaves-- > 0 ) {

      cloud.addNoise(a);
      a *= amp;
      s *= 2;
      cloud.resize(s, s, false);
      cloud.boxBlur(5, 5);
   }
   cloud.normalizeLevels();
   return cloud;
}

function desaturateLuminosity( tex ) {

   tex.mult([0.2126, 0.7152, 0.0722]);
   var tmp = new Texture(tex.width, tex.height, 1).desaturate(tex, Texture.desaturateSum);
   tex.swap(tmp);
   tmp.free();
}

function addAlphaChannel( tex ) {

   if ( tex.channels == 1 )
      new Texture(tex.width, tex.height, 2).setChannel(0, tex, 0).swap(tex);
   else if ( tex.channels == 3 )
      new Texture(tex.width, tex.height, 4).setChannel(0, tex, 0).setChannel(1, tex, 1).setChannel(2, tex, 2).swap(tex);
}
}}}

$H example 2
 This example shows how to save a texture to the disk.
{{{
loadModule('jsstd');
loadModule('jsio');
loadModule('jsprotex');
loadModule('jsimage');

var bacteria = new Texture(256,256,3);
bacteria.set(0);
bacteria.addCracks(100, 100, 2, undefined, function(v) v);
bacteria.boxBlur(5,5);
bacteria.mirrorLevels(0.5, false);
bacteria.boxBlur(2,2);

new File('test.png').content = encodePngImage(bacteria.export());
}}}

$H example 3
 This is a complete example that displays a texture in real-time in a OpenGL environment.
{{{
loadModule('jsstd');
loadModule('jsio');
loadModule('jsgraphics');
loadModule('jsprotex');
loadModule('jsimage');

with (Ogl) {

   var texture = new Texture(128, 128, 3);
   texture.set(0);
   // play here for static textures

   function updateTexture(imageIndex) {

      // play here for dynamic textures
      texture.set(0);
      texture.addNoise(1);
   }

   var win = new Window();
   win.open();
   win.createOpenGLContext();
   win.rect = [200,200,800,800];

   function resizeWindow(w, h) {

      Ogl.viewport(0,0,w,h);
      Ogl.matrixMode(PROJECTION);
      Ogl.loadIdentity();
      Ogl.ortho(0,0,10,10, -1, 1);
      render();
   }

   shadeModel(FLAT);
   frontFace(CCW);
   clearColor(0, 0, 0, 0);
   enable(TEXTURE_2D);
   tid = genTexture();
   bindTexture(TEXTURE_2D, tid);
   texParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST); // GL_LINEAR
   texParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
   clear( COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT );

   function render(imageIndex) {

      updateTexture();
      defineTextureImage(TEXTURE_2D, undefined, texture);
      matrixMode(MODELVIEW);
      loadIdentity();
      scale(1, -1, 1);
      color(1,1,1);
      begin(QUADS);
      texCoord( 0, 0 );
      vertex( -1, -1, 0 );
      texCoord( 1, 0, 0 );
      vertex( 1, -1 );
      texCoord( 1, 1 );
      vertex( 1, 1 );
      texCoord( 0, 1 );
      vertex( -1, 1 );
      end();
      win.swapBuffers();
      maybeCollectGarbage();
   }

   win.onsize = resizeWindow;
   var end = false;
   win.onkeydown = function( key, l ) { end = ( key == 0x1B ) }
   while (!end) {

      win.processEvents();
      render();
   }
   win.close();
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
