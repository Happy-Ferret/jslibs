#include "stdafx.h"
#include "world.h"
#include "body.h"

#include "../smtools/smtools.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void world_Finalize(JSContext *cx, JSObject *obj) {

	ode::dWorldID worldId = (ode::dWorldID)JS_GetPrivate( cx, obj );
	if ( worldId != NULL ) {
		ode::dWorldDestroy(worldId); // Destroy a world and everything in it.
		JS_SetPrivate(cx,obj,NULL);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool world_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_CONSTRUCTING(&world_class);
	ode::dWorldID worldId = ode::dWorldCreate();
	JS_SetPrivate(cx, obj, worldId); 
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool world_destroy(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	
	world_Finalize(cx, obj);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool world_step(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_CLASS(obj, &world_class);
	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate( cx, obj );
	RT_ASSERT( worldID != NULL, RT_ERROR_NOT_INITIALIZED );
	jsdouble value;
	JS_ValueToNumber(cx, argv[0], &value);
	if ( argc >= 2 && argv[1] == JSVAL_TRUE ) // quick ?
		ode::dWorldQuickStep(worldID, value);
	else
		ode::dWorldStep(worldID, value);
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec world_FunctionSpec[] = { // *name, call, nargs, flags, extra
	{ "Destroy" , world_destroy, 0, 0, 0 },
	{ "Step"    , world_step   , 0, 0, 0 },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool world_getter_gravity(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate( cx, obj );
	RT_ASSERT( worldID != NULL, RT_ERROR_NOT_INITIALIZED );
	ode::dVector3 gravity;
	ode::dWorldGetGravity(worldID, gravity);
	FloatVectorToArray(cx, 3, gravity, vp);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool world_setter_gravity(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate( cx, obj );
	RT_ASSERT( worldID != NULL, RT_ERROR_NOT_INITIALIZED );
	ode::dVector3 gravity;
	FloatArrayToVector(cx, 3, vp, gravity);
	ode::dWorldSetGravity( worldID, gravity[0], gravity[1], gravity[2] );
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum { ERP, CFM, quickStepNumIterations, contactSurfaceLayer };

JSBool world_setter_real(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

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

JSBool world_getter_real(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool world_get_static(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
	
	if ( *vp == JSVAL_VOID ) { //  create it if it does not exist

		JSObject *staticBody = JS_NewObject(cx, &body_class, NULL, NULL);
		RT_ASSERT_ALLOC(staticBody);
		JS_SetPrivate(cx, staticBody, (ode::dBodyID)0);
		*vp = OBJECT_TO_JSVAL(staticBody);	
	}

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec world_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "gravity"               , 0, JSPROP_PERMANENT|JSPROP_SHARED, world_getter_gravity, world_setter_gravity },
	{ "ERP"                   , ERP, JSPROP_PERMANENT|JSPROP_SHARED, world_getter_real, world_setter_real },
	{ "CFM"                   , CFM, JSPROP_PERMANENT|JSPROP_SHARED, world_getter_real, world_setter_real },
	{ "quickStepNumIterations", quickStepNumIterations, JSPROP_PERMANENT|JSPROP_SHARED, world_getter_real, world_setter_real },
	{ "contactSurfaceLayer"   , contactSurfaceLayer, JSPROP_PERMANENT|JSPROP_SHARED, world_getter_real, world_setter_real },
	{ "static"                , 0, JSPROP_PERMANENT|JSPROP_READONLY, world_get_static, NULL },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass world_class = { "World", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, world_Finalize,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *worldInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &world_class, world_construct, 0, world_PropertySpec, world_FunctionSpec, NULL, NULL );
}


/****************************************************************

*/