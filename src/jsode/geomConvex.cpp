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
BEGIN_CLASS( GeomConvex )

DEFINE_FINALIZE() {

	FinalizeGeom(cx, obj);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( [ space ] )
  It is up to the user to store new object to prevent it to be garbage collected.
  TBD
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_RANGE(0, 1);
	ode::dSpaceID space;
	if ( JL_ARG_ISDEF(1) ) { // place it in a space ?

		JL_CHK( JL_JsvalToSpaceID(cx, JL_ARG(1), &space) );
		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_GEOM_SPACEOBJECT, JL_ARG(1)) );
	} else {

		space = 0;
	}

// planes: An array of planes in the form: normal X, normal Y, normal Z,Distance
// points: An array of points X,Y,Z
// polygons: An array of indices to the points of each polygon, it should be the number of vertices 
//           followed by that amount of indices to "points" in counter clockwise order


	ode::dGeomID geomId = ode::dCreateConvex(space, NULL, 0, NULL, 0, NULL);
	JL_SetPrivate(cx, obj, geomId);
	JL_CHK( SetupReadMatrix(cx, obj) );
	ode::dGeomSetData(geomId, obj); // 'obj' do not need to be rooted because Goem's data is reset to NULL when 'obj' is finalized.
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 3414 $"))
	HAS_PROTOTYPE( Geom )
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(3)

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

END_CLASS
