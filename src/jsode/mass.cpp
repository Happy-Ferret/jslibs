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
#include "body.h"
#include "mass.h"



// (TBD) Mass object seems to be useless. Try to merge Body with Mass
/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Mass )

JSBool GetBodyAndMass(JSContext *cx, JSObject *massObject, ode::dBodyID *pBodyID, ode::dMass *pMass) {

	jsval bodyVal;
	JL_CHK( JL_GetReservedSlot(cx, massObject, MASS_SLOT_BODY, &bodyVal) );
	JL_S_ASSERT_THIS_OBJECT_STATE( JSVAL_IS_OBJECT(bodyVal) );
	JSObject *bodyObject;
	bodyObject = JSVAL_TO_OBJECT(bodyVal);
	JL_S_ASSERT_CLASS(bodyObject, JL_CLASS(Body));
	*pBodyID = (ode::dBodyID)JL_GetPrivate(cx, bodyObject);
	ode::dBodyGetMass(*pBodyID, pMass);
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( vec3 )
  TBD dMassTranslate + dBodySetMass
**/

DEFINE_FUNCTION( Translate ) {

	JL_DEFINE_FUNCTION_OBJ;

	ode::dBodyID bodyID;
	ode::dMass mass;
	JL_S_ASSERT_ARG_MIN(1);
	JL_CHK( GetBodyAndMass(cx, JL_OBJ, &bodyID, &mass) );
	real translation[3];
//	JL_CHK( FloatArrayToVector(cx, 3, &argv[0], translation) );
	uint32 length;
	JL_CHK( JL_JsvalToODERealVector(cx, JL_ARG(1), translation, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	ode::dMassTranslate(&mass, translation[0], translation[1], translation[2]);
	ode::dBodySetMass(bodyID, &mass);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mass )
  TBD dBodyGetMass, dMassAdjust, dBodySetMass
**/
DEFINE_FUNCTION( Adjust ) {

	JL_DEFINE_FUNCTION_OBJ;

	ode::dBodyID bodyID;
	ode::dMass mass;
	JL_S_ASSERT_ARG_MIN(1);
	JL_CHK( GetBodyAndMass(cx, JL_OBJ, &bodyID, &mass) );
	jsdouble newMass;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &newMass) );
	ode::dMassAdjust(&mass, (ode::dReal)newMass);
	ode::dBodySetMass(bodyID, &mass);
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  TBD dBodyGetMass, dMassSetZero, dBodySetMass
**/
DEFINE_FUNCTION( SetZero ) {

	JL_DEFINE_FUNCTION_OBJ;

	ode::dBodyID bodyID;
	ode::dMass mass;
	JL_CHK( GetBodyAndMass(cx, JL_OBJ, &bodyID, &mass) );
	ode::dMassSetZero(&mass);
	ode::dBodySetMass(bodyID, &mass);
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mass, vec3 )
  TBD dBodyGetMass, dMassSetBoxTotal, dBodySetMass
**/
DEFINE_FUNCTION( SetBoxTotal ) {

	JL_DEFINE_FUNCTION_OBJ;

	ode::dMass mass;
	JL_S_ASSERT_ARG_MIN(2);
// get mass object
	ode::dBodyID bodyID;
	JL_CHK( GetBodyAndMass(cx, JL_OBJ, &bodyID, &mass) );
// arg 0
	jsdouble totalMass;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &totalMass) );
// arg 1
	real dimensions[3];
	//	JL_CHK( FloatArrayToVector(cx, 3, &argv[1], dimensions) );
	uint32 length;
	JL_CHK( JL_JsvalToODERealVector(cx, JL_ARG(2), dimensions, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );

// apply the formulae
	ode::dMassSetBoxTotal(&mass, (ode::dReal)totalMass, dimensions[0], dimensions[0], dimensions[0]);
// set mass object
	ode::dBodySetMass(bodyID, &mass);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME
  TBD dBodyGetMass, dBodySetMass
**/
DEFINE_PROPERTY_SETTER( value ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	JL_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	jsdouble massValue;
	JL_CHK( JL_JsvalToNative(cx, *vp, &massValue) );
	mass.mass = (ode::dReal)massValue;
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( value ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	JL_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	JL_CHK( JL_NativeToJsval(cx, mass.mass, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
  TBD dBodyGetMass, dBodySetMass
  get/set a _vec3_
**/
DEFINE_PROPERTY_SETTER( center ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	JL_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
//	jsdouble massValue;
//	jsdouble translation[3];
	//JL_CHK( FloatArrayToVector(cx, 3, vp, mass.c) );
	uint32 length;
	JL_CHK( JL_JsvalToODERealVector(cx, *vp, mass.c, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( center ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	JL_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	//JL_CHK( ODERealVectorToArray(cx, 3, mass.c, vp) );
	JL_CHK( ODERealVectorToJsval(cx, mass.c, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))

	BEGIN_FUNCTION_SPEC
		FUNCTION( Translate )
		FUNCTION( Adjust )
		FUNCTION( SetBoxTotal )
		FUNCTION( SetZero )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( value )
		PROPERTY( center )
	END_PROPERTY_SPEC

	HAS_RESERVED_SLOTS(1)

END_CLASS
