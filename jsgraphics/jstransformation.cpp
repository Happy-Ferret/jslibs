#include "stdafx.h"
#include "jstransformation.h"
#include "../smtools/object.h"
#include "../common/jsNativeInterface.h"

#include "vector3.h"
#include "matrix44.h"

#define _USE_MATH_DEFINES
#include <math.h>


static int ReadMatrix(void *pv, float **m) { // Doc: __declspec(noinline) tells the compiler to never inline a particular function.

	*m = ((Matrix44*)pv)->raw;
	return true;
}

BEGIN_CLASS

DEFINE_FINALIZE() {

	// Doc: For example, if you use malloc, the result depends on the operand size. If arg >= 8, 
	//      alignment will be 8 byte aligned. If arg < 8, alignment will be the first power of 2 less than arg. 
	//      For example, if you use malloc(7), alignment is 4 bytes.
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
	JSBool status;
	JS_SetPrivate(cx, obj, m);
	status = SetNativeInterface(cx, obj, NI_READ_MATRIX44, ReadMatrix, m); // [TBD] check return status
	RT_ASSERT( status == JS_TRUE, "Unable SetNativeInterface." );
	return JS_TRUE;
}

DEFINE_FUNCTION( Load ) {

	RT_ASSERT_ARGC(1)
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	RT_ASSERT( JSVAL_IS_OBJECT(argv[0]), "Argument must be an object." );

	// [TBD] here check if arg is Transformation ( and then read it )

	JSObject *argObj = JSVAL_TO_OBJECT(argv[0]);
	NIMatrix44Read ReadMatrix;
	void *descriptor;
	GetNativeInterface(cx, argObj, NI_READ_MATRIX44, (void**)&ReadMatrix, &descriptor); // NI is for NativeInterface, nothing to see with Monty Python
	if ( ReadMatrix != NULL && descriptor != NULL ) {
		
		float *tmp = m->raw; // prepare a 'modifiable' pointer
		ReadMatrix(descriptor, &tmp ); // ReadMatrix will copy data into tmp OR replace tmp by its own float pointer
		if ( tmp != m->raw ) // check if the pointer has been modified
			memcpy(m->raw,tmp, sizeof(Matrix44) ); // if it is, copy the data
		return JS_TRUE;
	}
	REPORT_ERROR( "Unable to read a matrix." );
}


DEFINE_FUNCTION( Translate ) {

	RT_ASSERT_ARGC(3); // x, y, z
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	jsdouble x, y, z;
	JS_ValueToNumber(cx, argv[0], &x);
	JS_ValueToNumber(cx, argv[1], &y);
	JS_ValueToNumber(cx, argv[2], &z);
	Vector3 vector;
	Vector3Set(&vector, x,y,z);
	Matrix44 t;
	Matrix44Identity(&t);
	Matrix44Translation(&t, &vector);
	Matrix44Multiply(m, &t);
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
	Matrix44Rotation(&r, &axis, -angle*M_PI/360.0f);
	Matrix44Multiply(m, &r);
	return JS_TRUE;
}


DEFINE_FUNCTION( Invert ) {

	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	Matrix44Invert(m);
	return JS_TRUE;
}


DEFINE_FUNCTION( Multiply ) {
	
	RT_ASSERT_ARGC(1)
	Matrix44 *m = (Matrix44*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(m);
	RT_ASSERT( JSVAL_IS_OBJECT(argv[0]), "Argument must be an object." );

	JSObject *argObj = JSVAL_TO_OBJECT(argv[0]);
	if ( JS_GetClass(argObj) == thisClass ) {

		Matrix44 *mx = (Matrix44*)JS_GetPrivate(cx, argObj);
		RT_ASSERT_RESOURCE(mx);
		Matrix44Multiply(m,mx); // <- mult
		return JS_TRUE;
	}

	NIMatrix44Read ReadMatrix;
	void *descriptor;
	GetNativeInterface(cx, argObj, NI_READ_MATRIX44, (void**)&ReadMatrix, &descriptor);
	if ( ReadMatrix != NULL && descriptor != NULL ) {

		Matrix44 mx;
		float *tmp = mx.raw;
		ReadMatrix(descriptor, &tmp );
		Matrix44Multiply(m, (Matrix44*)tmp); // <- mult
		return JS_TRUE;
	}
	REPORT_ERROR( "Unable to read a matrix." );
}



BEGIN_FUNCTION_MAP
	FUNCTION( Load )
	FUNCTION( Translate )
	FUNCTION( Rotate )
	FUNCTION( Multiply )
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
