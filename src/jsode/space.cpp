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

/*

check this:
	void dSpaceSetCleanup (dSpaceID space, int mode);
	int dSpaceGetCleanup (dSpaceID space);
*/

#include "stdafx.h"
#include "space.h"

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Space )

/* WARNING: a previous comment says: This class cannot have a Finalize ( see readme.txt )
Everything that may be GC SHOULD not have a Finalize:
- Body, Joint, Space, Geom
But these functions should have a Destroy function
*/
DEFINE_FINALIZE() {

//	if ( obj == JL_THIS_PROTOTYPE )
//		return;
	ode::dSpaceID spaceId = (ode::dSpaceID)JL_GetPrivate(cx, obj);
	if ( spaceId == NULL )
		return;
/*
	int numGeom = ode::dSpaceGetNumGeoms(spaceId);
	for ( int i = 0; i < numGeom; ++i ) {
		
		ode::dGeomID geomId = dSpaceGetGeom(spaceId, i);
		ode::dSpaceRemove(spaceId, geomId);
	}
*/
//	ode::dSpaceClean(spaceId);
//	ode::dSpaceDestroy(spaceId);
}


/**doc
$TOC_MEMBER $INAME
 $INAME( parentSpace )
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	ode::dSpaceID parentSpace;
	if ( argc >= 1 )
		JL_CHK( JL_JsvalToSpaceID(cx, JL_ARG(1), &parentSpace) );
	else
		parentSpace = 0;
//	ode::dSpaceID spaceId = ode::dSimpleSpaceCreate(parentSpace);
	ode::dSpaceID spaceId = ode::dHashSpaceCreate(parentSpace);
	ode::dSpaceSetCleanup(spaceId, 0);
	JL_SetPrivate(cx, obj, spaceId); // dSimpleSpaceCreate / dHashSpaceCreate / dQuadTreeSpaceCreate
//	ode::dHashSpaceSetLevels(spaceId,
	// (TBD) use this
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
**/
DEFINE_FUNCTION( Destroy ) {
	
	JL_DEFINE_FUNCTION_OBJ;

	ode::dSpaceID spaceId = (ode::dSpaceID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(spaceId);
	if ( spaceId != NULL ) {

		ode::dSpaceClean(spaceId);
		ode::dSpaceDestroy(spaceId);
	}
	JL_SetPrivate(cx, obj, NULL);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Destroy )
	END_FUNCTION_SPEC

END_CLASS
