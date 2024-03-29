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
#include "joint.h"

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3491 $
**/
BEGIN_CLASS( SurfaceParameters )

DEFINE_FINALIZE() {

	ode::dSurfaceParameters *pv = (ode::dSurfaceParameters*)JL_GetPrivate(obj);
	if ( !pv )
		return;
	JL_freeop(fop, pv);
}

/**doc
$TOC_MEMBER $INAME
 $INAME()
  $H note
   by default, mu is set to Infinity.
**/
DEFINE_CONSTRUCTOR() {

	ode::dSurfaceParameters *pv = NULL;

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	pv = (ode::dSurfaceParameters*)JS_malloc(cx, sizeof(ode::dSurfaceParameters));
	JL_CHK( pv );
	pv->mu = dInfinity;
	pv->mode = 0;
	
	JL_SetPrivate(obj, pv);
	return true;

bad:
	JS_free(cx, pv);
	return false;
}

#define SETBIT(value, mask, polarity) ((value) = (polarity) ? (value) | (mask) : (value) & ~(mask) )
#define GETBIT(value, mask) (!!((value) | (mask)))

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 * $REAL *mu*
  Coulomb friction coefficient. This must be in the range 0 to dInfinity. 0 results in a frictionless contact, and dInfinity results in a contact that never slips.
  Note that frictionless contacts are less time consuming to compute than ones with friction, and infinite friction contacts can be cheaper than contacts with finite friction.
  This must always be set.
 
 * $REAL *mu2*
  Optional Coulomb friction coefficient for friction direction 2 (0..dInfinity).

 * $REAL *bounce*
  Restitution parameter (0..1). 0 means the surfaces are not bouncy at all, 1 is maximum bouncyness.

 * $REAL *bounceVel*
  The minimum incoming velocity necessary for bounce. Incoming velocities below this will effectively have a bounce parameter of 0.

 * $REAL *softERP*
  Contact normal "softness" parameter. 

 * $REAL *softCFM*
  Contact normal "softness" parameter.

 * $REAL *motion1*, $REAL *motion2*, $REAL motionN
  Surface velocity in friction directions 1 and 2 and along the normal.

 * $REAL *slip1*, $REAL *slip2*
  The coefficients of force-dependent-slip (FDS) for friction directions 1 and 2. 

 $H note
  Use $UNDEF as value to reset the property.

 $H ODE API
  [http://opende.sourceforge.net/wiki/index.php/Manual_(Joint_Types_and_Functions)#Contact dSurfaceParameters]
**/

enum { mu, mu2, fDir1, bounce, bounceVel, softERP, softCFM, motion1, motion2, motionN, slip1, slip2 };

DEFINE_PROPERTY_GETTER( surface ) {

	ode::dSurfaceParameters *surface = (ode::dSurfaceParameters*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(surface); // (TBD) check if NULL is meaningful for joints !
	
	*vp = JSVAL_VOID;
	switch ( JSID_TO_INT(id) ) {
		case mu:
			JL_CHK( ODERealToJsval(cx, surface->mu, vp) );
			break;
		case mu2:
			if ( GETBIT(surface->mode, ode::dContactMu2) )
				JL_CHK( ODERealToJsval(cx, surface->mu2, vp) );
			break;
		case fDir1:
			if ( GETBIT(surface->mode, ode::dContactFDir1) )
				JL_CHK( ODERealToJsval(cx, surface->mu2, vp) );
		case bounce:
			if ( GETBIT(surface->mode, ode::dContactBounce) )
				JL_CHK( ODERealToJsval(cx, surface->bounce, vp) );
			break;
		case bounceVel:
			if ( GETBIT(surface->mode, ode::dContactBounce) )
				JL_CHK( ODERealToJsval(cx, surface->bounce_vel, vp) );
			break;
		case softERP:
			if ( GETBIT(surface->mode, ode::dContactSoftERP) )
				JL_CHK( ODERealToJsval(cx, surface->soft_erp, vp) );
			break;
		case softCFM:
			if ( GETBIT(surface->mode, ode::dContactSoftCFM) )
				JL_CHK( ODERealToJsval(cx, surface->soft_cfm, vp) );
			break;
		case motion1:
			if ( GETBIT(surface->mode, ode::dContactMotion1) )
				JL_CHK( ODERealToJsval(cx, surface->motion1, vp) );
			break;
		case motion2:
			if ( GETBIT(surface->mode, ode::dContactMotion2) )
				JL_CHK( ODERealToJsval(cx, surface->motion2, vp) );
			break;
		case motionN:
			if ( GETBIT(surface->mode, ode::dContactMotionN) )
				JL_CHK( ODERealToJsval(cx, surface->motionN, vp) );
			break;
		case slip1:
			if ( GETBIT(surface->mode, ode::dContactSlip1) )
				JL_CHK( ODERealToJsval(cx, surface->slip1, vp) );
			break;
		case slip2:
			if ( GETBIT(surface->mode, ode::dContactSlip2) )
				JL_CHK( ODERealToJsval(cx, surface->slip2, vp) );
			break;
	}

	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( surface ) {

	JL_IGNORE( strict );

	ode::dSurfaceParameters *surface = (ode::dSurfaceParameters*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(surface); // (TBD) check if NULL is meaningful for joints !

	ode::dReal value;
	bool set;
	if ( vp.isUndefined() ) {

		set = false;
		IFDEBUG( value = 0 ); // avoid "potentially uninitialized local variable" warning
	} else {

		set = true;
		JL_CHK( JsvalToODEReal(cx, *vp, &value) );
	}

	switch ( JSID_TO_INT(id) ) {
		case mu:
			if ( set )
				surface->mu = value;
			else
				surface->mu = dInfinity;
			break;
		case mu2:
			SETBIT( surface->mode, ode::dContactMu2, set );
			if ( set )
				surface->mu2 = value;
			break;
		case fDir1:
			SETBIT( surface->mode, ode::dContactFDir1, set );
			// (TBD) manage the value
			break;
		case bounce:
			SETBIT( surface->mode, ode::dContactBounce, set );
			if ( set )
				surface->bounce = value;
			break;
		case bounceVel:
			SETBIT( surface->mode, ode::dContactBounce, set );
			if ( set )
				surface->bounce_vel = value;
			break;
		case softERP:
			SETBIT( surface->mode, ode::dContactSoftERP, set );
			if ( set )
				surface->soft_erp = value;
			break;
		case softCFM:
			SETBIT( surface->mode, ode::dContactSoftCFM, set );
			if ( set )
				surface->soft_cfm = value;
			break;
		case motion1:
			SETBIT( surface->mode, ode::dContactMotion1, set );
			if ( set )
				surface->motion1 = value;
			break;
		case motion2:
			SETBIT( surface->mode, ode::dContactMotion2, set );
			if ( set )
				surface->motion2 = value;
			break;
		case motionN:
			SETBIT( surface->mode, ode::dContactMotionN, set );
			if ( set )
				surface->motionN = value;
			break;
		case slip1:
			SETBIT( surface->mode, ode::dContactSlip1, set );
			if ( set )
				surface->slip1 = value;
			break;
		case slip2:
			SETBIT( surface->mode, ode::dContactSlip2, set );
			if ( set )
				surface->slip2 = value;
			break;
	}

// Doc: http://opende.sourceforge.net/wiki/index.php/Manual_%28Joint_Types_and_Functions%29#Contact
// (TBD) manage this:
// dContactFDir1 ?
//	dContactApprox0	= 0x0000,
//	dContactApprox1_1	= 0x1000,
//	dContactApprox1_2	= 0x2000,
//	dContactApprox1	= 0x3000

	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3491 $"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_PROPERTY_SPEC
		PROPERTY_SWITCH( mu       , surface )
		PROPERTY_SWITCH( mu2      , surface )
		PROPERTY_SWITCH( bounce   , surface )
		PROPERTY_SWITCH( bounceVel, surface )
		PROPERTY_SWITCH( softERP  , surface )
		PROPERTY_SWITCH( softCFM  , surface )
		PROPERTY_SWITCH( motion1  , surface )
		PROPERTY_SWITCH( motion2  , surface )
		PROPERTY_SWITCH( motionN  , surface )
		PROPERTY_SWITCH( slip1    , surface )
		PROPERTY_SWITCH( slip2    , surface )
	END_PROPERTY_SPEC

END_CLASS
