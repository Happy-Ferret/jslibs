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

/* notes:
A(BC) = (AB)C
T(S(x)) = TS(x)
(AB)^T = B^T x A^T
M^T . M = I

Note that post-multiplying with column-major matrices produces the same result as pre-multiplying with row-major matrices.
*/

/*
	source:
		http://nebuladevice.svn.sourceforge.net/viewvc/nebuladevice/trunk/nebula2/code/nebula2/inc/mathlib/_matrix44_sse.h?view=markup
		http://nebuladevice.svn.sourceforge.net/viewvc/nebuladevice/trunk/nebula2/code/nebula2/inc/mathlib/_matrix44.h?view=markup

	SSE Intro:
		http://www.codeproject.com/cpp/sseintro.asp

	INTEL Approximate Math Library:
		http://www.intel.com/design/pentiumiii/devtools/AMaths.zip

	Some asm math code snippets:
		http://www.caustik.com/cxbx/download/Cxbx-0.7.8c-Source.zip/Cxbx-0.7.8c-Source/OpenXDK/src/xlibc/math/
		http://ftp.osuosl.org/pub/FreeBSD/distfiles/egl-v0.2.5-src.zip/shared/
		:pserver:anonymous@cvs.ogre3d.org:/cvsroot/ogre ogrenew/OgreMain/include/
		http://glide64.emuxhaven.net/files/Glide64_WonderPlus_src.zip/
*/

//#undef SSE

#ifndef _MATRIX44_H_
#define _MATRIX44_H_

#include <stdlib.h>
#include <math.h>

#include "vector3.h"
#include "vector4.h"

#ifdef SSE // SSE (Streaming SIMD Extensions)

#include <xmmintrin.h>
//#include <fvec.h>

typedef union { //  __declspec(align(16))
	struct {
		__m128 m1;
		__m128 m2;
		__m128 m3;
		__m128 m4;
	};
	struct {
		__m128 m128[4];
	};
	struct {
		float m[4][4]; // m[line][col]
	};
	struct {
		float raw[16];
	};
} Matrix44;

#else // SSE (Streaming SIMD Extensions)

typedef union {
	struct {
		float m[4][4];
	};
	struct {
		float raw[16];
	};
} Matrix44;

#endif // SSE (Streaming SIMD Extensions)

static Matrix44 Matrix44IdentityValue = {

    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};


inline void Matrix44Free( Matrix44 *m ) {

#ifdef SSE
	_aligned_free(m);
#else // SSE
	jl_free(m);
#endif // SSE

}

inline Matrix44 *Matrix44Alloc() {

#ifdef SSE
	// Doc: For example, if you use malloc, the result depends on the operand size. If arg >= 8,
	//      alignment will be 8 byte aligned. If arg < 8, alignment will be the first power of 2 less than arg.
	//      For example, if you use malloc(7), alignment is 4 bytes.
	return (Matrix44*)_aligned_malloc(sizeof(Matrix44), __alignof(Matrix44));
#else // SSE
	return (Matrix44*)jl_malloc(sizeof(Matrix44));
#endif // SSE
}


inline bool Matrix44IsIdentity( Matrix44 *m ) {

#ifdef SSE

	__m128 r0, r1, r2, r3;
	r0 = _mm_cmpeq_ps( m->m1, Matrix44IdentityValue.m1 );
	r1 = _mm_cmpeq_ps( m->m2, Matrix44IdentityValue.m2 );
	r2 = _mm_cmpeq_ps( m->m3, Matrix44IdentityValue.m3 );
	r3 = _mm_cmpeq_ps( m->m4, Matrix44IdentityValue.m4 );
	r0 = _mm_and_ps( r0, r1 );
	r2 = _mm_and_ps( r2, r3 );
	r0 = _mm_and_ps( r0, r2 );
	if ( r0.m128_u32[0] != 0xFFFFFFFF || r0.m128_u32[1] != 0xFFFFFFFF || r0.m128_u32[2] != 0xFFFFFFFF || r0.m128_u32[3] != 0xFFFFFFFF )
		return false;
	return true;

#else // SSE

	return memcmp(m->raw, Matrix44IdentityValue.raw, sizeof(Matrix44IdentityValue)) == 0;

#endif // SSE

}


inline void Matrix44Identity( Matrix44 *m ) {

#ifdef SSE

	m->m1 = Matrix44IdentityValue.m1;
	m->m2 = Matrix44IdentityValue.m2;
	m->m3 = Matrix44IdentityValue.m3;
	m->m4 = Matrix44IdentityValue.m4;

	//register __m128 tmp = { 1.0f, 0.0f, 0.0f, 0.0f };
	//m->m1 = tmp;
	//m->m2 = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(1,1,0,1));
	//m->m3 = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(1,0,1,1));
	//m->m4 = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0,1,1,1));

#else // SSE
	memcpy(m->raw, Matrix44IdentityValue.raw, sizeof(Matrix44IdentityValue));
#endif // SSE

}

