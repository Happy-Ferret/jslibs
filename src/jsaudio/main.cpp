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
#include <jslibsModule.cpp>

DECLARE_CLASS(Oal)
DECLARE_CLASS(OalBuffer)
DECLARE_CLASS(OalSource)
DECLARE_CLASS(OalListener)
DECLARE_CLASS(OalEffectSlot);
DECLARE_CLASS(OalEffect);
DECLARE_CLASS(OalFilter);


//static ALCcontext *context = NULL;


/**doc t:header
$MODULE_HEADER
 Support 2D and 3D sound source and listener using OpenAL library.
$FILE_TOC
**/

/**doc t:footer

== more information ==
[http://connect.creativelabs.com/openal/Documentation/oalspecs-specs.pdf OpenAL Specification and Reference]
[http://connect.creativelabs.com/openal/Documentation/OpenAL%201.1%20Specification.pdf OpenAL 1.1 Specification and Reference]

$MODULE_FOOTER
**/

bool
ModuleInit( JSContext *cx, JS::HandleObject obj ) {

	JLDisableThreadNotifications();

	JL_ASSERT( jl::Host::getJLHost( cx ).checkCompatId( JL_HOST_VERSIONID ), E_MODULE, E_NOTCOMPATIBLE, E_HOST );

/* * *
	soubok  02/09/2008 00:12:41           
	- jsaudio: move OpenAL initialization code from main.cpp to Open() and Close() API functions. Testing EFX.
	- jsaudio/source.cpp: Replace position and velocity setters by Position() and Velocity() API function.

	//JL_ASSERT( context == NULL, "Invalid initialization context." );

	ALCcontext *context;
	ALCdevice *device;

	// Initialize the OpenAL library (cf. alutInit)

	// Doc: alcOpenDevice() open the Device specified. Current options are:
	//   "Generic Hardware"
	//   "Generic Software"
	//   "DirectSound3D" (for legacy)
	//   "DirectSound"
	//   "MMSYSTEM"
	// If no device name is specified, we will attempt to use DS3D.
	device = alcOpenDevice ("Generic Software");
	if (device == NULL)
		JL_REPORT_ERROR("ALUT_ERROR_OPEN_DEVICE");
	context = alcCreateContext (device, NULL);
	if (context == NULL) {
		alcCloseDevice (device);
		JL_REPORT_ERROR("ALUT_ERROR_CREATE_CONTEXT");
	}
	if (!alcMakeContextCurrent(context)) {

		alcDestroyContext (context);
		alcCloseDevice (device);
		JL_REPORT_ERROR("ALUT_ERROR_MAKE_CONTEXT_CURRENT");
	}
*/

	INIT_CLASS( OalError );
	INIT_CLASS( Oal );
	INIT_CLASS( OalBuffer );
	INIT_CLASS( OalSource );
	INIT_CLASS( OalListener );
	INIT_CLASS( OalEffectSlot );
	INIT_CLASS( OalEffect );
	INIT_CLASS( OalFilter );

	return true;
	JL_BAD;
}


bool
ModuleRelease(JSContext *cx, void *pv) {

	ALCcontext *context = alcGetCurrentContext();
	if ( context == NULL )
		return true; // already closed

	// cf. alutExit
	ALCdevice *device;
	if (!alcMakeContextCurrent (NULL))
		JL_ERR( E_LIB, E_STR("OpenAL"), E_OPERATION, E_COMMENT("ALUT_ERROR_MAKE_CONTEXT_CURRENT") );
	device = alcGetContextsDevice (context);
	if (alcGetError (device) != ALC_NO_ERROR )
		JL_ERR( E_LIB, E_STR("OpenAL"), E_OPERATION, E_COMMENT("ALUT_ERROR_ALC_ERROR_ON_ENTRY") );
	alcDestroyContext (context);
	if (alcGetError (device) != ALC_NO_ERROR)
		JL_ERR( E_LIB, E_STR("OpenAL"), E_OPERATION, E_COMMENT("ALUT_ERROR_DESTROY_CONTEXT") );
	if (!alcCloseDevice (device))
		JL_ERR( E_LIB, E_STR("OpenAL"), E_OPERATION, E_COMMENT("ALUT_ERROR_CLOSE_DEVICE") );

	return true;
	JL_BAD;
}
