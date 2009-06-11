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

// (TBD) move this in the class private

jl::Pool matrixPool;


static int GetMatrix(JSContext *cx, JSObject *obj, float **m) { // Doc: __declspec(noinline) tells the compiler to never inline a particular function.
	
	Matrix44 *objMatrix = (Matrix44*)JL_GetPrivate(cx, obj);
	*m = objMatrix->raw; // returns its private pointer. Caller SHOULD not modify it
	return true;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Transformation )

DEFINE_FINALIZE() {
	
	if ( obj == *_prototype ) {

		while ( !PoolIsEmpty(&matrixPool) )
			Matrix44Free((Matrix44*)jl::PoolPop(&matrixPool));
		jl::PoolFinalize(&matrixPool);
		return;
	}
//	printf("Fin:%d\n", matrixPoolLength);
	Matrix44 *m = (Matrix44*)JL_GetPrivate(cx, obj);
	if ( !jl::PoolPush(&matrixPool, m) )
		Matrix44Free(m);
}


/**doc
$TOC_MEMBER $INAME
 $INAME( [init] )
  Creates a new Transformation object. 
  If no argument is given, the transformation is not initialized.
  If _init_ is $UNDEF, the transformation is initialized to identity.
  If _init_ is a matrix, the transormation is initialized with the matrix.
**/
DEFINE_CONSTRUCTOR() {

//	printf("New:%d\n", matrixPoolLength);

	JL_S_ASSERT_CONSTRUCTING();
	Matrix44 *m;
	if ( PoolIsEmpty(&matrixPool) )
		m = Matrix44Alloc();
	else
		m = (Matrix44*)jl::PoolPop(&matrixPool);
	JL_S_ASSERT_ALLOC(m);

	if ( JL_ARGC >= 1 ) {
		
		if ( JSVAL_IS_VOID(JL_ARG(1)) )
			Matrix44Identity(m);
		else
			JL_CHK( GetMatrixHelper(cx, JL_ARG(1), &m) );
	}

	JL_SetPrivate(cx, JL_OBJ, m);
	JL_CHK( SetMatrix44GetInterface(cx, obj, GetMatrix) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  Reset the current transformation (set to identity).
**/
DEFINE_FUNCTION_FAST( Clear ) {

	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(tm);
	Matrix44Identity(tm);
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  Clear the rotation part of the current transformation.
**/
DEFINE_FUNCTION_FAST( ClearRotation ) {

	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(tm);
	Matrix44ClearRotation(tm);
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  Clear the translation part of the current transformation.
**/
DEFINE_FUNCTION_FAST( ClearTranslation ) {

	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(tm);
	Matrix44ClearTranslation(tm);
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( matrix )
  Load a 4x4 matrix as the current transformation.
  $H arguments
   $ARG $VAL matrix: an Array or an object that supports NIMatrix44Read native interface.
**/
DEFINE_FUNCTION_FAST( Load ) {

	JL_S_ASSERT_ARG_MIN(1);
	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(tm);
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);

	Matrix44 *m = tm;
	JL_CHK( GetMatrixHelper(cx, JL_FARG(1), &m) ); // GetMatrixHelper will copy data into tmp OR replace tmp by its own float pointer
	if ( m != tm ) // check if the pointer has been modified
		memcpy(tm, m, sizeof(Matrix44)); // if it is, copy the data
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( matrix )
  Load the rotation part of another matrix to the current matrix.
  $H arguments
   $ARG $VAL matrix: an Array or an object that supports NIMatrix44Read native interface.
**/
DEFINE_FUNCTION_FAST( LoadRotation ) {

	JL_S_ASSERT_ARG_MIN(1);
	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ); // tm for thisMatrix
	JL_S_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	JL_CHK( GetMatrixHelper(cx, JL_FARG(1), &m) );
	tm->raw[0]  = m->raw[0] ; //L1
	tm->raw[1]  = m->raw[1] ;
	tm->raw[2]  = m->raw[2] ;

	tm->raw[4]  = m->raw[4] ; //L2
	tm->raw[5]  = m->raw[5] ;
	tm->raw[6]  = m->raw[6] ;

	tm->raw[8]  = m->raw[8] ; //L3
	tm->raw[9]  = m->raw[9] ;
	tm->raw[10] = m->raw[10];
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( matrix )
  Load the translation part of another matrix to the current matrix.
  $H arguments
   $ARG $VAL matrix: an Array or an object that supports NIMatrix44Read native interface.
**/
DEFINE_FUNCTION_FAST( LoadTranslation ) {

	JL_S_ASSERT_ARG_MIN(1);
	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ); // tm for thisMatrix
	JL_S_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	JL_CHK( GetMatrixHelper(cx, JL_FARG(1), &m) );
	tm->raw[3]  = m->raw[3];
	tm->raw[7]  = m->raw[7];
	tm->raw[11] = m->raw[11];
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( x, y, z )
  Sets the translation part of the current transformation.
  $H arguments
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
**/
DEFINE_FUNCTION_FAST( Translation ) {

	JL_S_ASSERT_ARG_MIN(3); // x, y, z
	Matrix44 *m = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(m);
	float x, y, z;
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &x) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(2), &y) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(3), &z) ); 
	Matrix44SetTranslation(m, x,y,z);
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( x, y, z )
  Apply a translation to the current ransformation.
  $H arguments
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
**/
DEFINE_FUNCTION_FAST( Translate ) {

	JL_S_ASSERT_ARG_MIN(3); // x, y, z
	Matrix44 *m = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(m);
	float x, y, z;
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &x) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(2), &y) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(3), &z) ); 
	Matrix44 t;
	Matrix44Identity(&t);
	Matrix44SetTranslation(&t, x, y, z);
	Matrix44Product(m, &t);
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( w, x, y, z )
  Sets the rotation part from a quaternion.
  $H arguments
   $ARG $REAL w
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
**/
DEFINE_FUNCTION_FAST( RotationFromQuaternion ) {

	JL_S_ASSERT_ARG_MIN(4); // w, x, y, z
	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ); // tm for thisMatrix
	JL_S_ASSERT_RESOURCE(tm);

	float w, x, y, z;
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &w) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(2), &x) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(3), &y) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(4), &z) ); 


	float fTx  = 2.0 * x;
	float fTy  = 2.0 * y;
	float fTz  = 2.0 * z;
	float fTwx = fTx * w;
	float fTwy = fTy * w;
	float fTwz = fTz * w;
	float fTxx = fTx * x;
	float fTxy = fTy * x;
	float fTxz = fTz * x;
	float fTyy = fTy * y;
	float fTyz = fTz * y;
	float fTzz = fTz * z;

	tm->m[0][0] = 1.0-(fTyy+fTzz);
	tm->m[0][1] = fTxy-fTwz;
	tm->m[0][2] = fTxz+fTwy;
	tm->m[1][0] = fTxy+fTwz;
	tm->m[1][1] = 1.0-(fTxx+fTzz);
	tm->m[1][2] = fTyz-fTwx;
	tm->m[2][0] = fTxz-fTwy;
	tm->m[2][1] = fTyz+fTwx;
	tm->m[2][2] = 1.0-(fTxx+fTyy);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( roll, pitch, yaw )
  Sets the Tait-Bryan rotation.
  $H arguments
   $ARG $REAL roll
   $ARG $REAL pitch
   $ARG $REAL yaw
