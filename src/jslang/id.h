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

JSObject* CreateId( JSContext *cx, u_int32_t name, size_t length );
bool IsIdType( JSContext *cx, JSObject *idObj, u_int32_t idType );
void** IdPtr( JSContext *cx, JSObject *idObj );

/*

	JSObject *o = CreateId(cx, 'VBO', 2);

	IsIdType(cx, o, 'VBO');

	IdPtr(cx, o)[0] = "test0";
	IdPtr(cx, o)[1] = "test1";

	char *s = (char*)IdPtr(cx, o)[1];

*/