inline void Matrix44Load( Matrix44 *m, const Matrix44 *m1 ) {

#ifdef SSE
	m->m1 = m1->m1;
	m->m2 = m1->m2;
	m->m3 = m1->m3;
	m->m4 = m1->m4;
#else // SSE
	memcpy(m->raw, m1->raw, sizeof(Matrix44));
#endif // SSE

}


inline void Matrix44LoadFromPtr( Matrix44 *m, const float *ptr ) {

#ifdef SSE
	m->m1 = _mm_loadu_ps(ptr+0);
	m->m2 = _mm_loadu_ps(ptr+4);
	m->m3 = _mm_loadu_ps(ptr+8);
	m->m4 = _mm_loadu_ps(ptr+12);
#else // SSE
	memcpy(m->raw, ptr, sizeof(Matrix44));
#endif // SSE

}

inline void Matrix44LoadToPtr( Matrix44 *m, float *ptr ) {

#ifdef SSE
	_mm_storeu_ps(ptr+ 0, m->m1);
	_mm_storeu_ps(ptr+ 4, m->m2);
	_mm_storeu_ps(ptr+ 8 ,m->m3);
	_mm_storeu_ps(ptr+12, m->m4);
#else // SSE
	memcpy(ptr, m->raw, sizeof(Matrix44));
#endif // SSE

}


inline void Matrix44Mult( Matrix44 *rm, const Matrix44 *m, const Matrix44 *mx ) {

#ifdef SSE
	
	// see. http://www.61nord.no/agar/trunk/math/m_matrix44_sse.h
	// see. http://software.intel.com/en-us/articles/optimized-matrix-library-for-use-with-the-intel-pentiumr-4-processors-sse2-instructions/
	// see. http://homepages.cae.wisc.edu/~sedlacek/Doxygen/html/class_matrix_s_s_e.html
	// see. http://www.google.com/codesearch/p?hl=en&sa=N&cd=70&ct=rc#45SLCQu26VI/zooengine 24/Basic/ZooMatrix.cpp&q=_mm_add_ps _mm_mul_ps _mm_shuffle_ps

	#define RM rm->
	#define M1 m->
	#define M2 mx->

	register __m128 l1 = M1 m1;
	register __m128 l2 = M1 m2; 
	register __m128 l3 = M1 m3; 
	register __m128 l4 = M1 m4; 
	register __m128 tmp;
	
	tmp = mx->m1;
	RM m1 = _mm_add_ps(_mm_add_ps(_mm_add_ps(
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0,0,0,0)), l1),
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(1,1,1,1)), l2)),
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(2,2,2,2)), l3)),
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3,3,3,3)), l4));

	tmp = mx->m2;
	RM m2 = _mm_add_ps(_mm_add_ps(_mm_add_ps(
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0,0,0,0)), l1),
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(1,1,1,1)), l2)),
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(2,2,2,2)), l3)),
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3,3,3,3)), l4));

	tmp = mx->m3;
	RM m3 = _mm_add_ps(_mm_add_ps(_mm_add_ps(
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0,0,0,0)), l1),
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(1,1,1,1)), l2)),
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(2,2,2,2)), l3)),
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3,3,3,3)), l4));

	tmp = mx->m4;
	RM m4 = _mm_add_ps(_mm_add_ps(_mm_add_ps(
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0,0,0,0)), l1),
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(1,1,1,1)), l2)),
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(2,2,2,2)), l3)),
		_mm_mul_ps(_mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3,3,3,3)), l4));

#else // SSE

	Matrix44 tmp;
