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
$SVN_REVISION $Revision: 3524 $
**/
BEGIN_CLASS( GeomBox )

DEFINE_FINALIZE() {

	JL_IGNORE(fop);

	FinalizeGeom(obj);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( [ space ] )
  It is up to the user to store new object to prevent it to be garbage collected.
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

	geomId = ode::dCreateBox(space, 1.0f,1.0f,1.0f); // default lengths are 1
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
 $TYPE vec3 $INAME
  Is the x, y, z size of the box.
**/
DEFINE_PROPERTY_SETTER( lengths ) {

	JL_IGNORE(strict, id);

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	JL_ASSERT_IS_ARRAY(*vp, "");
	ode::dVector3 vector;
//	FloatArrayToVector(cx, 3, vp, vector);
	uint32_t length;
	JL_CHK( JsvalToODERealVector(cx, *vp, vector, 3, &length) );
	JL_ASSERT( length >= 3, E_VALUE, E_TYPE, E_TY_NVECTOR(3) );
	ode::dGeomBoxSetLengths(geom, vector[0], vector[1], vector[2]);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( lengths ) {

	JL_IGNORE(id);

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	ode::dVector3 result;
	ode::dGeomBoxGetLengths(geom, result);
	JL_CHK( ODERealVectorToJsval(cx, result, 3, vp) );
	return true;
	JL_BAD;
}

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3524 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(3)

	HAS_PROTOTYPE( Geom )
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_PROPERTY_SPEC
		PROPERTY( lengths )
	END_PROPERTY_SPEC

END_CLASS
