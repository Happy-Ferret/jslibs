#include "stdafx.h"
#include "jstransformation.h"


#define _USE_MATH_DEFINES
#include <math.h>


static int ReadMatrix(void *pv, float **m) { // Doc: __declspec(noinline) tells the compiler to never inline a particular function.

	*m = ((Matrix44*)pv)->raw; // returns its private pointer. Caller SHOULD not modify it
	return true;
}

BEGIN_CLASS

DEFINE_FINALIZE() {

	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	if ( m != NULL ) {

		Matrix44Free(m);
		JS_SetPrivate(cx, obj, NULL);
	}
}


DEFINE_FUNCTION( ClassConstruct ) {

	Matrix44 *m = Matrix44Alloc();
	RT_ASSERT_ALLOC(m);
	Matrix44Identity(m);
	JS_SetPrivate(cx, obj, m);
	JSBool status = SetNativeInterface(cx, obj, NI_READ_MATRIX44, (FunctionPointer)ReadMatrix, m); // [TBD] check return status
	RT_ASSERT( status == JS_TRUE, "Unable SetNativeInterface." );
	return JS_TRUE;
}

DEFINE_FUNCTION( Dump ) {

	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tm);
	FloatVectorToArray(cx, 16, tm->raw, rval);
	return JS_TRUE;
}


DEFINE_FUNCTION( Clear ) {

	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tm);
	Matrix44Identity(tm);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

DEFINE_FUNCTION( ClearRotation ) {

	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tm);
	Matrix44ClearRotation(tm);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

DEFINE_FUNCTION( ClearTranslation ) {

	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tm);
	Matrix44ClearTranslation(tm);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Load ) {

	RT_ASSERT_ARGC(1)
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(tm);
	Matrix44 *m = tm;
	if ( GetMatrixHelper(cx, argv[0], &m) == JS_FALSE ) // GetMatrixHelper will copy data into tmp OR replace tmp by its own float pointer
		return JS_FALSE;
	if ( m != tm ) // check if the pointer has been modified
		memcpy(tm, m, sizeof(Matrix44)); // if it is, copy the data
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( LoadRotation ) {

	RT_ASSERT_ARGC(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, obj); // tm for thisMatrix
	RT_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	if ( GetMatrixHelper(cx, argv[0], &m) == JS_FALSE ) 
		return JS_FALSE;
	tm->raw[0]  = m->raw[0] ; //L1
	tm->raw[1]  = m->raw[1] ;
	tm->raw[2]  = m->raw[2] ;
	tm->raw[4]  = m->raw[4] ; //L2
	tm->raw[5]  = m->raw[5] ;
	tm->raw[6]  = m->raw[6] ;
	tm->raw[8]  = m->raw[8] ; //L3
	tm->raw[9]  = m->raw[9] ;
	tm->raw[10] = m->raw[10];
	return JS_TRUE;
} 

DEFINE_FUNCTION( LoadTranslation ) {

	RT_ASSERT_ARGC(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, obj); // tm for thisMatrix
	RT_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	if ( GetMatrixHelper(cx, argv[0], &m) == JS_FALSE ) 
		return JS_FALSE;
	tm->raw[3]  = m->raw[3];
	tm->raw[7]  = m->raw[7];
	tm->raw[11] = m->raw[11];
	return JS_TRUE;
} 



DEFINE_FUNCTION( Translation ) {

	RT_ASSERT_ARGC(3); // x, y, z
	RT_ASSERT_NUMBER(argv[0]);
	RT_ASSERT_NUMBER(argv[1]);
	RT_ASSERT_NUMBER(argv[2]);

	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);

	jsdouble x, y, z;
	JS_ValueToNumber(cx, argv[0], &x);
	JS_ValueToNumber(cx, argv[1], &y);
	JS_ValueToNumber(cx, argv[2], &z);
//	Vector3 vector;
//	Vector3Set(&vector, x,y,z);
	Matrix44SetTranslation(m, x,y,z);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

/*
DEFINE_FUNCTION( Translate ) {

	RT_ASSERT_ARGC(3); // x, y, z
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	jsdouble x, y, z;
	JS_ValueToNumber(cx, argv[0], &x);
	JS_ValueToNumber(cx, argv[1], &y);
	JS_ValueToNumber(cx, argv[2], &z);
//	Vector3 vector;
//	Vector3Set(&vector, x,y,z);
	Matrix44 t;
	Matrix44Identity(&t);
	Matrix44SetTranslation(&t, x, y, z);
	Matrix44Product(m, &t);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}
*/

DEFINE_FUNCTION( Rotation ) {

	RT_ASSERT_ARGC(1);
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, obj); // tm for thisMatrix
	RT_ASSERT_RESOURCE(tm);

	RT_ASSERT_ARGC(4); // angle, x, y, z
	jsdouble angle, x, y, z;
	JS_ValueToNumber(cx, argv[0], &angle);
	JS_ValueToNumber(cx, argv[1], &x);
	JS_ValueToNumber(cx, argv[2], &y);
	JS_ValueToNumber(cx, argv[3], &z);
	Vector3 axis;
	Vector3Set(&axis, x,y,z);
	Matrix44SetRotation(tm, &axis, -angle*M_PI/360.0f);
	return JS_TRUE;
}


