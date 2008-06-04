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


#ifndef _HOST_H_
#define _HOST_H_

#include <jsapi.h>

JSContext* CreateHost(size_t maxMem, size_t maxAlloc);
JSBool InitHost( JSContext *cx, bool unsafeMode, JSFastNative stdOut, JSFastNative stdErr );
void DestroyHost( JSContext *cx );
JSBool ExecuteScript( JSContext *cx, const char *scriptFileName, bool compileOnly, int argc, const char * const * argv, jsval *rval );

#endif // _HOST_H_