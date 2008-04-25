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


static int ReadMatrix(void *pv, float **m) { // Doc: __declspec(noinline) tells the compiler to never inline a particular function.

	*m = ((Matrix44*)pv)->raw; // returns its private pointer. Caller SHOULD not modify it
	return true;
}

BEGIN_CLASS( Transformation )


DEFINE_FINALIZE() {

	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	if ( m != NULL ) {

		Matrix44Free(m);
		JS_SetPrivate(cx, obj, NULL);
	}
}


DEFINE_CONSTRUCTOR() {

	Matrix44 *m = Matrix44Alloc();
	RT_ASSERT_ALLOC(m);
//	Matrix44Identity(m);
	JS_SetPrivate(cx, obj, m);
	JSBool status = SetNativeInterface(cx, obj, NI_READ_MATRIX44, (FunctionPointer)ReadMatrix, m);
	RT_ASSERT( status == JS_TRUE, "Unable SetNativeInterface." );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Clear ) {

	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tm);
	Matrix44Identity(tm);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( ClearRotation ) {

	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tm);
	Matrix44ClearRotation(tm);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( ClearTranslation ) {

	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tm);
	Matrix44ClearTranslation(tm);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Load ) {

	J_S_ASSERT_ARG_MIN(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(tm);

	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);

	/* GetMatrixHelper already to the following
	if ( J_JSVAL_IS_ARRAY( J_FARG(1) ) ) {

		jsuint length = 16;
		J_JSVAL_TO_REAL_VECTOR( J_FARG(1), &(*tm->raw), length );
		J_S_ASSERT( length == 16, "Too few elements in the array." );

		return JS_TRUE;
	}
	*/
	/* GetMatrixHelper already to the following
	else if (J_JSVAL_IS_CLASS( J_FARG(1), &classTransformation )) {

		Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, JSVAL_TO_OBJECT(J_FARG(1)));
		RT_ASSERT_RESOURCE(m);
		memcpy(tm, m, sizeof(Matrix44));
		return JS_TRUE;
	}
	*/
//	else {

		Matrix44 *m = tm;
		RT_CHECK_CALL( GetMatrixHelper(cx, J_FARG(1), &m) ); // GetMatrixHelper will copy data into tmp OR replace tmp by its own float pointer
		if ( m != tm ) // check if the pointer has been modified
			memcpy(tm, m, sizeof(Matrix44)); // if it is, copy the data
		return JS_TRUE;
//	}

	J_REPORT_ERROR("Invalid matrix.");
	return JS_FALSE;
}


