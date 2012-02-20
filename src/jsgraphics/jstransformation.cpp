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


ALWAYS_INLINE float DEG_TO_RAD( const float angle ) {

	return -(angle) * ((float)M_PI) / 180.0f;
}

ALWAYS_INLINE float RAD_TO_DEG( const float rad ) {

	return rad * (-180.f / (float)M_PI);
}

// (TBD) move this in the class private

jl::Pool matrixPool; // (TBD) manage thread safety / use modulePrivate


int GetMatrix(JSContext *cx, JSObject *obj, float **m) { // Doc: __declspec(noinline) tells the compiler to never inline a particular function.

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	if ( !pv )
		return false;
	*m = pv->mat->raw; // returns its private pointer. Caller SHOULD not modify it
	return true;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Transformation )

DEFINE_FINALIZE() {

	if ( JL_GetHostPrivate(cx)->canSkipCleanup ) // do not cleanup in unsafe mode.
		return;

	if ( obj == JL_THIS_PROTOTYPE ) {

		while ( !PoolIsEmpty(&matrixPool) )
			Matrix44Free((Matrix44*)jl::PoolPop(&matrixPool));
		jl::PoolFinalize(&matrixPool);
		JL_THIS_PROTOTYPE = NULL; // last GC
		return;
	}

	//	printf("Fin:%d\n", matrixPoolLength);
	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	if ( pv == NULL )
		return;

	//beware: prototype may be finalized before the object
	if ( JL_THIS_PROTOTYPE != NULL ) { // add to the pool if the pool is still alive !

		if ( /*JL_EngineEnding(cx) ||*/ !jl::PoolPush(&matrixPool, pv->mat) ) // if the runtime is shutting down, there is no more need to fill the pool.
			Matrix44Free(pv->mat);
	} else {

		Matrix44Free(pv->mat);
	}

	JS_free(cx, pv);
}


