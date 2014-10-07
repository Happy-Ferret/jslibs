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
#include <jslibsModule.h>

// used by ogg_static and libsndfile
void* jl_malloc_fct(size_t n) { return jl_malloc(n); }
void* jl_realloc_fct(void *p, size_t n) { return jl_realloc(p, n); }
void* jl_calloc_fct(size_t n, size_t s) { return jl_calloc(n, s); }
void jl_free_fct(void *p) { jl_free(p); }


/**doc t:header
$MODULE_HEADER
 Support wav, aiff, au, voc, sd2, flac, ... sound format using libsndfile
 and ogg vorbis using libogg and libvorbis.
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

DECLARE_STATIC()
//DECLARE_CLASS(Sound)
DECLARE_CLASS(SoundFileDecoder)
DECLARE_CLASS(OggVorbisDecoder)
DECLARE_CLASS(VorbisEncoder)


bool
ModuleInit(JSContext *cx, JS::HandleObject obj) {

	JL_ASSERT( jl::Host::getJLHost(cx)->checkCompatId(JL_HOST_VERSIONID), E_MODULE, E_NOTCOMPATIBLE, E_HOST );


	INIT_STATIC();
//	INIT_CLASS( Sound );
	INIT_CLASS( SoundFileDecoder );
	INIT_CLASS( OggVorbisDecoder );
	INIT_CLASS( VorbisEncoder );

	return true;
	JL_BAD;
}
