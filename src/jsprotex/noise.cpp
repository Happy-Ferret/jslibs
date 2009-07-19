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

#define RNDLEN 8192

static unsigned long rnd[RNDLEN];

void InitNoise() {

	init_genrand(0);
	for ( int i = 0; i < RNDLEN; i++ )
		rnd[i] = genrand_int32();
}


double Noise3DInteger( long x, long y, long z ) {
	
	unsigned long a = rnd[ x % RNDLEN ] ^ rnd[ ( x / RNDLEN ) % RNDLEN ];
	unsigned long b = rnd[ (a+y) % RNDLEN ] ^ rnd[ ( (a+y) / RNDLEN ) % RNDLEN ];
	unsigned long c = rnd[ (b+z) % RNDLEN ] ^ rnd[ ( (b+z) / RNDLEN ) % RNDLEN ];
	return (double)(c) / (double)0xffffffff;
}


double Noise3D( double x, double y, double z ) {

	double xr = x-long(x);
	double yr = y-long(y);
	double zr = z-long(z);

	double x1 = Noise3DInteger(x  ,y  ,z  ) * (1-xr) + Noise3DInteger(x+1,y  ,z  ) * xr;
	double x2 = Noise3DInteger(x  ,y+1,z  ) * (1-xr) + Noise3DInteger(x+1,y+1,z  ) * xr;
	double x3 = Noise3DInteger(x  ,y  ,z+1) * (1-xr) + Noise3DInteger(x+1,y  ,z+1) * xr;
	double x4 = Noise3DInteger(x  ,y+1,z+1) * (1-xr) + Noise3DInteger(x+1,y+1,z+1) * xr;

	double y1 = x1 * (1-yr) + x2 * yr;
	double y2 = x3 * (1-yr) + x4 * yr;

	double z1 = y1 * (1-zr) + y2 * zr;

	return z1;
}

double Noise3DPerlin( double x, double y, double z, double alpha, double beta, int n ) {

   int i;
   double val,sum = 0;
   double scale = 1;

   for ( i = 0; i < n; i++ ) {

      val = Noise3D(x,y,z);
      sum += val / scale;
      scale *= alpha;
      x *= beta;
      y *= beta;
      z *= beta;
   }
   return(sum);


}