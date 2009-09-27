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

#include "static.h"
//#include "sound.h"

bool _unsafeMode = false;


/**doc t:header
$MODULE_HEADER
 Support wav, aiff, au, voc, sd2, flac, ... sound format using libsndfile
 and ogg vorbis using libogg and libvorbis.
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

DECLARE_CLASS(SoundFileDecoder)
DECLARE_CLASS(OggVorbisDecoder)


EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	_unsafeMode = GetHostPrivate(cx)->unsafeMode;

	JL_CHK( INIT_STATIC() );
//	INIT_CLASS( Sound );

	INIT_CLASS( SoundFileDecoder );
	INIT_CLASS( OggVorbisDecoder );

	return JS_TRUE;
	JL_BAD;
}

EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

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