/*
	#define A(row,col)  m->raw[(col<<2)+row]
	#define B(row,col)  mx->raw[(col<<2)+row]
	#define P(row,col)  tmp.raw[(col<<2)+row]
	for (int i = 0; i < 4; i++) {
		const float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
		P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
		P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
		P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
		P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
	}
	#undef A
	#undef B
	#undef P
*/

	tmp.raw[0]  = m->raw[0]*mx->raw[0]  + m->raw[4]*mx->raw[1]  + m->raw[8] *mx->raw[2]  + m->raw[12]*mx->raw[3];
	tmp.raw[1]  = m->raw[1]*mx->raw[0]  + m->raw[5]*mx->raw[1]  + m->raw[9] *mx->raw[2]  + m->raw[13]*mx->raw[3];
	tmp.raw[2]  = m->raw[2]*mx->raw[0]  + m->raw[6]*mx->raw[1]  + m->raw[10]*mx->raw[2]  + m->raw[14]*mx->raw[3];
	tmp.raw[3]  = m->raw[3]*mx->raw[0]  + m->raw[7]*mx->raw[1]  + m->raw[11]*mx->raw[2]  + m->raw[15]*mx->raw[3];


	tmp.raw[4]  = m->raw[0]*mx->raw[4]  + m->raw[4]*mx->raw[5]  + m->raw[8] *mx->raw[6]  + m->raw[12]*mx->raw[7];
	tmp.raw[5]  = m->raw[1]*mx->raw[4]  + m->raw[5]*mx->raw[5]  + m->raw[9] *mx->raw[6]  + m->raw[13]*mx->raw[7];
	tmp.raw[6]  = m->raw[2]*mx->raw[4]  + m->raw[6]*mx->raw[5]  + m->raw[10]*mx->raw[6]  + m->raw[14]*mx->raw[7];
	tmp.raw[7]  = m->raw[3]*mx->raw[4]  + m->raw[7]*mx->raw[5]  + m->raw[11]*mx->raw[6]  + m->raw[15]*mx->raw[7];


	tmp.raw[8]  = m->raw[0]*mx->raw[8]  + m->raw[4]*mx->raw[9]  + m->raw[8] *mx->raw[10] + m->raw[12]*mx->raw[11];
	tmp.raw[9]  = m->raw[1]*mx->raw[8]  + m->raw[5]*mx->raw[9]  + m->raw[9] *mx->raw[10] + m->raw[13]*mx->raw[11];
	tmp.raw[10] = m->raw[2]*mx->raw[8]  + m->raw[6]*mx->raw[9]  + m->raw[10]*mx->raw[10] + m->raw[14]*mx->raw[11];
	tmp.raw[11] = m->raw[3]*mx->raw[8]  + m->raw[7]*mx->raw[9]  + m->raw[11]*mx->raw[10] + m->raw[15]*mx->raw[11];


	tmp.raw[12] = m->raw[0]*mx->raw[12] + m->raw[4]*mx->raw[13] + m->raw[8] *mx->raw[14] + m->raw[12]*mx->raw[15];
	tmp.raw[13] = m->raw[1]*mx->raw[12] + m->raw[5]*mx->raw[13] + m->raw[9] *mx->raw[14] + m->raw[13]*mx->raw[15];
	tmp.raw[14] = m->raw[2]*mx->raw[12] + m->raw[6]*mx->raw[13] + m->raw[10]*mx->raw[14] + m->raw[14]*mx->raw[15];
	tmp.raw[15] = m->raw[3]*mx->raw[12] + m->raw[7]*mx->raw[13] + m->raw[11]*mx->raw[14] + m->raw[15]*mx->raw[15];

	Matrix44Load(rm, &tmp);

#endif // SSE

}


// see. http://www.google.com/codesearch/p?hl=en&sa=N&cd=6&ct=rc#DSL5kIMUUeU/Utils.cpp&q=xorps%20xmm0,xmm0%20movaps%20fsincos&l=91

inline void Matrix44SetXRotation( Matrix44 *m, float radAngle ) {

#ifdef SSE
	__asm { // Intel's copyright notice: the content of the following __asm if provided by Intel (matlib)
		xorps	xmm0,xmm0
		mov 	eax, m;
		fld		float ptr radAngle
		movaps	[eax+0x10], xmm0		// clear line _L2
		fsincos
		fst		float ptr [eax+0x14]	// set element _22
		movaps	[eax+0x20], xmm0		// clear line _L3
		fstp	float ptr [eax+0x28]	// set element _33
		fst		float ptr [eax+0x18]	// set element _23
		fchs
		movaps	[eax+0x00], xmm0		// clear line _L1
		fstp	float ptr [eax+0x24]	// set element _32
		fld1
		fst		float ptr [eax+0x00]	// set element _11
		movaps	[eax+0x30], xmm0		// clear line _L4
		fstp	float ptr [eax+0x3C]	// set element _44
	}
#else // SSE
	// (TBD)
#endif // SSE
}

inline void Matrix44SetYRotation( Matrix44 *m, float radAngle ) {

#ifdef SSE
	__asm { // Intel's copyright notice: the content of the following __asm if provided by Intel (matlib)
		xorps	xmm0,xmm0
		mov 	eax, m
		fld		float ptr radAngle
		movaps	[eax+0x00], xmm0		// clear line _L1
		fsincos
		fst		float ptr [eax+0x00]	// set element _11
		movaps	[eax+0x20], xmm0		// clear line _L3
		fstp	float ptr [eax+0x28]	// set element _33
		fst		float ptr [eax+0x20]	// set element _31
		fchs
		movaps	[eax+0x10], xmm0		// clear line _L2
		fstp	float ptr [eax+0x08]	// set element _13
		fld1
		fst		float ptr [eax+0x14]	// set element _22
		movaps	[eax+0x30], xmm0		// clear line _L4
		fstp	float ptr [eax+0x3C]	// set element _44
	}
#else // SSE
	// (TBD)
#endif // SSE
}


