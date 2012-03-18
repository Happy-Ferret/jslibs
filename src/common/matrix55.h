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


#pragma once

#include <stdlib.h>
#include <math.h>

#include "vector5.h"

typedef union {
	struct {
		float m[5][5]; // m[line][col]
	};
	struct {
		float raw[25];
	};
} Matrix55;

//#endif // SSE (Streaming SIMD Extensions)

static Matrix55 Matrix55IdentityValue = {

 { { { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
     { 0.0f, 1.0f, 0.0f, 0.0f, 0.0f },
     { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
     { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f },
     { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f } } }
};


inline void Matrix55Free( Matrix55 *m ) {

	jl_free(m);
}


inline Matrix55 *Matrix55Alloc() {

	return (Matrix55*)jl_malloc(sizeof(Matrix55));
}


inline bool Matrix55IsIdentity( Matrix55 *m ) {

	return memcmp(m->raw, Matrix55IdentityValue.raw, sizeof(Matrix55IdentityValue)) == 0;
}


inline void Matrix55Identity( Matrix55 *m ) {

	jl_memcpy(m->raw, Matrix55IdentityValue.raw, sizeof(Matrix55IdentityValue));
}


inline void Matrix55Load( Matrix55 *m, const Matrix55 *m1 ) {

	jl_memcpy(m->raw, m1->raw, sizeof(Matrix55));
}


inline void Matrix55LoadFromPtr( Matrix55 *m, const float *ptr ) {

	jl_memcpy(m->raw, ptr, sizeof(Matrix55));
}


inline void Matrix55Mult( Matrix55 *rm, const Matrix55 *m, const Matrix55 *mx ) {

	Matrix55 tmp;

	#define A(row,col)  m->raw[col*5+row]
	#define B(row,col)  mx->raw[col*5+row]
	#define P(row,col)  tmp.raw[col*5+row]
	for (int i = 0; i < 5; i++) {
		const float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3),  ai4=A(i,4);
		P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0) + ai4 * B(4,0);
		P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1) + ai4 * B(4,1);
		P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2) + ai4 * B(4,2);
		P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3) + ai4 * B(4,3);
		P(i,4) = ai0 * B(0,4) + ai1 * B(1,4) + ai2 * B(2,4) + ai3 * B(3,4) + ai4 * B(4,4);
	}
	#undef A
	#undef B
	#undef P

	Matrix55Load(rm, &tmp);
}


//inline void Matrix55MultVector5( Matrix55 *m, Vector5 *dst, Vector5 *src ) {
//}
