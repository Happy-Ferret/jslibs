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
	First Person Shooter tricks:
		http://www.delphi3d.net/articles/printarticle.php?article=viewing.htm
*/

#include "stdafx.h"
#include "jstransformation.h"

#define _USE_MATH_DEFINES
#include <math.h>



/**doc fileIndex:top
$CLASS_HEADER
$SVN_REVISION $Revision: 2633 $
**/
BEGIN_CLASS( Math3d )

/**doc
=== Static functions ===
**/


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( transformation , array ] )
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
 $ARRAY $INAME( vector | x [,y,z] )
  {{{
  $INAME(1,2,3) == $INAME([1,2,3]);
  }}}
**/
DEFINE_FUNCTION_FAST( Vector3Length ) {
	
	Vector3 v;
	if ( argc == 1 )	{

		size_t len;
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), v.raw, 3, &len) );
		JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
		JL_CHK( FloatToJsval(cx, Vector3Length(&v), JL_FRVAL) );
		return JS_TRUE;
	}
	if ( argc == 3 ) {

		JL_CHK( JsvalToFloat(cx, JL_FARG(1), &v.x) );
		JL_CHK( JsvalToFloat(cx, JL_FARG(2), &v.y) );
		JL_CHK( JsvalToFloat(cx, JL_FARG(3), &v.z) );
		JL_CHK( FloatToJsval(cx, Vector3Length(&v), JL_FRVAL) );
		return JS_TRUE;
	}
	JL_REPORT_ERROR( "Invalid vector." );
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME([minx, miny, minz,  maxx, maxy, maxz])
**/
DEFINE_FUNCTION_FAST( BoxToCircumscribedSphere ) {
	
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_ARRAY(JL_FARG(1));

	float aabb[6];

	size_t len;
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



/** doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( transformation )
**/
/*
DEFINE_FUNCTION_FAST( PixelWidth ) {
	
	JL_S_ASSERT_ARG(1);

	Matrix44 tmpMat, *m = &tmpMat;
	JL_CHK( GetMatrixHelper(cx, JL_FARG(1), (float**)&m) );
}	
*/	
	
CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 2633 $"))

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST_ARGC( FrustumSphere, 2 )
		FUNCTION_FAST_ARGC( Vector3Length, 3 )
		FUNCTION_FAST_ARGC( BoxToCircumscribedSphere, 1 )
	END_STATIC_FUNCTION_SPEC

END_CLASS
