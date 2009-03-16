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
#include "static.h"
#include "jsdbgapi.h"

DECLARE_CLASS( Debugger );

extern bool _unsafeMode = false;
extern jl::Queue *scriptFileList = NULL;

/**doc t:header
$MODULE_HEADER
 Various debug tools.
**/

/**doc t:footer
$MODULE_FOOTER
**/


void NewScriptHook(JSContext *cx, const char *filename, uintN lineno, JSScript *script, JSFunction *fun, void *callerdata) {

//	printf( "add - %s:%d - %s - %d - %p\n", filename, lineno, fun ? JS_GetFunctionName(fun):"", script->staticDepth, script );

	jl::QueueCell *it;
	jl::Queue *scriptList = NULL;

	// find the right script filename
	for ( it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

		scriptList = (jl::Queue*)jl::QueueGetData(it);
		JSScript *s = (JSScript*)jl::QueueGetData(jl::QueueBegin(scriptList));
		if ( strcmp(filename, s->filename) == 0 )
			break;
	}

	if ( it == NULL ) { // if not found, create one
	
		scriptList = jl::QueueConstruct();
		jl::QueueUnshift(scriptFileList, scriptList);
		jl::QueuePush(scriptList, script);
	} else { // add the script at the right place in the queue

		for ( it = jl::QueueBegin(scriptList); it; it = jl::QueueNext(it) ) {

			JSScript *s = (JSScript*)jl::QueueGetData(it);
			if ( script->staticDepth >= s->staticDepth ) {

				jl::QueueInsertCell(scriptList, it, script);
				break;
			}
		}
		if ( it == NULL )
			jl::QueueUnshift(scriptList, script);
	}
}


void DestroyScriptHook(JSContext *cx, JSScript *script, void *callerdata) {

//	printf( "del - %s:%d - ? - %d - %p\n", script->filename, script->lineno, script->staticDepth, script );

	jl::QueueCell *it, *it1;
	jl::Queue *scriptList = NULL;

	// find the right script filename
	for ( it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

		scriptList = (jl::Queue*)jl::QueueGetData(it);
		for ( it1 = jl::QueueBegin(scriptList); it1; it1 = jl::QueueNext(it1) ) {

			JSScript *s = (JSScript*)jl::QueueGetData(it1);
			if ( s == script ) {

				jl::QueueRemoveCell(scriptList, it1);
				if ( jl::QueueIsEmpty(scriptList) )
					jl::QueueRemoveCell(scriptFileList, it);
				return;
			}
		}
	}
}


EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	_unsafeMode = GetHostPrivate(cx)->unsafeMode;

	scriptFileList = jl::QueueConstruct();
	JS_SetNewScriptHookProc(JS_GetRuntime(cx), NewScriptHook, NULL);
	JS_SetDestroyScriptHookProc(JS_GetRuntime(cx), DestroyScriptHook, NULL);

	INIT_STATIC();
	INIT_CLASS( Debugger );

	return JS_TRUE;
	JL_BAD;
}

EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

	JS_SetNewScriptHookProc(JS_GetRuntime(cx), NULL, NULL);
	JS_SetDestroyScriptHookProc(JS_GetRuntime(cx), NULL, NULL);

	for ( jl::QueueCell *it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

		jl::Queue *scriptList = (jl::Queue*)jl::QueueGetData(it);
		jl::QueueDestruct(scriptList);
	}
	jl::QueueDestruct(scriptFileList);

	return JS_FALSE;
}

EXTERN_C DLLEXPORT void ModuleFree() {
}

#ifdef XP_WIN
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	if ( fdwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hinstDLL);
	return TRUE;
}
#endif // XP_WIN
