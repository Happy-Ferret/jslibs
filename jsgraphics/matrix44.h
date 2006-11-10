#pragma once
/*
	source:
		http://nebuladevice.svn.sourceforge.net/viewvc/nebuladevice/trunk/nebula2/code/nebula2/inc/mathlib/_matrix44_sse.h?view=markup

	SSE Intro:
		http://www.codeproject.com/cpp/sseintro.asp
*/

#include <xmmintrin.h>

#include <stdlib.h>
#include <math.h>

//#include <fvec.h>

#include "vector3.h"

typedef __declspec(align(16)) union {
    struct {
        __m128 m1;
        __m128 m2;
        __m128 m3;
        __m128 m4;
    };
    struct {
        float m[4][4];
    };
	 struct {
		float raw[16];
	 };
} Matrix44;


static float Matrix44IdentityValue[] = {

    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f 
};

inline void Matrix44Free( Matrix44 *m ) {

	return _aligned_free(m);
}

inline Matrix44 *Matrix44Alloc() {

	return (Matrix44*)_aligned_malloc(sizeof(Matrix44),16);
}

inline void Matrix44Identity( Matrix44 *m ) {

	memcpy(m->raw, Matrix44IdentityValue, sizeof(Matrix44IdentityValue));
}

inline void Matrix44Multiply( Matrix44 *m, const Matrix44 *mx ) {

	__m128 sm1 = mx->m1;
	__m128 sm2 = mx->m2;
	__m128 sm3 = mx->m3;
	__m128 sm4 = mx->m4;

	__m128 mx1 = m->m1;
	__m128 mx2 = m->m2;
	__m128 mx3 = m->m3;
	__m128 mx4 = m->m4;

	m->m1 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(sm1, sm1, _MM_SHUFFLE(0,0,0,0)), mx1), _mm_mul_ps(_mm_shuffle_ps(sm1, sm1, _MM_SHUFFLE(1,1,1,1)), mx2)), _mm_mul_ps(_mm_shuffle_ps(sm1, sm1, _MM_SHUFFLE(2,2,2,2)), mx3)), _mm_mul_ps(_mm_shuffle_ps(sm1, sm1, _MM_SHUFFLE(3,3,3,3)), mx4));
	m->m2 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(sm2, sm2, _MM_SHUFFLE(0,0,0,0)), mx1), _mm_mul_ps(_mm_shuffle_ps(sm2, sm2, _MM_SHUFFLE(1,1,1,1)), mx2)), _mm_mul_ps(_mm_shuffle_ps(sm2, sm2, _MM_SHUFFLE(2,2,2,2)), mx3)), _mm_mul_ps(_mm_shuffle_ps(sm2, sm2, _MM_SHUFFLE(3,3,3,3)), mx4));
	m->m3 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(sm3, sm3, _MM_SHUFFLE(0,0,0,0)), mx1), _mm_mul_ps(_mm_shuffle_ps(sm3, sm3, _MM_SHUFFLE(1,1,1,1)), mx2)), _mm_mul_ps(_mm_shuffle_ps(sm3, sm3, _MM_SHUFFLE(2,2,2,2)), mx3)), _mm_mul_ps(_mm_shuffle_ps(sm3, sm3, _MM_SHUFFLE(3,3,3,3)), mx4));
	m->m4 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(sm4, sm4, _MM_SHUFFLE(0,0,0,0)), mx1), _mm_mul_ps(_mm_shuffle_ps(sm4, sm4, _MM_SHUFFLE(1,1,1,1)), mx2)), _mm_mul_ps(_mm_shuffle_ps(sm4, sm4, _MM_SHUFFLE(2,2,2,2)), mx3)), _mm_mul_ps(_mm_shuffle_ps(sm4, sm4, _MM_SHUFFLE(3,3,3,3)), mx4));
}

