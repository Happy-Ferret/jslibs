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

static void nearCallback(void *data, ode::dGeomID o1, ode::dGeomID o2) {

	// Doc: http://opende.sourceforge.net/wiki/index.php/Manual_%28Joint_Types_and_Functions%29
	ColideContextPrivate *ccp = (ColideContextPrivate*)data; // beware: *data is local to Step function
	JSContext *cx = ccp->cx;
	// ignore collisions between bodies that are connected by the same joint
	ode::dBodyID Body1 = NULL, Body2 = NULL;
	if (o1) Body1 = dGeomGetBody(o1);
	if (o2) Body2 = dGeomGetBody(o2);
	if (Body1 && Body2 && dAreConnected(Body1, Body2))
		return;

	ode::dContact contact[MAX_CONTACTS]; // (TBD) make this configurable.
	int n = ode::dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sizeof(ode::dContact)); // (TBD) or sizeof(ode::dContactGeom) ??? // It is an error for skip to be smaller than sizeof(dContactGeom).

	if ( n <= 0 )
		return;

	JSObject *obj1 = (JSObject*)ode::dGeomGetData(o1);
	JSObject *obj2 = (JSObject*)ode::dGeomGetData(o2);

	jsval func1, func2;

	if ( obj1 != NULL ) { // assert that the javascript object (over the Geom) is not finalized

		JS_GetProperty(cx, obj1, COLLIDE_FEEDBACK_FUNCTION_NAME, &func1);
		JL_S_ASSERT( JSVAL_IS_VOID(func1) || JsvalIsFunction(cx, func1), "Invalid impact function." );
	} else {

		func1 = JSVAL_VOID;
	}


	if ( obj2 != NULL ) { // assert that the javascript object (over the Geom) is not finalized

		JS_GetProperty(cx, obj2, COLLIDE_FEEDBACK_FUNCTION_NAME, &func2);
		JL_S_ASSERT( JSVAL_IS_VOID(func2) || JsvalIsFunction(cx, func2), "Invalid impact function." );
	} else {

		func2 = JSVAL_VOID;
	}


	for ( int i=0; i<n; i++ ) {

//			ode::dVector3 vel;
		if ( !JSVAL_IS_VOID( func1 ) || !JSVAL_IS_VOID( func2 ) ) { // compute impact velocity only if needed
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
			//FloatVectorToArray(cx, 3, contact[i].geom.pos, &pos);
			FloatVectorToJsval(cx, contact[i].geom.pos, 3, &pos);

			if ( !JSVAL_IS_VOID( func1 ) ) {

				jsval tmp, argv[] = { INT_TO_JSVAL(i), OBJECT_TO_JSVAL(obj1), obj2 ? OBJECT_TO_JSVAL(obj2) : JSVAL_VOID, pos }; // INT_TO_JSVAL(vel[0]), INT_TO_JSVAL(vel[1]), INT_TO_JSVAL(vel[2])
				// GC protection seems to be not needed because these objects (obj1 and obj2) have an owner
				// (TBD) check the previous comment.
				JL_CHK( JS_CallFunctionValue(cx, obj1, func1, COUNTOF(argv), argv, &tmp) );
			}

			if ( !JSVAL_IS_VOID( func2 ) ) {

				jsval tmp, argv[] = { INT_TO_JSVAL(i), OBJECT_TO_JSVAL(obj2), obj1 ? OBJECT_TO_JSVAL(obj1) : JSVAL_VOID, pos }; // INT_TO_JSVAL(vel[0]), INT_TO_JSVAL(vel[1]), INT_TO_JSVAL(vel[2])
				// GC protection seems to be not needed because these objects (obj1 and obj2) have an owner
				// (TBD) check the previous comment.
				JL_CHK( JS_CallFunctionValue( cx, obj2, func2, COUNTOF(argv), argv, &tmp) );
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

		ode::dJointID c = ode::dJointCreateContact(ccp->worldId, ccp->contactGroupId, &contact[i]);
		ode::dJointAttach(c, dGeomGetBody(contact[i].geom.g1), dGeomGetBody(contact[i].geom.g2));
	}

bad:
	return;
}

/**doc fileIndex:top
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( World )


DEFINE_FINALIZE() {

	ode::dWorldID worldId = (ode::dWorldID)JL_GetPrivate( cx, obj );
	if ( worldId != NULL ) {

//		jsval val;
//		JS_GetReservedSlot(cx, obj, WORLD_SLOT_CONTACTGROUP, &val);
//		ode::dJointGroupDestroy((ode::dJointGroupID)JSVAL_TO_PRIVATE(val));
		ode::dWorldDestroy(worldId); // (TBD) Destroy a world and everything in it.
		JL_SetPrivate(cx,obj,NULL);
	}
}

/**doc
$TOC_MEMBER $INAME
 $INAME()
  dWorldCreate
  $H note
   This function creates also a Space (space) object and a SurfaceParameters object (defaultSurfaceParameters).
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();

	ode::dWorldID worldId = ode::dWorldCreate();
	JL_SetPrivate(cx, obj, worldId);
	//ode::dJointGroupID contactgroup = ode::dJointGroupCreate(0);
	//JL_S_ASSERT( contactgroup != NULL, "Unable to create contact group." );
	//JS_SetReservedSlot(cx, obj, WORLD_SLOT_CONTACTGROUP, PRIVATE_TO_JSVAL(contactgroup));

	JSObject *spaceObject = JS_ConstructObject(cx, classSpace, NULL, NULL); // no arguments = create a topmost space object
	JL_S_ASSERT( spaceObject != NULL, "Unable to construct Space for the World." );
	JS_DefineProperty(cx, obj, WORLD_SPACE_PROPERTY_NAME, OBJECT_TO_JSVAL(spaceObject), NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY);

//	JS_SetReservedSlot(cx, obj, WORLD_SLOT_SPACE, PRIVATE_TO_JSVAL(spaceObject));
//	JS_DefineObject(cx, obj, WORLD_SPACE_PROPERTY_NAME, &classSpace, NULL, JSPROP_PERMANENT|JSPROP_READONLY );

	JSObject *surfaceParameters = JS_ConstructObject(cx, classSurfaceParameters, NULL, NULL);
	JL_S_ASSERT( surfaceParameters != NULL, "Unable to construct classSurfaceParameters." );
	JS_DefineProperty(cx, obj, DEFAULT_SURFACE_PARAMETERS_PROPERTY_NAME, OBJECT_TO_JSVAL(surfaceParameters), NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY );

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $INAME()
  TBD
**/
DEFINE_FUNCTION( Destroy ) {

	Finalize(cx, obj); // shortcut
	return JS_TRUE;
}

/**doc
$TOC_MEMBER $INAME
 $INAME( time [, iterations] )
  $H note
   If the _iterations_ argument is given, this uses an iterative method that takes time on the order of m*N and memory on the order of m, where m is the total number of constraint rows and N is the number of iterations.
**/

DEFINE_FUNCTION( Step ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_CLASS(obj, classWorld);
	ode::dWorldID worldID = (ode::dWorldID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE(worldID);
	jsdouble value;
	JS_ValueToNumber(cx, argv[0], &value);

	//jsval val;
	//JS_GetReservedSlot(cx, obj, WORLD_SLOT_SPACE, &val);
	//ode::dSpaceID space = (ode::dSpaceID)JSVAL_TO_PRIVATE(val);
	//ode::dSpaceCollide(space,0,&nearCallback);

	ode::dSpaceID spaceId;
	jsval val;
	JS_GetProperty(cx, obj, WORLD_SPACE_PROPERTY_NAME, &val);
	JL_S_ASSERT_DEFINED( val );
	JL_CHK( ValToSpaceID(cx, val, &spaceId) );

	jsval defaultSurfaceParametersObject;
	JS_GetProperty(cx, obj, DEFAULT_SURFACE_PARAMETERS_PROPERTY_NAME, &defaultSurfaceParametersObject);
	JL_S_ASSERT_DEFINED( defaultSurfaceParametersObject );
	JL_S_ASSERT_OBJECT( defaultSurfaceParametersObject );
	JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT(defaultSurfaceParametersObject), classSurfaceParameters ); // (TBD) simplify RT_ASSERT
	ode::dSurfaceParameters *defaultSurfaceParameters = (ode::dSurfaceParameters*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(defaultSurfaceParametersObject)); // beware: local variable !
	JL_S_ASSERT_RESOURCE( defaultSurfaceParameters );

//	JS_GetReservedSlot(cx, obj, WORLD_SLOT_CONTACTGROUP, &val);
//	ode::dJointGroupID contactgroup = (ode::dJointGroupID)JSVAL_TO_PRIVATE(val);
	ode::dJointGroupID contactgroup = ode::dJointGroupCreate(0);

	ColideContextPrivate ccp;
	ccp.cx = cx; // the context will only be used while the worls step.
	ccp.defaultSurfaceParameters = defaultSurfaceParameters;
	ccp.contactGroupId = contactgroup;
	ccp.worldId = worldID;

	ode::dSpaceCollide(spaceId, (void*)&ccp, &nearCallback);

	// (TBD) see dWorldSetQuickStepW and dWorldSetAutoEnableDepthSF1

	if ( argc >= 2 ) {

		JL_S_ASSERT_INT(argv[1]);
		ode::dWorldSetQuickStepNumIterations(worldID, JSVAL_TO_INT(argv[1]));
		ode::dWorldQuickStep(worldID, value);
	} else {

		ode::dWorldStep(worldID, value);
	}
	ode::dJointGroupDestroy(contactgroup); // dJointGroupEmpty calls dJointGroupEmpty

	if (JS_IsExceptionPending(cx)) // an exception may be throw in geom.impact
		return JS_FALSE;

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
  Gets or sets the gravity vector for a given world.
**/
DEFINE_PROPERTY( gravityGetter ) {

	ode::dWorldID worldID = (ode::dWorldID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( worldID );
	ode::dVector3 gravity;
	ode::dWorldGetGravity(worldID, gravity);
	//FloatVectorToArray(cx, 3, gravity, vp);
	JL_CHK( FloatVectorToJsval(cx, gravity, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( gravitySetter ) {

	ode::dWorldID worldID = (ode::dWorldID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( worldID );
	ode::dVector3 gravity;
	//FloatArrayToVector(cx, 3, vp, gravity);
	size_t length;
	JL_CHK( JsvalToFloatVector(cx, *vp, gravity, 3, &length) );
	JL_S_ASSERT( length == 3, "Invalid array size." );
	ode::dWorldSetGravity( worldID, gravity[0], gravity[1], gravity[2] );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL *ERP*
  dWorldGetERP

 * $REAL *CFM*
  dWorldGetCFM

 * $REAL *contactSurfaceLayer*
  dWorldGetContactSurfaceLayer
**/

enum { ERP, CFM, /*quickStepNumIterations,*/ contactSurfaceLayer };

DEFINE_PROPERTY( realSetter ) {

	ode::dWorldID worldID = (ode::dWorldID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( worldID );
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
	JL_BAD;
}


DEFINE_PROPERTY( realGetter ) {

	ode::dWorldID worldID = (ode::dWorldID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( worldID );
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
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE Body $INAME $READONLY
  Returns the environment object that is the fixed body of this world (like the ground).
**/
DEFINE_PROPERTY( env ) {

	if ( JSVAL_IS_VOID( *vp ) ) { //  create it if it does not exist and store it (cf. PROPERTY_READ_STORE)

		JSObject *staticBody = JS_NewObject(cx, classBody, NULL, NULL);
		JL_CHK(staticBody);
		JL_SetPrivate(cx, staticBody, (ode::dBodyID)0);
		*vp = OBJECT_TO_JSVAL(staticBody);
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 *defaultSurfaceParameters* $READONLY
  This defines the default properties of the colliding grometries surfaces.
  $LF
  The property is read-only but not i'ts content.

 * *space* $READONLY
  This is the default space object that is bound to the world.
**/

CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
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
