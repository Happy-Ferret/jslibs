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

	RT_ASSERT_CONSTRUCTING(_class);
	ode::dSurfaceParameters *data = (ode::dSurfaceParameters*)malloc(sizeof(ode::dSurfaceParameters));
	RT_ASSERT_ALLOC(data);
	JS_SetPrivate(cx, obj, data);
	return JS_TRUE;
}


enum { mu, mu2, bounce, bounceVel, softErp, softCfm, motion1, motion2, slip1, slip2 };

DEFINE_PROPERTY_NULL( surfaceGetter );
DEFINE_PROPERTY( surfaceSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId); // [TBD] check if NULL is meaningful for joints !
	jsdouble dval;
	JS_ValueToNumber(cx, *vp, &dval);
	ode::dContact *contact = NULL;
	switch(JSVAL_TO_INT(id)) {
		case mu:
			contact->surface.mu = dval;
			break;
		case mu2:
			contact->surface.mode |= ode::dContactMu2;
			contact->surface.mu2 = dval;
			break;
		case bounce:
			contact->surface.mode |= ode::dContactBounce;
			contact->surface.bounce = dval;
			break;
		case bounceVel:
			contact->surface.mode |= ode::dContactBounce;
			contact->surface.bounce_vel = dval;
			break;
		case softErp:
			contact->surface.mode |= ode::dContactSoftERP;
			contact->surface.soft_erp = dval;
			break;
		case softCfm:
			contact->surface.mode |= ode::dContactSoftCFM;
			contact->surface.soft_cfm = dval;
			break;
		case motion1:
			contact->surface.mode |= ode::dContactMotion1;
			contact->surface.motion1 = dval;
			break;
		case motion2:
			contact->surface.mode |= ode::dContactMotion2;
			contact->surface.motion2 = dval;
			break;
		case slip1:
			contact->surface.mode |= ode::dContactSlip1;
			contact->surface.slip1 = dval;
			break;
		case slip2:
			contact->surface.mode |= ode::dContactSlip2;
			contact->surface.slip2 = dval;
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
		PROPERTY_SWITCH_STORE( softErp, surface )
		PROPERTY_SWITCH_STORE( softCfm, surface )
		PROPERTY_SWITCH_STORE( motion1, surface )
		PROPERTY_SWITCH_STORE( motion2, surface )
		PROPERTY_SWITCH_STORE( slip1, surface )
		PROPERTY_SWITCH_STORE( slip2, surface )
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
