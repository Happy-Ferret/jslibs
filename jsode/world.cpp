#include "stdafx.h"
#include "world.h"
#include "body.h"

BEGIN_CLASS

DEFINE_FINALIZE() {

	ode::dWorldID worldId = (ode::dWorldID)JS_GetPrivate( cx, obj );
	if ( worldId != NULL )
		ode::dWorldDestroy(worldId); // [TBD] Destroy a world and everything in it.
}


DEFINE_FUNCTION( ClassConstruct ) {

	RT_ASSERT_CONSTRUCTING(&classWorld);
	ode::dWorldID worldId = ode::dWorldCreate();
	JS_SetPrivate(cx, obj, worldId); 
	return JS_TRUE;
}


DEFINE_FUNCTION( Destroy ) {
	
	ode::dWorldID worldId = (ode::dWorldID)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( worldId );
	ode::dWorldDestroy(worldId); // [TBD] Destroy a world and everything in it.
	JS_SetPrivate(cx,obj,NULL);
	return JS_TRUE;
}


DEFINE_FUNCTION( Step ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_CLASS(obj, &classWorld);
	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE(worldID);
	jsdouble value;
	JS_ValueToNumber(cx, argv[0], &value);
	if ( argc >= 2 && argv[1] == JSVAL_TRUE ) // quick ?
		ode::dWorldQuickStep(worldID, value);
	else
		ode::dWorldStep(worldID, value);
	return JS_TRUE;
}

DEFINE_PROPERTY( gravityGetter ) {

	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate( cx, obj );
	RT_ASSERT( worldID != NULL, RT_ERROR_NOT_INITIALIZED );
	ode::dVector3 gravity;
	ode::dWorldGetGravity(worldID, gravity);
	FloatVectorToArray(cx, 3, gravity, vp);
	return JS_TRUE;
}

DEFINE_PROPERTY( gravitySetter ) {

	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate( cx, obj );
	RT_ASSERT( worldID != NULL, RT_ERROR_NOT_INITIALIZED );
	ode::dVector3 gravity;
	FloatArrayToVector(cx, 3, vp, gravity);
	ode::dWorldSetGravity( worldID, gravity[0], gravity[1], gravity[2] );
	return JS_TRUE;
}


enum { ERP, CFM, quickStepNumIterations, contactSurfaceLayer };

DEFINE_PROPERTY( realSetter ) {

	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate( cx, obj );
	RT_ASSERT( worldID != NULL, RT_ERROR_NOT_INITIALIZED );
	jsdouble value;
	JS_ValueToNumber(cx, *vp, &value);
	switch(JSVAL_TO_INT(id)) {
		case ERP:
			ode::dWorldSetERP(worldID, value);
			break;
		case CFM:
			ode::dWorldSetCFM(worldID, value);
			break;
		case quickStepNumIterations:
			ode::dWorldSetQuickStepNumIterations(worldID, (int)value);
			break;
		case contactSurfaceLayer:
			ode::dWorldSetContactSurfaceLayer(worldID, value);
			break;
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( realGetter ) {

	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate( cx, obj );
	RT_ASSERT( worldID != NULL, RT_ERROR_NOT_INITIALIZED );
	jsdouble value;
	switch(JSVAL_TO_INT(id)) {
		case ERP:
			value = ode::dWorldGetERP(worldID);
			break;
		case CFM:
			value = ode::dWorldGetCFM(worldID);
			break;
		case quickStepNumIterations:
			value = ode::dWorldGetQuickStepNumIterations(worldID);
			break;
		case contactSurfaceLayer:
			value = ode::dWorldGetContactSurfaceLayer(worldID);
			break;
	}
	JS_NewDoubleValue(cx, value, vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( body ) {

	if ( *vp == JSVAL_VOID ) { //  create it if it does not exist

		JSObject *staticBody = JS_NewObject(cx, &classBody, NULL, NULL);
		RT_ASSERT_ALLOC(staticBody);
		JS_SetPrivate(cx, staticBody, (ode::dBodyID)0);
		*vp = OBJECT_TO_JSVAL(staticBody);	
	}
	return JS_TRUE;
}


BEGIN_FUNCTION_MAP
	FUNCTION( Step )
	FUNCTION( Destroy )
END_MAP

BEGIN_PROPERTY_MAP
	READWRITE( gravity )
	READONLY( body )
	PROPERTY_TABLE( ERP, real )
	PROPERTY_TABLE( CFM, real )
	PROPERTY_TABLE( quickStepNumIterations, real )
	PROPERTY_TABLE( contactSurfaceLayer, real )
END_MAP


NO_STATIC_FUNCTION_MAP
NO_STATIC_PROPERTY_MAP

//NO_CLASS_CONSTRUCT
NO_OBJECT_CONSTRUCT
//NO_FINALIZE
NO_CALL
NO_PROTOTYPE
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS( World, HAS_PRIVATE, NO_RESERVED_SLOT)
