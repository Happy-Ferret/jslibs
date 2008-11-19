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


static int ReadMatrix(JSContext *cx, JSObject *obj, float **m) { // Doc: __declspec(noinline) tells the compiler to never inline a particular function.
	
	Matrix44 *objMatrix = (Matrix44*)JS_GetPrivate(cx, obj);
	*m = objMatrix->raw; // returns its private pointer. Caller SHOULD not modify it
	return true;
}

/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Transformation )

DEFINE_FINALIZE() {

	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	if ( m != NULL ) {

		Matrix44Free(m);
		JS_SetPrivate(cx, obj, NULL);
	}
}

/**doc
 * $INAME()
  Creates a new uninitialized Transformation object.
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	Matrix44 *m = Matrix44Alloc();
	J_S_ASSERT_ALLOC(m);
//	Matrix44Identity(m);
	JS_SetPrivate(cx, J_OBJ, m);
	J_CHK( SetMatrix44ReadInterface(cx, obj, ReadMatrix) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME()
  Reset the current transformation (set to identity).
**/
DEFINE_FUNCTION_FAST( Clear ) {

	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tm);
	Matrix44Identity(tm);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME()
  Clear the rotation part of the current transformation.
**/
DEFINE_FUNCTION_FAST( ClearRotation ) {

	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tm);
	Matrix44ClearRotation(tm);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME()
  Clear the translation part of the current transformation.
**/
DEFINE_FUNCTION_FAST( ClearTranslation ) {

	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tm);
	Matrix44ClearTranslation(tm);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME( matrix )
  Load a 4x4 matrix as the current transformation.
  $H arguments
   $ARG value matrix: an Array or an object that supports NIMatrix44Read native interface.
**/
DEFINE_FUNCTION_FAST( Load ) {

	J_S_ASSERT_ARG_MIN(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(tm);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);

	Matrix44 *m = tm;
	J_CHK( GetMatrixHelper(cx, J_FARG(1), &m) ); // GetMatrixHelper will copy data into tmp OR replace tmp by its own float pointer
	if ( m != tm ) // check if the pointer has been modified
		memcpy(tm, m, sizeof(Matrix44)); // if it is, copy the data
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME( matrix )
  Load the rotation part of another matrix to the current matrix.
  $H arguments
   $ARG value matrix: an Array or an object that supports NIMatrix44Read native interface.
**/
DEFINE_FUNCTION_FAST( LoadRotation ) {

	J_S_ASSERT_ARG_MIN(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	J_S_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	J_CHK( GetMatrixHelper(cx, J_FARG(1), &m) );
	tm->raw[0]  = m->raw[0] ; //L1
	tm->raw[1]  = m->raw[1] ;
	tm->raw[2]  = m->raw[2] ;

	tm->raw[4]  = m->raw[4] ; //L2
	tm->raw[5]  = m->raw[5] ;
	tm->raw[6]  = m->raw[6] ;

	tm->raw[8]  = m->raw[8] ; //L3
	tm->raw[9]  = m->raw[9] ;
	tm->raw[10] = m->raw[10];
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
 * $THIS $INAME( matrix )
  Load the translation part of another matrix to the current matrix.
  $H arguments
   $ARG value matrix: an Array or an object that supports NIMatrix44Read native interface.
**/
DEFINE_FUNCTION_FAST( LoadTranslation ) {

	J_S_ASSERT_ARG_MIN(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	J_S_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	J_CHK( GetMatrixHelper(cx, J_FARG(1), &m) );
	tm->raw[3]  = m->raw[3];
	tm->raw[7]  = m->raw[7];
	tm->raw[11] = m->raw[11];
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME( x, y, z )
  Sets the translation part of the current transformation.
  $H arguments
   $ARG real x
   $ARG real y
   $ARG real z
**/
DEFINE_FUNCTION_FAST( Translation ) {

	J_S_ASSERT_ARG_MIN(3); // x, y, z
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(m);
	float x, y, z;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &x) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(2), &y) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(3), &z) ); 
	Matrix44SetTranslation(m, x,y,z);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME( x, y, z )
  Apply a translation to the current ransformation.
  $H arguments
   $ARG real x
   $ARG real y
   $ARG real z
**/
DEFINE_FUNCTION_FAST( Translate ) {

	J_S_ASSERT_ARG_MIN(3); // x, y, z
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(m);
	float x, y, z;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &x) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(2), &y) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(3), &z) ); 
	Matrix44 t;
	Matrix44Identity(&t);
	Matrix44SetTranslation(&t, x, y, z);
	Matrix44Product(m, &t);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
	JL_BAD;
}



/**doc
 * $THIS $INAME( w, x, y, z )
  Sets the rotation part from a quaternion.
  $H arguments
   $ARG real w
   $ARG real x
   $ARG real y
   $ARG real z
**/
DEFINE_FUNCTION_FAST( RotationFromQuaternion ) {

	J_S_ASSERT_ARG_MIN(4); // w, x, y, z
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	J_S_ASSERT_RESOURCE(tm);

	float w, x, y, z;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &w) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(2), &x) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(3), &y) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(4), &z) ); 


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
 * $THIS $INAME( roll, pitch, yaw )
  Sets the Tait-Bryan rotation.
  $H arguments
   $ARG real roll
   $ARG real pitch
   $ARG real yaw
