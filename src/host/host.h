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

#define NAME_MODULE_INIT "ModuleInit"
#define NAME_MODULE_RELEASE "ModuleRelease"
#define NAME_MODULE_FREE "ModuleFree"

#define NAME_GLOBAL_CLASS "Global"

#define NAME_GLOBAL_FUNCTION_LOAD_MODULE "loadModule"
#define NAME_GLOBAL_FUNCTION_UNLOAD_MODULE "unloadModule"

typedef int (*HostInput)( void *privateData, char *buffer, size_t bufferLength );
typedef int (*HostOutput)( void *privateData, const char *buffer, size_t length );

JSContext* CreateHost( uint32_t maxMem, uint32_t maxAlloc, uint32_t maybeGCInterval );
JSBool InitHost( JSContext *cx, bool unsafeMode, HostInput stdIn, HostOutput stdOut, HostOutput stdErr, void* userPrivateData );
JSBool DestroyHost( JSContext *cx, bool skipCleanup );
JSBool ExecuteScriptText( JSContext *cx, const char *scriptFileName, bool compileOnly, jsval *rval );
JSBool ExecuteScriptFileName( JSContext *cx, const char *scriptFileName, bool compileOnly, jsval *rval );
JSBool ExecuteBootstrapScript( JSContext *cx, void *xdrScript, uint32_t xdrScriptLength, jsval *rval );

bool InitializeMemoryManager( jl_malloc_t *malloc, jl_calloc_t *calloc, jl_memalign_t *memalign, jl_realloc_t *realloc, jl_msize_t *msize, jl_free_t *free );
JSBool MemoryManagerEnableGCEvent( JSContext *cx );
JSBool MemoryManagerDisableGCEvent( JSContext *cx );
bool FinalizeMemoryManager( bool freeQueue, jl_malloc_t *malloc, jl_calloc_t *calloc, jl_memalign_t *memalign, jl_realloc_t *realloc, jl_msize_t *msize, jl_free_t *free );
