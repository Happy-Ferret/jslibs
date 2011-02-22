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


/**doc fileIndex:topmost **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_STATIC


/**doc
=== Static functions ===
**/


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( value )
 $ARRAY $INAME( [x, y, z] )
 $ARRAY $INAME( x, y, z )
**/
DEFINE_FUNCTION( Vec3 ) {
	
	Vector3 v;
	if ( argc == 1 ) {

		if ( JL_IsArray(cx, JL_ARG(1)) ) {

			uint32 len;
			JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), v.raw, 3, &len) );
			JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
		} else {

			JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &v.x) );
			v.z = v.y = v.x;
		}
	} else
	if ( argc == 3 ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &v.x) );
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &v.y) );
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &v.z) );
	} else
		JL_REPORT_ERROR_NUM(cx, JLSMSG_LOGIC_ERROR, "invalid vector3");

	return JL_NativeVectorToJsval(cx, v.raw, 3, JL_RVAL);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( vector )
 $REAL $INAME( vector, vector2 )
 $REAL $INAME( x,y,z )
  {{{
  $INAME(1,2,3) == $INAME([1,2,3]);
  }}}
**/
DEFINE_FUNCTION( Vec3Length ) {
	
	Vector3 v;
	if ( argc == 1 )	{

		uint32 len;
		JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), v.raw, 3, &len) );
		JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
	} else
	if ( argc == 2 ) {

		Vector3 v2;
		uint32 len;
		JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), v.raw, 3, &len) );
		JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
		JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(2), v2.raw, 3, &len) );
		JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
		Vector3SubVector3(&v, &v, &v2);
	} else
	if ( argc == 3 ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &v.x) );
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &v.y) );
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &v.z) );
	} else
		JL_REPORT_ERROR_NUM(cx, JLSMSG_LOGIC_ERROR, "unsupported operation");

	return JL_NativeToJsval(cx, Vector3Length(&v), JL_RVAL);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( vector [ , dest ] )
  $H example 1
  {{{
  var v = [6,7,8];
  Vector3Normalize( v );
  Print( v ); // prints: 0.491455078125,0.5733642578125,0.6552734375
  }}}
  $H example 2
  {{{
  var v = { length:3, 0:6, 1:7, 2:8 };
  Print( Array.slice( Vector3Normalize( v ) ) ); // prints: 0.491455078125,0.5733642578125,0.6552734375
  }}}
**/
DEFINE_FUNCTION( Vec3Normalize ) {
	
	Vector3 v;
	uint32 len;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), v.raw, 3, &len) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );

	Vector3Normalize(&v, &v);

	*JL_RVAL = JL_ARG(JL_ARG_ISDEF(2) ? 2 : 1);
	return JL_NativeVectorToJsval(cx, v.raw, 3, JL_RVAL, true);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( vector, vector2 [ , dest ] )
  vector += vector2
**/
DEFINE_FUNCTION( Vec3Add ) {
	
	Vector3 v, v2;
	uint32 len, len2;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), v.raw, 3, &len) );
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(2), v2.raw, 3, &len2) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len2 );

	Vector3AddVector3(&v, &v, &v2);

	*JL_RVAL = JL_ARG(JL_ARG_ISDEF(3) ? 3 : 1);
	return JL_NativeVectorToJsval(cx, v.raw, 3, JL_RVAL, true);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( vector, vector2 [ , dest ] )
  vector -= vector2
**/
DEFINE_FUNCTION( Vec3Sub ) {
	
	Vector3 v, v2;
	uint32 len, len2;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), v.raw, 3, &len) );
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(2), v2.raw, 3, &len2) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len2 );

	Vector3SubVector3(&v, &v, &v2);

	*JL_RVAL = JL_ARG(JL_ARG_ISDEF(3) ? 3 : 1);
	return JL_NativeVectorToJsval(cx, v.raw, 3, JL_RVAL, true);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( vector, vector2 [ , dest ] )
  vector *= vector2
**/
DEFINE_FUNCTION( Vec3Cross ) {
	
	Vector3 v, v2;
	uint32 len, len2;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), v.raw, 3, &len) );
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(2), v2.raw, 3, &len2) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len2 );

	Vector3Cross(&v, &v, &v2);

	*JL_RVAL = JL_ARG(JL_ARG_ISDEF(3) ? 3 : 1);
	return JL_NativeVectorToJsval(cx, v.raw, 3, JL_RVAL, true);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( vector, vector2 [ , dest ] )
  vector . vector2