DEFINE_FUNCTION( Rotate ) {

	RT_ASSERT_ARGC(4); // angle, x, y, z
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	jsdouble angle, x, y, z;
	JS_ValueToNumber(cx, argv[0], &angle);
	JS_ValueToNumber(cx, argv[1], &x);
	JS_ValueToNumber(cx, argv[2], &y);
	JS_ValueToNumber(cx, argv[3], &z);
	Vector3 axis;
	Vector3Set(&axis, x,y,z);
	Matrix44 r;
	Matrix44Identity(&r);
	Matrix44SetRotation(&r, &axis, -angle*M_PI/360.0f);
	Matrix44Product(m, &r);
	return JS_TRUE;
}


DEFINE_FUNCTION( RotationX ) {

	RT_ASSERT_ARGC(1); // angle
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	jsdouble angle;
	JS_ValueToNumber(cx, argv[0], &angle);
//	Matrix44 r;
	Matrix44SetXRotation(m, -angle*M_PI/360.0f);
//	Matrix44Product(m, &r);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( RotationY ) {

	RT_ASSERT_ARGC(1); // angle
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	jsdouble angle;
	JS_ValueToNumber(cx, argv[0], &angle);
//	Matrix44 r;
	Matrix44SetYRotation(m, -angle*M_PI/360.0f);
//	Matrix44Product(m, &r);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( RotationZ ) {

	RT_ASSERT_ARGC(1); // angle
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	jsdouble angle;
	JS_ValueToNumber(cx, argv[0], &angle);
//	Matrix44 r;
	Matrix44SetZRotation(m, -angle*M_PI/360.0f);
//	Matrix44Product(m, &r);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}



DEFINE_FUNCTION( LookAt ) {

	RT_ASSERT_ARGC(3); // x, y, z
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	jsdouble x, y, z;
	JS_ValueToNumber(cx, argv[0], &x);
	JS_ValueToNumber(cx, argv[1], &y);
	JS_ValueToNumber(cx, argv[2], &z);
	Vector3 up;
	Vector3Set(&up, 0,0,1);
	Vector3 to;
	Vector3Set(&to, x,y,z);
	Matrix44LookAt(m, &to, &up);
	return JS_TRUE;
}


DEFINE_FUNCTION( Invert ) {

	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	Matrix44Invert(m);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Product ) {
	
	RT_ASSERT_ARGC(1)
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, obj); // tm for thisMatrix
	RT_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	if ( GetMatrixHelper(cx, argv[0], &m) == JS_FALSE )
		return JS_FALSE;
	Matrix44Product(tm,m); // <- mult
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( InverseProduct ) {

	RT_ASSERT_ARGC(1)
	Matrix44 *tm = (Matrix44*)JS_GetPrivate(cx, obj); // tm for thisMatrix
	RT_ASSERT_RESOURCE(tm);
	Matrix44 tmp, *m = &tmp;
	if ( GetMatrixHelper(cx, argv[0], &m) == JS_FALSE )
		return JS_FALSE;
	Matrix44InverseProduct(tm,m); // <- mult
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}



BEGIN_FUNCTION_MAP
	FUNCTION( Dump )
	FUNCTION( Clear )
	FUNCTION( Load )
	FUNCTION( LoadRotation )
	FUNCTION( LoadTranslation )
	FUNCTION( Product )
	FUNCTION( InverseProduct )
	FUNCTION( Invert )
//	FUNCTION( Translate )
	FUNCTION( Translation )
	FUNCTION( ClearRotation )
	FUNCTION( ClearTranslation )
	FUNCTION( Rotate )
	FUNCTION( RotationX )
	FUNCTION( RotationY )
	FUNCTION( RotationZ )
	FUNCTION( Rotation )
	FUNCTION( LookAt )
END_MAP

BEGIN_PROPERTY_MAP
END_MAP

NO_STATIC_FUNCTION_MAP
//BEGIN_STATIC_FUNCTION_MAP
//END_MAP

NO_STATIC_PROPERTY_MAP
//BEGIN_STATIC_PROPERTY_MAP
//END_MAP

//NO_CLASS_CONSTRUCT
NO_OBJECT_CONSTRUCT
//NO_FINALIZE
NO_CALL
NO_PROTOTYPE
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS(Transformation, HAS_PRIVATE, NO_RESERVED_SLOT)


/*

	First Person Shooter tricks:
		http://www.delphi3d.net/articles/printarticle.php?article=viewing.htm


*/