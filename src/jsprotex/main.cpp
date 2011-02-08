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
#include "jlhelper.cpp"
#include "jslibsModule.cpp"

DECLARE_STATIC()
DECLARE_CLASS( Texture )

/**doc t:header
$MODULE_HEADER
 jsprotex is a procedural texture generation module to let you create
 high resolution textures that fit in few lines of source code.
 The texture generator is separated into small operators with each their set of parameters.
 These operators can be connected togethers to produce the final result.
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	JL_CHK( InitJslibsModule(cx, id)  );

	INIT_STATIC();
	INIT_CLASS( Texture );

	return JS_TRUE;
	JL_BAD;
}
