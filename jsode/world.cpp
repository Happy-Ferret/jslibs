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

#include "stdafx.h"
#include "world.h"
#include "space.h"
#include "body.h"
#include "geom.h"

#include "jsobj.h"

#include "stdlib.h"

#define MAX_CONTACTS 50

struct ColideContextPrivate {
	JSContext *cx;
	ode::dSurfaceParameters *defaultSurfaceParameters;
	ode::dJointGroupID contactGroupId;
	ode::dWorldID worldId;
//	int contactCount;
};

static void nearCallback (void *data, ode::dGeomID o1, ode::dGeomID o2) {

	// Doc: http://opende.sourceforge.net/wiki/index.php/Manual_%28Joint_Types_and_Functions%29
	ColideContextPrivate *ccp = (ColideContextPrivate*)data; // beware: *data is local to Step function
	JSContext *cx = ccp->cx;
	// ignore collisions between bodies that are connected by the same joint
	ode::dBodyID Body1 = NULL, Body2 = NULL;
	if (o1) Body1 = dGeomGetBody(o1);
	if (o2) Body2 = dGeomGetBody(o2);
	if (Body1 && Body2 && dAreConnected(Body1, Body2))
		return;

	ode::dContact contact[MAX_CONTACTS];
	int n = ode::dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sizeof(ode::dContact)); // (TBD) or sizeof(ode::dContactGeom) ??? // It is an error for skip to be smaller than sizeof(dContactGeom).

	if ( n > 0 ) {

		JSObject *obj1 = (JSObject*)ode::dGeomGetData(o1);
		JSObject *obj2 = (JSObject*)ode::dGeomGetData(o2);

		jsval func1 = JSVAL_VOID, func2 = JSVAL_VOID;

		if ( obj1 != NULL ) { // assert that the javascript object (over the Geom) is not finalized

			JS_GetProperty(cx, obj1, COLLIDE_FEEDBACK_FUNCTION_NAME, &func1);
			RT_SAFE( if ( func1 != JSVAL_VOID && JS_TypeOfValue(cx, func1) != JSTYPE_FUNCTION ) { JS_ReportError(cx, RT_ERROR_UNEXPECTED_TYPE " Function expected."); return; } ); // we need a function, nothing else
		}

		if ( obj2 != NULL ) { // assert that the javascript object (over the Geom) is not finalized

			JS_GetProperty(cx, obj2, COLLIDE_FEEDBACK_FUNCTION_NAME, &func2);
			RT_SAFE( if ( func2 != JSVAL_VOID && JS_TypeOfValue(cx, func2) != JSTYPE_FUNCTION ) { JS_ReportError(cx, RT_ERROR_UNEXPECTED_TYPE " Function expected."); return; } ); // we need a function, nothing else
		}

		for ( int i=0; i<n; i++ ) {

//			ode::dVector3 vel;
			if ( func1 != JSVAL_VOID || func2 != JSVAL_VOID ) { // compute impact velocity only if needed
/*

				ode::dVector3 *pos = &contact[i].geom.pos;
				ode::dVector3 vel2;
				if ( Body1 != NULL ) {

					ode::dBodyGetPointVel( Body1, *pos[0], *pos[1], *pos[2], vel ); // dReal px, dReal py, dReal pz, dVector3 result
				} else { // static thing

					vel[0] = 0; vel[1] = 0; vel[2] = 0;
				}

				if ( Body2 != NULL ) {

					ode::dBodyGetPointVel( Body2, *pos[0], *pos[1], *pos[2], vel2 ); // dReal px, dReal py, dReal pz, dVector3 result
				} else { // static thing

					vel2[0] = 0; vel2[1] = 0; vel2[2] = 0;
				}

				vel[0] += vel2[0];
				vel[1] += vel2[1];
				vel[2] += vel2[2];
*/

				jsval pos;
				FloatVectorToArray(cx, 3, contact[i].geom.pos, &pos);

				if ( func1 != JSVAL_VOID ) {

					jsval rval, argv[] = { INT_TO_JSVAL(i), OBJECT_TO_JSVAL(obj1), obj2 ? OBJECT_TO_JSVAL(obj2) : JSVAL_VOID, pos }; // INT_TO_JSVAL(vel[0]), INT_TO_JSVAL(vel[1]), INT_TO_JSVAL(vel[2])
					JS_CallFunctionValue( cx, obj1, func1, sizeof(argv)/sizeof(*argv), argv, &rval); // JS_CallFunction() DO NOT WORK !!!
				}


				if ( func2 != JSVAL_VOID ) {

					jsval rval, argv[] = { INT_TO_JSVAL(i), OBJECT_TO_JSVAL(obj2), obj1 ? OBJECT_TO_JSVAL(obj1) : JSVAL_VOID, pos }; // INT_TO_JSVAL(vel[0]), INT_TO_JSVAL(vel[1]), INT_TO_JSVAL(vel[2])
					JS_CallFunctionValue( cx, obj2, func2, sizeof(argv)/sizeof(*argv), argv, &rval); // JS_CallFunction() DO NOT WORK !!!
				}
			}

			contact[i].surface = *ccp->defaultSurfaceParameters;

			// mixing :
			//dReal mu;		// min
			//dReal mu2;	// min
			//dReal bounce;	// average
			//dReal bounce_vel; // min
			//dReal soft_erp; // average
			//dReal soft_cfm; // average
			//dReal motion1,motion2;	// add
			//dReal slip1,slip2;	// ?

			ode::dJointID c = ode::dJointCreateContact(ccp->worldId,ccp->contactGroupId,&contact[i]);
			ode::dJointAttach(c, dGeomGetBody(contact[i].geom.g1), dGeomGetBody(contact[i].geom.g2));
		}
	}
}


