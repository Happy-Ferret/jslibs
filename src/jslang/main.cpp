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

#include "jslang.h"

static const uint32_t moduleId = JL_CAST_CSTR_TO_UINT32("lang");

DECLARE_CLASS( Handle )
DECLARE_CLASS( Blob )
DECLARE_CLASS( Stream )
DECLARE_STATIC();

/**doc t:header
$MODULE_HEADER
 This module contains all common classes used by other jslibs modules.
 $H note
  This module is automatically loaded by jshost and jswinhost. Then LoadModule call is not needed.
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

JSBool jslangModuleInit(JSContext *cx, JSObject *obj) {

	ModulePrivate *mpv = (ModulePrivate*)jl_calloc(sizeof(ModulePrivate), 1);
	JL_CHK( SetModulePrivate(cx, moduleId, mpv) );

	mpv->metaPollSignalEventSem = JLCreateSemaphore(0);

	INIT_CLASS( Handle );
	INIT_CLASS( Blob );
	INIT_CLASS( Stream );
	INIT_STATIC();

	JL_CHK( JL_RegisterNativeClass(cx, JL_CLASS(Blob)) );
	JL_CHK( JL_RegisterNativeClass(cx, JL_CLASS(Handle)) );

	return JS_TRUE;
	JL_BAD;
}


JSBool jslangModuleRelease(JSContext *cx) {

	ModulePrivate *mpv = (ModulePrivate*)GetModulePrivate(cx, moduleId);

	for ( int i = 0; i < COUNTOF(mpv->metaPollThreadInfo); ++i ) {
		
		MetaPollThreadInfo *ti = &mpv->metaPollThreadInfo[i];
		if ( ti->thread != 0 ) {

			ti->isEnd = true;
			JLReleaseSemaphore(ti->start);
			JLFreeSemaphore(&ti->start);
		}
		JLFreeSemaphore(&mpv->metaPollSignalEventSem);
	}
	
	jl_free( mpv );

	return JS_TRUE;
	JL_BAD;
}


void jslangModuleFree() {

}
