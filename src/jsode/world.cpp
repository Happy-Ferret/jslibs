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

// see. dxWorld, dxBody, dxJointNode, dxJoint, ...
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

void nearCallback(void *data, ode::dGeomID geom1, ode::dGeomID geom2) {
	
	// Doc: http://opende.sourceforge.net/wiki/index.php/Manual_%28Joint_Types_and_Functions%29
	ode::dContact contact;
	int n = ode::dCollide(geom1, geom2, 1, &contact.geom, sizeof(ode::dContact));
	if ( n == 0 )
		return;

/*
		// tentative of fusion two differents surface parameters:
		//   http://www.google.com/codesearch/p?hl=en#OdJU363NVS8/plugins/scmsvn/viewcvs.php/Feedback/Sources/PsOde/PsOdeWorld.cc%3Frev%3D3&amp;root%3Dopenmask&amp;view%3Dmarkup-001&q=dSurfaceParameters&l=51
		// doc
		//   http://opende.sourceforge.net/wiki/index.php/Manual_(Joint_Types_and_Functions)#Contact

		jsval obj1surf, obj2surf;
		JL_CHK( JL_GetReservedSlot(cx, obj1, SLOT_GEOM_SURFACEPARAMETER, &obj1surf) );
		if ( JL_IsClass(obj1surf, classSurfaceParameters) ) {
			
			ode::dSurfaceParameters *surf = (ode::dSurfaceParameters*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(obj1surf));
			JL_ASSERT_OBJECT_STATE( surf, classSurfaceParameters->name );
			contact[i].surface = *surf;
		} else {

			contact[i].surface = *ccp->defaultSurfaceParameters;
		}

		JL_CHK( JL_GetReservedSlot(cx, obj2, SLOT_GEOM_SURFACEPARAMETER, &obj2surf) );
		if ( JL_IsClass(obj2surf, classSurfaceParameters) ) {

			ode::dSurfaceParameters *surf = (ode::dSurfaceParameters*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(obj1surf));
			JL_ASSERT_OBJECT_STATE( surf, classSurfaceParametres->name );

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

	bool doContact = true;

	ColideContextPrivate *ccp = (ColideContextPrivate*)data; // beware: *data is local to Colide function

	JSContext *cx = ccp->cx;

	ode::dBodyID body1 = dGeomGetBody(geom1);
	ode::dBodyID body2 = dGeomGetBody(geom2);

	bool geom1HasObj, geom2HasObj;
	geom1HasObj = GeomHasJsObj(geom1);
	geom2HasObj = GeomHasJsObj(geom2);

	if ( geom1HasObj || geom2HasObj ) {

		jsval valGeom1, valGeom2;
		JL_CHK( GeomToJsval(cx, geom1, &valGeom1) );
		JL_CHK( GeomToJsval(cx, geom2, &valGeom2) );

		jsval func1, func2;
		if ( geom1HasObj )
			JL_CHK( JL_GetReservedSlot(cx, JSVAL_TO_OBJECT(valGeom1), SLOT_GEOM_CONTACT_FUNCTION, &func1) );
		else
			func1 = JSVAL_VOID;

		if ( geom2HasObj )
			JL_CHK( JL_GetReservedSlot(cx, JSVAL_TO_OBJECT(valGeom2), SLOT_GEOM_CONTACT_FUNCTION, &func2) );
		else
			func2 = JSVAL_VOID;

		if ( !JSVAL_IS_VOID( func1 ) || !JSVAL_IS_VOID( func2 ) ) {

//			int arity1 = JS_GetFunctionArity(JS_ValueToFunction(cx, func1));
//			int arity2 = JS_GetFunctionArity(JS_ValueToFunction(cx, func2));
//			if ( arity1 >= 3 || arity2 >= 3 ) { // only compute the following if needed. aka the function will use it. UGH! miss arguments variable

			ode::dVector3 *pos = &contact.geom.pos;

			ode::dReal px = (*pos)[0];
			ode::dReal py = (*pos)[1];
			ode::dReal pz = (*pos)[2];

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
			ode::dReal contactVelocity = Vector3Dot(&vel, &normal);


			jsval argv[9];
			argv[0] = JSVAL_NULL; // rval

			// geom.contact = function(thisGeom, otherGeom, contactVelocity, contactX, contactY, contactZ, side1, side2) { }

			JL_CHK(JL_NativeToJsval(cx, contactVelocity, &argv[3]) ); // JL_CHK(JL_NativeToJsval(cx, contact.geom.depth, &argv[3]) );
			JL_CHK(JL_NativeToJsval(cx, contact.geom.pos[0], &argv[4]) );
			JL_CHK(JL_NativeToJsval(cx, contact.geom.pos[1], &argv[5]) );
			JL_CHK(JL_NativeToJsval(cx, contact.geom.pos[2], &argv[6]) );

			if ( !JSVAL_IS_VOID( func1 ) ) {

				argv[1] = valGeom1;
				argv[2] = valGeom2;
				//...
				argv[7] = INT_TO_JSVAL( contact.geom.side1 ); // TriIndex
				argv[8] = INT_TO_JSVAL( contact.geom.side2 );
				JL_CHK( JS_CallFunctionValue(cx, JSVAL_TO_OBJECT(valGeom1), func1, COUNTOF(argv)-1, argv+1, argv) );
				if ( *argv == JSVAL_FALSE )
					doContact = false;
			}

			if ( !JSVAL_IS_VOID( func2 ) ) {

				argv[1] = valGeom2;
				argv[2] = valGeom1;
				//...
				argv[7] = INT_TO_JSVAL( contact.geom.side2 ); // TriIndex
				argv[8] = INT_TO_JSVAL( contact.geom.side1 );
				JL_CHK( JS_CallFunctionValue(cx, JSVAL_TO_OBJECT(valGeom2), func2, COUNTOF(argv)-1, argv+1, argv) );
				if ( *argv == JSVAL_FALSE )
					doContact = false;
			}
		}
	}

	if ( doContact ) {

		contact.surface = *ccp->defaultSurfaceParameters;
		if ( ccp->defaultSurfaceParameters->mode & ode::dContactFDir1 ) {

			// doc. fdir1 — Returns the "first friction direction" vector that defines a direction along which frictional 
			//      force is applied if the contact’s surfaceuseFrictionDirection? flag is true.
			//      If useFrictionDirection? is false, this setting is unused, though it can still be set.
			contact.fdir1[0] = 0;
			contact.fdir1[1] = 0;
			contact.fdir1[2] = 0;
		}

		ode::dJointID contactJoint = ode::dJointCreateContact(ccp->worldId, ccp->contactGroupId, &contact);

		//ode::dJointFeedback *fb = (ode::dJointFeedback*)jl_calloc(1, sizeof(ode::dJointFeedback));
		//JL_CHK(fb);
		//ode::dJointSetFeedback(contactJoint, fb);
		//fb destruction: (TBD)

		ode::dJointAttach(contactJoint, body1, body2);
	}

	return;
bad:
	return;
}

/**doc fileIndex:top
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
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
			JL_SetPrivate(cx, bodyObj, NULL);
			// ode::dBodySetData(bodyIt, NULL);

		// same rule for geoms
		for ( ode::dxGeom *geomIt = bodyIt->geom; geomIt; geomIt = ode::dGeomGetBodyNext(geomIt) ) {

			JSObject *geomObj = (JSObject*)ode::dGeomGetData(geomIt);
			if ( geomObj != NULL )
				JL_SetPrivate(cx, geomObj, NULL);
			// ode::dGeomSetData(geomIt, NULL);
	  }

		// same rule for joints
		for ( ode::dxJointNode *jointIt = bodyIt->firstjoint; jointIt; jointIt = jointIt->next ) {
		
			JSObject *jointObj = (JSObject*)ode::dJointGetData(jointIt->joint);
			if ( jointObj != NULL )
				JL_SetPrivate(cx, jointObj, NULL);
			// ode::dJointSetData(jointIt, NULL);
		}
	}

	// same rule for joints
	for ( ode::dxJoint *jointIt = world->firstjoint; jointIt; jointIt = (ode::dxJoint*)jointIt->next ) {
	
		JSObject *jointObj = (JSObject*)ode::dJointGetData(jointIt);
		if ( jointObj != NULL )
			JL_SetPrivate(cx, jointObj, NULL);
		// ode::dJointSetData(jointIt, NULL);
	}


	//	ode::dSpaceClean(pv->spaceId);
	while ( ode::dSpaceGetNumGeoms(pv->spaceId) ) {

		ode::dGeomID geomId = ode::dSpaceGetGeom(pv->spaceId, 0);

		JSObject *geomObj = (JSObject*)ode::dGeomGetData(geomId);
		if ( geomObj != NULL )
			JL_SetPrivate(cx, geomObj, NULL);
//		ode::dSpaceRemove(pv->spaceId, geomId);
		ode::dGeomDestroy(geomId);
	}
	ode::dSpaceDestroy(pv->spaceId);

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

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;


	WorldPrivate *pv = (WorldPrivate*)JS_malloc(cx, sizeof(WorldPrivate));
	JL_CHK( pv );
	JL_SetPrivate(cx, obj, pv);
	
//	pv->stepTmpCx = NULL;

	pv->worldId = ode::dWorldCreate();
	pv->contactGroupId = ode::dJointGroupCreate(0); // see nearCallback()

	JSObject *spaceObject = JS_ConstructObject(cx, JL_CLASS(Space), /*JL_PROTOTYPE(cx, Space),*/ NULL); // no arguments = create a topmost space object
	JL_CHK( spaceObject );
	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_WORLD_SPACE, OBJECT_TO_JSVAL(spaceObject)) );
	pv->spaceId = (ode::dSpaceID)JL_GetPrivate(cx, spaceObject);


	JSObject *surfaceParameters = JS_ConstructObject(cx, JL_CLASS(SurfaceParameters), /*JL_PROTOTYPE(cx, SurfaceParameters),*/ NULL);
	JL_CHK( surfaceParameters );
	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_WORLD_DEFAULTSURFACEPARAMETERS, OBJECT_TO_JSVAL(surfaceParameters)) );

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
DEFINE_FUNCTION( destroy ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ode::dJointGroupDestroy(pv->contactGroupId);
	ode::dWorldDestroy(pv->worldId);
	JL_SetPrivate(cx, obj, NULL);
	return JS_TRUE;
	JL_BAD;
}
*/



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [ spaceOrGeom [, spaceOrGeom2 ] ] )
**/
DEFINE_FUNCTION( collide ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_RANGE(0,2);
	JL_ASSERT_INSTANCE(obj, JL_CLASS(World));
	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*JL_RVAL = JSVAL_VOID;

	ode::dJointGroupEmpty(pv->contactGroupId); // contactGroupId will be reused at the next step!


	//jsval val;
	//JL_GetReservedSlot(cx, obj, WORLD_SLOT_SPACE, &val);
	//ode::dSpaceID space = (ode::dSpaceID)JSVAL_TO_PRIVATE(val);
	//ode::dSpaceCollide(space,0,&nearCallback);

	void *sg1Id, *sg2Id; // space or geom (see http://opende.sourceforge.net/wiki/index.php/Manual_%28Collision_Detection%29)

	if ( JL_ARG_ISDEF(2) ) {

		// doc. dSpaceCollide2 ... It can also test a single non-space geom against a space ...
		JL_ASSERT_ARG_IS_OBJECT(2);
		if ( JL_JsvalIsSpace(JL_ARG(2)) ) {

			JL_CHK( JL_JsvalToSpaceID(cx, JL_ARG(2), (ode::dSpaceID*)&sg2Id) );
		} else {

			JL_ASSERT_INSTANCE(JSVAL_TO_OBJECT(JL_ARG(2)), JL_CLASS(Geom));
			JL_CHK( JL_JsvalToGeom(cx, JL_ARG(2), (ode::dGeomID*)&sg2Id) );
		}
	} else {

		sg2Id = NULL;
	}


	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_OBJECT(1);
		if ( sg2Id == NULL ) {

			JL_ASSERT_INSTANCE(JSVAL_TO_OBJECT(JL_ARG(1)), JL_CLASS(Space));
			JL_CHK( JL_JsvalToSpaceID(cx, JL_ARG(1), (ode::dSpaceID*)&sg1Id) );
		} else {

			if ( JL_JsvalIsSpace(JL_ARG(1)) ) {

				JL_CHK( JL_JsvalToSpaceID(cx, JL_ARG(1), (ode::dSpaceID*)&sg1Id) );
			} else {

				JL_ASSERT_INSTANCE(JSVAL_TO_OBJECT(JL_ARG(1)), JL_CLASS(Geom));
				JL_CHK( JL_JsvalToGeom(cx, JL_ARG(1), (ode::dGeomID*)&sg1Id) );
			}
		}
	} else {

		jsval val;
		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_WORLD_SPACE, &val) );
		JL_CHK( JL_JsvalToSpaceID(cx, val, (ode::dSpaceID*)&sg1Id) );
	}

	jsval defaultSurfaceParametersVal;
	JL_GetReservedSlot(cx, obj, SLOT_WORLD_DEFAULTSURFACEPARAMETERS, &defaultSurfaceParametersVal);
	//	JL_ASSERT_INSTANCE( JSVAL_TO_OBJECT(defaultSurfaceParametersObject), JL_CLASS(SurfaceParameters) ); // (TBD) simplify RT_ASSERT
	ode::dSurfaceParameters *defaultSurfaceParameters = (ode::dSurfaceParameters*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(defaultSurfaceParametersVal)); // beware: local variable !
	JL_ASSERT_OBJECT_STATE( defaultSurfaceParameters, JL_CLASS_NAME(SurfaceParameters) );

	ColideContextPrivate ccp;
	ccp.cx = cx; // the context will only be used while the worls step.
	ccp.defaultSurfaceParameters = defaultSurfaceParameters;
	ccp.contactGroupId = pv->contactGroupId;
	ccp.worldId = pv->worldId;

	if ( sg2Id == NULL )
		ode::dSpaceCollide((ode::dSpaceID)sg1Id, (void*)&ccp, &nearCallback);
	else
		ode::dSpaceCollide2((ode::dGeomID)sg1Id, (ode::dGeomID)sg2Id, (void*)&ccp, &nearCallback);

	return !JL_IsExceptionPending(cx); // an exception may have been thrown in nearCallback
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( stepsize )
  $H arguments
   $ARG real stepsize: The number of milliseconds that the simulation has to advance.