**/
DEFINE_FUNCTION_FAST( TaitBryanRotation ) {

	J_S_ASSERT_ARG_MIN(3); // roll, pitch, yaw
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	J_S_ASSERT_RESOURCE(tm);

	float roll, pitch, yaw;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &roll) );
	J_CHK( JsvalToFloat(cx, J_FARG(2), &pitch) );
	J_CHK( JsvalToFloat(cx, J_FARG(3), &yaw) );

	// (TBD)

	return JS_TRUE;
	JL_BAD;
}



/**doc
 * $THIS $INAME( angle, x, y, z )
  Sets the rotation part of the current transformation.
  $H arguments
   $ARG real angle in degres
   $ARG real x
   $ARG real y
   $ARG real z
**/
DEFINE_FUNCTION_FAST( Rotation ) {

	J_S_ASSERT_ARG_MIN(4); // angle, x, y, z
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	J_S_ASSERT_RESOURCE(tm);

	float angle, x, y, z;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &angle) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(2), &x) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(3), &y) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(4), &z) ); 
	Vector3 axis;
	Vector3Set(&axis, x,y,z);
	Matrix44SetRotation(tm, &axis, -angle * M_PI / 360.0f);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME( angle, x, y, z )
  Apply a rotation to the current ransformation.
  $H arguments
   $ARG real angle in degres
   $ARG real x
   $ARG real y
   $ARG real z
**/
DEFINE_FUNCTION_FAST( Rotate ) {

	J_S_ASSERT_ARG_MIN(4); // angle, x, y, z
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(m);
	float angle, x, y, z;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &angle) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(2), &x) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(3), &y) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(4), &z) ); 
	Vector3 axis;
	Vector3Set(&axis, x,y,z);
	Matrix44 r;
	Matrix44Identity(&r);
	Matrix44SetRotation(&r, &axis, -angle * M_PI / 360.0f);
	Matrix44Product(m, &r);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME( angle )
  Set the rotation around the X axis.
  $H arguments
   $ARG real angle in degres
**/
DEFINE_FUNCTION_FAST( RotationX ) {

	J_S_ASSERT_ARG_MIN(1); // angle
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(m);
	float angle;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &angle) ); 
//	Matrix44 r;
	Matrix44SetXRotation(m, -angle * M_PI / 360.0f);
//	Matrix44Product(m, &r);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME( angle )
  Set the rotation around the Y axis.
  $H arguments
   $ARG real angle in degres
**/
DEFINE_FUNCTION_FAST( RotationY ) {

	J_S_ASSERT_ARG_MIN(1); // angle
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(m);
	float angle;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &angle) ); 
//	Matrix44 r;
	Matrix44SetYRotation(m, -angle * M_PI / 360.0f);
//	Matrix44Product(m, &r);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME( angle )
  Set the rotation around the Z axis.
  $H arguments
   $ARG real angle in degres
**/
DEFINE_FUNCTION_FAST( RotationZ ) {

	J_S_ASSERT_ARG_MIN(1); // angle
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(m);
	float angle;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &angle) ); 
//	Matrix44 r;
	Matrix44SetZRotation(m, -angle * M_PI / 360.0f);
//	Matrix44Product(m, &r);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME( x, y, z )
  unavailable: need to be fixed.
  $H arguments
   $ARG real x
   $ARG real y
   $ARG real z
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

	J_REPORT_ERROR("LookAt is buggy !! dont' use it");

	J_S_ASSERT_ARG_MIN(3); // x, y, z
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(m);
	float x, y, z;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &x) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(2), &y) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(3), &z) ); 
	Vector3 up;
	Vector3Set(&up, 0,0,1);
	Vector3 to;
	Vector3Set(&to, x,y,z);
	Matrix44LookAt(m, &to, &up);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION_FAST( RotateToVector ) {

	J_S_ASSERT_ARG_MIN(3); // x, y, z
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(m);
	float x, y, z;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &x) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(2), &y) ); 
	J_CHK( JsvalToFloat(cx, J_FARG(3), &z) ); 


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
	*J_FRVAL = JSVAL_VOID;

	return JS_TRUE;
	JL_BAD;
}

