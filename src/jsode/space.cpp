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
**/
BEGIN_CLASS( Space )


/* This class cannot have a Finalize ( see readme.txt )
DEFINE_FINALIZE() {

	ode::dSpaceID spaceId = (ode::dSpaceID)JS_GetPrivate(cx,obj);
	if ( spaceId != NULL )
		ode::dSpaceDestroy(spaceId);
}
*/

/**doc
 * $INAME( parentSpace )
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	ode::dSpaceID parentSpace = 0;
	if ( argc >= 1 )
		if ( ValToSpaceID(cx, argv[0], &parentSpace) == JS_FALSE )
			return JS_FALSE;
	ode::dSpaceID spaceId = ode::dSimpleSpaceCreate(parentSpace);
	JS_SetPrivate(cx, obj, spaceId); // dSimpleSpaceCreate / dHashSpaceCreate / dQuadTreeSpaceCreate
	// ode::dHashSpaceSetLevels(spaceId,
	// (TBD) use this
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
//	HAS_FINALIZE
	HAS_PRIVATE

END_CLASS