inline void Matrix44SetZRotation( Matrix44 *m, float radAngle ) {

#ifdef SSE

	__asm { // Intel's copyright notice: the content of the following __asm if provided by Intel (matlib)
		xorps	xmm0,xmm0
		mov 	eax, m
		fld		float ptr radAngle
		movaps	[eax+0x00], xmm0		// clear line _L1
		fsincos
		fst		float ptr [eax+0x00]	// set element _11
		movaps	[eax+0x10], xmm0		// clear line _L2
		fstp	float ptr [eax+0x14]	// set element _22
		fst		float ptr [eax+0x04]	// set element _12
		fchs
		movaps	[eax+0x20], xmm0		// clear line _L3
		fstp	float ptr [eax+0x10]	// set element _21
		fld1
		fst		float ptr [eax+0x28]	// set element _33
		movaps	[eax+0x30], xmm0		// clear line _L4
		fstp	float ptr [eax+0x3C]	// set element _44
	}

#else // SSE

    float c = cosf(radAngle);
    float s = sinf(radAngle);
    int i;
    for (i=0; i<4; i++) {

        float mi0 = m->m[i][0];
        float mi1 = m->m[i][1];
        m->m[i][0] = mi0*c + mi1*-s;
        m->m[i][1] = mi0*s + mi1*c;
    }

#endif // SSE

}


inline float Sin(float angle) {

#ifdef SSE
	__asm fld	angle
	__asm fsin
#else // SSE
	return sinf(angle);
#endif // SSE
}

inline float Cos(float angle) {

#ifdef SSE
	__asm fld	angle
	__asm fcos
#else // SSE
	return cosf(angle);
#endif // SSE
}

inline void SinCos(float angle, float *sinVal, float *cosVal) {

#ifdef SSE
	__asm fld		[angle]
	__asm fsincos
	__asm mov		eax,cosVal
	__asm fstp		dword ptr [eax]
	__asm mov		eax,sinVal
	__asm fstp		dword ptr [eax]
#else // SSE
	*sinVal = cosf(angle);
	*cosVal = cosf(angle);
#endif // SSE
}

inline float Sqrt(float val) {

#ifdef SSE
	__asm fld		val
	__asm fsqrt
#else // SSE
	return sqrtf(val);
#endif // SSE
}


inline void Matrix44SetRotation( Matrix44 *m, const Vector3 *axis, float radAngle ) {

	Vector3 v;
	Vector3Load(&v, axis);
	Vector3Normalize(&v, &v);
	float sa = (float) sinf(-radAngle);
	float ca = (float) cosf(-radAngle);

	m->m[0][0] = ca + (1.0f - ca) * v.x * v.x;
	m->m[0][1] = (1.0f - ca) * v.x * v.y - sa * v.z;
	m->m[0][2] = (1.0f - ca) * v.z * v.x + sa * v.y;
	m->m[1][0] = (1.0f - ca) * v.x * v.y + sa * v.z;
	m->m[1][1] = ca + (1.0f - ca) * v.y * v.y;
	m->m[1][2] = (1.0f - ca) * v.y * v.z - sa * v.x;
	m->m[2][0] = (1.0f - ca) * v.z * v.x - sa * v.y;
	m->m[2][1] = (1.0f - ca) * v.y * v.z + sa * v.x;
	m->m[2][2] = ca + (1.0f - ca) * v.z * v.z;

/*
	Vector3 v = *axis;
	Vector3Normalize(&v); // (TBD) Opt: check if already normalized
	float sa = (float) sinf(radAngle);
	float ca = (float) cosf(radAngle);
//	Matrix44 rot;
//	Matrix44Identity(&rot);
//	rot.m1 = _mm_set1_ps(
	m->m[0][0] = ca + (1.0f - ca) * v.x * v.x;
	m->m[0][1] = (1.0f - ca) * v.x * v.y - sa * v.z;
	m->m[0][2] = (1.0f - ca) * v.z * v.x + sa * v.y;
	m->m[1][0] = (1.0f - ca) * v.x * v.y + sa * v.z;
	m->m[1][1] = ca + (1.0f - ca) * v.y * v.y;
	m->m[1][2] = (1.0f - ca) * v.y * v.z - sa * v.x;
	m->m[2][0] = (1.0f - ca) * v.z * v.x - sa * v.y;
	m->m[2][1] = (1.0f - ca) * v.y * v.z + sa * v.x;
	m->m[2][2] = ca + (1.0f - ca) * v.z * v.z;
//	Matrix44Mult(m, &rot);
*/


/*
	float w,x,y,z;
	float ax = axis->x;
	float ay = axis->y;
	float az = axis->z;
	float l = ax*ax + ay*ay + az*az;
	if (l > 0.f) {

		float s;
		SinCos(-radAngle/2, &s, &w); // w ??
		l = s / Sqrt(l);
//		radAngle /= 2;
//		w = cos(radAngle);
//		l = sin(radAngle) / sqrtf(l);

		x = ax*l;
		y = ay*l;
		z = az*l;
	} else {

		w = 1;
		x = 0;
		y = 0;
		z = 0;
	}

	float xx = x * x;
	float xy = x * y;
	float xz = x * z;
	float xw = x * w;

	float yy = y * y;
	float yz = y * z;
	float yw = y * w;

	float zz = z * z;
	float zw = z * w;

	m->raw[0]  = 1 - 2 * ( yy + zz );
	m->raw[1]  =     2 * ( xy - zw );
	m->raw[2]  =     2 * ( xz + yw );

	m->raw[4]  =     2 * ( xy + zw );
	m->raw[5]  = 1 - 2 * ( xx + zz );
	m->raw[6]  =     2 * ( yz - xw );

	m->raw[8]  =     2 * ( xz - yw );
	m->raw[9]  =     2 * ( yz + xw );
	m->raw[10] = 1 - 2 * ( xx + yy );
*/

}