/**doc
 * $THIS $INAME()
  invert the current transformation.
**/
DEFINE_FUNCTION_FAST( Invert ) {

	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(m);
	Matrix44Invert(m);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME( newTransformation )
  Apply a _newTransformation_ to the current transformation.
  this = this . new
  $H arguments
   $ARG value newTransformation: an Array or an object that supports NIMatrix44Read native interface.
**/
DEFINE_FUNCTION_FAST( Product ) {

	J_S_ASSERT_ARG_MIN(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	J_S_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	J_CHK( GetMatrixHelper(cx, J_FARG(1), &m) );
	Matrix44Product(tm, m); // <- mult
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $THIS $INAME( otherTransformation )
  Apply the current transformation to the _otherTransformation_ and stores the result to the current transformation.
  this = new . this
  $H arguments
   $ARG value otherTransformation: an Array or an object that supports NIMatrix44Read native interface.
**/
DEFINE_FUNCTION_FAST( ReverseProduct ) {

	J_S_ASSERT_ARG_MIN(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	J_S_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	J_CHK( GetMatrixHelper(cx, J_FARG(1), &m) );
	Matrix44ReverseProduct(tm, m); // <- mult
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $VOID $INAME( vector )
  transforms the 3D or 4D _vector_ by the current transformation.
  $H arguments
   $ARG Array vector
**/
DEFINE_FUNCTION_FAST( TransformVector ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_ARRAY( J_FARG(1) );

	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	J_S_ASSERT_RESOURCE(tm);

	size_t length;
//	J_JSVAL_TO_ARRAY_LENGTH( J_FARG(1), length );
	jsuint tmp;
	J_CHK( JS_GetArrayLength(cx, JSVAL_TO_OBJECT(J_FARG(1)), &tmp) );
	length = tmp;

	jsval tmpValue;
	if ( length == 3 ) {

		Vector3 src, dst;
		J_CHK( JsvalToFloatVector(cx, J_FARG(1), src.raw, 3, &length ) );
		Matrix44MultVector3( tm, &src, &dst );

		J_CHK( JS_NewNumberValue(cx, dst.x, &tmpValue) );
		J_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( J_FARG(1) ), 0, &tmpValue) );

		J_CHK( JS_NewNumberValue(cx, dst.y, &tmpValue) );
		J_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( J_FARG(1) ), 1, &tmpValue) );

		J_CHK( JS_NewNumberValue(cx, dst.z, &tmpValue) );
		J_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( J_FARG(1) ), 2, &tmpValue) );
	} else
	if ( length == 4 ) {

		Vector4 src, dst;
		J_CHK( JsvalToFloatVector(cx, J_FARG(1), src.raw, 4, &length ) );
		Matrix44MultVector4( tm, &src, &dst );

		J_CHK( JS_NewNumberValue(cx, dst.x, &tmpValue) );
		J_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( J_FARG(1) ), 0, &tmpValue) );

		J_CHK( JS_NewNumberValue(cx, dst.y, &tmpValue) );
		J_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( J_FARG(1) ), 1, &tmpValue) );

		J_CHK( JS_NewNumberValue(cx, dst.z, &tmpValue) );
		J_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( J_FARG(1) ), 2, &tmpValue) );

		J_CHK( JS_NewNumberValue(cx, dst.w, &tmpValue) );
		J_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT( J_FARG(1) ), 3, &tmpValue) );
	} else {

		J_REPORT_ERROR_1( "Unsupported vector length (%d).", length );
	}

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/*
DEFINE_NEW_RESOLVE() {

	if (!JSVAL_IS_INT(id) || (flags & JSRESOLVE_ASSIGNING))
		return JS_TRUE;
	jsint slot = JSVAL_TO_INT( id );
	J_S_ASSERT( slot >= 0 && slot <= 15, "Out of range." );
	JS_DefineProperty(cx, obj, (char*)slot, JSVAL_VOID, NULL, NULL, JSPROP_INDEX | JSPROP_SHARED );
	*objp = obj;
	return JS_TRUE;
	JL_BAD;
}
*/

/**doc
 * $REAL $INAME
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

		Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, obj); // tm for thisMatrix
		J_S_ASSERT_RESOURCE(tm);
		jsint slot = JSVAL_TO_INT( id );
		J_S_ASSERT( slot >= 0 && slot <= 15, "Out of range." );
		J_CHK( JS_NewNumberValue(cx, tm->raw[slot], vp) );
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_SET_PROPERTY() {

	if ( JSVAL_IS_INT(id) ) {

		Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, obj); // tm for thisMatrix
		J_S_ASSERT_RESOURCE(tm);
		J_S_ASSERT_NUMBER(*vp);
		jsint slot = JSVAL_TO_INT( id );
		J_S_ASSERT( slot >= 0 && slot <= 15, "Out of range." );
		tm->raw[slot] = JSVAL_IS_DOUBLE(*vp) ? *JSVAL_TO_DOUBLE(*vp) : JSVAL_TO_INT(*vp);
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Native Interface ===
 *NIMatrix44Read*: the current transformation matrix.
**/

CONFIGURE_CLASS

	HAS_PRIVATE 

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

END_CLASS


/* links:
- Convert Euler angles to/from matrix or quat - http://tog.acm.org/GraphicsGems/gemsiv/euler_angle/
- Some math tools - http://www.google.fr/codesearch?hl=fr&q=show:aE3NaT1Jblw:D0uDO1Bfdv4&ct=rdl&cs_p=http://davehillier.googlecode.com/svn&cs_f=trunk/day1/maths

*/
