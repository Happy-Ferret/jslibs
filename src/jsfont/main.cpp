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

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H


DECLARE_CLASS( Font )


/**doc t:header
$MODULE_HEADER
 Support text rendering (text to image) from the following font format:$LF
 TrueType, Type 1, CID-keyed Type 1, CFF, OpenType TrueType, OpenType CFF, SFNT-based bitmap, X11 PCF, Windows FNT, BDF, PFR, Type 42
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/


void*
JsfontAlloc( FT_Memory memory, long size ) {

	return jl_malloc(size);
}


void
JsfontFree( FT_Memory memory, void* block ) {

	jl_free(block);
}


void*
JsfontRealloc( FT_Memory  memory, long cur_size, long new_size, void* block ) {

	return jl_realloc(block, new_size);
}


JSBool
ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	JsfontModulePrivate *mpv = (JsfontModulePrivate*)ModulePrivateAlloc(sizeof(JsfontModulePrivate));

	JL_CHK( InitJslibsModule(cx, id) );

	FT_Memory memory; // http://www.freetype.org/freetype2/docs/design/design-4.html
	memory = (FT_Memory)jl_malloc(sizeof(*memory));
	JL_ASSERT_ALLOC( memory );
	memory->user = NULL;
	memory->alloc = JsfontAlloc;
	memory->free = JsfontFree;
	memory->realloc = JsfontRealloc;
	FT_Error status;
	status = FT_New_Library(memory, &mpv->ftLibrary);

	JL_ASSERT( status == 0, E_LIB, "freetype2", E_INIT, E_ERRNO(status) );

	FT_Add_Default_Modules(mpv->ftLibrary);

	mpv->GetFTSymbols = GetFTSymbols;

	JL_CHK( JL_DefineProperty(cx, GetHostObject(cx), "_jsfontModulePrivate", (void*)mpv, false, false) );

	INIT_CLASS(Font);

	return JS_TRUE;
	JL_BAD;
}


JSBool
ModuleRelease(JSContext *cx) {

	return JS_TRUE;
}


void
ModuleFree() {

	JsfontModulePrivate *mpv = (JsfontModulePrivate*)ModulePrivateGet();

	FT_Done_FreeType(mpv->ftLibrary);
	ModulePrivateFree();
}
