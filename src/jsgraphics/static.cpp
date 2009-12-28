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

#include "jstransformation.h"

#define _USE_MATH_DEFINES
#include <math.h>


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
 $ARRAY $INAME( vector | x [,y,z] )
**/
DEFINE_FUNCTION_FAST( NewVector3 ) {
	
	Vector3 v;
	if ( argc == 1 )	{

		uint32 len;
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), v.raw, 3, &len) );
		JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
	}
	if ( argc == 3 ) {

		JL_CHK( JsvalToFloat(cx, JL_FARG(1), &v.x) );
		JL_CHK( JsvalToFloat(cx, JL_FARG(2), &v.y) );
		JL_CHK( JsvalToFloat(cx, JL_FARG(3), &v.z) );
	}
	return FloatVectorToJsval(cx, v.raw, 3, JL_FRVAL);
	JL_REPORT_ERROR( "Invalid vector." );
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
DEFINE_FUNCTION_FAST( Vector3Length ) {
	
	Vector3 v;
	if ( argc == 1 )	{

		uint32 len;
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), v.raw, 3, &len) );
		JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
	} else
	if ( argc == 2 ) {

		Vector3 v2;
		uint32 len;
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), v.raw, 3, &len) );
		JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(2), v2.raw, 3, &len) );
		JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
		Vector3SubVector3(&v, &v, &v2);
	} else
	if ( argc == 3 ) {

		JL_CHK( JsvalToFloat(cx, JL_FARG(1), &v.x) );
		JL_CHK( JsvalToFloat(cx, JL_FARG(2), &v.y) );
		JL_CHK( JsvalToFloat(cx, JL_FARG(3), &v.z) );
	}
	return FloatToJsval(cx, Vector3Length(&v), JL_FRVAL);
	JL_REPORT_ERROR( "Unsupported vector type." );
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
DEFINE_FUNCTION_FAST( Vector3Normalize ) {
	
	Vector3 v;
	uint32 len;
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), v.raw, 3, &len) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );

	Vector3Normalize(&v, &v);

	*JL_FRVAL = JL_FARG(JL_FARG_ISDEF(2) ? 2 : 1);
	return FloatVectorToJsval(cx, v.raw, 3, JL_FRVAL, true);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( vector, vector2 [ , dest ] )
  vector += vector2
**/
DEFINE_FUNCTION_FAST( Vector3Add ) {
	
	Vector3 v, v2;
	uint32 len;
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), v.raw, 3, &len) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(2), v2.raw, 3, &len) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );

	Vector3AddVector3(&v, &v, &v2);

	*JL_FRVAL = JL_FARG(JL_FARG_ISDEF(3) ? 3 : 1);
	return FloatVectorToJsval(cx, v.raw, 3, JL_FRVAL, true);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( vector, vector2 [ , dest ] )
  vector -= vector2
**/
DEFINE_FUNCTION_FAST( Vector3Sub ) {
	
	Vector3 v, v2;
	uint32 len;
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), v.raw, 3, &len) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(2), v2.raw, 3, &len) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );

	Vector3SubVector3(&v, &v, &v2);

	*JL_FRVAL = JL_FARG(JL_FARG_ISDEF(3) ? 3 : 1);
	return FloatVectorToJsval(cx, v.raw, 3, JL_FRVAL, true);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( vector, vector2 [ , dest ] )
  vector *= vector2
**/
DEFINE_FUNCTION_FAST( Vector3Cross ) {
	
	Vector3 v, v2;
	uint32 len;
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), v.raw, 3, &len) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(2), v2.raw, 3, &len) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );

	Vector3Cross(&v, &v, &v2);

	*JL_FRVAL = JL_FARG(JL_FARG_ISDEF(3) ? 3 : 1);
	return FloatVectorToJsval(cx, v.raw, 3, JL_FRVAL, true);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( vector, vector2 [ , dest ] )
  vector . vector2
