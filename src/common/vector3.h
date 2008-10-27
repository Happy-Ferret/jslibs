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

#ifndef _VECTOR3_H_
#define _VECTOR3_H_

/*
source: http://nebuladevice.svn.sourceforge.net/viewvc/nebuladevice/trunk/nebula2/code/nebula2/inc/mathlib/_vector3_sse.h?view=markup
*/



#include <stdlib.h>

#ifdef SSE

#include <xmmintrin.h>
#include <ivec.h>


static const int X = 0;
static const int Y = 1;
static const int Z = 2;
static const int W = 3;

/*
#define X 0
#define Y 1
#define Z 2
#define W 3
*/

typedef __declspec(align(4)) union {
    __m128 m128;
    struct { float x, y, z, pad; };
    float raw[4];
} Vector3;

#else // SSE

#include <math.h>

typedef union {
    struct { float x, y, z; };
    float raw[3];
} Vector3;

#endif // SSE

inline void Vector3Free( Vector3 *m ) {

#ifdef SSE
	return _aligned_free(m);
#else // SSE
	return free(m);
#endif // SSE
}

inline Vector3 *Vector3Alloc() {

#ifdef SSE
	return (Vector3*)_aligned_malloc(sizeof(Vector3),16);
#else // SSE
	return (Vector3*)malloc(sizeof(Vector3));
#endif // SSE
}


inline void Vector3Identity( Vector3 *v ) {

#ifdef SSE
	v->m128 = _mm_set_ps(0.0f,0.0f,0.0f,0.0f);
#else // SSE
	v->x = 0;
	v->y = 0;
	v->z = 0;
#endif // SSE
}


inline void Vector3Set( Vector3 *v, const float _x, const float _y, const float _z ) {

#ifdef SSE
    v->m128 = _mm_set_ps(0.0f, _z, _y, _x);
#else // SSE
	v->x = _x;
	v->y = _y;
	v->z = _z;
#endif // SSE
}


inline float Vector3Len( Vector3 *v ) {

#ifdef SSE
    __m128 a = _mm_mul_ps(v->m128, v->m128);
    // horizontal add
    __m128 b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(X,X,X,X)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(Y,Y,Y,Y)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(Z,Z,Z,Z))));
    __m128 l = _mm_sqrt_ss(b);
    return l.m128_f32[X];
#else // SSE
	return sqrt( v->x * v->x + v->y * v->y + v->z * v->z );
#endif // SSE
}

#ifdef SSE
static __forceinline __m128 rsqrt_nr(const __m128& x) {
	static const __m128 v0pt5 = { 0.5f, 0.5f, 0.5f, 0.5f };
	static const __m128 v3pt0 = { 3.0f, 3.0f, 3.0f, 3.0f };
	__m128 t = _mm_rsqrt_ps(x);
	return _mm_mul_ps(_mm_mul_ps(v0pt5, t), _mm_sub_ps(v3pt0, _mm_mul_ps(_mm_mul_ps(x, t), t)));
}
#else // SSE
#endif // SSE


inline void Vector3Normalize( Vector3 *v ) {

#ifdef SSE
	__m128 m128 = v->m128;
    __m128 a = _mm_mul_ps(m128, m128);

    // horizontal add
    __m128 b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(X,X,X,X)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(Y,Y,Y,Y)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(Z,Z,Z,Z))));

    // get reciprocal of square root of squared length
    __m128 f = _mm_rsqrt_ss(b);

	 __m128 oneDivLen = _mm_shuffle_ps(f, f, _MM_SHUFFLE(X,X,X,X));
    v->m128 = _mm_mul_ps(m128, oneDivLen);

/*
	__m128 vec = v->m128;

	__m128 r = _mm_mul_ps(vec,vec);
	r = _mm_add_ps(_mm_movehl_ps(r,r),r);
	__m128 t = _mm_add_ss(_mm_shuffle_ps(r,r,1), r);
	#ifdef ZERO_VECTOR
		t = _mm_cmpneq_ss(t, _mm_setzero_ps()) & rsqrt_nr(t);
	#else
		t = rsqrt_nr(t);
	#endif
	v->m128 = _mm_mul_ps(vec, _mm_shuffle_ps(t,t,0x00));
*/
#else // SSE
	float len = Vector3Len(v);
	v->x /= len;
	v->y /= len;
	v->z /= len;
#endif // SSE
}

inline void Vector3Add( Vector3 *v, Vector3 *v1 ) {

#ifdef SSE
	v->m128 = _mm_add_ps(v->m128, v1->m128);
#else // SSE
	v->x += v1->x;
	v->y += v1->y;
	v->z += v1->z;
#endif // SSE
}


inline void Vector3Sub( Vector3 *v, const Vector3 *v1 ) {

#ifdef SSE
	v->m128 = _mm_sub_ps(v->m128, v1->m128);
#else // SSE
	v->x -= v1->x;
	v->y -= v1->y;
	v->z -= v1->z;
#endif // SSE
}


inline void Vector3Mult( Vector3 *v, float s ) {

#ifdef SSE
	__m128 packed = _mm_set1_ps(s);
    v->m128 = _mm_mul_ps(v->m128, packed);
#else // SSE
	v->x *= s;
	v->y *= s;
	v->z *= s;
#endif // SSE
}


inline float Vector3Dot(const Vector3 *v0, const Vector3 *v1) { // Dot Product

#ifdef SSE
	__m128 a = _mm_mul_ps(v0->m128, v1->m128);
	__m128 b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0,0,0,0)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(1,1,1,1)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(2,2,2,2))));
	return b.m128_f32[0];
#else // SSE
	return v0->x * v1->x + v0->y * v1->y + v0->z * v1->z;
#endif // SSE
}


inline void Vector3Cross(Vector3 *v, const Vector3 *v0, const Vector3 *v1) { // Cross Product

#ifdef SSE
	__m128 a = _mm_shuffle_ps(v0->m128, v0->m128, _MM_SHUFFLE(W, X, Z, Y));
	__m128 b = _mm_shuffle_ps(v1->m128, v1->m128, _MM_SHUFFLE(W, Y, X, Z));
	__m128 c = _mm_shuffle_ps(v0->m128, v0->m128, _MM_SHUFFLE(W, Y, X, Z));
	__m128 d = _mm_shuffle_ps(v1->m128, v1->m128, _MM_SHUFFLE(W, X, Z, Y));

	__m128 e = _mm_mul_ps(a, b);
	__m128 f = _mm_mul_ps(c, d);

	v->m128 = _mm_sub_ps(e, f);
#else // SSE
	v->x = v0->y * v1->z - v0->z * v1->y;
	v->y = v0->z * v1->x - v0->x * v1->z;
	v->z = v0->x * v1->y - v0->y * v1->x;
#endif // SSE
}


#endif // _VECTOR3_H_

/*

SSE/SSE2 Toolbox SSE/SSE2 Toolbox Solutions for Solutions for Real Real-Life SIMD Life SIMD Problems Problems :
	http://www.gamasutra.com/features/gdcarchive/2001E/Alex_Klimovitski3.pdf

vector3 SSE
	http://www.google.com/codesearch?hl=en&q=show:lATM9JcpEDw:VAe0nw1kH7g:FsV6pRjFWes&sa=N&ct=rd&cs_p=http://davehillier.googlecode.com/svn&cs_f=trunk/src/maths/_vector3_sse.h&start=1

*/

