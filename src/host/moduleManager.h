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



typedef unsigned long int ModuleId;

typedef JSBool (*ModuleInitFunction)(JSContext *, JSObject *);
typedef JSBool (*ModuleReleaseFunction)(JSContext *cx);
typedef void (*ModuleFreeFunction)(void);

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj);
EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx);
EXTERN_C DLLEXPORT void ModuleFree();

ModuleId ModuleLoad( const char *fileName, JSContext *cx, JSObject *obj );
bool ModuleIsUnloadable( ModuleId id );
bool ModuleUnload( ModuleId id, JSContext *cx );
//bool ModuleIsLoaded( const char *fileName );


void ModuleReleaseAll( JSContext *cx );
void ModuleFreeAll();