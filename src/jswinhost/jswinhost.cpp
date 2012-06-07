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

// set stack to 2MB:
#pragma comment (linker, "/STACK:0x200000")

#include <jslibsModule.cpp>

#define NO_NED_NAMESPACE
#define NO_MALLINFO 1
#include "../../libs/nedmalloc/nedmalloc.h"

static volatile bool disabledFree = false;

void nedfree_handlenull(void *mem) {
	
	if ( mem != NULL && !disabledFree )
		nedfree(mem);
}

size_t nedblksize_msize(void *mem) {

	return nedblksize(0, mem);
}


static unsigned char embeddedBootstrapScript[] =
	#include "embeddedBootstrapScript.js.xdr.cres"
;


int HostStderr( void *, const char *buffer, size_t length ) {

	char *tmp = (char*)alloca(length+1);
	jl::memcpy(tmp, buffer, length);
	tmp[length] = '\0';
	::OutputDebugString(tmp);
	return 1;
}


// to be used in the main() function only
#define HOST_MAIN_ASSERT( condition, errorMessage ) \
	JL_MACRO_BEGIN \
		if ( !(condition) ) { \
			HostStderr(NULL, errorMessage, strlen(errorMessage)); \
			goto bad; \
		} \
	JL_MACRO_END


int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR, int ) {

	HANDLE heap = GetProcessHeap();
	ULONG enable = 2;
	HeapSetInformation(heap, HeapCompatibilityInformation, &enable, sizeof(enable));

	JSContext *cx = NULL;

	errno_t err;
	CHAR moduleName[PATH_MAX], scriptName[PATH_MAX], mutexName[PATH_MAX];
	DWORD moduleNameLen = GetModuleFileName(hInstance, moduleName, sizeof(moduleName));
	HOST_MAIN_ASSERT( moduleNameLen > 0, "Invalid module filename." );

	//doc: If you need to detect whether another instance already exists, create a uniquely named mutex using the CreateMutex function. 
	//CreateMutex will succeed even if the mutex already exists, but the function will return ERROR_ALREADY_EXISTS. 
	//This indicates that another instance of your application exists, because it created the mutex first.

	// (TBD) use file index as mutexName. note: If the file is on an NTFS volume, you can get a unique 64 bit identifier for it with GetFileInformationByHandle.  The 64 bit identifier is the "file index". 
	
	bool isFirstInstance;
	err = strncpy_s(mutexName, sizeof(mutexName), moduleName, moduleNameLen);
	HOST_MAIN_ASSERT( err == 0, "Buffer overflow." );
	
	// normalize the mutex name
	char *pos;
	while ( (pos = strchr(mutexName, '\\')) != 0 )
		*pos = '/';
	SetLastError(0);
	HANDLE instanceCheckMutex = ::CreateMutex(NULL, TRUE, mutexName); // see Global\\ and Local\\ prefixes for mutex name.
	switch ( GetLastError() ) {
		case ERROR_SUCCESS:
			isFirstInstance = true;
			break;
		case ERROR_ALREADY_EXISTS:
			isFirstInstance = false;
			break;
		default: {
			char message[1024];
			JLLastSysetmErrorMessage(message, sizeof(message));
			HOST_MAIN_ASSERT( false,  message );
		}
	}

	jl_malloc = nedmalloc;
	jl_calloc = nedcalloc;
	jl_memalign = nedmemalign;
	jl_realloc = nedrealloc;
	jl_msize = nedblksize_msize;
	jl_free = nedfree_handlenull;
/*
	jl_malloc = malloc;
	jl_calloc = calloc;
	jl_memalign = memalign;
	jl_realloc = realloc;
	jl_msize = msize;
	jl_free = free;
*/

	JL_CHK( InitializeMemoryManager(&jl_malloc, &jl_calloc, &jl_memalign, &jl_realloc, &jl_msize, &jl_free) );

	cx = CreateHost((uint32_t)-1, (uint32_t)-1, 30000); // 30 seconds
	JL_CHK( cx != NULL );

	MemoryManagerEnableGCEvent(cx);

	HostPrivate *hpv = JL_GetHostPrivate(cx);

	// custom memory allocators are transfered to the modules through the HostPrivate structure:
	hpv->alloc.malloc = jl_malloc;
	hpv->alloc.calloc = jl_calloc;
	hpv->alloc.memalign = jl_memalign;
	hpv->alloc.realloc = jl_realloc;
	hpv->alloc.msize = jl_msize;
	hpv->alloc.free = jl_free;


	JL_CHK( InitHost(cx, true, NULL, NULL, HostStderr, NULL) );
	CHAR moduleFileName[PATH_MAX];
	strcpy(moduleFileName, moduleName);
	char *name = strrchr(moduleFileName, '\\');
	JL_CHK( name );
	*name = '\0';
	name++;


	err = strncpy_s(scriptName, sizeof(scriptName), moduleName, moduleNameLen);
	JL_ASSERT( err == 0, E_LIB, E_INTERNAL );
//	DWORD scriptNameLen = GetModuleFileName(hInstance, scriptName, sizeof(scriptName));
	char *dotPos = strrchr(scriptName, '.');
	JL_CHK( dotPos );
	*dotPos = '\0';
	err = strcat_s(scriptName, sizeof(scriptName), ".js");
	JL_ASSERT( err == 0, E_LIB, E_INTERNAL );

	JSObject *hostObj = JL_GetHostPrivate(cx)->hostObject;

	JL_CHK( JL_NativeToProperty(cx, hostObj, JLID(cx, path), moduleFileName) );
	JL_CHK( JL_NativeToProperty(cx, hostObj, JLID(cx, name), name) );
	JL_CHK( JL_NativeToProperty(cx, hostObj, JLID(cx, isFirstInstance), isFirstInstance) );

	jsval arguments;
	JL_CHK( JL_NativeVectorToJsval(cx, __argv, __argc, &arguments) );
	JL_CHK( JS_SetPropertyById(cx, hostObj, JLID(cx, arguments), &arguments) );

	jsval rval;

	if ( sizeof(embeddedBootstrapScript)-1 > 0 ) {

		uint32_t prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) & ~JSOPTION_DONT_REPORT_UNCAUGHT); // report uncautch exceptions !
		JSScript *script = JS_DecodeScript(cx, embeddedBootstrapScript, sizeof(embeddedBootstrapScript)-1, NULL, NULL); // -1 because sizeof("") == 1
		JL_CHK( script );
		JL_CHK( JS_ExecuteScript(cx, JL_GetGlobal(cx), script, &rval) );
		JS_SetOptions(cx, prevOpt);
	}

	if ( ExecuteScriptFileName(cx, scriptName, false, &rval) != JS_TRUE )
		if ( JL_IsExceptionPending(cx) )
			JS_ReportPendingException(cx); // see JSOPTION_DONT_REPORT_UNCAUGHT option.

	disabledFree = true;
	MemoryManagerDisableGCEvent(cx);
	FinalizeMemoryManager(!disabledFree, &jl_malloc, &jl_calloc, &jl_memalign, &jl_realloc, &jl_msize, &jl_free);

	JS_SetGCCallback(JL_GetRuntime(cx), NULL);
	DestroyHost(cx, disabledFree);
	cx = NULL;
	JS_ShutDown();

	CloseHandle( instanceCheckMutex ); //ReleaseMutex
	return 0;

