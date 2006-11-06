#include "stdafx.h"
#include "body.h"
#include "mass.h"

#include "../smtools/smtools.h"

JSBool GetBodyAndMass(JSContext *cx, JSObject *massObject, ode::dBodyID *pBodyID, ode::dMass *pMass) {

	jsval bodyVal;
	JS_GetReservedSlot(cx, massObject, MASS_SLOT_BODY, &bodyVal);
	JSObject *bodyObject;
	JS_ValueToObject(cx, bodyVal, &bodyObject);
	RT_ASSERT_CLASS(bodyObject, &body_class);
	*pBodyID = (ode::dBodyID)JS_GetPrivate(cx, bodyObject);
	ode::dBodyGetMass(*pBodyID, pMass);
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool mass_translate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool mass_adjust(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool mass_setBoxTotal(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec mass_FunctionSpec[] = { // *name, call, nargs, flags, extra
	{ "Translate" , mass_translate, 0, 0, 0 },
	{ "Adjust" , mass_adjust, 0, 0, 0 },
	{ "SetBoxTotal" , mass_setBoxTotal, 0, 0, 0 },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool mass_get_mass(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	if ( GetBodyAndMass(cx, obj, &bodyID, &mass) == JS_FALSE)
		return JS_FALSE;
	JS_NewDoubleValue(cx, mass.mass, vp);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool mass_set_mass(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool mass_set_center(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	if ( GetBodyAndMass(cx, obj, &bodyID, &mass) == JS_FALSE)
		return JS_FALSE;
	jsdouble massValue;
//	jsdouble translation[3];
	if (FloatArrayToVector(cx, 3, vp, mass.c) == JS_FALSE)
		return JS_FALSE;
	ode::dBodySetMass(bodyID, &mass);


	return JS_TRUE;
}

JSBool mass_get_center(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	if ( GetBodyAndMass(cx, obj, &bodyID, &mass) == JS_FALSE)
		return JS_FALSE;
	if (FloatVectorToArray(cx, 3, mass.c, vp) == JS_FALSE)
		return JS_FALSE;
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec mass_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "mass"  , 0, JSPROP_PERMANENT|JSPROP_SHARED, mass_get_mass, mass_set_mass },
	{ "center", 0, JSPROP_PERMANENT|JSPROP_SHARED, mass_get_center, mass_set_center },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass mass_class = { "Mass", JSCLASS_HAS_RESERVED_SLOTS(1),
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *massInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &mass_class, NULL, 0, mass_PropertySpec, mass_FunctionSpec, NULL, NULL );
}


/****************************************************************

*/