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

namespace ode {
	#include <../ode/src/objects.h>
	#include <../ode/src/joints/joint.h>
}

#include "world.h"
#include "space.h"
#include "body.h"
#include "geom.h"


struct ColideContextPrivate {
	JSContext *cx;
	ode::dSurfaceParameters *defaultSurfaceParameters;
	ode::dJointGroupID contactGroupId;
	ode::dWorldID worldId;
};

static void nearCallback(void *data, ode::dGeomID geom1, ode::dGeomID geom2) {
	
	// Doc: http://opende.sourceforge.net/wiki/index.php/Manual_%28Joint_Types_and_Functions%29

	ColideContextPrivate *ccp = (ColideContextPrivate*)data; // beware: *data is local to Step function

	ode::dContact contact;
	int n = ode::dCollide(geom1, geom2, 1, &contact.geom, sizeof(ode::dContact));
	if ( n <= 0 )
		return;

	JSContext *cx = ccp->cx;

	ode::dBodyID body1 = dGeomGetBody(geom1);
	ode::dBodyID body2 = dGeomGetBody(geom2);

	jsval valGeom1, valGeom2;
	JL_CHK( GeomToJsval(cx, geom1, &valGeom1) );
	JL_CHK( GeomToJsval(cx, geom2, &valGeom2) );

		
/*
		// tentative of fusion two differents surface parameters:
		//   http://www.google.com/codesearch/p?hl=en#OdJU363NVS8/plugins/scmsvn/viewcvs.php/Feedback/Sources/PsOde/PsOdeWorld.cc%3Frev%3D3&amp;root%3Dopenmask&amp;view%3Dmarkup-001&q=dSurfaceParameters&l=51
		// doc
		//   http://opende.sourceforge.net/wiki/index.php/Manual_(Joint_Types_and_Functions)#Contact

		jsval obj1surf, obj2surf;
		JL_CHK( JL_GetReservedSlot(cx, obj1, SLOT_GEOM_SURFACEPARAMETER, &obj1surf) );
		if ( JsvalIsClass(obj1surf, classSurfaceParameters) ) {
			
			ode::dSurfaceParameters *surf = (ode::dSurfaceParameters*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(obj1surf));
			JL_S_ASSERT_RESOURCE( surf );
			contact[i].surface = *surf;
		} else {

			contact[i].surface = *ccp->defaultSurfaceParameters;
		}

		JL_CHK( JL_GetReservedSlot(cx, obj2, SLOT_GEOM_SURFACEPARAMETER, &obj2surf) );
		if ( JsvalIsClass(obj2surf, classSurfaceParameters) ) {

			ode::dSurfaceParameters *surf = (ode::dSurfaceParameters*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(obj1surf));
			JL_S_ASSERT_RESOURCE( surf );

		}
*/

	// mixing :
	//dReal mu;		// min
	//dReal mu2;	// min
	//dReal bounce;	// average
	//dReal bounce_vel; // min
	//dReal soft_erp; // average
	//dReal soft_cfm; // average
	//dReal motion1,motion2;	// add
	//dReal slip1,slip2;	// ?

	jsval func1, func2;
	JL_CHK( JL_GetReservedSlot(cx, JSVAL_TO_OBJECT(valGeom1), SLOT_GEOM_IMPACT_FUNCTION, &func1) );
	JL_CHK( JL_GetReservedSlot(cx, JSVAL_TO_OBJECT(valGeom2), SLOT_GEOM_IMPACT_FUNCTION, &func2) );

	if ( !JSVAL_IS_VOID( func1 ) || !JSVAL_IS_VOID( func2 ) ) {

		ode::dVector3 *pos = &contact.geom.pos;

		float px = (*pos)[0];
		float py = (*pos)[1];
		float pz = (*pos)[2];

		Vector3 vel, tmp, normal;

		if ( body2 != NULL )
			ode::dBodyGetPointVel(body2, px, py, pz, vel.raw);
		else
			Vector3Identity(&vel);
		if ( body1 != NULL )
			ode::dBodyGetPointVel(body1, px, py, pz, tmp.raw);
		else
			Vector3Identity(&tmp);
		Vector3SubVector3(&vel, &vel, &tmp);
		Vector3LoadPtr(&normal, contact.geom.normal);
		float impactVelocity = Vector3Dot(&vel, &normal);

		JSTempValueRooter tvr;
		jsval argv[9];
		JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr);

		argv[0] = JSVAL_NULL; // rval
//		JL_CHKB( FloatToJsval(cx, contact.geom.depth, &argv[3]), bad_poproot );
		JL_CHKB( FloatToJsval(cx, impactVelocity, &argv[3]), bad_poproot );
		JL_CHKB( FloatToJsval(cx, contact.geom.pos[0], &argv[4]), bad_poproot );
		JL_CHKB( FloatToJsval(cx, contact.geom.pos[1], &argv[5]), bad_poproot );
		JL_CHKB( FloatToJsval(cx, contact.geom.pos[2], &argv[6]), bad_poproot );

		//contact.geom.side1; // TriIndex
		//contact.geom.side2; // TriIndex

		if ( !JSVAL_IS_VOID( func1 ) ) {

			argv[1] = valGeom1;
			argv[2] = valGeom2;
			argv[7] = INT_TO_JSVAL( contact.geom.side1 );
			argv[8] = INT_TO_JSVAL( contact.geom.side2 );
			JL_CHKB( JS_CallFunctionValue(cx, JSVAL_TO_OBJECT(valGeom1), func1, COUNTOF(argv)-1, argv+1, argv), bad_poproot );
		}

		if ( !JSVAL_IS_VOID( func2 ) ) {

			argv[1] = valGeom2;
			argv[2] = valGeom1;
			argv[7] = INT_TO_JSVAL( contact.geom.side2 );
			argv[8] = INT_TO_JSVAL( contact.geom.side1 );
			JL_CHKB( JS_CallFunctionValue(cx, JSVAL_TO_OBJECT(valGeom2), func2, COUNTOF(argv)-1, argv+1, argv), bad_poproot );
		}
	
	bad_poproot:
		JS_POP_TEMP_ROOT(cx, &tvr);
	}

	contact.surface = *ccp->defaultSurfaceParameters;
	// doc. fdir1 — Returns the "first friction direction" vector that defines a direction along which frictional 
	//      force is applied if the contact’s surfaceuseFrictionDirection? flag is true.
	//      If useFrictionDirection? is false, this setting is unused, though it can still be set.
	if ( ccp->defaultSurfaceParameters->mode & ode::dContactFDir1 ) {

		contact.fdir1[0] = 0;
		contact.fdir1[1] = 0;
		contact.fdir1[2] = 0;
	}

	ode::dJointID contactJoint = ode::dJointCreateContact(ccp->worldId, ccp->contactGroupId, &contact);
	ode::dJointAttach(contactJoint, body1, body2);

	return;
