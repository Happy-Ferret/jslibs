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

#ifndef _MATRIX44_H_
#define _MATRIX44_H_

#include <stdlib.h>
#include <math.h>

#include "../common/vector3.h"
#include "../common/vector4.h"

#ifdef SSE // SSE (Streaming SIMD Extensions)

#include <xmmintrin.h>
#include <fvec.h>

typedef __declspec(align(16)) union {
	struct {
		__m128 m1;
		__m128 m2;
		__m128 m3;
		__m128 m4;
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
	free(m);
#endif // SSE

}

inline Matrix44 *Matrix44Alloc() {

#ifdef SSE
	// Doc: For example, if you use malloc, the result depends on the operand size. If arg >= 8,
	//      alignment will be 8 byte aligned. If arg < 8, alignment will be the first power of 2 less than arg.
	//      For example, if you use malloc(7), alignment is 4 bytes.
	return (Matrix44*)_aligned_malloc(sizeof(Matrix44), __alignof(Matrix44));
#else // SSE
	return (Matrix44*)malloc(sizeof(Matrix44));
#endif // SSE
}

inline void Matrix44Identity( Matrix44 *m ) {

#ifdef SSE
	m->m1 = Matrix44IdentityValue.m1;
	m->m2 = Matrix44IdentityValue.m2;
	m->m3 = Matrix44IdentityValue.m3;
	m->m4 = Matrix44IdentityValue.m4;
#else // SSE
	memcpy(m->raw, Matrix44IdentityValue.raw, sizeof(Matrix44IdentityValue));
#endif // SSE

}

inline void Matrix44Load( Matrix44 *m, Matrix44 *m1 ) {

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


inline void Matrix44MultSimple( Matrix44 *rm, Matrix44 *m, Matrix44 *mx ) {

#ifdef SSE

	rm->m1 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(m->m1, m->m1, _MM_SHUFFLE(0,0,0,0)), mx->m1), _mm_mul_ps(_mm_shuffle_ps(m->m1, m->m1, _MM_SHUFFLE(1,1,1,1)), mx->m2)), _mm_mul_ps(_mm_shuffle_ps(m->m1, m->m1, _MM_SHUFFLE(2,2,2,2)), mx->m3)), _mm_mul_ps(_mm_shuffle_ps(m->m1, m->m1, _MM_SHUFFLE(3,3,3,3)), mx->m4));
	rm->m2 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(m->m2, m->m2, _MM_SHUFFLE(0,0,0,0)), mx->m1), _mm_mul_ps(_mm_shuffle_ps(m->m2, m->m2, _MM_SHUFFLE(1,1,1,1)), mx->m2)), _mm_mul_ps(_mm_shuffle_ps(m->m2, m->m2, _MM_SHUFFLE(2,2,2,2)), mx->m3)), _mm_mul_ps(_mm_shuffle_ps(m->m2, m->m2, _MM_SHUFFLE(3,3,3,3)), mx->m4));
	rm->m3 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(m->m3, m->m3, _MM_SHUFFLE(0,0,0,0)), mx->m1), _mm_mul_ps(_mm_shuffle_ps(m->m3, m->m3, _MM_SHUFFLE(1,1,1,1)), mx->m2)), _mm_mul_ps(_mm_shuffle_ps(m->m3, m->m3, _MM_SHUFFLE(2,2,2,2)), mx->m3)), _mm_mul_ps(_mm_shuffle_ps(m->m3, m->m3, _MM_SHUFFLE(3,3,3,3)), mx->m4));
	rm->m4 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(m->m4, m->m4, _MM_SHUFFLE(0,0,0,0)), mx->m1), _mm_mul_ps(_mm_shuffle_ps(m->m4, m->m4, _MM_SHUFFLE(1,1,1,1)), mx->m2)), _mm_mul_ps(_mm_shuffle_ps(m->m4, m->m4, _MM_SHUFFLE(2,2,2,2)), mx->m3)), _mm_mul_ps(_mm_shuffle_ps(m->m4, m->m4, _MM_SHUFFLE(3,3,3,3)), mx->m4));
	
#else // SSE

	for (int i=0; i<4; i++) {

		float mi0 = m->m[i][0];
		float mi1 = m->m[i][1];
		float mi2 = m->m[i][2];
		m->m[i][0] = mi0*mx->m[0][0] + mi1*mx->m[1][0] + mi2*mx->m[2][0];
		m->m[i][1] = mi0*mx->m[0][1] + mi1*mx->m[1][1] + mi2*mx->m[2][1];
		m->m[i][2] = mi0*mx->m[0][2] + mi1*mx->m[1][2] + mi2*mx->m[2][2];
	}
	m->m[3][0] += mx->m[3][0];
	m->m[3][1] += mx->m[3][1];
	m->m[3][2] += mx->m[3][2];
	m->m[0][3] = 0.0f;
	m->m[1][3] = 0.0f;
	m->m[2][3] = 0.0f;
	m->m[3][3] = 1.0f;

#endif // SSE

}




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
	// (TBD)
#endif // SSE
}


inline FORCEINLINE float Sin(float angle) {

#ifdef SSE
	__asm fld	angle
	__asm fsin
#else // SSE
	// (TBD)
#endif // SSE
}

inline FORCEINLINE float Cos(float angle)	{

#ifdef SSE
	__asm fld	angle
	__asm fcos
#else // SSE
	// (TBD)
#endif // SSE
}