/**doc
$TOC_MEMBER $INAME
 $INAME( [init] )
  Creates a new Transformation object.
  If no argument is given, the transformation is not initialized.
  If _init_ is $NULL, the transformation is initialized to identity.
  If _init_ is a matrix, the transormation is initialized with the matrix.
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_ARGC_RANGE(0,16);
//	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	TransformationPrivate *pv = (TransformationPrivate *)JS_malloc(cx, sizeof(TransformationPrivate));
	JL_CHK(pv);
	JL_SetPrivate(cx, JL_OBJ, pv);

	pv->mat = PoolIsEmpty(&matrixPool) ? Matrix44Alloc() : (Matrix44*)jl::PoolPop(&matrixPool);
	JL_ASSERT_ALLOC(pv->mat);

	if ( JL_ARGC >= 1 ) {
			
		if ( JSVAL_IS_OBJECT(JL_ARG(1)) ) {

			JL_ASSERT_ARGC(1);
			Matrix44 *m = pv->mat;
			JL_CHK( GetMatrixHelper(cx, JL_ARG(1), (float**)&m) );
			if ( m != pv->mat ) // check if the pointer has been modified
				Matrix44Load(pv->mat, m);
			pv->isIdentity = false;
		} else
/*
		if ( JSVAL_IS_NULL(JL_ARG(1)) ) {

			Matrix44Identity(pv->mat);
			pv->isIdentity = true;
		} else
*/
		if ( JL_ARGC == 16 ) {
			
			float *tmp = (float*)&pv->mat->raw;
			for ( int i = 0; i < 16; ++i )
				JL_CHK( JL_JsvalToNative(cx, JL_ARGV[i], (tmp++)) );
			// see JL_CHK( JL_JsvalToNativeVector(cx, *JL_ARGV, tmp, 16, &len) );
			pv->isIdentity = false;
		} else {

			JL_ERR( E_ARGC, E_EQUALS, E_NUM(16) );
		}
	}
	// else uninitialized matrix

	JL_CHK( SetMatrix44GetInterface(cx, obj, GetMatrix) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( matrix )
 $THIS $INAME( v0, v1, ..., v15 )
  Load a 4x4 matrix as the current transformation.
  $H arguments
   $ARG $VAL matrix: an Array or an object that supports NIMatrix44Read native interface.
**/
DEFINE_FUNCTION( load ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_ARGC_MAX(16);

	if ( JSVAL_IS_OBJECT(JL_ARG(1)) ) {

		JL_ASSERT_ARGC(1);
		Matrix44 *m = pv->mat;
		JL_CHK( GetMatrixHelper(cx, JL_ARG(1), (float**)&m) );

		if ( m != pv->mat ) // check if the pointer has been modified
			Matrix44Load(pv->mat, m);
		pv->isIdentity = false;
	} else
/*
		if ( JSVAL_IS_NULL(JL_ARG(1)) ) {

		Matrix44Identity(pv->mat);
		pv->isIdentity = true;
	} else
*/
	if ( JL_ARGC == 16 ) {
		
		float *tmp = (float*)&pv->mat->raw;
		for ( int i = 0; i < 16; ++i )
			JL_CHK( JL_JsvalToNative(cx, JL_ARGV[i], (tmp++)) );
		// see JL_CHK( JL_JsvalToNativeVector(cx, *JL_ARGV, tmp, 16, &len) );
		pv->isIdentity = false;
	} else {

		JL_ERR( E_ARGC, E_EQUALS, E_NUM(16) );
	}
	
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  Reset the current transformation (set to identity).
**/
DEFINE_FUNCTION( clear ) {
	
	JL_IGNORE(argc);
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	Matrix44Identity(pv->mat);
	pv->isIdentity = true;
	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  Clear the rotation part of the current transformation.
**/
DEFINE_FUNCTION( clearRotation ) {

	JL_IGNORE(argc);
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	Matrix44ClearRotation(pv->mat);
	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  Clear the translation part of the current transformation.
**/
DEFINE_FUNCTION( clearTranslation ) {

	JL_IGNORE(argc);
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	Matrix44ClearTranslation(pv->mat);
	*JL_RVAL = OBJECT_TO_JSVAL(obj);

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
DEFINE_FUNCTION( loadRotation ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	Matrix44 tmp, *m = &tmp;
	JL_CHK( GetMatrixHelper(cx, JL_ARG(1), (float**)&m) );

	pv->mat->raw[0]  = m->raw[0] ; //L1
	pv->mat->raw[1]  = m->raw[1] ;
	pv->mat->raw[2]  = m->raw[2] ;

	pv->mat->raw[4]  = m->raw[4] ; //L2
	pv->mat->raw[5]  = m->raw[5] ;
	pv->mat->raw[6]  = m->raw[6] ;

	pv->mat->raw[8]  = m->raw[8] ; //L3
	pv->mat->raw[9]  = m->raw[9] ;
	pv->mat->raw[10] = m->raw[10];

	pv->isIdentity = false; // (TBD) detect identity

	*JL_RVAL = OBJECT_TO_JSVAL(obj);

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
DEFINE_FUNCTION( loadTranslation ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	Matrix44 tmp, *m = &tmp;
	JL_CHK( GetMatrixHelper(cx, JL_ARG(1), (float**)&m) );

	pv->mat->raw[3]  = m->raw[3];
	pv->mat->raw[7]  = m->raw[7];
	pv->mat->raw[11] = m->raw[11];

	pv->isIdentity = false; // (TBD) detect identity

	*JL_RVAL = OBJECT_TO_JSVAL(obj);

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
DEFINE_FUNCTION( translate ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(3);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	float x, y, z;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &x) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &y) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &z) );

	if ( pv->isIdentity ) {

		Matrix44SetTranslation(pv->mat, x, y, z);
		pv->isIdentity = false;
	} else {

		Matrix44 t;
		Matrix44Identity(&t);
		Matrix44SetTranslation(&t, x, y, z);
		Matrix44Mult(pv->mat, pv->mat, &t);
	}

	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( x [ , y, z ] )
  Scale the current ransformation.
  $H arguments
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
**/
DEFINE_FUNCTION( scale ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(1, 3);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	float x, y, z;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &x) );

	if ( argc >= 3 ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &y) );
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &z) );
	} else {

		y = x;
		z = x;
	}

	if ( pv->isIdentity ) {

		Matrix44SetScale(pv->mat, x, y, z);
		pv->isIdentity = false;
	} else {

		Matrix44 t;
		Matrix44Identity(&t);
		Matrix44SetScale(&t, x, y, z);
		Matrix44Mult(pv->mat, pv->mat, &t);
	}

	*JL_RVAL = OBJECT_TO_JSVAL(obj);

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
DEFINE_FUNCTION( rotationFromQuaternion ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(4);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	float w, x, y, z;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &w) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &x) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &y) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &z) );

	float fTx  = 2.0f * x;
	float fTy  = 2.0f * y;
	float fTz  = 2.0f * z;
	float fTwx = fTx * w;
	float fTwy = fTy * w;
	float fTwz = fTz * w;
	float fTxx = fTx * x;
	float fTxy = fTy * x;
	float fTxz = fTz * x;
	float fTyy = fTy * y;
	float fTyz = fTz * y;
	float fTzz = fTz * z;

	pv->mat->m[0][0] = 1.0f-(fTyy+fTzz);
	pv->mat->m[0][1] = fTxy-fTwz;
	pv->mat->m[0][2] = fTxz+fTwy;
	pv->mat->m[1][0] = fTxy+fTwz;
	pv->mat->m[1][1] = 1.0f-(fTxx+fTzz);
	pv->mat->m[1][2] = fTyz-fTwx;
	pv->mat->m[2][0] = fTxz-fTwy;
	pv->mat->m[2][1] = fTyz+fTwx;
	pv->mat->m[2][2] = 1.0f-(fTxx+fTyy);

	pv->isIdentity = false;

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( roll, pitch, yaw ) ,,not implemented yet,,
  Sets the Tait-Bryan rotation.
  $H arguments
   $ARG $REAL roll
   $ARG $REAL pitch
   $ARG $REAL yaw
