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

DECLARE_CLASS( Jabber )
DECLARE_STATIC()

/**doc t:header
$MODULE_HEADER
 jsjabber is a wrapper to the gloox library.$LF
 gloox is a full-featured Jabber/XMPP client library.
 $FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/


bool
ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	JL_CHK( InitJslibsModule(cx, id)  );

	INIT_STATIC();
	INIT_CLASS( Jabber );

	return true;
	JL_BAD;
}
