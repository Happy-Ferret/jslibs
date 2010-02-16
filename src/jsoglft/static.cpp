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

#include "jlmoduleprivate.h"
#include "jsoglft.h"

#include "../jsfont/jsfont.h"


#include "oglft/config.h"
#include "oglft/OGLFT.h"


/**doc fileIndex:topmost **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3060 $
**/
BEGIN_STATIC

DEFINE_FUNCTION_FAST( DrawText ) {

	JL_S_ASSERT_ARG_RANGE(2,2);
	
	JL_S_ASSERT_OBJECT(JL_FARG(1));

	FT_Face ftface = GetFace(cx, JSVAL_TO_OBJECT(JL_FARG(1)));

	const char *text;
	JL_CHK( JsvalToString(cx, &JL_FARG(2), &text) );

	JsoglftPrivate *mpv = (JsoglftPrivate*)ModulePrivateGet();

	OGLFT::Filled* face = new OGLFT::Filled(ftface);

//	face->setForegroundColor( 1., 0., 0. );

	face->draw(text);

	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision: 3060 $"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST( DrawText )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

END_STATIC
