#include "stdafx.h"
#include "body.h"
#include "mass.h"

// [TBD] Mass object seems to be useless. Try to merge Body with Mass
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
	if ( GetBodyAndMass(cx, obj, &bodyID, &mass) == JS_FALSE)
		return JS_FALSE;
	real translation[3];
	if (FloatArrayToVector(cx, 3, &argv[0], translation) == JS_FALSE)
		return JS_FALSE;
	ode::dMassTranslate(&mass, translation[0], translation[1], translation[2]);
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
}


DEFINE_FUNCTION( Adjust ) {

	RT_ASSERT_ARGC(1);
	ode::dBodyID bodyID;
	ode::dMass mass;
	if ( GetBodyAndMass(cx, obj, &bodyID, &mass) == JS_FALSE)
		return JS_FALSE;
	jsdouble newMass;
	JS_ValueToNumber(cx, argv[0], &newMass);
	ode::dMassAdjust(&mass, newMass);
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
}


DEFINE_FUNCTION( SetBoxTotal ) {

	RT_ASSERT_ARGC(2);
// get mass object
	ode::dBodyID bodyID;
	ode::dMass mass;
	if ( GetBodyAndMass(cx, obj, &bodyID, &mass) == JS_FALSE)
		return JS_FALSE;
// arg 0
	jsdouble totalMass;
	JS_ValueToNumber(cx, argv[0], &totalMass);
// arg 1
	real dimensions[3];
	if (FloatArrayToVector(cx, 3, &argv[1], dimensions) == JS_FALSE)
		return JS_FALSE;
// apply the formulae
	ode::dMassSetBoxTotal(&mass, totalMass, dimensions[0], dimensions[0], dimensions[0]);
// set mass object
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
}


DEFINE_PROPERTY( valueSetter ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	if ( GetBodyAndMass(cx, obj, &bodyID, &mass) == JS_FALSE)
		return JS_FALSE;
	jsdouble massValue;
	JS_ValueToNumber(cx, *vp, &massValue);
	mass.mass = massValue;
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
}


DEFINE_PROPERTY( valueGetter ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	if ( GetBodyAndMass(cx, obj, &bodyID, &mass) == JS_FALSE)
		return JS_FALSE;
	JS_NewDoubleValue(cx, mass.mass, vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( centerSetter ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	if ( GetBodyAndMass(cx, obj, &bodyID, &mass) == JS_FALSE)
		return JS_FALSE;
//	jsdouble massValue;
//	jsdouble translation[3];
	if (FloatArrayToVector(cx, 3, vp, mass.c) == JS_FALSE)
		return JS_FALSE;
	ode::dBodySetMass(bodyID, &mass);
	return JS_TRUE;
}


DEFINE_PROPERTY( centerGetter ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	if ( GetBodyAndMass(cx, obj, &bodyID, &mass) == JS_FALSE)
		return JS_FALSE;
	if (FloatVectorToArray(cx, 3, mass.c, vp) == JS_FALSE)
		return JS_FALSE;
	return JS_TRUE;
}


CONFIGURE_CLASS

	BEGIN_FUNCTION_SPEC
		FUNCTION( Translate )
		FUNCTION( Adjust )
		FUNCTION( SetBoxTotal )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( value )
		PROPERTY( center )
	END_PROPERTY_SPEC

	HAS_RESERVED_SLOTS(1)

END_CLASS