**/
DEFINE_FUNCTION_FAST( TaitBryanRotation ) {

	JL_S_ASSERT_ARG_MIN(3); // roll, pitch, yaw
	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ); // tm for thisMatrix
	JL_S_ASSERT_RESOURCE(tm);

	float roll, pitch, yaw;
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &roll) );
	JL_CHK( JsvalToFloat(cx, JL_FARG(2), &pitch) );
	JL_CHK( JsvalToFloat(cx, JL_FARG(3), &yaw) );

	// (TBD)

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( angle, x, y, z )
  Sets the rotation part of the current transformation.
  $H arguments
   $ARG $REAL angle in degres
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
**/
DEFINE_FUNCTION_FAST( Rotation ) {

	JL_S_ASSERT_ARG_MIN(4); // angle, x, y, z
	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ); // tm for thisMatrix
	JL_S_ASSERT_RESOURCE(tm);

	float angle, x, y, z;
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &angle) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(2), &x) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(3), &y) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(4), &z) ); 
	Vector3 axis;
	Vector3Set(&axis, x,y,z);
	Matrix44SetRotation(tm, &axis, -angle * M_PI / 360.0f);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( angle, x, y, z )
  Apply a rotation to the current ransformation.
  $H arguments
   $ARG $REAL angle in degres
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
**/
DEFINE_FUNCTION_FAST( Rotate ) {

	JL_S_ASSERT_ARG_MIN(4); // angle, x, y, z
	Matrix44 *m = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(m);
	float angle, x, y, z;
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &angle) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(2), &x) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(3), &y) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(4), &z) ); 
	Vector3 axis;
	Vector3Set(&axis, x,y,z);
	Matrix44 r;
	Matrix44Identity(&r);
	Matrix44SetRotation(&r, &axis, -angle * M_PI / 360.0f);
	Matrix44Product(m, &r);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( angle )
  Set the rotation around the X axis.
  $H arguments
   $ARG $REAL angle in degres