**/
DEFINE_FUNCTION( step ) {

	JL_DEFINE_FUNCTION_OBJ;

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_INSTANCE(JL_OBJ, JL_CLASS(World));
	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ode::dReal stepSize;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &stepSize) );
	if ( ode::dWorldGetQuickStepNumIterations(pv->worldId) == 0 )
		ode::dWorldStep(pv->worldId, stepSize / 1000.f);
	else
		ode::dWorldQuickStep(pv->worldId, stepSize / 1000.f);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( $TYPE vec3 force, stepSize )
  _stepSize_ is the step size for the next step that will be taken. 

**/
DEFINE_FUNCTION( scaleImpulse ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_ASSERT_ARGC_MIN(1);
	ode::dVector3 force;
	uint32_t len;
	JL_CHK( JsvalToODERealVector(cx, JL_ARG(1), force, COUNTOF(force), &len) );
	JL_ASSERT( len >= 3, E_ARG, E_NUM(1), E_TYPE, E_TY_NVECTOR(3) );

	float stepSize;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &stepSize) );
	ode::dWorldImpulseToForce(pv->worldId, stepSize / 1000, force[0], force[1], force[2], force);
	
	JSObject *objArr = JSVAL_TO_OBJECT(JL_ARG(1));
	for ( jsint i = 0; i < 3; i++ ) {

		JL_CHK( JL_NativeToJsval(cx, force[i], JL_RVAL) );
		JL_CHK( JL_SetElement(cx, objArr, i, JL_RVAL) );
	}

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/