bad:
	return;
}

/**doc fileIndex:top
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( World )


DEFINE_FINALIZE() {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	if ( !pv )
		return;

	ode::dJointGroupDestroy(pv->contactGroupId);

	// <soubok> If an object A has a reference to an object B, should I expect B finalizer to be called after A finalizer ?  <shaver> no
	// world may be destroy before bodies, then we have to
	// destroy the javascript part of bodies before before bodies finalizers being called

	ode::dxWorld *world = (ode::dxWorld*)pv->worldId;
	for ( ode::dxBody *bodyIt = world->firstbody; bodyIt; bodyIt = (ode::dxBody*)bodyIt->next ) {
	
		JSObject *bodyObj = (JSObject*)ode::dBodyGetData(bodyIt);
		if ( bodyObj != NULL )
			JS_SetPrivate(cx, bodyObj, NULL);
			// ode::dBodySetData(bodyIt, NULL);

		// same rule for geoms
		for ( ode::dxGeom *geomIt = bodyIt->geom; geomIt; geomIt = ode::dGeomGetBodyNext(geomIt) ) {

			JSObject *geomObj = (JSObject*)ode::dGeomGetData(geomIt);
			if ( geomObj != NULL )
				JS_SetPrivate(cx, geomObj, NULL);
			// ode::dGeomSetData(geomIt, NULL);
	  }

		// same rule for joints
		for ( ode::dxJointNode *jointIt = bodyIt->firstjoint; jointIt; jointIt = jointIt->next ) {
		
			JSObject *jointObj = (JSObject*)ode::dJointGetData(jointIt->joint);
			if ( jointObj != NULL )
				JS_SetPrivate(cx, jointObj, NULL);
			// ode::dJointSetData(jointIt, NULL);
		}
	}

	// same rule for joints
	for ( ode::dxJoint *jointIt = world->firstjoint; jointIt; jointIt = (ode::dxJoint*)jointIt->next ) {
	
		JSObject *jointObj = (JSObject*)ode::dJointGetData(jointIt);
		if ( jointObj != NULL )
			JS_SetPrivate(cx, jointObj, NULL);
		// ode::dJointSetData(jointIt, NULL);
	}

	ode::dWorldDestroy(pv->worldId);
	JS_free(cx, pv);
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

	WorldPrivate *pv = (WorldPrivate*)JS_malloc(cx, sizeof(WorldPrivate));
	JL_CHK( pv );
	JL_SetPrivate(cx, obj, pv);

	pv->worldId = ode::dWorldCreate();
	pv->contactGroupId = ode::dJointGroupCreate(0); // see nearCallback()

	JSObject *spaceObject = JS_ConstructObject(cx, JL_CLASS(Space), NULL, NULL); // no arguments = create a topmost space object
	JL_S_ASSERT( spaceObject != NULL, "Unable to construct Space for the World." );
	JL_CHK( JS_DefineProperty(cx, obj, WORLD_SPACE_PROPERTY_NAME, OBJECT_TO_JSVAL(spaceObject), NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY) );

	JSObject *surfaceParameters = JS_ConstructObject(cx, JL_CLASS(SurfaceParameters), NULL, NULL);
	JL_S_ASSERT( surfaceParameters != NULL, "Unable to construct classSurfaceParameters." );
	JL_CHK( JS_DefineProperty(cx, obj, DEFAULT_SURFACE_PARAMETERS_PROPERTY_NAME, OBJECT_TO_JSVAL(surfaceParameters), NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/** doc
$TOC_MEMBER $INAME
 $INAME()
  TBD
**/
/* ISSUE: all underlying objects have to be notified of this operation else on their finalizers,
          they will try to reset ode object private data that has already been freed !!
DEFINE_FUNCTION( Destroy ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	ode::dJointGroupDestroy(pv->contactGroupId);
	ode::dWorldDestroy(pv->worldId);
	JL_SetPrivate(cx, obj, NULL);
	return JS_TRUE;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( stepsize )
  $H arguments
   $ARG real stepsize: The number of milliseconds that the simulation has to advance.
**/
DEFINE_FUNCTION( Step ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_CLASS(obj, JL_CLASS(World));
	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	float stepSize;
	JL_CHK( JsvalToFloat(cx, JL_ARG(1), &stepSize) );

	//jsval val;
	//JL_GetReservedSlot(cx, obj, WORLD_SLOT_SPACE, &val);
	//ode::dSpaceID space = (ode::dSpaceID)JSVAL_TO_PRIVATE(val);
	//ode::dSpaceCollide(space,0,&nearCallback);

	ode::dSpaceID spaceId;
	jsval val;
	JS_GetProperty(cx, obj, WORLD_SPACE_PROPERTY_NAME, &val);
	JL_S_ASSERT_DEFINED( val );
	JL_CHK( JsvalToSpaceID(cx, val, &spaceId) );

	jsval defaultSurfaceParametersObject;
	JS_GetProperty(cx, obj, DEFAULT_SURFACE_PARAMETERS_PROPERTY_NAME, &defaultSurfaceParametersObject);
	JL_S_ASSERT_OBJECT( defaultSurfaceParametersObject );
	JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT(defaultSurfaceParametersObject), JL_CLASS(SurfaceParameters) ); // (TBD) simplify RT_ASSERT
	ode::dSurfaceParameters *defaultSurfaceParameters = (ode::dSurfaceParameters*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(defaultSurfaceParametersObject)); // beware: local variable !
	JL_S_ASSERT_RESOURCE( defaultSurfaceParameters );

