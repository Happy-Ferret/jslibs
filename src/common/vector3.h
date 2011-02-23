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

// source: http://nebuladevice.svn.sourceforge.net/viewvc/nebuladevice/trunk/nebula2/code/nebula2/inc/mathlib/_vector3_sse.h?view=markup (MIT License)

#include <stdlib.h>

#include "vector4.h"


#ifdef SSE

#include <xmmintrin.h>
//#include <ivec.h>


typedef union { // __declspec(align(4)) 
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
	return jl_free(m);
#endif // SSE

}


inline Vector3 *Vector3Alloc() {

#ifdef SSE
	return (Vector3*)_aligned_malloc(sizeof(Vector3),16);
#else // SSE
	return (Vector3*)jl_malloc(sizeof(Vector3));
#endif // SSE

}


inline void Vector3Identity( Vector3 *v ) {

#ifdef SSE
	v->m128 = _mm_setzero_ps();
#else // SSE
	v->x = 0;
	v->y = 0;
	v->z = 0;
#endif // SSE

}


inline void Vector3Load( Vector3 *v, const Vector3 *v1 ) {

#ifdef SSE
	v->m128 = v1->m128;
#else // SSE
	v->x = v1->x;
	v->y = v1->y;
	v->z = v1->z;
#endif // SSE

}


inline void Vector3LoadVector4( Vector3 *v, const Vector4 *v1 ) {

#ifdef SSE
	v->m128 = v1->m128;
#else // SSE
	v->x = v1->x;
	v->y = v1->y;
	v->z = v1->z;
#endif // SSE

}


inline void Vector3LoadPtr( Vector3 *v, const float *ptr ) {

#ifdef SSE
    v->m128 = _mm_set_ps(0.0f, ptr[2], ptr[1], ptr[0]); // see	 _mm_loadu_ps
#else // SSE
	v->x = ptr[0];
	v->y = ptr[1];
	v->z = ptr[2];
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


inline bool Vector3IsNull( Vector3 *v ) {

//#ifdef SSE
	
//	return _mm_cmpeq_ps(v->m128, _mm_setzero_ps()) == 0;
//#else // SSE
	return v->x == 0.f && v->z == 0.f && v->z == 0.f;
//#endif // SSE
}


inline float Vector3Length( Vector3 *v ) {

#ifdef SSE
    register __m128 a = _mm_mul_ps(v->m128, v->m128);
    return _mm_sqrt_ss(_mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0,0,0,0)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(1,1,1,1)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(2,2,2,2))))).m128_f32[0];
#else // SSE
	return sqrt( v->x * v->x + v->y * v->y + v->z * v->z );
#endif // SSE

}

/*
#ifdef SSE
ALWAYS_INLINE __m128 rsqrt_nr(const __m128& x) {
	static const __m128 v0pt5 = { 0.5f, 0.5f, 0.5f, 0.5f };
	static const __m128 v3pt0 = { 3.0f, 3.0f, 3.0f, 3.0f };
	__m128 t = _mm_rsqrt_ps(x);
	return _mm_mul_ps(_mm_mul_ps(v0pt5, t), _mm_sub_ps(v3pt0, _mm_mul_ps(_mm_mul_ps(x, t), t)));
}
#else // SSE
#endif // SSE
*/

inline void Vector3Normalize( Vector3 *rv, Vector3 *v ) {

#ifdef SSE

	register __m128 m128 = v->m128;
	register __m128 a = _mm_mul_ps(m128, m128);
	register __m128 f = _mm_rsqrt_ss(_mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0,0,0,0)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(1,1,1,1)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(2,2,2,2)))));
	rv->m128 = _mm_mul_ps(m128, _mm_shuffle_ps(f, f, _MM_SHUFFLE(0,0,0,0)));

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

	float len = sqrt( v->x * v->x + v->y * v->y + v->z * v->z );
	v->x /= len;
	v->y /= len;
	v->z /= len;
#endif // SSE

}

inline void Vector3AddVector3( Vector3 *vr, const Vector3 *va, const Vector3 *vb ) {

#ifdef SSE
	vr->m128 = _mm_add_ps(va->m128, vb->m128);
#else // SSE
	vr->x = va->x + vb->x;
	vr->y = va->y + vb->y;
	vr->z = va->z + vb->z;
#endif // SSE

}


inline void Vector3SubVector3( Vector3 *vr, const Vector3 *va, const Vector3 *vb ) {

#ifdef SSE
	vr->m128 = _mm_sub_ps(va->m128, vb->m128);
#else // SSE
	vr->x = va->x - vb->x;
	vr->y = va->y - vb->y;
	vr->z = va->z - vb->z;
#endif // SSE

}


inline void Vector3Inv( Vector3 *rv, Vector3 *v ) {

#ifdef SSE
	rv->m128 = _mm_sub_ps(_mm_setzero_ps(), v->m128);
#else // SSE
	rv->x = -v->x;
	rv->y = -v->y;
	rv->z = -v->z;
#endif // SSE

}


inline void Vector3Mult( Vector3 *rv, Vector3 *v, float s ) {

#ifdef SSE
	rv->m128 = _mm_mul_ps(v->m128, _mm_set1_ps(s));
#else // SSE
	rv->x = v->x * s;
	rv->y = v->y * s;
	rv->z = v->z * s;
#endif // SSE

}


inline void Vector3Div( Vector3 *rv, Vector3 *v, float s ) {

#ifdef SSE
	rv->m128 = _mm_div_ps(v->m128, _mm_set1_ps(s));
#else // SSE
	rv->x = v->x / s;
	rv->y = v->y / s;
	rv->z = v->z / s;
#endif // SSE

}


inline float Vector3Dot(const Vector3 *v0, const Vector3 *v1) { // Dot Product

#ifdef SSE
	register __m128 a = _mm_mul_ps(v0->m128, v1->m128);
	return _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0,0,0,0)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(1,1,1,1)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(2,2,2,2)))).m128_f32[0];
#else // SSE
	return v0->x * v1->x + v0->y * v1->y + v0->z * v1->z;
#endif // SSE

}


inline void Vector3Cross(Vector3 *rv, const Vector3 *v0, const Vector3 *v1) { // Cross Product

#ifdef SSE
	rv->m128 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v0->m128, v0->m128, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(v1->m128, v1->m128, _MM_SHUFFLE(3, 1, 0, 2))), _mm_mul_ps(_mm_shuffle_ps(v0->m128, v0->m128, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(v1->m128, v1->m128, _MM_SHUFFLE(3, 0, 2, 1))));
#else // SSE
	rv->x = v0->y * v1->z - v0->z * v1->y;
	rv->y = v0->z * v1->x - v0->x * v1->z;
	rv->z = v0->x * v1->y - v0->y * v1->x;
#endif // SSE

}


/*

SSE/SSE2 Toolbox SSE/SSE2 Toolbox Solutions for Solutions for Real Real-Life SIMD Life SIMD Problems Problems :
	http://www.gamasutra.com/features/gdcarchive/2001E/Alex_Klimovitski3.pdf

vector3 SSE
	http://www.google.com/codesearch?hl=en&q=show:lATM9JcpEDw:VAe0nw1kH7g:FsV6pRjFWes&sa=N&ct=rd&cs_p=http://davehillier.googlecode.com/svn&cs_f=trunk/src/maths/_vector3_sse.h&start=1

*/

