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

extern "C" unsigned long genrand_int32(void);
extern "C" void init_genrand(unsigned long s);

#define RM 0x7ff // mask
#define R (RM+1)

static unsigned long rnd[R];

void InitNoise() {

	for ( int i = 0; i < R; i++ )
		rnd[i] = genrand_int32();
}

float Noise1DInteger( int x ) {

	unsigned long a = rnd[ x & RM ];
	unsigned long b = rnd[ (x/R+a) & RM ];
	return (float)b / (float)0xffffffff;
}


float Noise2DInteger( int x, int y ) {

	unsigned long a = rnd[ x & RM ];
	unsigned long b = rnd[ (x/R+y+a) & RM ];
	return (float)b / (float)0xffffffff;
}


float Noise3DInteger( int x, int y, int z ) {

	unsigned long a = rnd[ x & RM ];
	unsigned long b = rnd[ (x/R+y+a) & RM ];
	unsigned long c = rnd[ (y/R+z+b) & RM ];
	return (float)c / (float)0xffffffff;
}

float Smooth( float t ) {

	return t * t * (3. - 2. * t); // return t;
}

float Noise1D( float x ) {

	int flx = floor(x);
	float xr = Smooth(x-flx);

	return Noise1DInteger(flx) * (1.-xr) + Noise1DInteger(flx+1) * xr;
}

float Noise2D( float x, float y ) {

	int flx = floor(x);
	int fly = floor(y);
	float xr = Smooth(x-flx);
	float yr = Smooth(y-fly);

	float x1 = Noise2DInteger(flx, fly  ) * (1.-xr) + Noise2DInteger(flx+1, fly  ) * xr;
	float x2 = Noise2DInteger(flx, fly+1) * (1.-xr) + Noise2DInteger(flx+1, fly+1) * xr;
	return x1 * (1.-yr) + x2 * yr;
}

float Noise3D( float x, float y, float z ) {

	int flx = floor(x);
	int fly = floor(y);
	int flz = floor(z);
	float xr = Smooth(x-flx);
	float yr = Smooth(y-fly);
	float zr = Smooth(z-flz);

	float xr1 = 1.-xr;

	float flx1 = flx+1;
	float fly1 = fly+1;
	float flz1 = flz+1;

	float x1 = Noise3DInteger(flx,fly ,flz ) * xr1 + Noise3DInteger(flx1,fly ,flz ) * xr;
	float x2 = Noise3DInteger(flx,fly1,flz ) * xr1 + Noise3DInteger(flx1,fly1,flz ) * xr;
	float x3 = Noise3DInteger(flx,fly ,flz1) * xr1 + Noise3DInteger(flx1,fly ,flz1) * xr;
	float x4 = Noise3DInteger(flx,fly1,flz1) * xr1 + Noise3DInteger(flx1,fly1,flz1) * xr;
	float y1 = x1 * (1.-yr) + x2 * yr;
	float y2 = x3 * (1.-yr) + x4 * yr;
	return y1 * (1.-zr) + y2 * zr;
}

float Noise1DPerlin( float x, float alpha, float beta, int n ) {

   float sum = 0, max = 0, scale = 1;
   for ( int i = 0; i < n; i++ ) {

      sum += Noise1D(x) * scale;
		max += scale;
      scale *= alpha;
      x *= beta;
   }
   return sum / max;
}

float Noise2DPerlin( float x, float y, float alpha, float beta, int n ) {

	y = -y; // avoid [0,0] issue
   float sum = 0, max = 0, scale = 1;
   for ( int i = 0; i < n; i++ ) {

      sum += Noise2D(x, y) * scale;
		max += scale;
      scale *= alpha;
      x *= beta;
      y *= beta;
   }
   return sum / max;
}

float Noise3DPerlin( float x, float y, float z, float alpha, float beta, int n ) {

	y = -y; z = -z; // avoid [0,0,0] issue
	float sum = 0, max = 1, scale = 1;
   for ( int i = 0; i < n; i++ ) {

      sum += Noise3D(x, y, z) * scale;
		max += scale;
      scale *= alpha;
      x *= beta;
      y *= beta;
      z *= beta;
   }
   return sum / max;
}