inline void Matrix44ClearRotation( Matrix44 *m ) {

	m->raw[0]  = 1;
	m->raw[1]  = 0;
	m->raw[2]  = 0;

	m->raw[4]  = 0;
	m->raw[5]  = 1;
	m->raw[6]  = 0;

	m->raw[8]  = 0;
	m->raw[9]  = 0;
	m->raw[10] = 1;
}

/*
  void RotationSpherical( float latitude, float longitude, float angle ) {

    float sin_a    = sin( angle / 2 );
    float cos_a    = cos( angle / 2 );

    float sin_lat  = sin( latitude );
    float cos_lat  = cos( latitude );

    float sin_long = sin( longitude );
    float cos_long = cos( longitude );

    float x = sin_a * cos_lat * sin_long;
    float y = sin_a * sin_lat;
    float z = sin_a * sin_lat * cos_long;
    float w = cos_a;

    float xx = x * x;
    float xy = x * y;
    float xz = x * z;
    float xw = x * w;

    float yy = y * y;
    float yz = y * z;
    float yw = y * w;

    float zz = z * z;
    float zw = z * w;

    _m[0]  = 1 - 2 * ( yy + zz );
    _m[1]  =     2 * ( xy - zw );
    _m[2]  =     2 * ( xz + yw );

    _m[4]  =     2 * ( xy + zw );
    _m[5]  = 1 - 2 * ( xx + zz );
    _m[6]  =     2 * ( yz - xw );

    _m[8]  =     2 * ( xz - yw );
    _m[9]  =     2 * ( yz + xw );
    _m[10] = 1 - 2 * ( xx + yy );

    _type |= ROTATION;
    ++updateIndex;
    _crc = 0;
  }
*/

inline void Matrix44SetTranslation( Matrix44 *m, const float x, const float y, const float z ) {

	m->m[3][0] = x;
	m->m[3][1] = y;
	m->m[3][2] = z;
}

inline void Matrix44GetTranslation( const Matrix44 *m, float *x, float *y, float *z ) {

	*x = m->m[3][0];
	*y = m->m[3][1];
	*z = m->m[3][2];
}


inline void Matrix44ClearTranslation( Matrix44 *m ) {

	m->m[3][0] = 0;
	m->m[3][1] = 0;
	m->m[3][2] = 0;
}


inline void Matrix44SetScale( Matrix44 *m, const float x, const float y, const float z ) {

	m->m[0][0] = x;
	m->m[1][1] = y;
	m->m[2][2] = z;
}


//inline void Matrix44Scale( Matrix44 *m, const Vector3 *s ) {
//
//	// _vector3_sse have the w element set to zero, we need it at 1...
//	__m128 scale = _mm_add_ps(_mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f), s->m128);
//	m->m1 = _mm_mul_ps(m->m1, scale);
//	m->m2 = _mm_mul_ps(m->m2, scale);
//	m->m3 = _mm_mul_ps(m->m3, scale);
//	m->m4 = _mm_mul_ps(m->m4, scale);
//}

