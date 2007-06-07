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
	
	int seed;
	RT_JSVAL_TO_INT32( argv[0], seed );

	size_t size = tex->width * tex->height;

	for ( size_t i = 0; i < size; i++ ) {

		float noiseValue = IntegerNoise(seed * size + i); // no common noize chunk between two different seed
		tex->buffer[i].r = noiseValue;
		tex->buffer[i].g = noiseValue;
		tex->buffer[i].b = noiseValue;
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

	size_t size = tex->width * tex->height;

	for ( size_t i = 0; i < count; i++ ) {

		size_t pos = ( 1 + 0.5f * IntegerNoise(seed * count + i) ) * size;

		tex->buffer[pos].r = PMAX;
		tex->buffer[pos].g = PMAX;
		tex->buffer[pos].b = PMAX;
	}
	return JS_TRUE;
}

DEFINE_FUNCTION( Convolution ) {

	return JS_TRUE;
}

DEFINE_FUNCTION( Cells ) { // source: FxGen

	Texture *tex = (Texture *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tex);


	ubyte byRegularity;
	ubyte byDensity;
	uword wSeed;
	RGBA col;

	byDensity = byDensity>1?byDensity:1;

	size_t width = tex->width;
	size_t height = tex->height;
	Pixel* pPxDst = tex->buffer;

	//Init
	const float regularity = byRegularity / 255.0f;
	vec3 *cellPoints = (vec3*)NMemAlloc(sizeof(vec3)*byDensity*byDensity);	//###TOFIX### JN: MemAlloc

	//Render
	for (udword y=0; y<byDensity; ++y)
	{
		for (udword x=0; x<byDensity; ++x)
		{
			float rand1 = (float)myRandom() / 65536.0f;
			float rand2 = (float)myRandom() / 65536.0f;
			cellPoints[x+y*byDensity].x = (x+0.5f+(rand1-0.5f)*(1-regularity))/byDensity;
			cellPoints[x+y*byDensity].y = (y+0.5f+(rand2-0.5f)*(1-regularity))/byDensity;
			cellPoints[x+y*byDensity].z = 0;
		}
	}

	//Temporary buffeur to Texture
	for (udword y=0; y<h; y++)
	{
		for (udword x=0; x<w; x++)
		{
      vec3 pixelPos;
      pixelPos.x = x/(float)w,
      pixelPos.y = y/(float)h;
      pixelPos.z = 0;

      float minDist = 10;
      float nextMinDist = minDist;
      int xo = x*byDensity/w;
      int yo = y*byDensity/h;
      for (sdword v=-1; v<2; ++v)
      {
        int vo = ((yo+byDensity+v)%byDensity)*byDensity;
        for (sdword u=-1; u<2; ++u)
        {
          vec3 cellPos = cellPoints[((xo+byDensity+u)%byDensity) + vo];
          if (u==-1 && x*byDensity<w) cellPos.x-=1;
          if (v==-1 && y*byDensity<h) cellPos.y-=1;
          if (u==1 && x*byDensity>=w*(byDensity-1)) cellPos.x+=1;
          if (v==1 && y*byDensity>=h*(byDensity-1)) cellPos.y+=1;
          vec3 tmp;
          float dist = sub(tmp, pixelPos, cellPos).norm ();
          if (dist<minDist) 
          {
            nextMinDist = minDist;
            minDist = dist;
          } else if (dist<nextMinDist)
          {
            nextMinDist = dist;
          }
        }
      }

      minDist = (nextMinDist-minDist)*byDensity;
      if (minDist<0) minDist = 0;
      if (minDist>1) minDist = 1;

			pPxDst->r = (ubyte)(minDist*col.r);
			pPxDst->g = (ubyte)(minDist*col.g);
			pPxDst->b = (ubyte)(minDist*col.b);
			pPxDst->a = 255;

			pPxDst++;
		}
	}

	NMemFree(cellPoints);


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
		FUNCTION_ARGC( Pixels, 2 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY(prop)
	END_PROPERTY_SPEC

	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

END_CLASS
