#include "stdafx.h"
#include "world.h"
#include "space.h"
#include "body.h"

#include "stdlib.h"

#define MAX_CONTACTS 100

struct ColideContextPrivate {
	ode::dJointGroupID contactGroupId;
	ode::dWorldID worldId;
	int contactCount;
};

static void nearCallback (void *data, ode::dGeomID o1, ode::dGeomID o2) {

	// Doc: http://opende.sourceforge.net/wiki/index.php/Manual_%28Joint_Types_and_Functions%29
	ColideContextPrivate *ccp = (ColideContextPrivate*)data; // beware: *data is local to Step function

	// ignore collisions between bodies that are connected by the same joint
	ode::dBodyID Body1 = NULL, Body2 = NULL;
	if (o1) Body1 = dGeomGetBody(o1);
	if (o2) Body2 = dGeomGetBody(o2);
	if (Body1 && Body2 && dAreConnected(Body1, Body2))
		return;

	int i,n;
	ode::dContact contact[MAX_CONTACTS];
	n = ode::dCollide(o1,o2,MAX_CONTACTS,&contact[0].geom,sizeof(ode::dContact));
	ccp->contactCount = n;
	for ( i=0; i<n; i++ ) {

		contact[i].surface.mode = ode::dContactSlip1 | ode::dContactSlip2 | ode::dContactSoftERP | ode::dContactSoftCFM | ode::dContactApprox1;
		contact[i].surface.mu = dInfinity;
		contact[i].surface.slip1 = 0.1;
		contact[i].surface.slip2 = 0.1;
		contact[i].surface.soft_erp = 0.5;
		contact[i].surface.soft_cfm = 0.3;
/*
		contact[i].surface.mode = ode::dContactBounce | ode::dContactSlip1 | ode::dContactSlip2;
		contact[i].surface.mu = dInfinity;
		contact[i].surface.bounce = 0.2;
		contact[i].surface.bounce_vel = 0;
		contact[i].surface.slip1 = 0.1;
		contact[i].surface.slip2 = 0.1;
*/				
		ode::dJointID c = ode::dJointCreateContact(ccp->worldId,ccp->contactGroupId,&contact[i]);
		ode::dJointAttach(c, dGeomGetBody(contact[i].geom.g1), dGeomGetBody(contact[i].geom.g2));
	}
}


BEGIN_CLASS( World )

DEFINE_FINALIZE() {

	ode::dWorldID worldId = (ode::dWorldID)JS_GetPrivate( cx, obj );
	if ( worldId != NULL ) {

		jsval val;
		JS_GetReservedSlot(cx, obj, WORLD_SLOT_CONTACTGROUP, &val);
		ode::dJointGroupDestroy((ode::dJointGroupID)JSVAL_TO_PRIVATE(val));
		ode::dWorldDestroy(worldId); // [TBD] Destroy a world and everything in it.
		JS_SetPrivate(cx,obj,NULL);
	}
}


DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(&classWorld);
	ode::dWorldID worldId = ode::dWorldCreate();
	JS_SetPrivate(cx, obj, worldId);
	ode::dJointGroupID contactgroup = ode::dJointGroupCreate(0);
	RT_ASSERT( contactgroup != NULL, "Unable to create contact group." );
	JS_SetReservedSlot(cx, obj, WORLD_SLOT_CONTACTGROUP, PRIVATE_TO_JSVAL(contactgroup));

	JSObject *spaceObject = JS_ConstructObject(cx, &classSpace, NULL, NULL); // no arguments = create a topmost space object
	RT_ASSERT( spaceObject != NULL, "Unable to construct Space for the World." );
	JS_DefineProperty(cx, obj, "space", OBJECT_TO_JSVAL(spaceObject), NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY);

//	JS_SetReservedSlot(cx, obj, WORLD_SLOT_SPACE, PRIVATE_TO_JSVAL(contactgroup));
//	JS_DefineObject(cx, obj, "space", &classSpace, NULL, JSPROP_PERMANENT|JSPROP_READONLY );
	return JS_TRUE;
}


DEFINE_FUNCTION( Destroy ) {

	Finalize(cx, obj); // shortcut
	return JS_TRUE;
}


DEFINE_FUNCTION( Step ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_CLASS(obj, &classWorld);
	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE(worldID);
	jsdouble value;
	JS_ValueToNumber(cx, argv[0], &value);

	//jsval val;
	//JS_GetReservedSlot(cx, obj, WORLD_SLOT_SPACE, &val);
	//ode::dSpaceID space = (ode::dSpaceID)JSVAL_TO_PRIVATE(val);
	//ode::dSpaceCollide(space,0,&nearCallback);

	ode::dSpaceID spaceId;
	jsval val;
	JS_GetProperty(cx, obj, "space", &val);
	if ( ValToSpaceID(cx, val, &spaceId) == JS_FALSE )
		return JS_FALSE;

	JS_GetReservedSlot(cx, obj, WORLD_SLOT_CONTACTGROUP, &val);
	ode::dJointGroupID contactgroup = (ode::dJointGroupID)JSVAL_TO_PRIVATE(val);

	ColideContextPrivate ccp = { contactgroup, worldID, 0 };
	ode::dSpaceCollide(spaceId, (void*)&ccp, &nearCallback);

	if ( argc >= 2 && argv[1] == JSVAL_TRUE ) // quick ?
		ode::dWorldQuickStep(worldID, value);
	else
		ode::dWorldStep(worldID, value);

	if ( ccp.contactCount > 0 ) // do not empty if not needed
		ode::dJointGroupEmpty(contactgroup);

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


DEFINE_PROPERTY( env ) {

	if ( *vp == JSVAL_VOID ) { //  create it if it does not exist

		JSObject *staticBody = JS_NewObject(cx, &classBody, NULL, NULL);
		RT_ASSERT_ALLOC(staticBody);
		JS_SetPrivate(cx, staticBody, (ode::dBodyID)0);
		*vp = OBJECT_TO_JSVAL(staticBody);	
	}
	return JS_TRUE;
}




CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Step )
		FUNCTION( Destroy )
	END_FUNCTION_SPEC
	
	BEGIN_PROPERTY_SPEC
		PROPERTY( gravity )
		PROPERTY_READ_STORE( env )
		PROPERTY_SWITCH( ERP, real )
		PROPERTY_SWITCH( CFM, real )
		PROPERTY_SWITCH( quickStepNumIterations, real )
		PROPERTY_SWITCH( contactSurfaceLayer, real )
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2)
END_CLASS