**/
DEFINE_FUNCTION( Vec3Dot ) {
	
	Vector3 v, v2;
	uint32 len, len2;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), v.raw, 3, &len) );
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(2), v2.raw, 3, &len2) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len2 );

	*JL_RVAL = JL_ARG(JL_ARG_ISDEF(3) ? 3 : 1);
	return JL_NativeToJsval(cx, Vector3Dot(&v, &v2), JL_RVAL);
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( transformation [ , array ] ] )
  Returns the [x,y,z,radius] sphere that surrounds the frustrum and crosses the eye and far corners.
**/
DEFINE_FUNCTION( FrustumSphere ) {
	
	JL_S_ASSERT_ARG_RANGE(1,2);

	Matrix44 tmpMat, *m = &tmpMat;
	JL_CHK( GetMatrixHelper(cx, JL_ARG(1), (float**)&m) );

	// see. http://www.flipcode.com/archives/Frustum_Culling.shtml

	Vector4 tmp;
	Vector3 p0, p1, p2;

	Vector4Set(&tmp, 0, 0, 0, 1);
	Matrix44MultVector4(m, &tmp, &tmp);
	Vector4Div(&tmp, &tmp, tmp.w);
	Vector3LoadVector4(&p0, &tmp);

	Vector4Set(&tmp, 0, 0, 1, 1);
	Matrix44MultVector4(m, &tmp, &tmp);
	Vector4Div(&tmp, &tmp, tmp.w);
	Vector3LoadVector4(&p1, &tmp);
	
	Vector4Set(&tmp, 1, 1, 1, 1);
	Matrix44MultVector4(m, &tmp, &tmp);
	Vector4Div(&tmp, &tmp, tmp.w);
	Vector3LoadVector4(&p2, &tmp);

	Vector3SubVector3(&p1, &p1, &p0);
	Vector3SubVector3(&p2, &p2, &p0);
	Vector3Mult(&p1, &p1, 0.5f * Vector3Dot(&p2, &p2) / Vector3Dot(&p1, &p1)); // now p1 is the center of the frustum sphere.
	float radius = Vector3Length(&p1);
	Vector3AddVector3(&p1, &p1, &p0);

//	Vector3 tmp2;
//	Vector3SubVector3(&tmp2, &p2, &center);
//	float d1 = Vector3Length(&tmp2);
//	float d2 = Vector3Length(&center);

	if ( JL_ARG_ISDEF(2) ) {

		JL_CHK( JL_NativeVectorToJsval(cx, p1.raw, 3, &JL_ARG(2), true) );
		*JL_RVAL = JL_ARG(2);
	} else {

		JL_CHK( JL_NativeVectorToJsval(cx, p1.raw, 3, JL_RVAL) );
	}

	jsval tmpVal;
	JL_CHK(JL_NativeToJsval(cx, radius, &tmpVal) );
	JL_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT(*JL_RVAL), 3, &tmpVal) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( $ARR box )
  _box_ is a JavaScript Array: [minx, miny, minz,  maxx, maxy, maxz]
**/
DEFINE_FUNCTION( BoxToCircumscribedSphere ) {
	
	JL_S_ASSERT_ARG_COUNT(1);
	JL_S_ASSERT_ARRAY(JL_ARG(1));

	float aabb[6];

	uint32 len;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), aabb, 6, &len) );
	JL_S_ASSERT( len == 6, "Invalid vector length (%d).", len );

	Vector3 v1, v2, center;
	Vector3LoadPtr(&v1, &aabb[0]);
	Vector3LoadPtr(&v2, &aabb[3]);

	Vector3SubVector3(&center, &v1, &v2);
	Vector3Div(&center, &center, 2);
	float radius = Vector3Length(&center);
	Vector3AddVector3(&center, &center, &v2);
	
	*JL_RVAL = JL_ARG(1);
	JL_CHK( JL_NativeVectorToJsval(cx, center.raw, 3, JL_RVAL, true) );
	jsval tmpVal;
	JL_CHK(JL_NativeToJsval(cx, radius, &tmpVal) );
	JL_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT(*JL_RVAL), 3, &tmpVal) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME( $TYPE vec4 src [, $TYPE vec3 dest] )
  if _dest is $UNDEF, _src_ is used to store the result.
