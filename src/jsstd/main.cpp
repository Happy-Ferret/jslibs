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
#include <jlhelper.cpp>
#include <jslibsModule.cpp>
#include "jsstd.h"


DECLARE_STATIC()
//DECLARE_CLASS( Map )
DECLARE_CLASS( Buffer )
DECLARE_CLASS( Pack )
DECLARE_CLASS( ObjEx )

/**doc t:header
$MODULE_HEADER
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

JSBool
ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	JL_CHK( InitJslibsModule(cx, id)  );

	ModulePrivate *mpv;
	mpv = (ModulePrivate*)jl_malloc(sizeof(ModulePrivate));
	JL_ASSERT_ALLOC( mpv );
	JL_CHKM( JL_SetModulePrivate(cx, _moduleId, mpv), E_MODULE, E_INIT ); // "Module id already in use."

	mpv->objIdList = NULL;
	mpv->lastObjectId = 0;
	mpv->objectIdAllocated = 0;
	mpv->prevObjectIdGCCallback = NULL;
	mpv->prevJSGCCallback = NULL;

	INIT_STATIC();
//	INIT_CLASS( Map );
	INIT_CLASS( Buffer );
	INIT_CLASS( Pack );
	INIT_CLASS( ObjEx );

	return JS_TRUE;
	JL_BAD;
}


JSBool
ModuleRelease(JSContext *cx) {

	if ( JL_GetHostPrivate(cx)->canSkipCleanup ) // do not cleanup in unsafe mode.
		return JS_TRUE;

	ModulePrivate *mpv = (ModulePrivate*)JL_GetModulePrivate(cx, _moduleId);

	if ( mpv->objIdList )
		jl_free(mpv->objIdList);

	// (TBD) need to restore mpv->prevObjectIdGCCallback ?

	jl_free(mpv);

	return JS_TRUE;
}