**/
DEFINE_FUNCTION_FAST( RotationX ) {

	JL_S_ASSERT_ARG_MIN(1); // angle
	Matrix44 *m = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(m);
	float angle;
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &angle) ); 
//	Matrix44 r;
	Matrix44SetXRotation(m, -angle * M_PI / 360.0f);
//	Matrix44Product(m, &r);
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( angle )
  Set the rotation around the Y axis.
  $H arguments
   $ARG $REAL angle in degres
**/
DEFINE_FUNCTION_FAST( RotationY ) {

	JL_S_ASSERT_ARG_MIN(1); // angle
	Matrix44 *m = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(m);
	float angle;
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &angle) ); 
//	Matrix44 r;
	Matrix44SetYRotation(m, -angle * M_PI / 360.0f);
//	Matrix44Product(m, &r);
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( angle )
  Set the rotation around the Z axis.
  $H arguments
   $ARG $REAL angle in degres
**/
DEFINE_FUNCTION_FAST( RotationZ ) {

	JL_S_ASSERT_ARG_MIN(1); // angle
	Matrix44 *m = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(m);
	float angle;
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &angle) ); 
//	Matrix44 r;
	Matrix44SetZRotation(m, -angle * M_PI / 360.0f);
//	Matrix44Product(m, &r);
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( x, y, z )
  unavailable: need to be fixed.
  $H arguments
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
**/
DEFINE_FUNCTION_FAST( LookAt ) {

/*
    _vector3_sse z(from - to);
    z.norm();
    _vector3_sse y(up);
    _vector3_sse x(y * z);   // x = y cross z
    y = z * x;          // y = z cross x
    x.norm();
    y.norm();

    m1 = x.m128;
    m2 = y.m128;
    m3 = z.m128;
*/

	JL_REPORT_ERROR("LookAt is buggy !! dont' use it");

	JL_S_ASSERT_ARG_MIN(3); // x, y, z
	Matrix44 *m = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(m);
	float x, y, z;
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &x) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(2), &y) ); 
	JL_CHK( JsvalToFloat(cx, JL_FARG(3), &z) ); 
	Vector3 up;
	Vector3Set(&up, 0,0,1);
	Vector3 to;
	Vector3Set(&to, x,y,z);
	Matrix44LookAt(m, &to, &up);
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( x, y, z )
  Apply the (0,0,1)-(x,y,z) angle to the current transformation.