**/
DEFINE_FUNCTION( QuaternionToEuler ) {

	// see http://www.google.com/codesearch/p?hl=en#kpcXlMp9-Eg/cs7491/projects/proj002/cs7491.zip|pBok6PPJvB8/cs7491/MatrixLib/Quaternion.cpp&q=EulerToQuaternion%20lang:c++&l=185&t=1
	// eg. http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/steps/index.htm
	Vector4 quat;
	Vector3 euler;
	uint32 len;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), quat.raw, 4, &len) );
	JL_S_ASSERT( len == 4, "Invalid quaternion." );

/*
	float sqw, sqx, sqy, sqz;
	sqw = quat.w * quat.w;
	sqx = quat.x * quat.x;
	sqy = quat.y * quat.y;
	sqz = quat.z * quat.z;
	
	euler.x = atan2f(2.0 * ( quat.y * quat.z + quat.x * quat.w ), ( -sqx - sqy + sqz + sqw ));
	euler.y = asinf(-2.0 * ( quat.x * quat.z - quat.y * quat.w ));
	euler.z = atan2f(2.0 * ( quat.x * quat.y + quat.z * quat.w ), (  sqx - sqy - sqz + sqw ));
*/

	float q0q0, q1q1, q2q2, q3q3, q0q1, q0q2, q0q3, q1q2, q1q3, q2q3;

	q0q0 = quat.w*quat.w;
	q1q1 = quat.x*quat.x;
	q2q2 = quat.y*quat.y;
	q3q3 = quat.z*quat.z;
	q0q1 = quat.w*quat.x;
	q0q2 = quat.w*quat.y;
	q0q3 = quat.w*quat.z;
	q1q2 = quat.x*quat.y;
	q1q3 = quat.x*quat.z;
	q2q3 = quat.y*quat.z;
	euler.y = asinf(2*(q1q3+q0q2));

	// If cos(ry) is zero, then we must avoid dividing by zero. It
	// also becomes impossible to distinguish roll from yaw.
	// adopting the convention that the yaw angle is 0
	// temp = fabs(ry-PI*0.5);
	// temp1 = EPISOLON;
	//if(EPISOLON > fabs(ry-PI*0.5))
	//{
	//      rz = 0;
	//      rx = asin(2*(q2q3 + q0q1));
	//}
	//else
	//{

	euler.x = atan2f(-(2*(q2q3-q0q1)),(q0q0-q1q1-q2q2+q3q3));
	euler.z = atan2f(-(2*(q1q2-q0q3)),(q0q0+q1q1-q2q2-q3q3));

	*JL_RVAL = JL_ARG(JL_ARG_ISDEF(2) ? 2 : 1);
	JL_CHK( JL_NativeVectorToJsval(cx, euler.raw, 3, JL_RVAL, true) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE vec4 $INAME( $TYPE vec3 src [, $TYPE vec4 dest] )
  If _dest is omited, the result is stored in _src_.$LF
  To store the result in a new array, use: `$INAME( str, [] );`

**/
DEFINE_FUNCTION( EulerToQuaternion ) {

	Vector3 euler;
	Vector4 quat;
	uint32 len;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), euler.raw, 3, &len) );
	JL_S_ASSERT( len == 3, "Invalid euler rotation." );
	
	float cosx,cosy,cosz,sinx,siny,sinz,cc,cs,sc,ss;

	Vector3Mult( &euler, &euler, .5f );

	cosx = cosf(euler.x);
	cosy = cosf(euler.y);
	cosz = cosf(euler.z);
	sinx = sinf(euler.x);
	siny = sinf(euler.y);
	sinz = sinf(euler.z);

	cc = cosy*cosz;
   cs = cosy*sinz;
   sc = siny*cosz;
   ss = siny*sinz;

	quat.x = sinx*cc + cosx*ss;
	quat.y = cosx*sc - sinx*cs;
	quat.z = cosx*cs + sinx*sc;
	quat.w = cosx*cc - sinx*ss;