/*
// http://www.google.com/codesearch/p?hl=en&sa=N&cd=1&ct=rc#nnbtoOETxgg/trunk/day1/maths/_matrix44_sse.h&q=%22_mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps%22
inline void Matrix44LookAt( Matrix44 *m, const Vector3 *to ) {

	Vector3 from, z, y, x;
//	from.m128 = m->m4;
	from.x = m->raw[12];
	from.y = m->raw[13];
	from.z = m->raw[14];
	Vector3Sub(&z, &from, to);
	Vector3Normalize(&z);
	Vector3Set(&y, 0.f,0.f,1.f);
	Vector3Cross(&x, &y, &z);
	Vector3Cross(&y, &z, &x);
	Vector3Normalize(&x);
	Vector3Normalize(&y);
	m->m1 = x.m128;
	m->m2 = y.m128;
	m->m3 = z.m128;
}
*/


inline void Matrix44SetLookAt( Matrix44 *m, const Vector3 *from, const Vector3 *to, const Vector3 *up ) {
/*
	Matrix44 tmp;
	Vector3 f, s, u;

	Vector3Sub(&f, center, eye);
	Vector3Normalize(&f);
	Vector3Cross(&s, &f, up); // s = f x up
	Vector3Normalize(&s);
	Vector3Cross(&u, &s, &f); // u = s x f

	Matrix44Identity(&tmp);

	tmp.m[0][0] = s.x;
	tmp.m[1][0] = s.y;
	tmp.m[2][0] = s.z;
			  
	tmp.m[0][1] = u.x;
	tmp.m[1][1] = u.y;
	tmp.m[2][1] = u.z;
			  
	tmp.m[0][2] = -f.x;
	tmp.m[1][2] = -f.y;
	tmp.m[2][2] = -f.z;

//	Vector3Inv(&f, &f);
//	tmp.m1 = s.m128;
//	tmp.m2 = u.m128;
//	tmp.m3 = f.m128;
//	tmp.m4 = _mm_set_ps(1,0,0,0);

//	m->m4 = eye->m128;
//	m->[15] = 1;

//	tmp.m[3][0] -= eye->x;
//	tmp.m[3][1] -= eye->y;
//	tmp.m[3][2] -= eye->z;

	Matrix44MultSimple(m, &tmp);
*/

	Vector3 z;
	Vector3SubVector3(&z, from, to);
	Vector3Normalize(&z, &z); // z = |z|
	Vector3 y = *up;
//	y.pad = 0.0f;
	Vector3 x;
	Vector3Cross(&x, &y, &z); // x = y cross z
	Vector3Cross(&y, &z, &x); // z = x cross y
	Vector3Normalize(&x, &x); // x = |x|
	Vector3Normalize(&y, &y); // y = |y|
//	Vector3Inv(&z, &z);
#ifdef SSE

	m->m1 = x.m128;
	m->m2 = y.m128;
	m->m3 = z.m128;
	m->m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
#else // SSE
	// (TBD)
#endif // SSE

/*
	Matrix44Identity(m);
	m->m[0][0] = x.x;
	m->m[1][0] = x.y;
	m->m[2][0] = x.z;
	m->m[3][0] = 0.0f;
			  
	m->m[0][1] = y.x;
	m->m[1][1] = y.y;
	m->m[2][1] = y.z;
	m->m[3][1] = 0.0f;
			  
	m->m[0][2] = -z.x;
	m->m[1][2] = -z.y;
	m->m[2][2] = -z.z;
	m->m[3][2] = 0.0f;
	
//	m->m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
*/

}



inline void Matrix44Billboard( Matrix44 *m, const Vector3 *to, const Vector3 *up ) {

#ifdef SSE

	Vector3 z;
//	z.x = m->m[3][0];
//	z.y = m->m[3][1];
//	z.z = m->m[3][2];
	z.m128 = m->m4;
	Vector3SubVector3(&z, &z, to); // z = z - to
	Vector3Normalize(&z, &z); // z = |z|
	Vector3 y = *up;
	Vector3 x;
	Vector3Cross(&x, &y, &z); // x = y cross z
	Vector3Cross(&z, &x, &y); // z = x cross y
	Vector3Normalize(&x, &x); // x = |x|
	Vector3Normalize(&y, &y); // y = |y|
	Vector3Normalize(&z, &z); // z = |z|
	m->m1 = x.m128;
	m->m2 = y.m128;
	m->m3 = z.m128;

#else // SSE
	// (TBD)
#endif // SSE
}


// Code taken from Intel pdf "Streaming SIMD Extension - Inverse of 4x4 Matrix"

