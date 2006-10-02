#include "stdafx.h"
#include "mass.h"

#include "tools.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void mass_Finalize(JSContext *cx, JSObject *obj) {

	ode::dMass *mass = (ode::dMass*)JS_GetPrivate(cx, obj);
	if ( mass != NULL ) {

		free(mass);
		JS_SetPrivate(cx,obj,NULL);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool mass_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_CONSTRUCTING(&mass_class);
	ode::dMass *mass = (ode::dMass*)malloc(sizeof(ode::dMass));
	RT_ASSERT_ALLOC(mass);
//	ode::dMassSetZero(mass);

	jsdouble theMass = 1;
	if ( argc >= 1 )
		JS_ValueToNumber(cx, argv[0], &theMass);
	ode::dMassSetParameters( mass, theMass, 0, 0, 0, 1, 1, 1, 0, 0, 0 );

	JS_SetPrivate(cx, obj, mass); 
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JSBool mass_mass(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//	return JS_TRUE;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec mass_FunctionSpec[] = { // *name, call, nargs, flags, extra
//	{ "Mass" , mass_mass, 0, 0, 0 },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool mass_get_mass(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	RT_ASSERT_CLASS(obj, &mass_class);
	ode::dMass *mass = (ode::dMass*)JS_GetPrivate(cx, obj);
	RT_ASSERT(mass != NULL, RT_ERROR_NOT_INITIALIZED);

	JS_NewDoubleValue(cx, mass->mass, vp);

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool mass_set_mass(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	RT_ASSERT_CLASS(obj, &mass_class);
	ode::dMass *mass = (ode::dMass*)JS_GetPrivate(cx, obj);
	RT_ASSERT(mass != NULL, RT_ERROR_NOT_INITIALIZED);

	jsdouble massValue;
	JS_ValueToNumber(cx, *vp, &massValue);
	mass->mass = massValue;

	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec mass_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "mass", 0, JSPROP_PERMANENT|JSPROP_SHARED, mass_get_mass, mass_set_mass },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass mass_class = { "Mass", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, mass_Finalize,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *massInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &mass_class, mass_construct, 0, mass_PropertySpec, mass_FunctionSpec, NULL, NULL );
}


/****************************************************************

*/