/**doc
$TOC_MEMBER $INAME
 $TYPE real $INAME
  (TBD)
**/
DEFINE_PROPERTY_SETTER( autoDisableLinearThreshold ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ode::dReal threshold;
	JL_CHK( JsvalToODEReal(cx, *vp, &threshold) );
	ode::dWorldSetAutoDisableLinearThreshold(pv->worldId, threshold);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( autoDisableLinearThreshold ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ode::dReal threshold;
	threshold = ode::dWorldGetAutoDisableLinearThreshold(pv->worldId);
	JL_CHK( ODERealToJsval(cx, threshold, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE real $INAME
  (TBD)
**/
DEFINE_PROPERTY_SETTER( autoDisableAngularThreshold ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ode::dReal threshold;
	JL_CHK( JsvalToODEReal(cx, *vp, &threshold) );
	ode::dWorldSetAutoDisableAngularThreshold(pv->worldId, threshold);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( autoDisableAngularThreshold ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	ode::dReal threshold;
	threshold = ode::dWorldGetAutoDisableAngularThreshold(pv->worldId);
	JL_CHK( ODERealToJsval(cx, threshold, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
  Gets or sets the gravity vector for a given world.
**/
DEFINE_PROPERTY_GETTER( gravity ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ode::dVector3 gravity;
	ode::dWorldGetGravity(pv->worldId, gravity);
	//ODERealVectorToArray(cx, 3, gravity, vp);
	JL_CHK( ODERealVectorToJsval(cx, gravity, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( gravity ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ode::dVector3 gravity;
	//FloatArrayToVector(cx, 3, vp, gravity);
	uint32_t length;
	JL_CHK( JsvalToODERealVector(cx, *vp, gravity, 3, &length) );
	JL_ASSERT( length >= 3, E_VALUE, E_TYPE, E_TY_NVECTOR(3) );
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

DEFINE_PROPERTY_SETTER( real ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float value;
	JL_CHK( JL_JsvalToNative(cx, *vp, &value) );
	switch ( JSID_TO_INT(id) ) {
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


DEFINE_PROPERTY_GETTER( real ) {

	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float value;
	switch ( JSID_TO_INT(id) ) {
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
	JL_CHK(JL_NativeToJsval(cx, value, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Body $INAME $READONLY
  Returns the default surface parameters of the world.
**/
DEFINE_PROPERTY_GETTER( defaultSurfaceParameters ) {

	return JL_GetReservedSlot(cx, obj, SLOT_WORLD_DEFAULTSURFACEPARAMETERS, vp);
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Body $INAME $READONLY
  Returns the default space of the world.
**/
DEFINE_PROPERTY_GETTER( space ) {

	return JL_GetReservedSlot(cx, obj, SLOT_WORLD_SPACE, vp);
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Body $INAME $READONLY
  Returns the environment object that is the fixed body of this world (like the ground).
**/
DEFINE_PROPERTY_GETTER( env ) {

	JL_ASSERT_THIS_INSTANCE();
	JSObject *staticBody = JL_NewObjectWithGivenProto(cx, JL_CLASS(Body), JL_PROTOTYPE(cx, Body), NULL);
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

	REVISION(JL_SvnRevToInt("$Revision: 3533 $"))

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( collide, 2 ) // not FAST because colide is called from here
		FUNCTION_ARGC( step, 1 )
		FUNCTION_ARGC( scaleImpulse, 2 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( autoDisableLinearThreshold )
		PROPERTY( autoDisableAngularThreshold )
		PROPERTY( gravity )
		PROPERTY_GETTER( defaultSurfaceParameters )
		PROPERTY_GETTER( space )
		PROPERTY_GETTER( env )
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

END_CLASS
