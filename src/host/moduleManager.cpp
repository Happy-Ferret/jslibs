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

#ifdef XP_UNIX
#include <dlfcn.h>
#endif

#include "../common/jsNames.h"

#include "moduleManager.h"

#ifdef XP_UNIX
	typedef void *LibraryHandler;

	LibraryHandler DynamicLibraryOpen( const char *filename ) {
		dlerror();
		return dlopen( filename, RTLD_NOW );
	}

	bool DynamicLibraryClose( LibraryHandler lh ) {
		dlerror();
		return dlclose( lh ) == 0;
	}

	void *DynamicLibrarySymbol( LibraryHandler lh, const char *filename ) {
		dlerror();
		return dlsym( lh, filename );
	}
#endif

#ifdef XP_WIN
	typedef HMODULE LibraryHandler;
	LibraryHandler DynamicLibraryOpen( const char *filename ) {

		return LoadLibrary( filename );
	}

	bool DynamicLibraryClose( LibraryHandler lh ) {

		return FreeLibrary( lh ) == TRUE;
	}

	void *DynamicLibrarySymbol( LibraryHandler lh, const char *filename ) {

		return GetProcAddress( lh, filename );
	}

#endif


static LibraryHandler _moduleList[128] = {NULL}; // do not manage the module list dynamicaly, we allow a maximum of 32 modules

/* Note:
	Current the only way I found to detect if a module is already loaded is to load it and to compare its handler to already-opened modules.
	other solutions:
	- use the full motule path as unique Id
	- on linux, use the INode / on windows, use file index from GetFileInformationByHandle function / on macosx ???
*/
/*
bool ModuleIsLoaded( const char *fileName ) {

	if ( fileName == NULL || *fileName == '\0' )
		return false;

	LibraryHandler module = DynamicLibraryOpen(fileName);
	if ( module == NULL )
		return false;

	int MaxModuleSlot = sizeof(_moduleList)/sizeof(*_moduleList);
	int i;
	for ( i = 0; i < MaxModuleSlot; ++i )
		if ( _moduleList[i] == module )
			return true; // already loaded

	DynamicLibraryClose(module);
	return false;
}
*/


/* Note:
	ModuleLoad should return three values: 
	- ModuleId
	- (bool) module is already loaded
	- (bool) failure status
*/
ModuleId ModuleLoad( const char *fileName, JSContext *cx, JSObject *obj ) {

	if ( fileName == NULL || *fileName == '\0' )
		return 0;

	LibraryHandler module = DynamicLibraryOpen(fileName);
	if ( module == NULL )
		return 0;

	if ( sizeof(ModuleId) > sizeof(LibraryHandler) )
		return 0;

	int i;
	int MaxModuleSlot = COUNTOF(_moduleList);

	for ( i = 0; i < MaxModuleSlot; ++i )
		if ( _moduleList[i] == module ) {
			
			DynamicLibraryClose(module); // as it is aleready open, we can close this one (because reference counter?)
			return (ModuleId)module; // already loaded
		}
	
	for ( i = 0; _moduleList[i] != NULL && i < MaxModuleSlot; ++i ); // find a free module slot
	if ( i == MaxModuleSlot )
		return 0;

	ModuleInitFunction moduleInit = (ModuleInitFunction)DynamicLibrarySymbol( module, NAME_MODULE_INIT );
	if ( moduleInit == NULL ) {

		DynamicLibraryClose(module);
		return 0;
	}

	_moduleList[i] = module;

	if ( moduleInit(cx, obj) == JS_FALSE ) {

		DynamicLibraryClose(module);
		_moduleList[i] = NULL;
		return 0;
	}
	return (ModuleId)module;
}


bool ModuleIsUnloadable( ModuleId id ) {

	LibraryHandler module = (LibraryHandler)id;
	if ( module == NULL )
		return false;
	ModuleReleaseFunction moduleRelease = (ModuleReleaseFunction)DynamicLibrarySymbol( module, NAME_MODULE_RELEASE );
	if ( moduleRelease == NULL )
		return false;
	return true;
}


bool ModuleUnload( ModuleId id, JSContext *cx ) {

//	J_S_ASSERT( id != 0, "Invalid module id." );

	return false;

//	jsdouble dVal;
//	J_CHK( JS_ValueToNumber(cx, argv[0], &dVal) );

/*
	jsdouble dVal;
	J_CHK( JS_ValueToNumber(cx, argv[0], &dVal) );
	HMODULE module = (HMODULE)(unsigned int)dVal;

	for ( int i = sizeof(_moduleList) / sizeof(*_moduleList) - 1; i >= 0; --i ) // beware: 'i' must be signed // start from the end
		if ( _moduleList[i] == module ) {

			_moduleList[i] = NULL;

			typedef JSBool (*ModuleReleaseFunction)(JSContext *cx);
			ModuleReleaseFunction moduleRelease = (ModuleReleaseFunction)::GetProcAddress( module, NAME_MODULE_RELEASE );
			if ( moduleRelease != NULL )
				moduleRelease(cx);

			typedef void (*ModuleFreeFunction)(void);
			ModuleFreeFunction moduleFree = (ModuleFreeFunction)::GetProcAddress( _moduleList[i], NAME_MODULE_FREE );
			if ( moduleFree != NULL )
				moduleFree();

			::FreeLibrary(module);
		}

*/
	return true;
}


void ModuleReleaseAll( JSContext *cx ) {

	for ( int i = sizeof(_moduleList) / sizeof(*_moduleList) - 1; i >= 0; --i ) // beware: 'i' must be signed // start from the end
		if ( _moduleList[i] != NULL ) {

			ModuleReleaseFunction moduleRelease = (ModuleReleaseFunction)DynamicLibrarySymbol( _moduleList[i], NAME_MODULE_RELEASE );
			if ( moduleRelease != NULL )
				moduleRelease(cx);
		}
}

void ModuleFreeAll() {

	for ( int i = sizeof(_moduleList) / sizeof(*_moduleList) - 1; i >= 0; --i ) // beware: 'i' must be signed // start from the end
		if ( _moduleList[i] != NULL ) {

			ModuleFreeFunction moduleFree = (ModuleFreeFunction)DynamicLibrarySymbol( _moduleList[i], NAME_MODULE_FREE );
			if ( moduleFree != NULL )
				moduleFree();
			DynamicLibraryClose(_moduleList[i]);
		}
}


/*

Program Library HOWTO
	http://www.faqs.org/docs/Linux-HOWTO/Program-Library-HOWTO.html

Dynamic-Link Library Search Order
	http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/dynamic-link_library_search_order.asp

Dynamic-Link Library Redirection
	http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/dynamic_link_library_redirection.asp



*/