**/
DEFINE_FUNCTION_FAST( RotateToVector ) {

	JL_S_ASSERT_ARG_MIN(3); // x, y, z
	Matrix44 *m = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(m);
	float x, y, z;
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &x) );
	JL_CHK( JsvalToFloat(cx, JL_FARG(2), &y) );
	JL_CHK( JsvalToFloat(cx, JL_FARG(3), &z) );

	Vector3 to, up;
	Vector3Set(&to, x,y,z);
	Vector3Normalize(&to);
	Vector3Set(&up, 0,0,1);

	float angle = acos(Vector3Dot(&up, &to));
	
	Vector3Cross(&up, &up, &to);

	Matrix44 r;
	Matrix44Identity(&r);
	Matrix44SetRotation(&r, &up, -angle * M_PI / 360.0f);
	Matrix44Product(m, &r);
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  invert the current transformation.
**/
DEFINE_FUNCTION_FAST( Invert ) {

	Matrix44 *m = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(m);
	Matrix44Invert(m);
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( newTransformation )
  Apply a _newTransformation_ to the current transformation.
  this = this . new
  $H arguments
   $ARG $VAL newTransformation: an Array or an object that supports NIMatrix44Read native interface.
**/
DEFINE_FUNCTION_FAST( Product ) {

	JL_S_ASSERT_ARG_MIN(1);
	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ); // tm for thisMatrix
	JL_S_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	JL_CHK( GetMatrixHelper(cx, JL_FARG(1), &m) );
	Matrix44Product(tm, m); // <- mult
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( otherTransformation )
  Apply the current transformation to the _otherTransformation_ and stores the result to the current transformation.
  this = new . this
  $H arguments
   $ARG $VAL otherTransformation: an Array or an object that supports NIMatrix44Read native interface.
**/
DEFINE_FUNCTION_FAST( ReverseProduct ) {

	JL_S_ASSERT_ARG_MIN(1);
	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ); // tm for thisMatrix
	JL_S_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	JL_CHK( GetMatrixHelper(cx, JL_FARG(1), &m) );
	Matrix44ReverseProduct(tm, m); // <- mult
	*JL_FRVAL = OBJECT_TO_JSVAL(JL_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( vector )
  Transforms the 3D or 4D _vector_ by the current transformation.
  $H arguments
   $ARG $ARRAY vector
**/
DEFINE_FUNCTION_FAST( TransformVector ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_ARRAY( JL_FARG(1) );

	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, JL_FOBJ); // tm for thisMatrix
	JL_S_ASSERT_RESOURCE(tm);

	size_t length;
//	J_JSVAL_TO_ARRAY_LENGTH( JL_FARG(1), length );
	jsuint tmp;
	JL_CHK( JS_GetArrayLength(cx, JSVAL_TO_OBJECT(JL_FARG(1)), &tmp) );
	length = tmp;

	jsval tmpValue;
	if ( length == 3 ) {

		Vector3 src, dst;
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), src.raw, 3, &length ) );
		Matrix44MultVector3( tm, &src, &dst );

		JL_CHK( DoubleToJsval(cx, dst.x, &tmpValue) );
		JL_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( JL_FARG(1) ), 0, &tmpValue) );

		JL_CHK( DoubleToJsval(cx, dst.y, &tmpValue) );
		JL_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( JL_FARG(1) ), 1, &tmpValue) );

		JL_CHK( DoubleToJsval(cx, dst.z, &tmpValue) );
		JL_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( JL_FARG(1) ), 2, &tmpValue) );
	} else
	if ( length == 4 ) {

		Vector4 src, dst;
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), src.raw, 4, &length ) );
		Matrix44MultVector4( tm, &src, &dst );

		JL_CHK( DoubleToJsval(cx, dst.x, &tmpValue) );
		JL_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( JL_FARG(1) ), 0, &tmpValue) );

		JL_CHK( DoubleToJsval(cx, dst.y, &tmpValue) );
		JL_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( JL_FARG(1) ), 1, &tmpValue) );

		JL_CHK( DoubleToJsval(cx, dst.z, &tmpValue) );
		JL_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( JL_FARG(1) ), 2, &tmpValue) );

		JL_CHK( DoubleToJsval(cx, dst.w, &tmpValue) );
		JL_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( JL_FARG(1) ), 3, &tmpValue) );
	} else {

		JL_REPORT_ERROR( "Unsupported vector length (%d).", length );
	}

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
  Gets or sets the position component.