inline void Matrix44Invert( Matrix44 *m ) {

#ifdef SSE

	static __m128 undef;
	register float* src = m->raw;

    __m128 minor0, minor1, minor2, minor3;
    __m128 row0, row1, row2, row3;
    __m128 det, tmp1;

    tmp1 = _mm_loadh_pi(_mm_loadl_pi(undef, (__m64*)(src)), (__m64*)(src+ 4));
    row1 = _mm_loadh_pi(_mm_loadl_pi(undef, (__m64*)(src+8)), (__m64*)(src+12));

    row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
    row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);

    tmp1 = _mm_loadh_pi(_mm_loadl_pi(undef, (__m64*)(src+ 2)), (__m64*)(src+ 6));
    row3 = _mm_loadh_pi(_mm_loadl_pi(undef, (__m64*)(src+10)), (__m64*)(src+14));

    row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
    row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);

    tmp1 = _mm_mul_ps(row2, row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor0 = _mm_mul_ps(row1, tmp1);
    minor1 = _mm_mul_ps(row0, tmp1);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
    minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
    minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);

    tmp1 = _mm_mul_ps(row1, row2);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
    minor3 = _mm_mul_ps(row0, tmp1);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
    minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
    minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);

    tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    row2 = _mm_shuffle_ps(row2, row2, 0x4E);

    minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
    minor2 = _mm_mul_ps(row0, tmp1);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
    minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
    minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);

    tmp1 = _mm_mul_ps(row0, row1);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
    minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
    minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));

    tmp1 = _mm_mul_ps(row0, row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
    minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
    minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));

    tmp1 = _mm_mul_ps(row0, row2);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
    minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
    minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);

    det = _mm_mul_ps(row0, minor0);
    det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
    det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
    tmp1 = _mm_rcp_ss(det);

    det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
    det = _mm_shuffle_ps(det, det, 0x00);

    minor0 = _mm_mul_ps(det, minor0);
    _mm_storel_pi((__m64*)(src), minor0);    _mm_storeh_pi((__m64*)(src+2), minor0);

    minor1 = _mm_mul_ps(det, minor1);
    _mm_storel_pi((__m64*)(src+4), minor1);
    _mm_storeh_pi((__m64*)(src+6), minor1);

    minor2 = _mm_mul_ps(det, minor2);
    _mm_storel_pi((__m64*)(src+ 8), minor2);
    _mm_storeh_pi((__m64*)(src+10), minor2);

    minor3 = _mm_mul_ps(det, minor3);
    _mm_storel_pi((__m64*)(src+12), minor3);
    _mm_storeh_pi((__m64*)(src+14), minor3);


#else // SSE

	#define M11 m->m[0][0]
	#define M12 m->m[0][1]
	#define M13 m->m[0][2]
	#define M14 m->m[0][3]
	#define M21 m->m[1][0]
	#define M22 m->m[1][1]
	#define M23 m->m[1][2]
	#define M24 m->m[1][3]
	#define M31 m->m[2][0]
	#define M32 m->m[2][1]
	#define M33 m->m[2][2]
	#define M34 m->m[2][3]
	#define M41 m->m[3][0]
	#define M42 m->m[3][1]
	#define M43 m->m[3][2]
	#define M44 m->m[3][3]

	float s =
		(M11 * M22 - M12 * M21) * (M33 * M44 - M34 * M43)
		-(M11 * M23 - M13 * M21) * (M32 * M44 - M34 * M42)
		+(M11 * M24 - M14 * M21) * (M32 * M43 - M33 * M42)
		+(M12 * M23 - M13 * M22) * (M31 * M44 - M34 * M41)
		-(M12 * M24 - M14 * M22) * (M31 * M43 - M33 * M41)
		+(M13 * M24 - M14 * M23) * (M31 * M42 - M32 * M41);

	if ( s == 0.0 )
		return;
	s = 1/s;

	Matrix44 tmp;

	tmp.m[0][0] = s*(M22*(M33*M44 - M34*M43) + M23*(M34*M42 - M32*M44) + M24*(M32*M43 - M33*M42));
	tmp.m[0][1] = s*(M32*(M13*M44 - M14*M43) + M33*(M14*M42 - M12*M44) + M34*(M12*M43 - M13*M42));
	tmp.m[0][2] = s*(M42*(M13*M24 - M14*M23) + M43*(M14*M22 - M12*M24) + M44*(M12*M23 - M13*M22));
	tmp.m[0][3] = s*(M12*(M24*M33 - M23*M34) + M13*(M22*M34 - M24*M32) + M14*(M23*M32 - M22*M33));
	tmp.m[1][0] = s*(M23*(M31*M44 - M34*M41) + M24*(M33*M41 - M31*M43) + M21*(M34*M43 - M33*M44));
	tmp.m[1][1] = s*(M33*(M11*M44 - M14*M41) + M34*(M13*M41 - M11*M43) + M31*(M14*M43 - M13*M44));
	tmp.m[1][2] = s*(M43*(M11*M24 - M14*M21) + M44*(M13*M21 - M11*M23) + M41*(M14*M23 - M13*M24));
	tmp.m[1][3] = s*(M13*(M24*M31 - M21*M34) + M14*(M21*M33 - M23*M31) + M11*(M23*M34 - M24*M33));
	tmp.m[2][0] = s*(M24*(M31*M42 - M32*M41) + M21*(M32*M44 - M34*M42) + M22*(M34*M41 - M31*M44));
	tmp.m[2][1] = s*(M34*(M11*M42 - M12*M41) + M31*(M12*M44 - M14*M42) + M32*(M14*M41 - M11*M44));
	tmp.m[2][2] = s*(M44*(M11*M22 - M12*M21) + M41*(M12*M24 - M14*M22) + M42*(M14*M21 - M11*M24));
	tmp.m[2][3] = s*(M14*(M22*M31 - M21*M32) + M11*(M24*M32 - M22*M34) + M12*(M21*M34 - M24*M31));
	tmp.m[3][0] = s*(M21*(M33*M42 - M32*M43) + M22*(M31*M43 - M33*M41) + M23*(M32*M41 - M31*M42));
	tmp.m[3][1] = s*(M31*(M13*M42 - M12*M43) + M32*(M11*M43 - M13*M41) + M33*(M12*M41 - M11*M42));
	tmp.m[3][2] = s*(M41*(M13*M22 - M12*M23) + M42*(M11*M23 - M13*M21) + M43*(M12*M21 - M11*M22));
	tmp.m[3][3] = s*(M11*(M22*M33 - M23*M32) + M12*(M23*M31 - M21*M33) + M13*(M21*M32 - M22*M31));

	Matrix44Load(m, &tmp);

