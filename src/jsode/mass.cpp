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
**/
BEGIN_CLASS( Mass )

JSBool GetBodyAndMass(JSContext *cx, JSObject *massObject, ode::dBodyID *pBodyID, ode::dMass *pMass) {

	jsval bodyVal;
	JS_GetReservedSlot(cx, massObject, MASS_SLOT_BODY, &bodyVal);
	JSObject *bodyObject;
	JS_ValueToObject(cx, bodyVal, &bodyObject);
	J_S_ASSERT_CLASS(bodyObject, classBody);
	*pBodyID = (ode::dBodyID)JS_GetPrivate(cx, bodyObject);
	ode::dBodyGetMass(*pBodyID, pMass);
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Methods ===
**/

/**doc
 * $VOID $INAME( vec3 )
  TBD dMassTranslate + dBodySetMass
**/

DEFINE_FUNCTION( Translate ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	J_S_ASSERT_ARG_MIN(1);
	J_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	real translation[3];
//	J_CHK( FloatArrayToVector(cx, 3, &argv[0], translation) );
	size_t length;
	J_CHK( JsvalToFloatVector(cx, argv[0], translation, 3, &length) );
	J_S_ASSERT( length == 3, "Invalid array size." );
	ode::dMassTranslate(&mass, translation[0], translation[1], translation[2]);
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
	JL_BAD;
}

/**doc
 * $VOID $INAME( mass )
  TBD dBodyGetMass, dMassAdjust, dBodySetMass
**/
DEFINE_FUNCTION( Adjust ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	J_S_ASSERT_ARG_MIN(1);
	J_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	jsdouble newMass;
	JS_ValueToNumber(cx, argv[0], &newMass);
	ode::dMassAdjust(&mass, newMass);
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $VOID $INAME()
  TBD dBodyGetMass, dMassSetZero, dBodySetMass
**/
DEFINE_FUNCTION( SetZero ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	J_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	ode::dMassSetZero(&mass);
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $INAME( mass, vec3 )
  TBD dBodyGetMass, dMassSetBoxTotal, dBodySetMass
**/
DEFINE_FUNCTION( SetBoxTotal ) {

	ode::dMass mass;
	J_S_ASSERT_ARG_MIN(2);
// get mass object
	ode::dBodyID bodyID;
	J_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
// arg 0
	jsdouble totalMass;
	JS_ValueToNumber(cx, argv[0], &totalMass);
// arg 1
	real dimensions[3];
	//	J_CHK( FloatArrayToVector(cx, 3, &argv[1], dimensions) );
	size_t length;
	J_CHK( JsvalToFloatVector(cx, argv[1], dimensions, 3, &length) );
	J_S_ASSERT( length == 3, "Invalid array size." );

// apply the formulae
	ode::dMassSetBoxTotal(&mass, totalMass, dimensions[0], dimensions[0], dimensions[0]);
// set mass object
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
 * $REAL $INAME
  TBD dBodyGetMass, dBodySetMass
**/
DEFINE_PROPERTY( valueSetter ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	J_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	jsdouble massValue;
	JS_ValueToNumber(cx, *vp, &massValue);
	mass.mass = massValue;
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( valueGetter ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	J_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	JS_NewDoubleValue(cx, mass.mass, vp);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $TYPE vec3 $INAME
  TBD dBodyGetMass, dBodySetMass
  get/set a _vec3_
**/
DEFINE_PROPERTY( centerSetter ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	J_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
//	jsdouble massValue;
//	jsdouble translation[3];
	//J_CHK( FloatArrayToVector(cx, 3, vp, mass.c) );
	size_t length;
	J_CHK( JsvalToFloatVector(cx, *vp, mass.c, 3, &length) );
	J_S_ASSERT( length == 3, "Invalid array size." );
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( centerGetter ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	J_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	//J_CHK( FloatVectorToArray(cx, 3, mass.c, vp) );
	J_CHK( FloatVectorToJsval(cx, mass.c, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
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