**/
DEFINE_FUNCTION( taitBryanRotation ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(3);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	float roll, pitch, yaw;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &roll) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &pitch) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &yaw) );

	// (TBD)
	pv->isIdentity = false;

	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( rotate ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(4);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	float angle;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &angle) );
	if ( angle == 0.0f )
		return JS_TRUE;

	float x, y, z;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &x) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &y) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &z) );
	Vector3 axis;
	Vector3Set(&axis, x,y,z);

	if ( pv->isIdentity ) {

		Matrix44SetRotation(pv->mat, &axis, DEG_TO_RAD(angle));
		pv->isIdentity = false;
	} else {

		Matrix44 r;
		Matrix44Identity(&r);
		Matrix44SetRotation(&r, &axis, DEG_TO_RAD(angle));
		Matrix44Mult(pv->mat, pv->mat, &r);
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( angle )
  Rotate around the X axis.
  $H arguments
   $ARG $REAL angle in degres
**/
DEFINE_FUNCTION( rotateX ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	float angle;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &angle) );
	if ( angle == 0.0f )
		return JS_TRUE;

	if ( pv->isIdentity ) {

		Matrix44SetXRotation(pv->mat, DEG_TO_RAD(angle));
		pv->isIdentity = false;
	} else {

		Matrix44 r;
		Matrix44Identity(&r);
		Matrix44SetXRotation(&r, DEG_TO_RAD(angle));
		Matrix44Mult(pv->mat, pv->mat, &r);
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( angle )
  Rotate around the Y axis.
  $H arguments
   $ARG $REAL angle in degres
**/
DEFINE_FUNCTION( rotateY ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	float angle;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &angle) );
	if ( angle == 0.0f )
		return JS_TRUE;

	if ( pv->isIdentity ) {

		Matrix44SetYRotation(pv->mat, DEG_TO_RAD(angle));
		pv->isIdentity = false;
	} else {

		Matrix44 r;
		Matrix44Identity(&r);
		Matrix44SetYRotation(&r, DEG_TO_RAD(angle));
		Matrix44Mult(pv->mat, pv->mat, &r);
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( angle )
  Rotate around the Z axis.
  $H arguments
   $ARG $REAL angle in degres
**/
DEFINE_FUNCTION( rotateZ ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	float angle;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &angle) );
	if ( angle == 0.0f )
		return JS_TRUE;

	if ( pv->isIdentity ) {

		Matrix44SetZRotation(pv->mat, DEG_TO_RAD(angle));
		pv->isIdentity = false;
	} else {

		Matrix44 r;
		Matrix44Identity(&r);
		Matrix44SetZRotation(&r, DEG_TO_RAD(angle));
		Matrix44Mult(pv->mat, pv->mat, &r);
	}

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
DEFINE_FUNCTION( lookAt ) {

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

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(6, 9);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	float eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz;

	JL_JsvalToNative(cx, JL_ARG(1), &eyex);
	JL_JsvalToNative(cx, JL_ARG(2), &eyey);
	JL_JsvalToNative(cx, JL_ARG(3), &eyez);

	JL_JsvalToNative(cx, JL_ARG(4), &centerx);
	JL_JsvalToNative(cx, JL_ARG(5), &centery);
	JL_JsvalToNative(cx, JL_ARG(6), &centerz);

	Vector3 up;
	if ( argc >= 7 ) {

		JL_JsvalToNative(cx, JL_ARG(7), &upx);
		JL_JsvalToNative(cx, JL_ARG(8), &upy);
		JL_JsvalToNative(cx, JL_ARG(9), &upz);
		Vector3Set(&up, upx, upy, upz);
	} else {

		Vector3Set(&up, 0, 0, 1);
	}

	Vector3 eye;
	Vector3Set(&eye, eyex, eyey, eyez);
	Vector3 center;
	Vector3Set(&center, centerx, centery, centerz);

	Matrix44 tmp;
	Matrix44SetLookAt(&tmp, &eye, &center, &up);
	Matrix44Mult(pv->mat, pv->mat, &tmp);

//	pv->mat->m[3][0] = -eyex;
//	pv->mat->m[3][1] = -eyey;
//	pv->mat->m[3][2] = -eyez;

	pv->isIdentity = false;

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( x, y, z )
  Apply the (0,0,1)-(x,y,z) angle to the current transformation.
**/
DEFINE_FUNCTION( rotateToVector ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(3);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	float x, y, z;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &x) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &y) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &z) );

	Vector3 to, up;
	Vector3Set(&to, x,y,z);
	Vector3Normalize(&to, &to);
	Vector3Set(&up, 0,0,1);
	float angle = acos(Vector3Dot(&up, &to));
	Vector3Cross(&up, &up, &to);

	if ( pv->isIdentity ) {

		Matrix44SetRotation(pv->mat, &up, DEG_TO_RAD(angle));
		pv->isIdentity = false;
	} else {

		Matrix44 r;
		Matrix44Identity(&r);
		Matrix44SetRotation(&r, &up, DEG_TO_RAD(angle));
		Matrix44Mult(pv->mat, pv->mat, &r);
	}

	*JL_RVAL = OBJECT_TO_JSVAL(obj);

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  invert the current transformation.
**/
DEFINE_FUNCTION( invert ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(0);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	if ( !pv->isIdentity )
		Matrix44Invert(pv->mat);

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( _newTransformation_ [ , preMultiply ] )
  Apply the _newTransformation_ to the current transformation.
  $H arguments
   $ARG $VAL newTransformation: an Array or an object that supports NIMatrix44Read native interface.
**/
DEFINE_FUNCTION( product ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(1, 2);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	Matrix44 tmp, *m = &tmp;
	JL_CHK( GetMatrixHelper(cx, JL_ARG(1), (float**)&m) );

	bool preMultiply;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &preMultiply) );
	else
		preMultiply = false;

	if ( pv->isIdentity )
		Matrix44Load(pv->mat, m);
	else
		if ( preMultiply )
			Matrix44Mult(pv->mat, m, pv->mat);
		else
			Matrix44Mult(pv->mat, pv->mat, m);

	pv->isIdentity = false;

	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vector $INAME( vector )
  Transforms the 3D or 4D _vector_ by the current transformation.
  $H arguments
   $ARG $ARRAY vector