BEGIN_CLASS( World )

DEFINE_FINALIZE() {

	ode::dWorldID worldId = (ode::dWorldID)JS_GetPrivate( cx, obj );
	if ( worldId != NULL ) {

//		jsval val;
//		JS_GetReservedSlot(cx, obj, WORLD_SLOT_CONTACTGROUP, &val);
//		ode::dJointGroupDestroy((ode::dJointGroupID)JSVAL_TO_PRIVATE(val));
		ode::dWorldDestroy(worldId); // (TBD) Destroy a world and everything in it.
		JS_SetPrivate(cx,obj,NULL);
	}
}

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(&classWorld);
	ode::dWorldID worldId = ode::dWorldCreate();
	JS_SetPrivate(cx, obj, worldId);
	//ode::dJointGroupID contactgroup = ode::dJointGroupCreate(0);
	//RT_ASSERT( contactgroup != NULL, "Unable to create contact group." );
	//JS_SetReservedSlot(cx, obj, WORLD_SLOT_CONTACTGROUP, PRIVATE_TO_JSVAL(contactgroup));

	JSObject *spaceObject = JS_ConstructObject(cx, &classSpace, NULL, NULL); // no arguments = create a topmost space object
	RT_ASSERT( spaceObject != NULL, "Unable to construct Space for the World." );
	JS_DefineProperty(cx, obj, WORLD_SPACE_PROPERTY_NAME, OBJECT_TO_JSVAL(spaceObject), NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY);

//	JS_SetReservedSlot(cx, obj, WORLD_SLOT_SPACE, PRIVATE_TO_JSVAL(spaceObject));
//	JS_DefineObject(cx, obj, WORLD_SPACE_PROPERTY_NAME, &classSpace, NULL, JSPROP_PERMANENT|JSPROP_READONLY );

	JSObject *surfaceParameters = JS_ConstructObject(cx, &classSurfaceParameters, NULL, NULL);
	RT_ASSERT( surfaceParameters != NULL, "Unable to construct classSurfaceParameters." );
	JS_DefineProperty(cx, obj, DEFAULT_SURFACE_PARAMETERS_PROPERTY_NAME, OBJECT_TO_JSVAL(surfaceParameters), NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY );

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
	JS_GetProperty(cx, obj, WORLD_SPACE_PROPERTY_NAME, &val);
	RT_ASSERT_DEFINED( val );
	RT_CHECK_CALL( ValToSpaceID(cx, val, &spaceId) );

	jsval defaultSurfaceParametersObject;
	JS_GetProperty(cx, obj, DEFAULT_SURFACE_PARAMETERS_PROPERTY_NAME, &defaultSurfaceParametersObject);
	RT_ASSERT_DEFINED( defaultSurfaceParametersObject );
	RT_ASSERT_OBJECT( defaultSurfaceParametersObject );
	RT_ASSERT_CLASS( JSVAL_TO_OBJECT(defaultSurfaceParametersObject), &classSurfaceParameters ); // (TBD) simplify RT_ASSERT
	ode::dSurfaceParameters *defaultSurfaceParameters = (ode::dSurfaceParameters*)JS_GetPrivate(cx, JSVAL_TO_OBJECT(defaultSurfaceParametersObject)); // beware: local variable !
	RT_ASSERT_RESOURCE( defaultSurfaceParameters );

//	JS_GetReservedSlot(cx, obj, WORLD_SLOT_CONTACTGROUP, &val);
//	ode::dJointGroupID contactgroup = (ode::dJointGroupID)JSVAL_TO_PRIVATE(val);
	ode::dJointGroupID contactgroup = ode::dJointGroupCreate(0);

	ColideContextPrivate ccp = { cx, defaultSurfaceParameters, contactgroup, worldID };
	ode::dSpaceCollide(spaceId, (void*)&ccp, &nearCallback);

	// (TBD) see dWorldSetQuickStepW and dWorldSetAutoEnableDepthSF1

	if ( argc >= 2 ) {

		RT_ASSERT_INT(argv[1]);
		ode::dWorldSetQuickStepNumIterations(worldID, JSVAL_TO_INT(argv[1]));
		ode::dWorldQuickStep(worldID, value);
	} else {
		ode::dWorldStep(worldID, value);
	}
	ode::dJointGroupDestroy(contactgroup); // dJointGroupEmpty calls dJointGroupEmpty

	if (JS_IsExceptionPending(cx)) // need JS_ErrorFromException(...) ??
		return JS_FALSE;

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


enum { ERP, CFM, /*quickStepNumIterations,*/ contactSurfaceLayer };

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
//		case quickStepNumIterations:
//			ode::dWorldSetQuickStepNumIterations(worldID, (int)value);
//			break;
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
//		case quickStepNumIterations:
//			value = ode::dWorldGetQuickStepNumIterations(worldID);
//			break;
		case contactSurfaceLayer:
			value = ode::dWorldGetContactSurfaceLayer(worldID);
			break;
	}
	JS_NewDoubleValue(cx, value, vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( env ) {

	if ( *vp == JSVAL_VOID ) { //  create it if it does not exist and store it (cf. PROPERTY_READ_STORE)

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
//		PROPERTY_SWITCH( quickStepNumIterations, real )
		PROPERTY_SWITCH( contactSurfaceLayer, real )
	END_PROPERTY_SPEC

	HAS_PRIVATE // ode::dWorldID
	HAS_RESERVED_SLOTS(2) // contactGroup, space
END_CLASS
