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

#ifndef _VECTOR4_H_
#define _VECTOR4_H_

#include <stdlib.h>

#ifdef SSE

#include <xmmintrin.h>
//#include <ivec.h>

typedef union { // __declspec(align(4)) 
    __m128 m128;
    struct { float x, y, z, w; };
    float raw[4];
} Vector4;

#else // SSE

#include <math.h>

typedef union {
    struct { float x, y, z, w; };
    float raw[4];
} Vector4;

#endif // SSE


inline void Vector4Set( Vector4 *v, const float _x, const float _y, const float _z, const float _w ) {

#ifdef SSE
    v->m128 = _mm_set_ps(_w, _z, _y, _x);
#else // SSE
	v->x = _x;
	v->y = _y;
	v->z = _z;
	v->w = _w;
#endif // SSE

}

inline void Vector4LoadFromPtr( Vector4 *v, const float *ptr ) {

#ifdef SSE
	v->m128 = _mm_loadu_ps(ptr);
#else // SSE
	memcpy(v->raw, ptr, sizeof(Vector4));
#endif // SSE
}

inline void Vector4LoadToPtr( Vector4 *v, float *ptr ) {

#ifdef SSE
	_mm_storeu_ps(ptr, v->m128);
#else // SSE
	memcpy(ptr, v->raw, sizeof(Vector4));
#endif // SSE
}


inline void Vector4Mult( Vector4 *rv, Vector4 *v, float s ) {

#ifdef SSE
	rv->m128 = _mm_mul_ps(v->m128, _mm_set1_ps(s));
#else // SSE
	rv->x = v->x * s;
	rv->y = v->y * s;
	rv->z = v->z * s;
	rv->w = v->w * s;
#endif // SSE

}


inline void Vector4Div( Vector4 *rv, Vector4 *v, float s ) {

#ifdef SSE
	rv->m128 = _mm_div_ps(v->m128, _mm_set1_ps(s));
#else // SSE
	rv->x = v->x / s;
	rv->y = v->y / s;
	rv->z = v->z / s;
	rv->w = v->w / s;
#endif // SSE

}


inline void Vector4SubVector4( Vector4 *rv, Vector4 *v1, Vector4 *v2 ) {

#ifdef SSE
	rv->m128 = _mm_sub_ps(v1->m128, v2->m128);
#else // SSE
	rv->x -= v1->x - v2->x;
	rv->y -= v1->y - v2->y;
	rv->z -= v1->z - v2->z;
	rv->w -= v1->w - v2->w;
#endif // SSE

}


inline void Vector4AddVector4( Vector4 *rv, Vector4 *v1, Vector4 *v2 ) {

#ifdef SSE
	rv->m128 = _mm_add_ps(v1->m128, v2->m128);
#else // SSE
	rv->x -= v1->x + v2->x;
	rv->y -= v1->y + v2->y;
	rv->z -= v1->z + v2->z;
	rv->w -= v1->w + v2->w;
#endif // SSE

}


inline float Vector4Length( Vector4 *v ) {

#ifdef SSE
	register __m128 a = _mm_mul_ps(v->m128, v->m128);
	return _mm_sqrt_ss(_mm_add_ss(a, _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0,0,0,0)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(1,1,1,1)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(2,2,2,2)))))).m128_f32[0];
#else // SSE
	return sqrt( v->x * v->x + v->y * v->y + v->z * v->z );
#endif // SSE

}


inline void Vector4Normalize( Vector4 *v ) {

#ifdef SSE

	register __m128 m128 = v->m128;
	register __m128 a = _mm_mul_ps(m128, m128);
	register __m128 f = _mm_rsqrt_ss(_mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0,0,0,0)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(1,1,1,1)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(2,2,2,2)))));
	v->m128 = _mm_mul_ps(m128, _mm_shuffle_ps(f, f, _MM_SHUFFLE(0,0,0,0)));

#else // SSE

	float len = sqrt( v->x * v->x + v->y * v->y + v->z * v->z );
	v->x /= len;
	v->y /= len;
	v->z /= len;
#endif // SSE

}




#endif // _VECTOR4_H_