inline void Matrix44MultiplyWithoutSSE( Matrix44 *m, const Matrix44 *mx ) {

	float _tmp[16];

	_tmp[0]  = m->raw[0]*mx->raw[0]  + m->raw[4]*mx->raw[1]  + m->raw[8] *mx->raw[2]  + m->raw[12]*mx->raw[3];
	_tmp[1]  = m->raw[1]*mx->raw[0]  + m->raw[5]*mx->raw[1]  + m->raw[9] *mx->raw[2]  + m->raw[13]*mx->raw[3];
	_tmp[2]  = m->raw[2]*mx->raw[0]  + m->raw[6]*mx->raw[1]  + m->raw[10]*mx->raw[2]  + m->raw[14]*mx->raw[3];
	_tmp[3]  = m->raw[3]*mx->raw[0]  + m->raw[7]*mx->raw[1]  + m->raw[11]*mx->raw[2]  + m->raw[15]*mx->raw[3];

	_tmp[4]  = m->raw[0]*mx->raw[4]  + m->raw[4]*mx->raw[5]  + m->raw[8] *mx->raw[6]  + m->raw[12]*mx->raw[7];
	_tmp[5]  = m->raw[1]*mx->raw[4]  + m->raw[5]*mx->raw[5]  + m->raw[9] *mx->raw[6]  + m->raw[13]*mx->raw[7];
	_tmp[6]  = m->raw[2]*mx->raw[4]  + m->raw[6]*mx->raw[5]  + m->raw[10]*mx->raw[6]  + m->raw[14]*mx->raw[7];
	_tmp[7]  = m->raw[3]*mx->raw[4]  + m->raw[7]*mx->raw[5]  + m->raw[11]*mx->raw[6]  + m->raw[15]*mx->raw[7];

	_tmp[8]  = m->raw[0]*mx->raw[8]  + m->raw[4]*mx->raw[9]  + m->raw[8] *mx->raw[10] + m->raw[12]*mx->raw[11];
	_tmp[9]  = m->raw[1]*mx->raw[8]  + m->raw[5]*mx->raw[9]  + m->raw[9] *mx->raw[10] + m->raw[13]*mx->raw[11];
	_tmp[10] = m->raw[2]*mx->raw[8]  + m->raw[6]*mx->raw[9]  + m->raw[10]*mx->raw[10] + m->raw[14]*mx->raw[11];
	_tmp[11] = m->raw[3]*mx->raw[8]  + m->raw[7]*mx->raw[9]  + m->raw[11]*mx->raw[10] + m->raw[15]*mx->raw[11];

	_tmp[12] = m->raw[0]*mx->raw[12] + m->raw[4]*mx->raw[13] + m->raw[8] *mx->raw[14] + m->raw[12]*mx->raw[15];
	_tmp[13] = m->raw[1]*mx->raw[12] + m->raw[5]*mx->raw[13] + m->raw[9] *mx->raw[14] + m->raw[13]*mx->raw[15];
	_tmp[14] = m->raw[2]*mx->raw[12] + m->raw[6]*mx->raw[13] + m->raw[10]*mx->raw[14] + m->raw[14]*mx->raw[15];
	_tmp[15] = m->raw[3]*mx->raw[12] + m->raw[7]*mx->raw[13] + m->raw[11]*mx->raw[14] + m->raw[15]*mx->raw[15];

	memcpy(m->raw, _tmp, sizeof(_tmp) );
}



inline void Matrix44Rotation( Matrix44 *m, const Vector3 *axis, float radAngle ) {

	Vector3 v = *axis;
	// Opt: check if already normalized
	Vector3Normalize(&v);
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


/*
	float w,x,y,z;

	float ax = axis->x;
	float ay = axis->y;
	float az = axis->z;

	float l = ax*ax + ay*ay + az*az;
	if (l > 0.f) {

		radAngle /= 2;
		w = cos(radAngle);
		l = sin(radAngle) / sqrtf(l);
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

//	Matrix44 rot;
//	Matrix44Identity(&rot);

	m->raw[0]  = 1 - 2 * ( yy + zz );
	m->raw[1]  =     2 * ( xy - zw );
	m->raw[2]  =     2 * ( xz + yw );
	m->raw[4]  =     2 * ( xy + zw );
	m->raw[5]  = 1 - 2 * ( xx + zz );
	m->raw[6]  =     2 * ( yz - xw );
	m->raw[8]  =     2 * ( xz - yw );
	m->raw[9]  =     2 * ( yz + xw );
	m->raw[10] = 1 - 2 * ( xx + yy );
//	Matrix44Multiply(m, &rot);
*/
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

inline void Matrix44Translation( Matrix44 *m, const Vector3 *t ) {

	m->m[3][0] = t->x;
	m->m[3][1] = t->y;
	m->m[3][2] = t->z;
	//	m->m4 = t->m128;
	//m->m4 = _mm_add_ps(m->m4, t->m128);
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


inline void Matrix44LookAt( Matrix44 *m, const Vector3 *to, const Vector3 *up ) {

	Vector3 z;
	z.x = m->m[3][0];
	z.y = m->m[3][1];
	z.z = m->m[3][2];
	Vector3Sub(&z, to); // z = z - to
	Vector3Normalize(&z); // z = |z|
	Vector3 y = *up;
	Vector3 x;
	Vector3Cross(&x, &y, &z); // x = y cross z
	Vector3Cross(&y, &z, &x); // y = z cross x
	Vector3Normalize(&x); // x = |x|
	Vector3Normalize(&y); // y = |y|
	m->m1 = x.m128;
	m->m2 = y.m128;
	m->m3 = z.m128;
}


inline void Matrix44Billboard( Matrix44 *m, const Vector3 *to, const Vector3 *up ) {

	Vector3 z;
	z.x = m->m[3][0];
	z.y = m->m[3][1];
	z.z = m->m[3][2];
	Vector3Sub(&z, to); // z = z - to
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
}






inline void Matrix44Invert( Matrix44 *m ) {

	float* src = m->raw;

    __m128 minor0, minor1, minor2, minor3;
    __m128 row0, row1, row2, row3;
    __m128 det, tmp1;

    tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src)), (__m64*)(src+ 4));
    row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(src+8)), (__m64*)(src+12));

    row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
    row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);

    tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src+ 2)), (__m64*)(src+ 6));
    row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(src+10)), (__m64*)(src+14));

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
    _mm_storel_pi((__m64*)(src), minor0);
    _mm_storeh_pi((__m64*)(src+2), minor0);

    minor1 = _mm_mul_ps(det, minor1);
    _mm_storel_pi((__m64*)(src+4), minor1);
    _mm_storeh_pi((__m64*)(src+6), minor1);

    minor2 = _mm_mul_ps(det, minor2);
    _mm_storel_pi((__m64*)(src+ 8), minor2);
    _mm_storeh_pi((__m64*)(src+10), minor2);

    minor3 = _mm_mul_ps(det, minor3);
    _mm_storel_pi((__m64*)(src+12), minor3);
    _mm_storeh_pi((__m64*)(src+14), minor3);
}

