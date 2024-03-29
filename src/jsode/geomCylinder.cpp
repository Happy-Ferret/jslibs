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
#include "space.h"
#include "geom.h"

/**doc
$CLASS_HEADER Geom
$SVN_REVISION $Revision: 3414 $
**/
BEGIN_CLASS( GeomCylinder )

DEFINE_FINALIZE() {

	FinalizeGeom(obj);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( [ space ] )
  TBD
**/
DEFINE_CONSTRUCTOR() {

	ode::dGeomID geomId = NULL;

	JL_ASSERT_ARGC_RANGE(0, 1);
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	ode::dSpaceID space;
	if ( JL_ARG_ISDEF(1) ) { // place it in a space ?

		JL_CHK( JL_JsvalToSpaceID(cx, JL_ARG(1), &space) );
		JL_CHK( JL_SetReservedSlot( obj, SLOT_GEOM_SPACEOBJECT, JL_ARG(1)) );
	} else {

		space = 0;
	}
	geomId = ode::dCreateCylinder(space, 1, 1); // default radius and length are 1
	JL_ASSERT( geomId, E_STR(JL_THIS_CLASS_NAME), E_CREATE );

	JL_CHK( SetupReadMatrix(cx, obj) );
	ode::dGeomSetData(geomId, obj); // 'obj' do not need to be rooted because Goem's data is reset to NULL when 'obj' is finalized.

	JL_SetPrivate(obj, geomId);
	return true;

bad:
	if ( geomId )
		ode::dGeomDestroy(geomId);
	return false;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME
  TBD
**/
DEFINE_PROPERTY_SETTER( radius ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
		ode::dReal radius, length;
	ode::dGeomCylinderGetParams(geom, &radius, &length);
	double value;
	JL_CHK( jl::getValue(cx, *vp, &value) );
	ode::dGeomCylinderSetParams(geom, (ode::dReal)value, length);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( radius ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	ode::dReal radius, length;
	ode::dGeomCylinderGetParams(geom, &radius, &length);
	JL_CHK( ODERealToJsval(cx, radius, vp) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME
  Is the length of the Cylinder.
**/
DEFINE_PROPERTY_SETTER( length ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	ode::dReal radius, length;
	ode::dGeomCylinderGetParams(geom, &radius, &length);
	double value;
	JL_CHK( jl::getValue(cx, *vp, &value) );
	ode::dGeomCylinderSetParams(geom, radius, (ode::dReal)value);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( length ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	ode::dReal radius, length;
	ode::dGeomCylinderGetParams(geom, &radius, &length);
	JL_CHK( ODERealToJsval(cx, length, vp) ); // see JL_NewNumberValue and JS_NewDouble
	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3414 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(3)

	HAS_PROTOTYPE( Geom )
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_PROPERTY_SPEC
		PROPERTY( radius )
		PROPERTY( length )
	END_PROPERTY_SPEC

END_CLASS