//	JL_GetReservedSlot(cx, obj, WORLD_SLOT_CONTACTGROUP, &val);
//	ode::dJointGroupID contactgroup = (ode::dJointGroupID)JSVAL_TO_PRIVATE(val);

	ColideContextPrivate ccp;
	ccp.cx = cx; // the context will only be used while the worls step.
	ccp.defaultSurfaceParameters = defaultSurfaceParameters;
	ccp.contactGroupId = pv->contactGroupId;
	ccp.worldId = pv->worldId;

	ode::dSpaceCollide(spaceId, (void*)&ccp, &nearCallback);

//	pv->stepJsCx = cx;
	if ( ode::dWorldGetQuickStepNumIterations(pv->worldId) == 0 )
		ode::dWorldStep(pv->worldId, stepSize / 1000);
	else
		ode::dWorldQuickStep(pv->worldId, stepSize / 1000);
//	pv->stepJsCx = NULL; // avoid using the JSContext outside the step

	ode::dJointGroupEmpty(pv->contactGroupId); // contactGroupId will be reused at the next step!

	return !JS_IsExceptionPending(cx); // an exception may have been thrown in nearCallback
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME( $TYPE vec3 force, stepSize )
  _stepSize_ is the step size for the next step that will be taken. 

**/
DEFINE_FUNCTION_FAST( ScaleImpulse ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );
	JL_S_ASSERT_ARG_MIN(1);
	ode::dVector3 force;
	uint32 len;
	JL_CHK( JsvalToFloatVector(cx, JL_FARG(1), force, COUNTOF(force), &len) );
	JL_S_ASSERT( len >= 3, "Invalid array size." );

	float stepSize;
	JL_CHK( JsvalToFloat(cx, JL_FARG(2), &stepSize) );
	ode::dWorldImpulseToForce(pv->worldId, stepSize / 1000, force[0], force[1], force[2], force);
	
	JSObject *objArr = JSVAL_TO_OBJECT(JL_FARG(1));
	for ( size_t i = 0; i < COUNTOF(force); i++ ) {

		JL_CHK( FloatToJsval(cx, force[i], JL_FRVAL) );
		JL_CHK( JS_SetElement(cx, objArr, i, JL_FRVAL) );
	}

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

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	ode::dVector3 gravity;
	ode::dWorldGetGravity(pv->worldId, gravity);
	//FloatVectorToArray(cx, 3, gravity, vp);
	JL_CHK( FloatVectorToJsval(cx, gravity, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( gravitySetter ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	ode::dVector3 gravity;
	//FloatArrayToVector(cx, 3, vp, gravity);
	uint32 length;
	JL_CHK( JsvalToFloatVector(cx, *vp, gravity, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	ode::dWorldSetGravity( pv->worldId, gravity[0], gravity[1], gravity[2] );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER ERP
 $REAL *ERP*
  dWorldGetERP

$TOC_MEMBER CFM
 $REAL *CFM*
  dWorldGetCFM

$TOC_MEMBER contactSurfaceLayer
 $REAL *contactSurfaceLayer*
  dWorldGetContactSurfaceLayer

$TOC_MEMBER quickStepNumIterations
  $REAL *quickStepNumIterations*
   If greater than 0, step will uses an iterative method that takes time on the order of m*N and memory on the order of m, where m is the total number of constraint rows and N is the number of iterations.
**/

enum { ERP, CFM, quickStepNumIterations, quickStepW, contactSurfaceLayer, contactMaxCorrectingVel, linearDamping, linearDampingThreshold, angularDamping, angularDampingThreshold, maxAngularSpeed };

DEFINE_PROPERTY( realSetter ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	float value;
	JL_CHK( JsvalToFloat(cx, *vp, &value) );
	switch ( JSVAL_TO_INT(id) ) {
		case ERP:
			ode::dWorldSetERP(pv->worldId, value);
			break;
		case CFM:
			ode::dWorldSetCFM(pv->worldId, value);
			break;
		case quickStepNumIterations:
			ode::dWorldSetQuickStepNumIterations(pv->worldId, (int)value);
			break;
		case quickStepW:
			ode::dWorldSetQuickStepW(pv->worldId, value);
			break;
		case contactSurfaceLayer:
			ode::dWorldSetContactSurfaceLayer(pv->worldId, value);
			break;
		case contactMaxCorrectingVel:
			ode::dWorldSetContactMaxCorrectingVel(pv->worldId, value);
			break;
		case linearDamping:
			dWorldSetLinearDamping(pv->worldId, value);
			break;
		case linearDampingThreshold:
			ode::dWorldSetLinearDampingThreshold(pv->worldId, value);
			break;
		case angularDamping:
			dWorldSetAngularDamping(pv->worldId, value);
			break;
		case angularDampingThreshold:
			dWorldSetAngularDampingThreshold(pv->worldId, value);
			break;
		case maxAngularSpeed:
			ode::dWorldSetMaxAngularSpeed(pv->worldId, value);
			break;
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( realGetter ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	float value;
	switch ( JSVAL_TO_INT(id) ) {
		case ERP:
			value = ode::dWorldGetERP(pv->worldId);
			break;
		case CFM:
			value = ode::dWorldGetCFM(pv->worldId);
			break;
		case quickStepNumIterations:
			value = (float)ode::dWorldGetQuickStepNumIterations(pv->worldId);
			break;
		case quickStepW:
			value = ode::dWorldGetQuickStepW(pv->worldId);
			break;
		case contactSurfaceLayer:
			value = ode::dWorldGetContactSurfaceLayer(pv->worldId);
			break;
		case contactMaxCorrectingVel:
			value = ode::dWorldGetContactMaxCorrectingVel(pv->worldId);
			break;
		case linearDamping:
			value = ode::dWorldGetLinearDamping(pv->worldId);
			break;
		case linearDampingThreshold:
			value = ode::dWorldGetLinearDampingThreshold(pv->worldId);
			break;
		case angularDamping:
			value = ode::dWorldGetAngularDamping(pv->worldId);
			break;
		case angularDampingThreshold:
			value = ode::dWorldGetAngularDampingThreshold(pv->worldId);
			break;
		case maxAngularSpeed:
			value = ode::dWorldGetMaxAngularSpeed(pv->worldId);
			break;
	}
	JL_CHK( FloatToJsval(cx, value, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE Body $INAME $READONLY
  Returns the environment object that is the fixed body of this world (like the ground).
**/
DEFINE_PROPERTY( env ) {

	JSObject *staticBody = JS_NewObject(cx, JL_CLASS(Body), NULL, NULL);
	JL_CHK(staticBody);
	JL_SetPrivate(cx, staticBody, (ode::dBodyID)0);
	*vp = OBJECT_TO_JSVAL(staticBody);
	return JL_StoreProperty(cx, obj, id, vp, true);
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

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( Step, 1 ) // not FAST because colide is called from here
		FUNCTION_FAST_ARGC( ScaleImpulse, 2 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( gravity )
		PROPERTY_READ( env )
		PROPERTY_SWITCH( ERP, real )
		PROPERTY_SWITCH( CFM, real )
		PROPERTY_SWITCH( quickStepNumIterations, real )
		PROPERTY_SWITCH( contactSurfaceLayer, real )
		PROPERTY_SWITCH( quickStepW, real )
		PROPERTY_SWITCH( contactMaxCorrectingVel, real )
		PROPERTY_SWITCH( linearDamping, real )
		PROPERTY_SWITCH( linearDampingThreshold, real )
		PROPERTY_SWITCH( angularDamping, real )
		PROPERTY_SWITCH( angularDampingThreshold, real )
		PROPERTY_SWITCH( maxAngularSpeed, real )
	END_PROPERTY_SPEC

	HAS_PRIVATE
END_CLASS
