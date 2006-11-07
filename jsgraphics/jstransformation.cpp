#include "stdafx.h"
#include "jstransformation.h"
#include "../smtools/object.h"
#include "../tools3d/nativeTransform.h"

#include "vector3.h"
#include "matrix44.h"

// __declspec(align(2))

int ReadTransformationMatrix(void *pv, float *m) {

	float* src = &(((Matrix44*)pv)->m[0][0]);
	if (src == NULL)
		return false;
	memcpy( m, src, sizeof(float)*16 );
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

	FPReadTransformationMatrix fp = ReadTransformationMatrix; // this lina allows the compiler to check if the function prototype is good

//	void *x = JSVAL_TO_PRIVATE( PRIVATE_TO_JSVAL( fp ) );

	if ( SetNamedPrivate(cx, obj, NATIVE_READ_TRANSFORMATION_MATRIX, fp) == JS_FALSE )
		return JS_FALSE;
	if ( SetNamedPrivate(cx, obj, NATIVE_TRANSFORMATION_MATRIX_PRIVATE, m)  == JS_FALSE )
		return JS_FALSE;

	return JS_TRUE;
}

DEFINE_FUNCTION( Translate ) {

	RT_ASSERT_ARGC(3); // x, y, z
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	jsdouble x, y, z;
	JS_ValueToNumber(cx, argv[0], &x);
	JS_ValueToNumber(cx, argv[1], &y);
	JS_ValueToNumber(cx, argv[2], &z);
	Vector3 axis;
	axis.x = x;
	axis.y = y;
	axis.z = z;
	Matrix44Translate(m, &axis);
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
	axis.x = x;
	axis.y = y;
	axis.z = z;
	Matrix44Rotate(m, &axis, angle);
	return JS_TRUE;
}


DEFINE_FUNCTION( Invert ) {

	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	Matrix44Invert(m);
	return JS_TRUE;
}

DEFINE_FUNCTION( Mult ) {
	
	RT_ASSERT_ARGC(1)

	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	RT_ASSERT( JSVAL_IS_OBJECT(argv[0]), "Argument must be an object." );

	JSObject *argObj = JSVAL_TO_OBJECT(argv[0]);
	if ( JS_GetClass(argObj) == thisClass ) {

		Matrix44 *mx = (Matrix44*)JS_GetPrivate(cx, argObj);
		RT_ASSERT_RESOURCE(mx);
		Matrix44Mult(m,mx); // <- mult
		return JS_TRUE;
	}

	// if it is not an jsransformation object, try if object provide an interface to read matrix
	void *mtp;
	GetNamedPrivate(cx, obj, NATIVE_TRANSFORMATION_MATRIX_PRIVATE, &mtp);
	if ( mtp != NULL ) {

		Matrix44 mx;
		FPReadTransformationMatrix fp;
		GetNamedPrivate(cx, argObj, NATIVE_READ_TRANSFORMATION_MATRIX, (void**)&fp);
		fp(mtp, (float*)mx.m); // [TBD] avoid copy
		Matrix44Mult(m, &mx); // <- mult
		return JS_TRUE;
	}

	REPORT_ERROR( "Unable to read a matrix." );

	return JS_FALSE;
}



BEGIN_FUNCTION_MAP
	FUNCTION( Translate )
	FUNCTION( Rotate )
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