/* normalize the quaternion ?
	float distance;
	distance = quat.x*quat.x + quat.y*quat.y + quat.z*quat.z + quat.w*quat.w;
	if (distance == 0.0)
		distance = 1.0;
	//distance = sqrtf(distance);
	quat.x /= distance;
	quat.y /= distance;
	quat.z /= distance;
	quat.w /= distance;
*/

	*JL_RVAL = JL_ARG(JL_ARG_ISDEF(2) ? 2 : 1);
	JL_CHK( JL_NativeVectorToJsval(cx, quat.raw, 4, JL_RVAL, true) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME( $TYPE vec4 src [, $TYPE vec4 dest] )
  _dest_ has the following form: [ x, y, z, angle ]
  If _dest is omited, the result is stored in _src_.$LF
**/
DEFINE_FUNCTION( QuaternionToAxisAngle ) {

	Vector4 quat;
	uint32 len;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), quat.raw, 4, &len) );
	JL_S_ASSERT( len == 4, "Invalid quaternion." );

	float halfAngle, sn;
	halfAngle = acosf(quat.w);
	sn = sinf(halfAngle);

	if ( sn > 0.00001f || sn < -0.00001f ) {

		Vector4Div(&quat, &quat, sn);
		quat.w = halfAngle * 2.f;
	} else {

		Vector4Set(&quat, 0,0,0,0);
	}

	*JL_RVAL = JL_ARG(JL_ARG_ISDEF(2) ? 2 : 1);
	JL_CHK( JL_NativeVectorToJsval(cx, quat.raw, 4, JL_RVAL, true) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME( $TYPE vec4 src [, $TYPE vec4 dest] )
  _src_ has the following form: [ x, y, z, angle ]
**/
DEFINE_FUNCTION( AxisAngleToQuaternion ) {

	Vector4 axisAngle;
	uint32 len;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), axisAngle.raw, 4, &len) );
	JL_S_ASSERT( len == 4, "Invalid quaternion." );

	float halfAngle, cs, sn;
	halfAngle = axisAngle.w / 2.f;
	cs = cosf(halfAngle);
	sn = sinf(halfAngle);

	Vector4Mult(&axisAngle, &axisAngle, sn);
	axisAngle.w = cs;

	*JL_RVAL = JL_ARG(JL_ARG_ISDEF(2) ? 2 : 1);
	JL_CHK( JL_NativeVectorToJsval(cx, axisAngle.raw, 4, JL_RVAL, true) );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE vec16 $INAME( $TYPE matrix44 matrix )
**/
DEFINE_FUNCTION( GetMatrix ) {

	JL_S_ASSERT_ARG_COUNT(1);
	float tmp[16], *m = tmp;

	JL_S_ASSERT_OBJECT( JL_ARG(1) );
	JSObject *matrixObj = JSVAL_TO_OBJECT( JL_ARG(1) );
	NIMatrix44Get fct = Matrix44GetInterface(cx, matrixObj);
	JL_S_ASSERT( fct, "Invalid Matrix44 interface." );
	JL_CHK( fct(cx, matrixObj, &m) );
	JL_CHK( JL_NativeVectorToJsval(cx, m, 16, JL_RVAL, false) );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE vec4 $INAME( $TYPE vec3 p1, $TYPE vec3 p2, $TYPE vec3 p3 )
  Find the plane equation given 3 points.
**/
DEFINE_FUNCTION( PlaneFromPoints ) {

	float plane[4], v0[3], v1[3], v2[3];

	jsuint len1, len2, len3;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), v0, 3, &len1) );
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(2), v1, 3, &len2) );
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(3), v2, 3, &len3) );
	JL_S_ASSERT( len1 == 3, "Invalid point (arg1)." );
	JL_S_ASSERT( len2 == 3, "Invalid point (arg2)." );
	JL_S_ASSERT( len3 == 3, "Invalid point (arg3)." );


	float vec0[3], vec1[3];

	/* Need 2 vectors to find cross product. */
	vec0[0] = v1[0] - v0[0];
	vec0[1] = v1[1] - v0[1];
	vec0[2] = v1[2] - v0[2];

	vec1[0] = v2[0] - v0[0];
	vec1[1] = v2[1] - v0[1];
	vec1[2] = v2[2] - v0[2];

	/* find cross product to get A, B, and C of plane equation */
	plane[0] = vec0[1] * vec1[2] - vec0[2] * vec1[1];
	plane[1] = -(vec0[0] * vec1[2] - vec0[2] * vec1[0]);
	plane[2] = vec0[0] * vec1[1] - vec0[1] * vec1[0];

	plane[3] = -(plane[0] * v0[0] + plane[1] * v0[1] + plane[2] * v0[2]);

	return JL_NativeVectorToJsval(cx, plane, 4, JL_RVAL, false);
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE vec4 $INAME( $TYPE vec4 plane, $TYPE vec4 lightPos [, $ARR destArray] )
  Create a matrix that will project the desired shadow.
 $H see
  http://www.opengl.org/resources/code/samples/advanced/advanced97/notes/node100.html
