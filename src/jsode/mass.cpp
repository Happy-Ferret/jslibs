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
BEGIN_CLASS( Mass )

JSBool GetBodyAndMass(JSContext *cx, JSObject *massObject, ode::dBodyID *pBodyID, ode::dMass *pMass) {

	jsval bodyVal;
	JS_GetReservedSlot(cx, massObject, MASS_SLOT_BODY, &bodyVal);
	JSObject *bodyObject;
	JS_ValueToObject(cx, bodyVal, &bodyObject);
	RT_ASSERT_CLASS(bodyObject, &classBody);
	*pBodyID = (ode::dBodyID)JS_GetPrivate(cx, bodyObject);
	ode::dBodyGetMass(*pBodyID, pMass);
	return JS_TRUE;
}


DEFINE_FUNCTION( Translate ) {

	RT_ASSERT_ARGC(1);
	ode::dBodyID bodyID;
	ode::dMass mass;
	RT_CHECK_CALL( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	real translation[3];
	RT_CHECK_CALL( FloatArrayToVector(cx, 3, &argv[0], translation) );
	ode::dMassTranslate(&mass, translation[0], translation[1], translation[2]);
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
}


DEFINE_FUNCTION( Adjust ) {

	RT_ASSERT_ARGC(1);
	ode::dBodyID bodyID;
	ode::dMass mass;
	RT_CHECK_CALL( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	jsdouble newMass;
	JS_ValueToNumber(cx, argv[0], &newMass);
	ode::dMassAdjust(&mass, newMass);
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
}


DEFINE_FUNCTION( SetZero ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	RT_CHECK_CALL( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	ode::dMassSetZero(&mass);
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
}


DEFINE_FUNCTION( SetBoxTotal ) {

	RT_ASSERT_ARGC(2);
// get mass object
	ode::dBodyID bodyID;
	ode::dMass mass;
	RT_CHECK_CALL( GetBodyAndMass(cx, obj, &bodyID, &mass) );
// arg 0
	jsdouble totalMass;
	JS_ValueToNumber(cx, argv[0], &totalMass);
// arg 1
	real dimensions[3];
	RT_CHECK_CALL( FloatArrayToVector(cx, 3, &argv[1], dimensions) );
// apply the formulae
	ode::dMassSetBoxTotal(&mass, totalMass, dimensions[0], dimensions[0], dimensions[0]);
// set mass object
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
}


DEFINE_PROPERTY( valueSetter ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	RT_CHECK_CALL( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	jsdouble massValue;
	JS_ValueToNumber(cx, *vp, &massValue);
	mass.mass = massValue;
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
}


DEFINE_PROPERTY( valueGetter ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	RT_CHECK_CALL( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	JS_NewDoubleValue(cx, mass.mass, vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( centerSetter ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	RT_CHECK_CALL( GetBodyAndMass(cx, obj, &bodyID, &mass) );
//	jsdouble massValue;
//	jsdouble translation[3];
	RT_CHECK_CALL( FloatArrayToVector(cx, 3, vp, mass.c) );
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
}


DEFINE_PROPERTY( centerGetter ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	RT_CHECK_CALL( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	RT_CHECK_CALL( FloatVectorToArray(cx, 3, mass.c, vp) );
	return JS_TRUE;
}


CONFIGURE_CLASS

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