inline FORCEINLINE void SinCos(float angle, float *sinVal, float *cosVal) {

#ifdef SSE
	__asm fld		[angle]
	__asm fsincos
	__asm mov		eax,cosVal
	__asm fstp		dword ptr [eax]
	__asm mov		eax,sinVal
	__asm fstp		dword ptr [eax]
#else // SSE
	// (TBD)
#endif // SSE
}

inline FORCEINLINE float Sqrt(float val) {

#ifdef SSE
	__asm fld		val
	__asm fsqrt
#else // SSE
	return sqrtf(val);
#endif // SSE
}

inline void Matrix44SetRotation( Matrix44 *m, const Vector3 *axis, float radAngle ) {

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

	float w,x,y,z;
	float ax = axis->x;
	float ay = axis->y;
	float az = axis->z;
	float l = ax*ax + ay*ay + az*az;
	if (l > 0.f) {

		float s;
		SinCos(radAngle/2, &s, &w);
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


inline void Matrix44LookAt( Matrix44 *m, const Vector3 *from, const Vector3 *to, const Vector3 *up ) {
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
	Vector3Sub(&z, from, to);
	Vector3Normalize(&z); // z = |z|
	Vector3 y = *up;
//	y.pad = 0.0f;
	Vector3 x;
	Vector3Cross(&x, &y, &z); // x = y cross z
	Vector3Cross(&y, &z, &x); // z = x cross y
	Vector3Normalize(&x); // x = |x|
	Vector3Normalize(&y); // y = |y|
//	Vector3Inv(&z, &z);
//	m->m1 = x.m128;
//	m->m2 = y.m128;
//	m->m3 = z.m128;
//	m->m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
//	_MM_TRANSPOSE4_PS(m->m1, m->m2, m->m3, m->m4);

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
	
	m->m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);

}



inline void Matrix44Billboard( Matrix44 *m, const Vector3 *to, const Vector3 *up ) {

#ifdef SSE

	Vector3 z;
//	z.x = m->m[3][0];
//	z.y = m->m[3][1];
//	z.z = m->m[3][2];
	z.m128 = m->m4;
	Vector3Sub(&z, &z, to); // z = z - to
	Vector3Normalize(&z); // z = |z|
	Vector3 y = *up;
	Vector3 x;
	Vector3Cross(&x, &y, &z); // x = y cross z
	Vector3Cross(&z, &x, &y); // z = x cross y
	Vector3Normalize(&x); // x = |x|
	Vector3Normalize(&y); // y = |y|
	Vector3Normalize(&z); // z = |z|
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

	static __m128 undef = {0,0,0,0};
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
	// (TBD)
#endif // SSE

}



inline void Matrix44MultVector3( Matrix44 *m, Vector3 *srcVector, Vector3 *dstVector ) { // dstVector = srcVector . m

#ifdef SSE

	__m128 src = srcVector->m128;
	dstVector->m128 =
		_mm_add_ps(
		_mm_add_ps(
		_mm_add_ps(
			_mm_mul_ps(m->m1, _mm_shuffle_ps(src, src, _MM_SHUFFLE(0,0,0,0))),
			_mm_mul_ps(m->m2, _mm_shuffle_ps(src, src, _MM_SHUFFLE(1,1,1,1)))),
			_mm_mul_ps(m->m3, _mm_shuffle_ps(src, src, _MM_SHUFFLE(2,2,2,2)))),
			_mm_mul_ps(m->m4, _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f)));

#else // SSE
	// (TBD)
#endif // SSE

}



inline void Matrix44MultVector4( Matrix44 *m, Vector4 *srcVector, Vector4 *dstVector ) { // dstVector = srcVector . m

#ifdef SSE

	__m128 src = srcVector->m128;
	dstVector->m128 =
		_mm_add_ps(
		_mm_add_ps(
		_mm_add_ps(
			_mm_mul_ps(m->m1, _mm_shuffle_ps(src, src, _MM_SHUFFLE(0,0,0,0))),
			_mm_mul_ps(m->m2, _mm_shuffle_ps(src, src, _MM_SHUFFLE(1,1,1,1)))),
			_mm_mul_ps(m->m3, _mm_shuffle_ps(src, src, _MM_SHUFFLE(2,2,2,2)))),
			_mm_mul_ps(m->m4, _mm_shuffle_ps(src, src, _MM_SHUFFLE(3,3,3,3))));

#else // SSE
	// (TBD)
#endif // SSE

}


inline void Matrix44Transpose( Matrix44 *m ) {

#ifdef SSE
	_MM_TRANSPOSE4_PS(m->m1, m->m2, m->m3, m->m4);
/*
	__m128 t3, t2, t1, t0;
	t0 = _mm_unpacklo_ps(m->m1, m->m2);
	t2 = _mm_unpackhi_ps(m->m1, m->m2);
	t1 = _mm_unpacklo_ps(m->m3, m->m4);
	t3 = _mm_unpackhi_ps(m->m3, m->m4);
	rm->m1 = _mm_movelh_ps(t0, t1);
	rm->m2 = _mm_movehl_ps(t1, t0);
	rm->m3 = _mm_movelh_ps(t2, t3);
	rm->m4 = _mm_movehl_ps(t3, t2);
*/
#else // SSE
	// (TBD)
#endif // SSE

}


#endif // _MATRIX44_H_
