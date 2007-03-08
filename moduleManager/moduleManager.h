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

#ifdef WIN32
	#define DLL_EXT ".dll"
#else
	#define DLL_EXT ".so"
#endif

#include "../common/jsNames.h"

typedef unsigned int ModuleId;

typedef JSBool (*ModuleInitFunction)(JSContext *, JSObject *);
typedef JSBool (*ModuleReleaseFunction)(JSContext *cx);
typedef void (*ModuleFreeFunction)(void);

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj);
extern "C" __declspec(dllexport) JSBool ModuleRelease(JSContext *cx);
extern "C" __declspec(dllexport) void ModuleFree();

ModuleId ModuleLoad( const char *fileName, JSContext *cx, JSObject *obj );
bool ModuleIsUnloadable( ModuleId id );
bool ModuleUnload( ModuleId id, JSContext *cx );


void ModuleReleaseAll( JSContext *cx );
void ModuleFreeAll();