**/
DEFINE_FUNCTION_FAST( Vector3Dot ) {
	
	Vector3 v, v2;
	uint32 len;
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), v.raw, 3, &len) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(2), v2.raw, 3, &len) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );

	*JL_FRVAL = JL_FARG(JL_FARG_ISDEF(3) ? 3 : 1);
	return FloatToJsval(cx, Vector3Dot(&v, &v2), JL_FRVAL);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( transformation [ , array ] ] )
  Returns the [x,y,z,radius] sphere that surrounds the frustrum and crosses the eye and far corners.
**/
DEFINE_FUNCTION_FAST( FrustumSphere ) {
	
	JL_S_ASSERT_ARG_RANGE(1,2);

	Matrix44 tmpMat, *m = &tmpMat;
	JL_CHK( GetMatrixHelper(cx, JL_FARG(1), (float**)&m) );

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
	Vector3Mult(&p1, &p1, 0.5 * Vector3Dot(&p2, &p2) / Vector3Dot(&p1, &p1)); // now p1 is the center of the frustum sphere.
	float radius = Vector3Length(&p1);
	Vector3AddVector3(&p1, &p1, &p0);

//	Vector3 tmp2;
//	Vector3SubVector3(&tmp2, &p2, &center);
//	float d1 = Vector3Length(&tmp2);
//	float d2 = Vector3Length(&center);

	if ( JL_FARG_ISDEF(2) ) {

		JL_CHK( FloatVectorToJsval(cx, p1.raw, 3, &JL_FARG(2), true) );
		*JL_FRVAL = JL_FARG(2);
	} else {

		JL_CHK( FloatVectorToJsval(cx, p1.raw, 3, JL_FRVAL) );
	}

	jsval tmpVal;
	JL_CHK( FloatToJsval(cx, radius, &tmpVal) );
	JL_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT(*JL_FRVAL), 3, &tmpVal) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( $ARR box )
  _box_ is a JavaScript Array: [minx, miny, minz,  maxx, maxy, maxz]
**/
DEFINE_FUNCTION_FAST( BoxToCircumscribedSphere ) {
	
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_ARRAY(JL_FARG(1));

	float aabb[6];

	uint32 len;
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), aabb, 6, &len) );
	JL_S_ASSERT( len == 6, "Invalid vector length (%d).", len );

	Vector3 v1, v2, center;
	Vector3LoadPtr(&v1, &aabb[0]);
	Vector3LoadPtr(&v2, &aabb[3]);

	Vector3SubVector3(&center, &v1, &v2);
	Vector3Div(&center, &center, 2);
	float radius = Vector3Length(&center);
	Vector3AddVector3(&center, &center, &v2);
	
	*JL_FRVAL = JL_FARG(1);
	JL_CHK( FloatVectorToJsval(cx, center.raw, 3, JL_FRVAL, true) );
	jsval tmpVal;
	JL_CHK( FloatToJsval(cx, radius, &tmpVal) );
	JL_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT(*JL_FRVAL), 3, &tmpVal) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME( $TYPE vec4 src [, $TYPE vec3 dest] )
  if _dest is $UNDEF, _src_ is used to store the result.