#endif // SSE

}



inline void Matrix44MultVector3( Matrix44 *m, Vector3 *dst, Vector3 *src ) { // dstVector = srcVector . m

#ifdef SSE

	dst->m128 =
		_mm_add_ps(
		_mm_add_ps(
		_mm_add_ps(
			_mm_mul_ps(m->m1, _mm_shuffle_ps(src->m128, src->m128, _MM_SHUFFLE(0,0,0,0))),
			_mm_mul_ps(m->m2, _mm_shuffle_ps(src->m128, src->m128, _MM_SHUFFLE(1,1,1,1)))),
			_mm_mul_ps(m->m3, _mm_shuffle_ps(src->m128, src->m128, _MM_SHUFFLE(2,2,2,2)))),
			_mm_mul_ps(m->m4, _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f)));

/*
	dst->m128 = 
		_mm_add_ps(
      _mm_add_ps(
      _mm_add_ps(
           _mm_mul_ps(m->m1, _mm_shuffle_ps(src->m128, src->m128, _MM_SHUFFLE(0,0,0,0))),
           _mm_mul_ps(m->m2, _mm_shuffle_ps(src->m128, src->m128, _MM_SHUFFLE(1,1,1,1)))),
           _mm_mul_ps(m->m3, _mm_shuffle_ps(src->m128, src->m128, _MM_SHUFFLE(2,2,2,2)))),
           _mm_mul_ps(m->m4, _mm_shuffle_ps(src->m128, src->m128, _MM_SHUFFLE(3,3,3,3))));
*/

#else // SSE
	// (TBD)
#endif // SSE

}



inline void Matrix44MultVector4( Matrix44 *m, Vector4 *dst, Vector4 *src ) {

#ifdef SSE

	dst->m128 = 
		_mm_add_ps(
      _mm_add_ps(
      _mm_add_ps(
           _mm_mul_ps(m->m1, _mm_shuffle_ps(src->m128, src->m128, _MM_SHUFFLE(0,0,0,0))),
           _mm_mul_ps(m->m2, _mm_shuffle_ps(src->m128, src->m128, _MM_SHUFFLE(1,1,1,1)))),
           _mm_mul_ps(m->m3, _mm_shuffle_ps(src->m128, src->m128, _MM_SHUFFLE(2,2,2,2)))),
           _mm_mul_ps(m->m4, _mm_shuffle_ps(src->m128, src->m128, _MM_SHUFFLE(3,3,3,3))));

#else // SSE
	// (TBD)
#endif // SSE

}


inline void Matrix44Transpose( Matrix44 *rm, Matrix44 *m ) {

#ifdef SSE

	__m128 t3, t2, t1, t0;
	t0 = _mm_unpacklo_ps(m->m1, m->m2);
	t2 = _mm_unpackhi_ps(m->m1, m->m2);
	t1 = _mm_unpacklo_ps(m->m3, m->m4);
	t3 = _mm_unpackhi_ps(m->m3, m->m4);
	rm->m1 = _mm_movelh_ps(t0, t1);
	rm->m2 = _mm_movehl_ps(t1, t0);
	rm->m3 = _mm_movelh_ps(t2, t3);
	rm->m4 = _mm_movehl_ps(t3, t2);

#else // SSE
	// (TBD)
#endif // SSE

}


#endif // _MATRIX44_H_
