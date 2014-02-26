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
#include <jlhelper.cpp> // (TBD) create a lib
#include "jslang.h"


DECLARE_CLASS( Handle )
DECLARE_CLASS( Stream )
DECLARE_CLASS( Serializer )
DECLARE_CLASS( Unserializer )
DECLARE_STATIC()

/**doc t:header
$MODULE_HEADER
 This module contains all common classes used by other jslibs modules.
 $H note
  This module is automatically loaded by jshost and jswinhost. Then loadModule call is not needed.
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/


bool jslangModuleInit(JSContext *cx, JS::HandleObject obj) {

	ModulePrivate *mpv = (ModulePrivate*)jl_calloc(sizeof(ModulePrivate), 1);

	jl::Host::getHost(cx).moduleManager().modulePrivate(jslangModuleId) = mpv;

	// JL_CHKM( , E_MODULE, E_INIT );
	
	mpv->processEventSignalEventSem = JLSemaphoreCreate(0);

	INIT_CLASS( Handle );
	INIT_CLASS( Stream );
	INIT_CLASS( Serializer );
	INIT_CLASS( Unserializer );
	INIT_STATIC();

	return true;
	JL_BAD;
}


bool jslangModuleRelease(JSContext *cx) {

	ModulePrivate *mpv = (ModulePrivate*)jl::Host::getHost(cx).moduleManager().modulePrivate(jslangModuleId);
	if ( !mpv )
		return false;

	for ( size_t i = 0; i < COUNTOF(mpv->processEventThreadInfo); ++i ) {

		ProcessEventThreadInfo *ti = &mpv->processEventThreadInfo[i];
		if ( ti->thread != 0 ) {

			ti->isEnd = true;
			ASSERT( ti->startSem );
			JLSemaphoreRelease(ti->startSem);
			JLThreadWait(ti->thread);
		}

		if ( ti->startSem != 0 ) {

			JLSemaphoreFree(&ti->startSem);
		}
	}
	JLSemaphoreFree(&mpv->processEventSignalEventSem);

	jl_free( mpv );

	return true;
}


void jslangModuleFree(bool skipCleanup) {
}
