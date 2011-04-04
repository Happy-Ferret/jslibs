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

#include <jlmoduleprivate.h>
DEFINE_MODULE_PRIVATE

FT_DEFINE_SYMBOLS;


DECLARE_CLASS( Font3D )
DECLARE_STATIC()

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
	JL_CHKM( JL_GetProperty(cx, GetHostObject(cx), "_jsfontModulePrivate", (void**)&jsfontMpv) && jsfontMpv != NULL, E_MODULE, E_NAME("jsfont"), E_REQUIRED );

	jsfontMpv->GetFTSymbols(&_ftSymbols);
	mpv->ftLibrary = jsfontMpv->ftLibrary;

/*
	CHKHEAP();
	char *t = new char[10000];
	CHKHEAP();
	delete[] t;

	jl_free(0);

	FT_Library ftLibrary;
	CHKHEAP();
	FT_Error fterr;
	CHKHEAP();
	fterr = FT_Init_FreeType(&ftLibrary);
	CHKHEAP();
	FT_Face face;
	CHKHEAP();
	fterr = FT_New_Face( ftLibrary, "c:\\windows\\fonts\\arial.ttf", 0, &face );
	CHKHEAP();

	OGLFT::Face *face3d = new OGLFT::Outline(face, 9);
	CHKHEAP();
	delete face3d;
	CHKHEAP();

*/

	// test: OGLFT::Filled* face = new OGLFT::Filled("c:\\windows\\fonts\\arial.ttf"); face->draw("test");

	INIT_STATIC();
	INIT_CLASS( Font3D );

	return JS_TRUE;
	JL_BAD;
}


EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

//	jl_free(GetModulePrivate(cx, _moduleId));

	return JS_TRUE;
}


EXTERN_C DLLEXPORT void ModuleFree() {

	ModulePrivateFree();
}
