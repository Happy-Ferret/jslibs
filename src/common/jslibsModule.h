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

#ifndef _JSLIBSMODULE_H_
#define _JSLIBSMODULE_H_

#include "jlplatform.h"
#include <sys/types.h>
#include <jstypes.h>

EXTERN_C void* jl_malloc_fct( size_t );
EXTERN_C void* jl_calloc_fct( size_t, size_t );
EXTERN_C void* jl_memalign_fct( size_t  size_t );
EXTERN_C void* jl_realloc_fct( void*, size_t );
EXTERN_C size_t jl_msize_fct( void* );
EXTERN_C void jl_free_fct( void* );

extern bool _unsafeMode;

extern uint32_t _moduleId;

typedef struct JSContext JSContext;
typedef struct JSObject JSObject;

JSBool InitJslibsModule( JSContext *cx );

typedef JSBool (*ModuleInitFunction)(JSContext *, JSObject *, uint32_t id);
typedef JSBool (*ModuleReleaseFunction)(JSContext *);
typedef void (*ModuleFreeFunction)();

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj, uint32_t id);
EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx);
EXTERN_C DLLEXPORT void ModuleFree();

#endif // _JSLIBSMODULE_H_
