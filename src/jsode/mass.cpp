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
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( Mass )

bool GetBodyAndMass(JSContext *cx, JSObject *massObject, ode::dBodyID *pBodyID, ode::dMass *pMass) {

	jsval bodyVal;
	JL_CHK( JL_GetReservedSlot( massObject, MASS_SLOT_BODY, &bodyVal) );
	JL_ASSERT_THIS_OBJECT_STATE( JSVAL_IS_OBJECT(bodyVal) );
	JSObject *bodyObject;
	bodyObject = JSVAL_TO_OBJECT(bodyVal);
	JL_ASSERT_INSTANCE(bodyObject, JL_CLASS(Body));
	*pBodyID = (ode::dBodyID)JL_GetPrivate(bodyObject);
	ASSERT(*pBodyID);
	ode::dBodyGetMass(*pBodyID, pMass);
	return true;
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

DEFINE_FUNCTION( translate ) {

	ode::dBodyID bodyID;
	ode::dMass mass;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN(1);

	JL_CHK( GetBodyAndMass(cx, JL_OBJ, &bodyID, &mass) );
	real translation[3];
//	JL_CHK( FloatArrayToVector(cx, 3, &argv[0], translation) );
	uint32_t length;
	JL_CHK( JsvalToODERealVector(cx, JL_ARG(1), translation, 3, &length) );
	JL_ASSERT( length >= 3, E_ARG, E_NUM(1), E_TYPE, E_TY_NVECTOR(3) );
	ode::dMassTranslate(&mass, translation[0], translation[1], translation[2]);
	ode::dBodySetMass(bodyID, &mass);

	*JL_RVAL = JSVAL_VOID;
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mass )
  TBD dBodyGetMass, dMassAdjust, dBodySetMass
**/
DEFINE_FUNCTION( adjust ) {

	ode::dBodyID bodyID;
	ode::dMass mass;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN(1);

	JL_CHK( GetBodyAndMass(cx, JL_OBJ, &bodyID, &mass) );
	double newMass;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &newMass) );
	ode::dMassAdjust(&mass, (ode::dReal)newMass);
	ode::dBodySetMass(bodyID, &mass);
	
	*JL_RVAL = JSVAL_VOID;
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  TBD dBodyGetMass, dMassSetZero, dBodySetMass
**/
DEFINE_FUNCTION( setZero ) {

	JL_DEFINE_FUNCTION_OBJ;

	ode::dBodyID bodyID;
	ode::dMass mass;
	JL_CHK( GetBodyAndMass(cx, JL_OBJ, &bodyID, &mass) );
	ode::dMassSetZero(&mass);
	ode::dBodySetMass(bodyID, &mass);
	
	*JL_RVAL = JSVAL_VOID;
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mass, vec3 )
  TBD dBodyGetMass, dMassSetBoxTotal, dBodySetMass
**/
DEFINE_FUNCTION( setBoxTotal ) {

	ode::dMass mass;
	ode::dBodyID bodyID;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN(2);

	JL_CHK( GetBodyAndMass(cx, JL_OBJ, &bodyID, &mass) );
// arg 0
	double totalMass;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &totalMass) );
// arg 1
	real dimensions[3];
	//	JL_CHK( FloatArrayToVector(cx, 3, &argv[1], dimensions) );
	uint32_t length;
	JL_CHK( JsvalToODERealVector(cx, JL_ARG(2), dimensions, 3, &length) );
	JL_ASSERT( length >= 3, E_ARG, E_NUM(2), E_TYPE, E_TY_NVECTOR(3) );

// apply the formulae
	ode::dMassSetBoxTotal(&mass, (ode::dReal)totalMass, dimensions[0], dimensions[0], dimensions[0]);
// set mass object
	ode::dBodySetMass(bodyID, &mass);

	*JL_RVAL = JSVAL_VOID;
	return true;
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
	double massValue;
	JL_CHK( JL_JsvalToNative(cx, *vp, &massValue) );
	mass.mass = (ode::dReal)massValue;
	ode::dBodySetMass(bodyID, &mass);
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( value ) {

	ode::dBodyID bodyID;
	ode::dMass mass;

	JL_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	JL_CHK( JL_NativeToJsval(cx, mass.mass, vp) );
	return true;
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
//	double massValue;
//	double translation[3];
	//JL_CHK( FloatArrayToVector(cx, 3, vp, mass.c) );
	uint32_t length;
	JL_CHK( JsvalToODERealVector(cx, *vp, mass.c, 3, &length) );
	JL_ASSERT( length >= 3, E_VALUE, E_TYPE, E_TY_NVECTOR(3) );

	ode::dBodySetMass(bodyID, &mass);
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( center ) {

	ode::dBodyID bodyID;
	ode::dMass mass;
	JL_CHK( GetBodyAndMass(cx, obj, &bodyID, &mass) );
	//JL_CHK( ODERealVectorToArray(cx, 3, mass.c, vp) );
	JL_CHK( ODERealVectorToJsval(cx, mass.c, 3, vp) );
	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))

	HAS_RESERVED_SLOTS(1)

	BEGIN_FUNCTION_SPEC
		FUNCTION( translate )
		FUNCTION( adjust )
		FUNCTION( setBoxTotal )
		FUNCTION( setZero )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( value )
		PROPERTY( center )
	END_PROPERTY_SPEC

END_CLASS
