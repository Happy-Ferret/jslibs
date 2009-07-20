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

double Noise1DInteger( int x ) {
	
	unsigned long a = rnd[ x & RM ];
	unsigned long b = rnd[ (x/R+a) & RM ];
	return (double)a / (double)0xffffffff;
}


double Noise2DInteger( int x, int y ) {
	
	unsigned long a = rnd[ x & RM ];
	unsigned long b = rnd[ (x/R+y+a) & RM ];
	return (double)b / (double)0xffffffff;
}


double Noise3DInteger( int x, int y, int z ) {
	
	unsigned long a = rnd[ x & RM ];
	unsigned long b = rnd[ (x/R+y+a) & RM ];
	unsigned long c = rnd[ (y/R+z+b) & RM ];
	return (double)c / (double)0xffffffff;
}

double Smooth( double t) {
	
	return t * t * (3. - 2. * t);
}

double Noise1D( double x ) {

	int fx = floor(x);
	double xr = Smooth(x-fx);

	return Noise1DInteger(fx) * (1.-xr) + Noise1DInteger(fx+1) * xr;
}

double Noise2D( double x, double y ) {

	int fx = floor(x);
	int fy = floor(y);
	double xr = Smooth(x-fx);
	double yr = Smooth(y-fy);

	double x1 = Noise2DInteger(fx, fy  ) * (1.-xr) + Noise2DInteger(fx+1, fy  ) * xr;
	double x2 = Noise2DInteger(fx, fy+1) * (1.-xr) + Noise2DInteger(fx+1, fy+1) * xr;
	return x1 * (1.-yr) + x2 * yr;
}

double Noise3D( double x, double y, double z ) {

	int fx = floor(x);
	int fy = floor(y);
	int fz = floor(z);
	double xr = Smooth(x-fx);
	double yr = Smooth(y-fy);
	double zr = Smooth(z-fz);

	double xr1 = 1.-xr;

	double tx1 = fx+1;
	double ty1 = fy+1;
	double tz1 = fz+1;

	double x1 = Noise3DInteger(fx,fy ,fz ) * xr1 + Noise3DInteger(tx1,fy ,fz ) * xr;
	double x2 = Noise3DInteger(fx,ty1,fz ) * xr1 + Noise3DInteger(tx1,ty1,fz ) * xr;
	double x3 = Noise3DInteger(fx,fy ,tz1) * xr1 + Noise3DInteger(tx1,fy ,tz1) * xr;
	double x4 = Noise3DInteger(fx,ty1,tz1) * xr1 + Noise3DInteger(tx1,ty1,tz1) * xr;
	double y1 = x1 * (1.-yr) + x2 * yr;
	double y2 = x3 * (1.-yr) + x4 * yr;
	return y1 * (1.-zr) + y2 * zr;
}

double Noise1DPerlin( double x, double alpha, double beta, int n ) {

   double sum = 0, max = 0, scale = 1;

   for ( int i = 0; i < n; i++ ) {

      sum += Noise1D(x) * scale;
		max += scale;
      scale *= alpha;
      x *= beta;
   }
   return sum / max;
}

double Noise2DPerlin( double x, double y, double alpha, double beta, int n ) {

   double sum = 0, max = 0, scale = 1;

   for ( int i = 0; i < n; i++ ) {

      sum += Noise2D(x, y) * scale;
		max += scale;
      scale *= alpha;
      x *= beta;
      y *= beta;
   }
   return sum / max;
}

double Noise3DPerlin( double x, double y, double z, double alpha, double beta, int n ) {

   double sum = 0, max = 1, scale = 1;

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