DEFINE_FUNCTION_FAST( LoadRotation ) {

	RT_ASSERT_ARGC(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	RT_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	RT_CHECK_CALL( GetMatrixHelper(cx, J_FARG(1), &m) );
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
}

DEFINE_FUNCTION_FAST( LoadTranslation ) {

	RT_ASSERT_ARGC(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	RT_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	RT_CHECK_CALL( GetMatrixHelper(cx, J_FARG(1), &m) );
	tm->raw[3]  = m->raw[3];
	tm->raw[7]  = m->raw[7];
	tm->raw[11] = m->raw[11];
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Translation ) {

	RT_ASSERT_ARGC(3); // x, y, z
	RT_ASSERT_NUMBER(J_FARG(1));
	RT_ASSERT_NUMBER(J_FARG(2));
	RT_ASSERT_NUMBER(J_FARG(3));

	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(m);

	jsdouble x, y, z;
	JS_ValueToNumber(cx, J_FARG(1), &x);
	JS_ValueToNumber(cx, J_FARG(2), &y);
	JS_ValueToNumber(cx, J_FARG(3), &z);
	Matrix44SetTranslation(m, x,y,z);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Translate ) {

	RT_ASSERT_ARGC(3); // x, y, z
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(m);
	jsdouble x, y, z;
	JS_ValueToNumber(cx, J_FARG(1), &x);
	JS_ValueToNumber(cx, J_FARG(2), &y);
	JS_ValueToNumber(cx, J_FARG(3), &z);
	Matrix44 t;
	Matrix44Identity(&t);
	Matrix44SetTranslation(&t, x, y, z);
	Matrix44Product(m, &t);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Rotation ) {

	RT_ASSERT_ARGC(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	RT_ASSERT_RESOURCE(tm);

	RT_ASSERT_ARGC(4); // angle, x, y, z
	jsdouble angle, x, y, z;
	JS_ValueToNumber(cx, J_FARG(1), &angle);
	JS_ValueToNumber(cx, J_FARG(2), &x);
	JS_ValueToNumber(cx, J_FARG(3), &y);
	JS_ValueToNumber(cx, J_FARG(4), &z);
	Vector3 axis;
	Vector3Set(&axis, x,y,z);
	Matrix44SetRotation(tm, &axis, -angle*M_PI/360.0f);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Rotate ) {

	RT_ASSERT_ARGC(4); // angle, x, y, z
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(m);
	jsdouble angle, x, y, z;
	JS_ValueToNumber(cx, J_FARG(1), &angle);
	JS_ValueToNumber(cx, J_FARG(2), &x);
	JS_ValueToNumber(cx, J_FARG(3), &y);
	JS_ValueToNumber(cx, J_FARG(4), &z);
	Vector3 axis;
	Vector3Set(&axis, x,y,z);
	Matrix44 r;
	Matrix44Identity(&r);
	Matrix44SetRotation(&r, &axis, -angle*M_PI/360.0f);
	Matrix44Product(m, &r);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( RotationX ) {

	RT_ASSERT_ARGC(1); // angle
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(m);
	jsdouble angle;
	JS_ValueToNumber(cx, J_FARG(1), &angle);
//	Matrix44 r;
	Matrix44SetXRotation(m, -angle*M_PI/360.0f);
//	Matrix44Product(m, &r);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( RotationY ) {

	RT_ASSERT_ARGC(1); // angle
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(m);
	jsdouble angle;
	JS_ValueToNumber(cx, J_FARG(1), &angle);
//	Matrix44 r;
	Matrix44SetYRotation(m, -angle*M_PI/360.0f);
//	Matrix44Product(m, &r);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( RotationZ ) {

	RT_ASSERT_ARGC(1); // angle
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(m);
	jsdouble angle;
	JS_ValueToNumber(cx, J_FARG(1), &angle);
//	Matrix44 r;
	Matrix44SetZRotation(m, -angle*M_PI/360.0f);
//	Matrix44Product(m, &r);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( LookAt ) {

	REPORT_ERROR("LookAt is buggy !! dont' use it");

	RT_ASSERT_ARGC(3); // x, y, z
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(m);
	jsdouble x, y, z;
	JS_ValueToNumber(cx, J_FARG(1), &x);
	JS_ValueToNumber(cx, J_FARG(2), &y);
	JS_ValueToNumber(cx, J_FARG(3), &z);
	Vector3 up;
	Vector3Set(&up, 0,0,1);
	Vector3 to;
	Vector3Set(&to, x,y,z);
	Matrix44LookAt(m, &to, &up);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Invert ) {

	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE(m);
	Matrix44Invert(m);
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Product ) {

	RT_ASSERT_ARGC(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	RT_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	RT_CHECK_CALL( GetMatrixHelper(cx, J_FARG(1), &m) );
	Matrix44Product(tm,m); // <- mult
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( ReverseProduct ) {

	RT_ASSERT_ARGC(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	RT_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	RT_CHECK_CALL( GetMatrixHelper(cx, J_FARG(1), &m) );
	Matrix44ReverseProduct(tm,m); // <- mult
	*J_FRVAL = OBJECT_TO_JSVAL(J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( TransformVector ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_ARRAY( J_FARG(1) );

	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, J_FOBJ); // tm for thisMatrix
	RT_ASSERT_RESOURCE(tm);

	Vector3 src, dst;
	jsuint length = 3;
	J_JSVAL_TO_REAL_VECTOR( J_FARG(1), src.raw, length );

	Matrix44MultVector3( tm, &src, &dst );

	jsval tmpValue;
	J_CHECK_CALL( JS_NewNumberValue(cx, dst.x, &tmpValue) );
	J_CHECK_CALL( JS_SetElement(cx, JSVAL_TO_OBJECT( J_FARG(1) ), 0, &tmpValue) );

	J_CHECK_CALL( JS_NewNumberValue(cx, dst.y, &tmpValue) );
	J_CHECK_CALL( JS_SetElement(cx, JSVAL_TO_OBJECT( J_FARG(1) ), 1, &tmpValue) );

	J_CHECK_CALL( JS_NewNumberValue(cx, dst.z, &tmpValue) );
	J_CHECK_CALL( JS_SetElement(cx, JSVAL_TO_OBJECT( J_FARG(1) ), 2, &tmpValue) );

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}

/*
DEFINE_NEW_RESOLVE() {

	if (!JSVAL_IS_INT(id) || (flags & JSRESOLVE_ASSIGNING))
		return JS_TRUE;
	jsint slot = JSVAL_TO_INT( id );
	J_S_ASSERT( slot >= 0 && slot <= 15, "Out of range." );
	JS_DefineProperty(cx, obj, (char*)slot, JSVAL_VOID, NULL, NULL, JSPROP_INDEX | JSPROP_SHARED );
	*objp = obj;
	return JS_TRUE;
}
*/

DEFINE_GET_PROPERTY() {

	if ( JSVAL_IS_INT(id) ) {

		Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, obj); // tm for thisMatrix
		J_S_ASSERT_RESOURCE(tm);
		jsint slot = JSVAL_TO_INT( id );
		J_S_ASSERT( slot >= 0 && slot <= 15, "Out of range." );
		J_CHECK_CALL( JS_NewNumberValue(cx, tm->raw[slot], vp) );
	}
	return JS_TRUE;
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
}


CONFIGURE_CLASS

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
		FUNCTION_FAST_ARGC( Rotate, 4 ) // angle, x, y, z
		FUNCTION_FAST_ARGC( RotationX, 1 ) // angle
		FUNCTION_FAST_ARGC( RotationY, 1 ) // angle
		FUNCTION_FAST_ARGC( RotationZ, 1 ) // angle
		FUNCTION_FAST_ARGC( Rotation, 4 ) // angle, x, y, z
		FUNCTION_FAST_ARGC( LookAt, 3 ) // x, y, z
		FUNCTION_FAST_ARGC( TransformVector, 1 )

	END_FUNCTION_SPEC

	HAS_PRIVATE  // private: BodyID

END_CLASS
