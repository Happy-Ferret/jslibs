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
DEFINE_MODULE_PRIVATE

#include "jsoglft.h"

#include "jslibsModule.cpp"

#include "../jsfont/jsfont.h"
FT_DEFINE_SYMBOLS;

#include "oglft/OGLFT.h"


DECLARE_STATIC()
DECLARE_CLASS( Font3D )

namespace OGLFT {

	FT_Library& Library::instance ( void ) {

		JsoglftPrivate *mpv = (JsoglftPrivate*)ModulePrivateGet();
		return mpv->ftLibrary;
	}
}


EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	JsoglftPrivate *mpv = (JsoglftPrivate*)ModulePrivateAlloc(sizeof(JsoglftPrivate));

	JL_CHK( InitJslibsModule(cx, id) );

	JsfontModulePrivate *jsfontMpv;
	JL_CHK( GetNativePrivatePointer(cx, JS_GetGlobalObject(cx), "_jsfontModulePrivate", (void**)&jsfontMpv) );
	JL_S_ASSERT( jsfontMpv != NULL, "jsfont module not loaded." );

	jsfontMpv->GetFTSymbols(&_ftSymbols);
	mpv->ftLibrary = jsfontMpv->ftLibrary;

	// test: OGLFT::Filled* face = new OGLFT::Filled("c:\\windows\\fonts\\arial.ttf"); face->draw("test");

	INIT_STATIC();
	INIT_CLASS( Font3D );

	return JS_TRUE;
	JL_BAD;
}

EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

//	jl_free(GetModulePrivate(cx, _moduleId));

	return JS_FALSE;
}

EXTERN_C DLLEXPORT void ModuleFree() {

	ModulePrivateFree();
}