**/
DEFINE_FUNCTION( transformVector ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_ARRAY(1);

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	uint32_t length;
//	J_JSVAL_TO_ARRAY_LENGTH( JL_ARG(1), length );
	JL_CHK( JS_GetArrayLength(cx, JSVAL_TO_OBJECT(JL_ARG(1)), &length) );

	jsval tmpValue;
	if ( length >= 3 ) {

		Vector3 src, dst;
		JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), src.raw, 3, &length ) );

		Matrix44MultVector3(pv->mat, &dst, &src);

		JL_CHK( JL_NativeToJsval(cx, dst.x, &tmpValue) );
		JL_CHK( JL_SetElement(cx, JSVAL_TO_OBJECT( JL_ARG(1) ), 0, &tmpValue) );

		JL_CHK( JL_NativeToJsval(cx, dst.y, &tmpValue) );
		JL_CHK( JL_SetElement(cx, JSVAL_TO_OBJECT( JL_ARG(1) ), 1, &tmpValue) );

		JL_CHK( JL_NativeToJsval(cx, dst.z, &tmpValue) );
		JL_CHK( JL_SetElement(cx, JSVAL_TO_OBJECT( JL_ARG(1) ), 2, &tmpValue) );
	} else
	if ( length == 4 ) {

		Vector4 src, dst;
		JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), src.raw, 4, &length ) );

		Matrix44MultVector4(pv->mat, &dst, &src);

		JL_CHK( JL_NativeToJsval(cx, dst.x, &tmpValue) );
		JL_CHK( JL_SetElement(cx, JSVAL_TO_OBJECT( JL_ARG(1) ), 0, &tmpValue) );

		JL_CHK( JL_NativeToJsval(cx, dst.y, &tmpValue) );
		JL_CHK( JL_SetElement(cx, JSVAL_TO_OBJECT( JL_ARG(1) ), 1, &tmpValue) );

		JL_CHK( JL_NativeToJsval(cx, dst.z, &tmpValue) );
		JL_CHK( JL_SetElement(cx, JSVAL_TO_OBJECT( JL_ARG(1) ), 2, &tmpValue) );

		JL_CHK( JL_NativeToJsval(cx, dst.w, &tmpValue) );
		JL_CHK( JL_SetElement(cx, JSVAL_TO_OBJECT( JL_ARG(1) ), 3, &tmpValue) );
	} else {

		JL_ERR( E_ARG, E_NUM(1), E_LENGTH, E_INTERVAL_NUM(3, 4) ); // "Invalid vector length"
	}

	*JL_RVAL = JL_ARG(1);
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

	JL_IGNORE(id);
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_IS_ARRAY( *vp, "" );

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	float pos[3];
	uint32_t len;
	JL_CHK( JL_JsvalToNativeVector(cx, *vp, pos, 3, &len) );
	Matrix44SetTranslation(pv->mat, pos[0], pos[1], pos[2]);
	pv->isIdentity = false;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( translation ) {

	JL_IGNORE(id);
	JL_ASSERT_THIS_INSTANCE();

	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	float pos[3];
	Matrix44GetTranslation(pv->mat, &pos[0], &pos[1], &pos[2]);
	JL_CHK( JL_NativeVectorToJsval(cx, pos, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}




/*
DEFINE_NEW_RESOLVE() {

	if (!JSVAL_IS_INT(id) || (flags & JSRESOLVE_ASSIGNING))
		return JS_TRUE;
	jsint slot = JSVAL_TO_INT( id );
	JL_ASSERT( slot >= 0 && slot <= 15, "Out of range." );
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
  t.clear();
  t[3] = 1;
  t[7] = 2;
  t[11] = 3;
  }}}
**/
DEFINE_GET_PROPERTY() {

	JL_ASSERT_THIS_INSTANCE();

	if ( !JSID_IS_INT(id) )
		return JS_TRUE;
	jsint slot = JSID_IS_INT( id );
	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_RANGE( slot, 0, 15, "[index]" );
	JL_CHK( JL_NativeToJsval(cx, pv->mat->raw[slot], vp) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_SET_PROPERTY() {

	JL_ASSERT_THIS_INSTANCE();

	if ( !JSID_IS_INT(id) )
		return JS_TRUE;
	jsint slot = JSID_IS_INT( id );
	TransformationPrivate *pv = (TransformationPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_IS_NUMBER(*vp, "");
	JL_ASSERT_RANGE( slot, 0, 15, "[index]" );

//	pv->mat->raw[slot] = JSVAL_IS_DOUBLE(*vp) ? JSVAL_TO_DOUBLE(*vp) : JSVAL_TO_INT(*vp);
	JL_CHK( JL_JsvalToNative(cx, *vp, &pv->mat->raw[slot]) );

	pv->isIdentity = false;
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Native Interface ===
 * *NIMatrix44Read*
  The current transformation matrix.
**/


DEFINE_INIT() {

	JL_IGNORE(obj);
	JL_IGNORE(proto);
	JL_IGNORE(sc);
	JL_IGNORE(cx);
	jl::PoolInitialize( &matrixPool, 8192 );
	return JS_TRUE;
}


#ifdef DEBUG
DEFINE_FUNCTION( test ) {

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
}
#endif //DEBUG


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
		FUNCTION_ARGC( clear, 0 )
		FUNCTION_ARGC( load, 1 )
		FUNCTION_ARGC( loadRotation, 1 )
		FUNCTION_ARGC( loadTranslation, 1 )
		FUNCTION_ARGC( product, 1 )
		FUNCTION_ARGC( invert, 0 )
		FUNCTION_ARGC( translate, 3 ) // x, y, z
		FUNCTION_ARGC( scale, 3 ) // x, y, z
		FUNCTION_ARGC( clearRotation, 0 )
		FUNCTION_ARGC( clearTranslation, 0 )
		FUNCTION_ARGC( rotationFromQuaternion, 4 ) // w,x,y,z
		FUNCTION_ARGC( taitBryanRotation, 3 ) // roll, pitch, yaw
		FUNCTION_ARGC( rotateToVector, 3 ) // x,y,z
		FUNCTION_ARGC( rotate, 4 ) // angle, x, y, z
		FUNCTION_ARGC( rotateX, 1 ) // angle
		FUNCTION_ARGC( rotateY, 1 ) // angle
		FUNCTION_ARGC( rotateZ, 1 ) // angle
		FUNCTION_ARGC( lookAt, 3 ) // x, y, z
		FUNCTION_ARGC( transformVector, 1 ) // 3D or 4D vector
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( translation )
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
#ifdef DEBUG
		FUNCTION( test )
#endif //DEBUG
	END_STATIC_FUNCTION_SPEC

END_CLASS


/* links:
- Convert Euler angles to/from matrix or quat - http://tog.acm.org/GraphicsGems/gemsiv/euler_angle/
- Some math tools - http://www.google.fr/codesearch?hl=fr&q=show:aE3NaT1Jblw:D0uDO1Bfdv4&ct=rdl&cs_p=http://davehillier.googlecode.com/svn&cs_f=trunk/day1/maths

*/
