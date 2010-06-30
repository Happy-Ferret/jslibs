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

#include "jslibsModule.cpp"

#include "jsdebug.h"


DECLARE_STATIC()
DECLARE_CLASS( Debugger )


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

	JL_ASSERT( filename != NULL );

	ModulePrivate *mpv = (ModulePrivate*)GetModulePrivate(cx, _moduleId);

	jl::Queue *scriptFileList = &mpv->scriptFileList;

	jl::QueueCell *it;
	jl::Queue *scriptList = NULL;

	// find the right script filename
	for ( it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

		scriptList = (jl::Queue*)jl::QueueGetData(it);
		JSScript *s = (JSScript*)jl::QueueGetData(jl::QueueBegin(scriptList));
		if ( s == script )
			goto done_scriptList; // (TBD) already added, check how it is possible.
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
				goto done_scriptList;
			}
		}
		jl::QueuePush(scriptList, script);
	}
done_scriptList:

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

//	for ( jl::QueueCell *it = jl::QueueBegin(newScriptHookList); it; it = jl::QueueNext(it) )
//		((JSNewScriptHook)jl::QueueGetData(it))(cx, filename, lineno, script, fun, callerdata);

	JSObject *moduleObject = (JSObject*)callerdata;
	JL_ASSERT( moduleObject != NULL );
	jsval jsHookFct;
	JS_GetPropertyById(cx, moduleObject, mpv->JLID_onNewScript, &jsHookFct); // try to use ids
	if ( JsvalIsFunction(cx, jsHookFct) ) {

		jsval argv[5] = { JSVAL_NULL, JSVAL_NULL, INT_TO_JSVAL( lineno ), OBJECT_TO_JSVAL( JS_NewScriptObject(cx, script) ), OBJECT_TO_JSVAL( JS_GetFunctionObject(fun) ) };
		JL_CHK( StringToJsval(cx, filename, &argv[1]) );

		JSBool status;
		JSRuntime *rt;
		rt = JS_GetRuntime(cx);
		// avoid nested calls (NewScriptHook)
		JS_SetNewScriptHook(rt, NULL, NULL); 
		status = JS_CallFunctionValue(cx, moduleObject, jsHookFct, COUNTOF(argv)-1, argv+1, argv);
		JS_SetNewScriptHook(rt, NewScriptHook, callerdata); 
	}

bad:
	return;
}


void DestroyScriptHook(JSContext *cx, JSScript *script, void *callerdata) {

//	printf( "del - %s:%d - ? - %d - %p\n", script->filename, script->lineno, script->staticLevel, script );

//	for ( jl::QueueCell *it = jl::QueueBegin(destroyScriptHookList); it; it = jl::QueueNext(it) )
//		((JSDestroyScriptHook)jl::QueueGetData(it))(cx, script, callerdata);

/* unable to do this while GC is running !!!
	JSObject *moduleObject = (JSObject*)callerdata;
	jsval jsHookFct;
	JS_GetProperty(cx, moduleObject, "onDestroyScript", &jsHookFct); // try to use ids
	if ( JsvalIsFunction(cx, jsHookFct) ) {

		jsval argv[4];
		JL_ASSERT( JSVAL_NULL == 0 );
		memset(argv, 0, sizeof(argv)); // { JSVAL_NULL }

		JSTempValueRooter tvr;
		JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr);
		JL_CHKB( StringToJsval(cx, JS_GetScriptFilename(cx, script), &argv[1]), bad_1 );
		argv[2] = INT_TO_JSVAL( 0 );
		argv[3] = OBJECT_TO_JSVAL( JS_GetScriptObject(script) );

		// avoid nested calls (NewScriptHook)
		JSBool status;
		status = JS_CallFunctionValue(cx, moduleObject, jsHookFct, COUNTOF(argv)-1, argv+1, argv);
	bad_1:
		JS_POP_TEMP_ROOT(cx, &tvr);
	}
*/

	jl::Queue *scriptFileList = &((ModulePrivate*)GetModulePrivate(cx, _moduleId))->scriptFileList;

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



JSScript *ScriptByLocation(JSContext *cx, jl::Queue *scriptFileList, const char *filename, uint32 lineno) {

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

		jl::Queue *scriptFileList = &((ModulePrivate*)GetModulePrivate(cx, _moduleId))->scriptFileList;

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


void SourceHandler(const char *filename, uintN lineno, jschar *str, size_t length, void **listenerTSData, void *closure) {
}



EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

//	if ( instanceCount++ > 0 )
//		JL_REPORT_ERROR( "Loading this module twice is not allowed." );

	JL_CHK( InitJslibsModule(cx, id)  );

	ModulePrivate *mpv;
	mpv = (ModulePrivate*)jl_malloc( sizeof(ModulePrivate) );
	JL_S_ASSERT_ALLOC(mpv);
	JL_CHKM( SetModulePrivate(cx, _moduleId, mpv), "Module id already in use." );
	
	mpv->JLID_onNewScript = StringToJsid(cx, "onNewScript"); // see NewScriptHook

	jl::QueueInitialize(&mpv->scriptFileList);

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
		NewScriptHook(cx, filename, lineno, script, fun, obj);
	}

	// records script creation/destruction from this point.
	JS_SetNewScriptHook(JS_GetRuntime(cx), NewScriptHook, obj); // obj is moduleObject in NewScriptHook()
	JS_SetDestroyScriptHook(JS_GetRuntime(cx), DestroyScriptHook, obj); // obj is moduleObject in NewScriptHook()

//	JS_SetSourceHandler(JS_GetRuntime(cx), SourceHandler, NULL);

	INIT_STATIC();
	INIT_CLASS( Debugger );

	return JS_TRUE;
	JL_BAD;
}

EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

	JS_SetNewScriptHookProc(JS_GetRuntime(cx), NULL, NULL);
	JS_SetDestroyScriptHookProc(JS_GetRuntime(cx), NULL, NULL);

	jl::Queue *scriptFileList = &((ModulePrivate*)GetModulePrivate(cx, _moduleId))->scriptFileList;
	for ( jl::QueueCell *it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

		jl::Queue *scriptList = (jl::Queue*)jl::QueueGetData(it);
		jl::QueueDestruct(scriptList);
	}

	jl_free(GetModulePrivate(cx, _moduleId));

	return JS_TRUE;
	JL_BAD;
}
