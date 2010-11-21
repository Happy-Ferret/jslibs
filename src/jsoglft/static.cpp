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

#include "oglft/OGLFT.h"


/**doc fileIndex:topmost **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3060 $
**/
BEGIN_STATIC


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( $TYPE Font font, str [, size] [, compileList] )
  Draw a 3D string using the given font.
  $H arguments
   $ARG $TYE Font: a font object from the jsfont module.
   $ARG $STR str: the string.
   $ARG $INT size: the point size of the font to generate. A point is essentially 1/72th of an inch.
**/
DEFINE_FUNCTION( Draw3DText ) {

	JLStr text;

	JL_S_ASSERT_ARG_RANGE( 2, 4 );
	JL_S_ASSERT_OBJECT(JL_ARG(1));

	JSObject *fontObj = JSVAL_TO_OBJECT(JL_ARG(1));

	FT_Face ftface = GetJsfontPrivate(cx, fontObj)->face;

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &text) );

	JsoglftPrivate *mpv = (JsoglftPrivate*)ModulePrivateGet();

	float size;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &size) );
	else
		size = float(ftface->size->metrics.y_scale / ftface->units_per_EM);

	bool compile;
	if ( JL_ARG_ISDEF(4) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &compile) );
	else
		compile = false;

	OGLFT::Filled* face = new OGLFT::Filled(ftface, size);
	
//	face->setTessellationSteps(2);

	if ( compile ) {

		GLfloat color[4];
		glGetFloatv(GL_CURRENT_COLOR, color);
		face->setForegroundColor(color);

		GLuint list = face->compile(text);
		*JL_RVAL = INT_TO_JSVAL(list);
	} else {

		face->draw(text);
		*JL_RVAL = JSVAL_VOID;
	}

	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision: 3060 $"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Draw3DText )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

END_STATIC
