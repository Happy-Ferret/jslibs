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

EXTERN_C unsigned long genrand_int32(void); /* generates a random number on [0,0xffffffff]-interval */


// from "JAVA REFERENCE IMPLEMENTATION OF IMPROVED NOISE - COPYRIGHT 2002 KEN PERLIN."
// see http://mrl.nyu.edu/~perlin/noise/

inline double pn_fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
inline double pn_lerp(double t, double a, double b) { return a + t * (b - a); }
inline double pn_grad(int hash, double x, double y, double z) {
   int h = hash & 15;
   double u = h<8 ? x : y, v = h<4 ? y : h==12||h==14 ? x : z;
   return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

int pn_permutation[] = { 151,160,137,91,90,15,
   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
            151,160,137,91,90,15,
   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

double PerlinNoise2(double x, double y, double z) {

	int fx = floor(x), fy = floor(y), fz = floor(z);
	int X = fx & 255, Y = fy & 255, Z = fz & 255;
   x -= fx; y -= fy; z -= fz;
   double u = pn_fade(x), v = pn_fade(y), w = pn_fade(z);
   int A = pn_permutation[X  ]+Y, AA = pn_permutation[A]+Z, AB = pn_permutation[A+1]+Z,
	    B = pn_permutation[X+1]+Y, BA = pn_permutation[B]+Z, BB = pn_permutation[B+1]+Z;
   return pn_lerp(w, pn_lerp(v, pn_lerp(u, pn_grad(pn_permutation[AA  ], x  , y  , z  ),
                                           pn_grad(pn_permutation[BA  ], x-1, y  , z  )),
                                pn_lerp(u, pn_grad(pn_permutation[AB  ], x  , y-1, z  ),
                                           pn_grad(pn_permutation[BB  ], x-1, y-1, z  ))),
	                  pn_lerp(v, pn_lerp(u, pn_grad(pn_permutation[AA+1], x  , y  , z-1),
                                           pn_grad(pn_permutation[BA+1], x-1, y  , z-1)),
	                             pn_lerp(u, pn_grad(pn_permutation[AB+1], x  , y-1, z-1),
                                           pn_grad(pn_permutation[BB+1], x-1, y-1, z-1))));
}



#define RM 0x3ff // mask (used for fast modulo)
#define R (RM+1)

static unsigned long rnd[R];

void InitNoise() {

	for ( int i = 0; i < R; i++ )
		rnd[i] = genrand_int32();
}

float Noise1D( int x ) {

	unsigned long a = rnd[ x & RM ];
	unsigned long b = rnd[ (x/R+a) & RM ];
	return (float)b / (float)0xffffffff;
}

float Noise2D( int x, int y ) {

	unsigned long a = rnd[ x & RM ];
	unsigned long b = rnd[ (x/R+y+a) & RM ];
	return (float)b / (float)0xffffffff;
}


float Noise3D( int x, int y, int z ) {

	unsigned long a = rnd[ x & RM ];
	unsigned long b = rnd[ (x/R+y+a) & RM ];
	unsigned long c = rnd[ (y/R+z+b) & RM ];
	return (float)c / (float)0xffffffff;
}

float Smooth( float t ) {

	return t * t * (3. - 2. * t);
//	return t * t * t * (t * (t * 6 - 15) + 10); // from ImprovedPerlinNoise, see http://mrl.nyu.edu/~perlin/paper445.pdf 
}

float INoise1D( float x ) {

	int flx = floor(x);
	float xr = Smooth(x-flx);

	return Noise1D(flx) * (1.-xr) + Noise1D(flx+1) * xr;
}

float INoise2D( float x, float y ) {

	int flx = floor(x);
	int fly = floor(y);
	float xr = Smooth(x-flx);
	float yr = Smooth(y-fly);

	float x1 = Noise2D(flx, fly  ) * (1.-xr) + Noise2D(flx+1, fly  ) * xr;
	float x2 = Noise2D(flx, fly+1) * (1.-xr) + Noise2D(flx+1, fly+1) * xr;
	return x1 * (1.-yr) + x2 * yr;
}

float INoise3D( float x, float y, float z ) {

	int flx = floor(x);
	int fly = floor(y);
	int flz = floor(z);
	float xr = Smooth(x-flx);
	float yr = Smooth(y-fly);
	float zr = Smooth(z-flz);

	float flx1 = flx+1;
	float fly1 = fly+1;
	float flz1 = flz+1;

	float xr1 = 1.-xr;

	float x1 = Noise3D(flx,fly ,flz ) * xr1 + Noise3D(flx1,fly ,flz ) * xr;
	float x2 = Noise3D(flx,fly1,flz ) * xr1 + Noise3D(flx1,fly1,flz ) * xr;
	float x3 = Noise3D(flx,fly ,flz1) * xr1 + Noise3D(flx1,fly ,flz1) * xr;
	float x4 = Noise3D(flx,fly1,flz1) * xr1 + Noise3D(flx1,fly1,flz1) * xr;
	float y1 = x1 * (1.-yr) + x2 * yr;
	float y2 = x3 * (1.-yr) + x4 * yr;
	return y1 * (1.-zr) + y2 * zr;
}


// "alpha" is the weight when the sum is formed. Typically it is 2, As this approaches 1 the function is noisier.
// "beta" is the harmonic scaling/spacing, typically 2.
// see http://code.google.com/p/jslibs/source/browse/trunk/src/jsprotex/perlin.c?r=2644#202
float Noise1DPerlin( float x, float alpha, float beta, int n ) {

   float sum = 0, max = 0, scale = 1;
   for ( int i = 0; i < n; i++ ) {

      sum += INoise1D(x) * scale;
		max += scale;
      scale *= alpha;
      x *= beta;
   }
   return sum / max;
}

float Noise2DPerlin( float x, float y, float alpha, float beta, int n ) {

	y += 12345.f; // avoid scale visual issue at 0
   float sum = 0, max = 0, scale = 1;
   for ( int i = 0; i < n; i++ ) {

      sum += INoise2D(x, y) * scale;
		max += scale;
      scale *= alpha;
      x *= beta;
      y *= beta;
   }
   return sum / max;
}

float Noise3DPerlin( float x, float y, float z, float alpha, float beta, int n ) {

	y += 12345.f; // avoid scale visual issue at 0
	float sum = 0, max = 1, scale = 1;
   for ( int i = 0; i < n; i++ ) {

      sum += INoise3D(x, y, z) * scale;
		max += scale;
      scale *= alpha;
      x *= beta;
      y *= beta;
      z *= beta;
   }
   return sum / max;
}