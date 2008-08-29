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

#include <AL/al.h>
#include <AL/alc.h>

#include "oal.h"

static ALCcontext *context;

static bool _defaultUnsafeMode = false;
extern bool *_pUnsafeMode = &_defaultUnsafeMode;

/**doc t:header
$MODULE_HEADER
 Support 2D and 3D sound source and listener using OpenAL library.
**/

/**doc t:footer
$MODULE_FOOTER
**/

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	jsval unsafeModePtrVal;
	J_CHK( GetConfigurationValue(cx, NAME_CONFIGURATION_UNSAFE_MODE_PTR, &unsafeModePtrVal) );
	if ( unsafeModePtrVal != JSVAL_VOID )
		_pUnsafeMode = (bool*)JSVAL_TO_PRIVATE(unsafeModePtrVal);

// Initialize the OpenAL library (cf. alutInit)
	ALCdevice *device;

	device = alcOpenDevice (NULL);
	if (device == NULL)
		J_REPORT_ERROR("ALUT_ERROR_OPEN_DEVICE");

	context = alcCreateContext (device, NULL);
	if (context == NULL) {

		alcCloseDevice (device);
		J_REPORT_ERROR("ALUT_ERROR_CREATE_CONTEXT");
	}

	if (!alcMakeContextCurrent(context)) {

		alcDestroyContext (context);
		alcCloseDevice (device);
		J_REPORT_ERROR("ALUT_ERROR_MAKE_CONTEXT_CURRENT");
	}

	J_CHK( INIT_CLASS( Oal ) );

	return JS_TRUE;
}


EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

	// cf. alutExit
	ALCdevice *device;

	if (!alcMakeContextCurrent (NULL))
		J_REPORT_ERROR("ALUT_ERROR_MAKE_CONTEXT_CURRENT");

	device = alcGetContextsDevice (context);
	if (alcGetError (device) != ALC_NO_ERROR )
		J_REPORT_ERROR("ALUT_ERROR_ALC_ERROR_ON_ENTRY");

	alcDestroyContext (context);
	if (alcGetError (device) != ALC_NO_ERROR)
		J_REPORT_ERROR("ALUT_ERROR_DESTROY_CONTEXT");

	if (!alcCloseDevice (device))
		J_REPORT_ERROR("ALUT_ERROR_CLOSE_DEVICE");

	return JS_FALSE;
}


EXTERN_C DLLEXPORT void ModuleFree() {

}


#ifdef XP_WIN
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	if ( fdwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hinstDLL);
	return TRUE;
}
#endif // XP_WIN
