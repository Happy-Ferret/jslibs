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

#pragma once

#include "jlplatform.h"

extern DLLLOCAL uint32_t _moduleId;

struct JSContext;
//class JSObject;

typedef bool (*ModuleInitFunction)(JSContext *, JS::HandleObject, uint32_t id);
typedef bool (*ModuleReleaseFunction)(JSContext *);
typedef void (*ModuleFreeFunction)();

EXTERN_C DLLEXPORT bool ModuleInit(JSContext *cx, JS::HandleObject obj, uint32_t id);
EXTERN_C DLLEXPORT bool ModuleRelease(JSContext *cx);
EXTERN_C DLLEXPORT void ModuleFree();

bool InitJslibsModule( JSContext *cx );
