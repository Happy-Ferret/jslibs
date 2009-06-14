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


/* This class cannot have a Finalize ( see readme.txt )
DEFINE_FINALIZE() {

	ode::dSpaceID spaceId = (ode::dSpaceID)JL_GetPrivate(cx,obj);
	if ( spaceId != NULL )
		ode::dSpaceDestroy(spaceId);
}
*/

/**doc
$TOC_MEMBER $INAME
 $INAME( parentSpace )
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();
	ode::dSpaceID parentSpace;
	if ( argc >= 1 )
		JL_CHK( JsvalToSpaceID(cx, argv[0], &parentSpace) );
	else
		parentSpace = 0;
	ode::dSpaceID spaceId = ode::dSimpleSpaceCreate(parentSpace);
	JL_SetPrivate(cx, obj, spaceId); // dSimpleSpaceCreate / dHashSpaceCreate / dQuadTreeSpaceCreate
	// ode::dHashSpaceSetLevels(spaceId,
	// (TBD) use this
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
//	HAS_FINALIZE
	HAS_PRIVATE

END_CLASS
