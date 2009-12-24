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

JLClassSpec *jlStaticSpec; // see DECLARE_STATIC() macro.

#define MAX_WXJS_MODULES 32

bool _unsafeMode = false;


#define wxString char*
#define wxT(S) (S)

// from [wxjs]/wxjs/src/engine/module.cpp:

static wxString WXJS_INIT_CLASS = wxT("wxJS_InitClass");
static wxString WXJS_DESTROY = wxT("wxJS_Destroy");

typedef bool (*WXJS_INIT_PROC)(JSContext *cx, JSObject *obj);
typedef void (*WXJS_DESTROY_PROC)();
typedef void (*WXJS_ERROR_PROC)(JSErrorReporter er);


bool _defaultUnsafeMode = false;
bool *_pUnsafeMode = &_defaultUnsafeMode;

static HMODULE _moduleList[MAX_WXJS_MODULES] = { NULL };

DEFINE_FUNCTION( LoadWXJSModule ) {

	JL_S_ASSERT_ARG_MIN(1);
	const char *fileName;
	JL_CHK( JsvalToString(cx, &argv[0], &fileName) );
	char libFileName[PATH_MAX];
	strcpy( libFileName, fileName );
	strcat( libFileName, DLL_EXT );
	HMODULE module = ::LoadLibrary(libFileName);
	JL_S_ASSERT( module != NULL, "Unable to load the library %s (error:%d).", libFileName, GetLastError() );
	int i;
	for ( i = 0; _moduleList[i] != NULL; ++i ); // find a free module slot
	JL_S_ASSERT( i < 32, "unable to load more libraries" );
	_moduleList[i] = module;

	WXJS_INIT_PROC moduleInit = (WXJS_INIT_PROC)::GetProcAddress( module, WXJS_INIT_CLASS );

	JL_S_ASSERT( moduleInit != NULL, "Module initialization function not found in %s.", libFileName );
	*rval = moduleInit( cx, obj ) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( UnloadWXJSModule ) {

	return JS_TRUE;
}

CONFIGURE_STATIC
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( LoadWXJSModule )
		FUNCTION( UnloadWXJSModule )
	END_STATIC_FUNCTION_SPEC
END_STATIC




EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	_unsafeMode = GetHostPrivate(cx)->unsafeMode;

	INIT_STATIC();
	return JS_TRUE;
	JL_BAD;
}

EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

// free used modules
	typedef void (*ModuleFreeFunction)(void);
	for ( int i = sizeof(_moduleList) / sizeof(*_moduleList) - 1; i >= 0; --i ) // beware: 'i' must be signed
		if ( _moduleList[i] != NULL ) {

			WXJS_DESTROY_PROC moduleFree = (WXJS_DESTROY_PROC)::GetProcAddress( _moduleList[i], WXJS_DESTROY );
			if ( moduleFree != NULL )
				moduleFree();
			::FreeLibrary(_moduleList[i]);
			_moduleList[i] = NULL;
		}

	return JS_TRUE;
}

#ifdef XP_WIN
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	if ( fdwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hinstDLL);
	return TRUE;
}
#endif // XP_WIN
