#include "stdafx.h"
#include "joint.h"

BEGIN_CLASS( SurfaceParameters )


DEFINE_FINALIZE() {
	
	ode::dSurfaceParameters *data = (ode::dSurfaceParameters*)JS_GetPrivate(cx, obj);
	if ( data != NULL )
		free(data);
	JS_SetPrivate(cx, obj, NULL);
}


DEFINE_CONSTRUCTOR() {

	static ode::dSurfaceParameters initSurfaceParameters = {0};
	RT_ASSERT_CONSTRUCTING(_class);
	ode::dSurfaceParameters *data = (ode::dSurfaceParameters*)malloc(sizeof(ode::dSurfaceParameters));
	*data = initSurfaceParameters;
	data->mu = dInfinity;
	RT_ASSERT_ALLOC(data);
	JS_SetPrivate(cx, obj, data);
	return JS_TRUE;
}


enum { mu, mu2, bounce, bounceVel, softERP, softCFM, motion1, motion2, slip1, slip2 };

DEFINE_PROPERTY_NULL( surfaceGetter )
DEFINE_PROPERTY( surfaceSetter ) {

	ode::dSurfaceParameters *surface = (ode::dSurfaceParameters*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(surface); // [TBD] check if NULL is meaningful for joints !
	RT_ASSERT_NUMBER( *vp );
	ode::dReal value = JSValToODEReal(cx, *vp);
	switch(JSVAL_TO_INT(id)) {
		case mu:
			surface->mu = value;
			break;
		case mu2:
			surface->mode |= ode::dContactMu2;
			surface->mu2 = value;
			break;
		case bounce:
			surface->mode |= ode::dContactBounce;
			surface->bounce = value;
			break;
		case bounceVel:
			surface->mode |= ode::dContactBounce;
			surface->bounce_vel = value;
			break;
		case softERP:
			surface->mode |= ode::dContactSoftERP;
			surface->soft_erp = value;
			break;
		case softCFM:
			surface->mode |= ode::dContactSoftCFM;
			surface->soft_cfm = value;
			break;
		case motion1:
			surface->mode |= ode::dContactMotion1;
			surface->motion1 = value;
			break;
		case motion2:
			surface->mode |= ode::dContactMotion2;
			surface->motion2 = value;
			break;
		case slip1:
			surface->mode |= ode::dContactSlip1;
			surface->slip1 = value;
			break;
		case slip2:
			surface->mode |= ode::dContactSlip2;
			surface->slip2 = value;
			break;
	}  
// Doc: http://opende.sourceforge.net/wiki/index.php/Manual_%28Joint_Types_and_Functions%29#Contact
// [TBD] manage this:
// dContactFDir1 ?
//	dContactApprox0	= 0x0000,
//	dContactApprox1_1	= 0x1000,
//	dContactApprox1_2	= 0x2000,
//	dContactApprox1	= 0x3000
	return JS_TRUE;
}



CONFIGURE_CLASS
	
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_PROPERTY_SPEC
		PROPERTY_SWITCH_STORE( mu, surface )
		PROPERTY_SWITCH_STORE( mu2, surface )
		PROPERTY_SWITCH_STORE( bounce, surface )
		PROPERTY_SWITCH_STORE( bounceVel, surface )
		PROPERTY_SWITCH_STORE( softERP, surface )
		PROPERTY_SWITCH_STORE( softCFM, surface )
		PROPERTY_SWITCH_STORE( motion1, surface )
		PROPERTY_SWITCH_STORE( motion2, surface )
		PROPERTY_SWITCH_STORE( slip1, surface )
		PROPERTY_SWITCH_STORE( slip2, surface )
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
