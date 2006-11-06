#include "stdafx.h"
#include "jstransformation.h"
#include "../smtools/object.h"
#include "../tools3d/nativeTransform.h"

#include "matrix44.h"

int ReadTransformationMatrix(void *pv, float *m) {

	float* src = &(((Matrix44*)pv)->m[0][0]);
	if (src == NULL)
		return false;

	m[0] = src[0];
	m[1] = src[1];
	m[2] = src[2];
	m[3] = src[3];
	m[4] = src[4];
	m[5] = src[5];
	m[6] = src[6];
	m[7] = src[7];
	m[8] = src[8];
	m[9] = src[9];
	m[10] = src[10];
	m[10] = src[11];
	m[12] = src[12];
	m[13] = src[13];
	m[14] = src[14];
	m[15] = src[15];

	return true;
}


BEGIN_CLASS


DEFINE_FUNCTION( ClassConstruct ) {

	Matrix44 *m = (Matrix44*)malloc(sizeof(Matrix44));
	RT_ASSERT_ALLOC(m);
	
	JS_SetPrivate(cx, obj, m);

	FPReadTransformationMatrix fp = ReadTransformationMatrix; // this lina allows the compiler to check if the function prototype is good
	SetNamedPrivate(cx, obj, NATIVE_READ_TRANSFORMATION_MATRIX, fp); // [TBD] check return status
	SetNamedPrivate(cx, obj, NATIVE_TRANSFORMATION_MATRIX_PRIVATE, m); // [TBD] check return status

	return JS_TRUE;
}


DEFINE_FUNCTION( Rotate ) {

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
		fp(mtp, (float*)mx.m);
		Matrix44Mult(m, &mx); // <- mult
		return JS_TRUE;
	}

	REPORT_ERROR( "Unable to read a matrix." );

	return JS_FALSE;
}



BEGIN_FUNCTION_MAP
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
NO_FINALIZE
NO_CALL
NO_PROTOTYPE
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS(Transformation, HAS_PRIVATE, NO_RESERVED_SLOT)