**/
DEFINE_FUNCTION( ShadowMatrix ) {

	JL_S_ASSERT_ARG_RANGE(2,3);

	double shadowMat[4][4], plane[4], lightpos[4];

	uint32 len, len1;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), plane, 4, &len) );
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(2), lightpos, 4, &len1) );
	JL_S_ASSERT( len == 4, "Invalid plane." );
	JL_S_ASSERT( len1 == 4, "Invalid light position." );

	/* Find dot product between light position vector and ground plane normal. */
	double dot;
	dot = plane[0] * lightpos[0] + plane[1] * lightpos[1] + plane[2] * lightpos[2] + plane[3] * lightpos[3];

	shadowMat[0][0] = dot - lightpos[0] * plane[0];
	shadowMat[1][0] = 0.f - lightpos[0] * plane[1];
	shadowMat[2][0] = 0.f - lightpos[0] * plane[2];
	shadowMat[3][0] = 0.f - lightpos[0] * plane[3];

	shadowMat[0][1] = 0.f - lightpos[1] * plane[0];
	shadowMat[1][1] = dot - lightpos[1] * plane[1];
	shadowMat[2][1] = 0.f - lightpos[1] * plane[2];
	shadowMat[3][1] = 0.f - lightpos[1] * plane[3];

	shadowMat[0][2] = 0.f - lightpos[2] * plane[0];
	shadowMat[1][2] = 0.f - lightpos[2] * plane[1];
	shadowMat[2][2] = dot - lightpos[2] * plane[2];
	shadowMat[3][2] = 0.f - lightpos[2] * plane[3];

	shadowMat[0][3] = 0.f - lightpos[3] * plane[0];
	shadowMat[1][3] = 0.f - lightpos[3] * plane[1];
	shadowMat[2][3] = 0.f - lightpos[3] * plane[2];
	shadowMat[3][3] = dot - lightpos[3] * plane[3];

	if ( JL_ARGC == 3 ) {

		JL_S_ASSERT_ARRAY(JL_ARG(3));
		*JL_RVAL = JL_ARG(3);
		return JL_NativeVectorToJsval(cx, (double*)shadowMat, 16, JL_RVAL, true);
	} else {

		return JL_NativeVectorToJsval(cx, (double*)shadowMat, 16, JL_RVAL, false);
	}
	JL_BAD;
}



CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_ARGC( Vec3, 3 )
		FUNCTION_ARGC( Vec3Length, 3 )
		FUNCTION_ARGC( Vec3Normalize, 1 )
		FUNCTION_ARGC( Vec3Add, 2 )
		FUNCTION_ARGC( Vec3Sub, 2 )
		FUNCTION_ARGC( Vec3Cross, 2 )
		FUNCTION_ARGC( Vec3Dot, 2 )

		FUNCTION_ARGC( FrustumSphere, 2 )
		FUNCTION_ARGC( BoxToCircumscribedSphere, 1 )
		FUNCTION_ARGC( QuaternionToEuler, 1 )
		FUNCTION_ARGC( EulerToQuaternion, 1 )
		FUNCTION_ARGC( QuaternionToAxisAngle, 1 )
		FUNCTION_ARGC( AxisAngleToQuaternion, 1 )
		
		FUNCTION_ARGC( GetMatrix, 1 )

		FUNCTION_ARGC( PlaneFromPoints, 3 )
		FUNCTION_ARGC( ShadowMatrix, 2 )

	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

END_STATIC
