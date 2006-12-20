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
/*
source: http://nebuladevice.svn.sourceforge.net/viewvc/nebuladevice/trunk/nebula2/code/nebula2/inc/mathlib/_vector3_sse.h?view=markup
*/
#include <xmmintrin.h>

#include <ivec.h>

static const int X = 0;
static const int Y = 1;
static const int Z = 2;
static const int W = 3;

typedef __declspec(align(4)) union {
    __m128 m128;
    struct { float x, y, z, pad; };
} Vector3;


inline void Vector3Free( Vector3 *m ) {

	return _aligned_free(m);
}

inline Vector3 *Vector3Alloc() {

	return (Vector3*)_aligned_malloc(sizeof(Vector3),16);
}


inline void Vector3Identity( Vector3 *v ) {

	v->m128 = _mm_set_ps(0.0f,0.0f,0.0f,0.0f);
}


inline void Vector3Set( Vector3 *v, const float _x, const float _y, const float _z ) {

    v->m128 = _mm_set_ps(0.0f, _z, _y, _x);
}


inline float Vector3Len( Vector3 *v ) {

    __m128 a = _mm_mul_ps(v->m128, v->m128);

    // horizontal add
    __m128 b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(X,X,X,X)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(Y,Y,Y,Y)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(Z,Z,Z,Z))));
    __m128 l = _mm_sqrt_ss(b);
    return l.m128_f32[X];
}

static __forceinline __m128 rsqrt_nr(const __m128& x) {
	static const __m128 v0pt5 = { 0.5f, 0.5f, 0.5f, 0.5f };
	static const __m128 v3pt0 = { 3.0f, 3.0f, 3.0f, 3.0f };
	__m128 t = _mm_rsqrt_ps(x);
	return _mm_mul_ps(_mm_mul_ps(v0pt5, t), _mm_sub_ps(v3pt0, _mm_mul_ps(_mm_mul_ps(x, t), t)));
}

inline void Vector3Normalize( Vector3 *v ) {

/*
	__m128 m128 = v->m128;
    __m128 a = _mm_mul_ps(m128, m128);

    // horizontal add
    __m128 b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(X,X,X,X)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(Y,Y,Y,Y)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(Z,Z,Z,Z))));

    // get reciprocal of square root of squared length
    __m128 f = _mm_rsqrt_ss(b);
    __m128 oneDivLen = _mm_shuffle_ps(f, f, _MM_SHUFFLE(X,X,X,X));

    v->m128 = _mm_mul_ps(m128, oneDivLen);
*/
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
}


inline void Vector3Add( Vector3 *v, Vector3 *v1 ) {

	v->m128 = _mm_add_ps(v->m128, v1->m128);
}


inline void Vector3Sub( Vector3 *v, const Vector3 *v1 ) {

	v->m128 = _mm_sub_ps(v->m128, v1->m128);
}


inline void Vector3Mult( Vector3 *v, float s ) {

	__m128 packed = _mm_set1_ps(s);
    v->m128 = _mm_mul_ps(v->m128, packed);
}


inline float Vector3Dot(const Vector3 *v0, const Vector3 *v1) { // Dot Product

	__m128 a = _mm_mul_ps(v0->m128, v1->m128);
	__m128 b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0,0,0,0)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(1,1,1,1)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(2,2,2,2))));
	return b.m128_f32[0];
}


inline void Vector3Cross(Vector3 *v, const Vector3 *v0, const Vector3 *v1) { // Cross Product

	__m128 a = _mm_shuffle_ps(v0->m128, v0->m128, _MM_SHUFFLE(W, X, Z, Y));
	__m128 b = _mm_shuffle_ps(v1->m128, v1->m128, _MM_SHUFFLE(W, Y, X, Z));
	__m128 c = _mm_shuffle_ps(v0->m128, v0->m128, _MM_SHUFFLE(W, Y, X, Z));
	__m128 d = _mm_shuffle_ps(v1->m128, v1->m128, _MM_SHUFFLE(W, X, Z, Y));

	__m128 e = _mm_mul_ps(a, b);
	__m128 f = _mm_mul_ps(c, d);

	v->m128 = _mm_sub_ps(e, f);
}