**/
DEFINE_PROPERTY_SETTER( translation ) {

	JL_S_ASSERT_ARRAY( *vp );
	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, obj); // tm for thisMatrix
	JL_S_ASSERT_RESOURCE(tm);
	float pos[3];
	size_t len;
	JL_CHK( JsvalToFloatVector(cx, *vp, pos, 3, &len) );
	Matrix44SetTranslation(tm, pos[0], pos[1], pos[2]);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( translation ) {

	Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, obj); // tm for thisMatrix
	JL_S_ASSERT_RESOURCE(tm);
	float pos[3];
	Matrix44GetTranslation(tm, &pos[0], &pos[1], &pos[2]);
	JL_CHK( FloatVectorToJsval(cx, pos, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/*
DEFINE_NEW_RESOLVE() {

	if (!JSVAL_IS_INT(id) || (flags & JSRESOLVE_ASSIGNING))
		return JS_TRUE;
	jsint slot = JSVAL_TO_INT( id );
	JL_S_ASSERT( slot >= 0 && slot <= 15, "Out of range." );
	JS_DefineProperty(cx, obj, (char*)slot, JSVAL_VOID, NULL, NULL, JSPROP_INDEX | JSPROP_SHARED );
	*objp = obj;
	return JS_TRUE;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME
  Get or set a element of the current transformation matrix.
  $H example
  {{{
  var t = new Transformation();
  t.Clear();
  t[3] = 1;
  t[7] = 2;
  t[11] = 3;
  }}}
**/
DEFINE_GET_PROPERTY() {

	if ( JSVAL_IS_INT(id) ) {

		Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, obj); // tm for thisMatrix
		JL_S_ASSERT_RESOURCE(tm);
		jsint slot = JSVAL_TO_INT( id );
		JL_S_ASSERT( slot >= 0 && slot <= 15, "Out of range." );
		JL_CHK( DoubleToJsval(cx, tm->raw[slot], vp) );
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_SET_PROPERTY() {

	if ( JSVAL_IS_INT(id) ) {

		Matrix44 *tm = (Matrix44*)JL_GetPrivate(cx, obj); // tm for thisMatrix
		JL_S_ASSERT_RESOURCE(tm);
		JL_S_ASSERT_NUMBER(*vp);
		jsint slot = JSVAL_TO_INT( id );
		JL_S_ASSERT( slot >= 0 && slot <= 15, "Out of range." );
		tm->raw[slot] = JSVAL_IS_DOUBLE(*vp) ? *JSVAL_TO_DOUBLE(*vp) : JSVAL_TO_INT(*vp);
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Native Interface ===
 * *NIMatrix44Read*
  The current transformation matrix.
**/

DEFINE_INIT() {

	jl::PoolInitialize( &matrixPool, 4096 );
	return JS_TRUE;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_INIT

	HAS_CONSTRUCTOR
	HAS_FINALIZE
//	HAS_NEW_RESOLVE
	HAS_GET_PROPERTY
	HAS_SET_PROPERTY

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST_ARGC( Clear, 0 )
		FUNCTION_FAST_ARGC( Load, 1 )
		FUNCTION_FAST_ARGC( LoadRotation, 1 )
		FUNCTION_FAST_ARGC( LoadTranslation, 1 )
		FUNCTION_FAST_ARGC( Product, 1 )
		FUNCTION_FAST_ARGC( ReverseProduct, 1 )
		FUNCTION_FAST_ARGC( Invert, 0 )
		FUNCTION_FAST_ARGC( Translate, 3 ) // x, y, z
		FUNCTION_FAST_ARGC( Translation, 3 ) // x, y, z
		FUNCTION_FAST_ARGC( ClearRotation, 0 )
		FUNCTION_FAST_ARGC( ClearTranslation, 0 )
		FUNCTION_FAST_ARGC( RotationFromQuaternion, 4 ) // w,x,y,z
		FUNCTION_FAST_ARGC( RotateToVector, 3 ) // x,y,z
		FUNCTION_FAST_ARGC( Rotate, 4 ) // angle, x, y, z
		FUNCTION_FAST_ARGC( RotationX, 1 ) // angle
		FUNCTION_FAST_ARGC( RotationY, 1 ) // angle
		FUNCTION_FAST_ARGC( RotationZ, 1 ) // angle
		FUNCTION_FAST_ARGC( Rotation, 4 ) // angle, x, y, z
		FUNCTION_FAST_ARGC( LookAt, 3 ) // x, y, z
		FUNCTION_FAST_ARGC( TransformVector, 1 )
	END_FUNCTION_SPEC


	BEGIN_PROPERTY_SPEC
		PROPERTY( translation )
	END_PROPERTY_SPEC

END_CLASS


/* links:
- Convert Euler angles to/from matrix or quat - http://tog.acm.org/GraphicsGems/gemsiv/euler_angle/
- Some math tools - http://www.google.fr/codesearch?hl=fr&q=show:aE3NaT1Jblw:D0uDO1Bfdv4&ct=rdl&cs_p=http://davehillier.googlecode.com/svn&cs_f=trunk/day1/maths

*/