**/
DEFINE_FUNCTION_FAST( QuaternionToEuler ) {

	// see http://www.google.com/codesearch/p?hl=en#kpcXlMp9-Eg/cs7491/projects/proj002/cs7491.zip|pBok6PPJvB8/cs7491/MatrixLib/Quaternion.cpp&q=EulerToQuaternion%20lang:c++&l=185&t=1
	// eg. http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/steps/index.htm
	Vector4 quat;
	Vector3 euler;
	uint32 len;
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), quat.raw, 4, &len) );
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

	*JL_FRVAL = JL_FARG(JL_FARG_ISDEF(2) ? 2 : 1);
	JL_CHK( FloatVectorToJsval(cx, euler.raw, 3, JL_FRVAL, true) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE vec4 $INAME( $TYPE vec3 src [, $TYPE vec4 dest] )
  If _dest is omited, the result is stored in _src_.$LF
  To store the result in a new array, use: `$INAME( str, [] );`

**/
DEFINE_FUNCTION_FAST( EulerToQuaternion ) {

	Vector3 euler;
	Vector4 quat;
	uint32 len;
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), euler.raw, 3, &len) );
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

	*JL_FRVAL = JL_FARG(JL_FARG_ISDEF(2) ? 2 : 1);
	JL_CHK( FloatVectorToJsval(cx, quat.raw, 4, JL_FRVAL, true) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME( $TYPE vec4 src [, $TYPE vec4 dest] )
  _dest_ has the following form: [ x, y, z, angle ]
  If _dest is omited, the result is stored in _src_.$LF
**/
DEFINE_FUNCTION_FAST( QuaternionToAxisAngle ) {

	Vector4 quat;
	uint32 len;
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), quat.raw, 4, &len) );
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

	*JL_FRVAL = JL_FARG(JL_FARG_ISDEF(2) ? 2 : 1);
	JL_CHK( FloatVectorToJsval(cx, quat.raw, 4, JL_FRVAL, true) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME( $TYPE vec4 src [, $TYPE vec4 dest] )
  _src_ has the following form: [ x, y, z, angle ]
**/
DEFINE_FUNCTION_FAST( AxisAngleToQuaternion ) {

	Vector4 axisAngle;
	uint32 len;
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), axisAngle.raw, 4, &len) );
	JL_S_ASSERT( len == 4, "Invalid quaternion." );

	float halfAngle, cs, sn;
	halfAngle = axisAngle.w / 2.f;
	cs = cosf(halfAngle);
	sn = sinf(halfAngle);

	Vector4Mult(&axisAngle, &axisAngle, sn);
	axisAngle.w = cs;

	*JL_FRVAL = JL_FARG(JL_FARG_ISDEF(2) ? 2 : 1);
	JL_CHK( FloatVectorToJsval(cx, axisAngle.raw, 4, JL_FRVAL, true) );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME( $TYPE matrix44 matrix )
**/
DEFINE_FUNCTION_FAST( GetMatrix ) {

	JL_S_ASSERT_ARG(1);
	float tmp[16], *m = tmp;

	JL_S_ASSERT_OBJECT( JL_FARG(1) );
	JSObject *matrixObj = JSVAL_TO_OBJECT( JL_FARG(1) );
	NIMatrix44Get fct = Matrix44GetInterface(cx, matrixObj);
	JL_S_ASSERT( fct, "Invalid Matrix44 interface." );
	JL_CHK( fct(cx, matrixObj, &m) );
	JL_CHK( FloatVectorToJsval(cx, m, 16, JL_FRVAL, false) );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST_ARGC( NewVector3, 3 )
		FUNCTION_FAST_ARGC( Vector3Length, 3 )
		FUNCTION_FAST_ARGC( Vector3Normalize, 1 )
		FUNCTION_FAST_ARGC( Vector3Add, 2 )
		FUNCTION_FAST_ARGC( Vector3Sub, 2 )
		FUNCTION_FAST_ARGC( Vector3Cross, 2 )
		FUNCTION_FAST_ARGC( Vector3Dot, 2 )

		FUNCTION_FAST_ARGC( FrustumSphere, 2 )
		FUNCTION_FAST_ARGC( BoxToCircumscribedSphere, 1 )
		FUNCTION_FAST_ARGC( QuaternionToEuler, 1 )
		FUNCTION_FAST_ARGC( EulerToQuaternion, 1 )
		FUNCTION_FAST_ARGC( QuaternionToAxisAngle, 1 )
		FUNCTION_FAST_ARGC( AxisAngleToQuaternion, 1 )
		
		FUNCTION_FAST_ARGC( GetMatrix, 1 )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

END_STATIC
