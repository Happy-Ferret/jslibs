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


DECLARE_STATIC()
DECLARE_CLASS( Template )


bool
ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	JL_CHK( InitJslibsModule(cx, id) );

	//ModulePrivate *mpv;
	//mpv = (ModulePrivate*)jl_malloc( sizeof(ModulePrivate) );
	//JL_ASSERT_ALLOC(mpv);
	//JL_CHKM( JL_SetModulePrivate(cx, _moduleId, mpv), "Module id already in use." );

	INIT_STATIC();
	INIT_CLASS( Template );

	return true;
	JL_BAD;
}

bool
ModuleRelease(JSContext *cx) {
	
	JL_IGNORE(cx);
	//jl_free(JL_GetModulePrivate(cx, _moduleId));

	return true;
}

//void
//ModuleFree() {
//}
