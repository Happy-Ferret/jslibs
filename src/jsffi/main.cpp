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

#include "ffi.h"



bool
ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	if ( InitJslibsModule(cx, id)  != true )
		return false;

	Init_JSNI(cx, obj);
	return true;
}

bool
ModuleRelease(JSContext *cx) {

	Release_JSNI(cx);
	Destroy_JSNI();
	return true;
}