bad:
	if ( cx ) {

		disabledFree = true;
		JS_SetGCCallback(JL_GetRuntime(cx), NULL);
		DestroyHost(cx, disabledFree);
	}
	JS_ShutDown();

	CloseHandle( instanceCheckMutex ); //ReleaseMutex
	return -1;
}

/**doc
#summary jswinhost executable
#labels doc

= jswinhost executable =
 [http://code.google.com/p/jslibs/ home] *>* [JSLibs] *>* [jswinhost] - [http://jslibs.googlecode.com/svn/trunk/jswinhost/jswinhost.cpp http://jslibs.googlecode.com/svn/wiki/source.png]

=== Description ===
 jswinhost (javascript windows host) is a small executable file that run javascript programs under a windows environment.
 The main difference with jshost is the jswinhost does not create any console windows.

=== Methods ===

 * status *loadModule*( moduleFileName )
  see [jshost]

=== Properties ===

 * $OBJ *global* $READONLY
  is the global object.

 * $ARRAY *arguments* $READONLY
  is the host path [0] and whole command line [1].

 * $BOOL *isfirstinstance* $READONLY
  is true if the current instance is the first one. This can help to avoid jswinhost to be run twice at the same time.

=== Host object ===
 see [jshost]

=== Remarks ===
 There is no way to specify the script to execute using the command line. You have to create a .js file using the name of the host.
 By default, jswinhost.exe is the name of the host and jswinhost.js is the script the host execute.
 
Because jwinshost do not use a console window, errors and printed messages will not be displayed.

However, you can write your own output system:
{{{
loadModule('jswinshell');
host.stdout = new Console().Write;
host.stderr = MessageBox;
loadModule('jsstd');
print('toto');
hkqjsfhkqsdu_error();
}}}

== Embedding JS scripts in your jswinhost binary ==
 see [jshost]

**/
