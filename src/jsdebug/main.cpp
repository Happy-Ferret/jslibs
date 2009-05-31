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
JSBool GetScriptLocation( JSContext *cx, jsval *val, uintN lineno, JSScript **script, jsbytecode **pc );

/**doc t:header
$MODULE_HEADER
 Various debug tools.
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

//////////////////////
// tools functions

void NewScriptHook(JSContext *cx, const char *filename, uintN lineno, JSScript *script, JSFunction *fun, void *callerdata) {

// (TBD) do we protect new file-based scripts against GC to allow later debugging them ?

//	printf( "add - %s:%d-%d - %s - %d - %p\n", filename, lineno, lineno+JS_GetScriptLineExtent(cx, script), fun ? JS_GetFunctionName(fun):"", script->staticLevel, script );

	jl::QueueCell *it;
	jl::Queue *scriptList = NULL;

	// find the right script filename
	for ( it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

		scriptList = (jl::Queue*)jl::QueueGetData(it);
		JSScript *s = (JSScript*)jl::QueueGetData(jl::QueueBegin(scriptList));
		if ( s == script )
			return; // (TBD) already added, check how it is possible.
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
			if ( script->staticLevel >= s->staticLevel ) {

				jl::QueueInsertCell(scriptList, it, script);
				return;
			}
		}
		jl::QueuePush(scriptList, script);
	}

/*
	{
	printf("<DUMP>\n");
	jl::QueueCell *it, *it1; jl::Queue *scriptList = NULL;
	for ( it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

		scriptList = (jl::Queue*)jl::QueueGetData(it);
		for ( it1 = jl::QueueBegin(scriptList); it1; it1 = jl::QueueNext(it1) ) {

			JSScript *s = (JSScript*)jl::QueueGetData(it1);
			printf( "script: - %s:%d-%d - %d - %p\n", s->filename, s->lineno, s->lineno+JS_GetScriptLineExtent(cx, s), script->staticLevel, s );
		}
	}
	printf("</DUMP>\n\n");
	}
*/
}


void DestroyScriptHook(JSContext *cx, JSScript *script, void *callerdata) {

//	printf( "del - %s:%d - ? - %d - %p\n", script->filename, script->lineno, script->staticLevel, script );

	jl::QueueCell *it, *it1;
	jl::Queue *scriptList = NULL;

	// find the right script filename
	for ( it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

		scriptList = (jl::Queue*)jl::QueueGetData(it);
		for ( it1 = jl::QueueBegin(scriptList); it1; it1 = jl::QueueNext(it1) ) {

			JSScript *s = (JSScript*)jl::QueueGetData(it1);
			if ( s == script ) {

				jl::QueueRemoveCell(scriptList, it1);
				if ( jl::QueueIsEmpty(scriptList) ) {

					jl::QueueRemoveCell(scriptFileList, it);
					jl::QueueDestruct(scriptList);
				}
				return;
			}
		}
	}
}



JSScript *ScriptByLocation(JSContext *cx, jl::Queue *scriptFileList, const char *filename, unsigned int lineno) {

	jl::QueueCell *it;
	jl::Queue *scriptList = NULL;

	// find the right script filename
	for ( it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

		scriptList = (jl::Queue*)jl::QueueGetData(it);
		JSScript *s = (JSScript*)jl::QueueGetData(jl::QueueBegin(scriptList));

		if ( strcmp(filename, s->filename) == 0 )
			break;
	}

	if ( it == NULL )
		return NULL;

	JSScript *script = NULL;
	for ( it = jl::QueueBegin(scriptList); it; it = jl::QueueNext(it) ) {

		script = (JSScript*)jl::QueueGetData(it);
		uintN extent = JS_GetScriptLineExtent(cx, script);

		if ( lineno >= script->lineno && lineno < script->lineno + extent )
			break;
		// else the last script in the list (depth 0) will be selected
	}
	return script;
}


JSBool GetScriptLocation( JSContext *cx, jsval *val, uintN lineno, JSScript **script, jsbytecode **pc ) {

	if ( JsvalIsFunction(cx, *val) ) {

		*script = JS_GetFunctionScript(cx, JS_ValueToFunction(cx, *val));
		if ( *script == NULL )
			return JS_TRUE;
		lineno += JS_GetScriptBaseLineNumber(cx, *script);
	} else
	if ( JsvalIsScript(cx, *val) ) {

		*script = (JSScript *) JL_GetPrivate(cx, JSVAL_TO_OBJECT(*val));
		if ( *script == NULL )
			return JS_TRUE;
		lineno += JS_GetScriptBaseLineNumber(cx, *script);
	} else {

		const char *filename;
		JL_CHK( JsvalToString(cx, val, &filename) );
		*script = ScriptByLocation(cx, scriptFileList, filename, lineno);
		if ( *script == NULL )
			return JS_TRUE;
	}
	*pc = JS_LineNumberToPC(cx, *script, lineno);
	return JS_TRUE;
	JL_BAD;
}



EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	_unsafeMode = GetHostPrivate(cx)->unsafeMode;

	scriptFileList = jl::QueueConstruct();

	// record the caller's scripts (at least).
	for ( JSStackFrame *fp = JL_CurrentStackFrame(cx); fp; fp = fp->down ) { // cf. JS_FrameIterator

		JSScript *script = JS_GetFrameScript(cx, fp);
		if ( !script ) // !JS_IsNativeFrame ?
			continue;
		const char *filename = JS_GetScriptFilename(cx, script);
		if ( !filename )
			continue;
		uintN lineno = JS_GetScriptBaseLineNumber(cx, script);
		JSFunction *fun = JS_GetFrameFunction(cx, fp);
		NewScriptHook(cx, filename, lineno, script, NULL, NULL);
	}

	// records script creation/destruction from this point.
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
