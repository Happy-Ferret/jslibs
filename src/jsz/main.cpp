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


#include <zlib.h>
#include "z.h"

DECLARE_CLASS( ZError )
DECLARE_CLASS( Z )
DECLARE_CLASS( ZipFile )
DECLARE_CLASS( ZipFileError )


/**doc t:header
$MODULE_HEADER
 This module manage zlib data compression and decompression. [http://en.wikipedia.org/wiki/Zlib more].
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/
EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	JL_CHK( InitJslibsModule(cx, id)  );

	INIT_CLASS( ZError );
	INIT_CLASS( Z );
	INIT_CLASS( ZipFile );
	INIT_CLASS( ZipFileError );

	return JS_TRUE;
	JL_BAD;
}
