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

//#include <jsapi.h>

#define NAME_MODULE_INIT "ModuleInit"
#define NAME_MODULE_RELEASE "ModuleRelease"
#define NAME_MODULE_FREE "ModuleFree"

#define NAME_GLOBAL_CLASS "Global"

#define NAME_GLOBAL_FUNCTION_LOAD_MODULE "LoadModule"
#define NAME_GLOBAL_FUNCTION_UNLOAD_MODULE "UnloadModule"

typedef int (*HostOutput)( void *privateData, const char *buffer, unsigned int length );

JSContext* CreateHost( size_t maxMem, size_t maxAlloc, size_t maybeGCInterval );
JSBool InitHost( JSContext *cx, bool unsafeMode, HostOutput stdOut, HostOutput stdErr, void* userPrivateData );
JSBool DestroyHost( JSContext *cx );
JSBool ExecuteScript( JSContext *cx, const char *scriptFileName, bool compileOnly, int argc, const char * const * argv, jsval *rval );
JSBool ExecuteScriptFileName( JSContext *cx, const char *scriptFileName, bool compileOnly, int argc, const char * const * argv, jsval *rval );
JSBool ExecuteBootstrapScript( JSContext *cx, void *xdrScript, unsigned int xdrScriptLength );

bool InitializeMemoryManager( jl_malloc_t *malloc, jl_calloc_t *calloc, jl_memalign_t *memalign, jl_realloc_t *realloc, jl_msize_t *msize, jl_free_t *free );
JSBool MemoryManagerEnableGCEvent( JSContext *cx );
JSBool MemoryManagerDisableGCEvent( JSContext *cx );
bool FinalizeMemoryManager( bool freeQueue, jl_malloc_t *malloc, jl_calloc_t *calloc, jl_memalign_t *memalign, jl_realloc_t *realloc, jl_msize_t *msize, jl_free_t *free );

#endif // _HOST_H_
