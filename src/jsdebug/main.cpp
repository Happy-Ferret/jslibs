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
#include <jslibsModule.cpp>


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


void NewScriptHook(JSContext *cx, const char *filename, unsigned lineno, JSScript *script, JSFunction *fun, void *callerdata) {

// (TBD) do we protect new file-based scripts against GC to allow later debugging them ?

//	printf( "add - %s:%d-%d - %s - %d - %p\n", filename, lineno, lineno+JS_GetScriptLineExtent(cx, script), fun ? JS_GetFunctionName(fun):"", script->staticLevel, script );

	if ( filename == NULL ) // happens when js_InitFunctionClass. At the moment, no way to support this script...
		return;
	//ASSERT( filename != NULL );

	ModulePrivate *mpv = (ModulePrivate*)JL_GetModulePrivate(cx, _moduleId);

	jl::Queue *scriptFileList = &mpv->scriptFileList;

	jl::QueueCell *it;
	jl::Queue *scriptList = NULL;

	// find the right script filename
	for ( it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

		scriptList = (jl::Queue*)jl::QueueGetData(it);
		JSScript *s = (JSScript*)jl::QueueGetData(jl::QueueBegin(scriptList));
		if ( s == script )
			goto done_scriptList; // (TBD) already added, check how it is possible.
		if ( strcmp(filename, JS_GetScriptFilename(cx, s)) == 0 )
			break;
	}

	if ( it == NULL ) { // if not found, create one

		scriptList = jl::QueueConstruct();
		jl::QueueUnshift(scriptFileList, scriptList);
		jl::QueuePush(scriptList, script);
	} else { // add the script at the right place in the queue

/*
		for ( it = jl::QueueBegin(scriptList); it; it = jl::QueueNext(it) ) {

			JSScript *s = (JSScript*)jl::QueueGetData(it);
			if ( script->staticLevel >= s->staticLevel ) {

				jl::QueueInsertCell(scriptList, it, script);
				goto done_scriptList;
			}
		}
*/
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
	ASSERT( moduleObject != NULL );
	jsval jsHookFct;


	JSCrossCompartmentCall *ccc = NULL;
	ccc = JS_EnterCrossCompartmentCall(cx, moduleObject);

	JS_GetPropertyById(cx, moduleObject, mpv->JLID_onNewScript, &jsHookFct); // try to use ids
	if ( JL_ValueIsCallable(cx, jsHookFct) ) {

#define JS_NewScriptObject(cx, script) (JSObject*)(script)
		// see. http://code.google.com/r/wes-js185/source/browse/spidermonkey/probe-jsapi.incl#376
		// see also. https://bugzilla.mozilla.org/show_bug.cgi?id=630209 
		jsval argv[] = { JSVAL_NULL, JSVAL_NULL, INT_TO_JSVAL( lineno ), OBJECT_TO_JSVAL( JS_NewScriptObject(cx, script) ), OBJECT_TO_JSVAL( JS_GetFunctionObject(fun) ) };
#undef JS_NewScriptObject
		JL_CHK( JL_NativeToJsval(cx, filename, &argv[1]) );

		bool status;
		JSRuntime *rt;
		rt = JL_GetRuntime(cx);
		// avoid nested calls (NewScriptHook)
		JS_SetNewScriptHook(rt, NULL, NULL);
		status = JS_CallFunctionValue(cx, moduleObject, jsHookFct, COUNTOF(argv)-1, argv+1, argv);
		JS_SetNewScriptHook(rt, NewScriptHook, callerdata);
	}

bad: // and good:

	if (ccc)
		JS_LeaveCrossCompartmentCall(ccc);
}


void DestroyScriptHook(JSFreeOp *fop, JSScript *script, void *callerdata) {

//	printf( "del - %s:%d - ? - %d - %p\n", script->filename, script->lineno, script->staticLevel, script );

//	for ( jl::QueueCell *it = jl::QueueBegin(destroyScriptHookList); it; it = jl::QueueNext(it) )
//		((JSDestroyScriptHook)jl::QueueGetData(it))(cx, script, callerdata);

/* unable to do this while GC is running !!!
	JSObject *moduleObject = (JSObject*)callerdata;
	jsval jsHookFct;
	JS_GetProperty(cx, moduleObject, "onDestroyScript", &jsHookFct); // try to use ids
	if ( JL_IsCallable(cx, jsHookFct) ) {

		jsval argv[4];
		ASSERT( JSVAL_NULL == 0 );
		memset(argv, 0, sizeof(argv)); // { JSVAL_NULL }

		JSTempValueRooter tvr;
		JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr);
		JL_CHKB( StringToJsval(cx, JS_GetScriptFilename(cx, script), &argv[1]), bad_1 );
		argv[2] = INT_TO_JSVAL( 0 );
		argv[3] = OBJECT_TO_JSVAL( JS_GetScriptObject(script) );

		// avoid nested calls (NewScriptHook)
		bool status;
		status = JS_CallFunctionValue(cx, moduleObject, jsHookFct, COUNTOF(argv)-1, argv+1, argv);
	bad_1:
		JS_POP_TEMP_ROOT(cx, &tvr);
	}
*/

	jl::Queue *scriptFileList = &((ModulePrivate*)JL_GetModulePrivate(fop->runtime(), _moduleId))->scriptFileList;

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



JSScript *ScriptByLocation(JSContext *cx, jl::Queue *scriptFileList, const char *filename, uint32_t lineno) {

	jl::QueueCell *it;
	jl::Queue *scriptList = NULL;

	// find the right script filename
	for ( it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

		scriptList = (jl::Queue*)jl::QueueGetData(it);
		JSScript *s = (JSScript*)jl::QueueGetData(jl::QueueBegin(scriptList));

		if ( strcmp(filename, JS_GetScriptFilename(cx, s)) == 0 )
			break;
	}

	if ( it == NULL )
		return NULL;

	JSScript *script = NULL;
	for ( it = jl::QueueBegin(scriptList); it; it = jl::QueueNext(it) ) {

		script = (JSScript*)jl::QueueGetData(it);
		unsigned extent = JS_GetScriptLineExtent(cx, script);

		if ( lineno >= JS_GetScriptBaseLineNumber(cx, script) && lineno < JS_GetScriptBaseLineNumber(cx, script) + extent )
			break;
		// else the last script in the list (depth 0) will be selected
	}
	return script;
}


bool GetScriptLocation( JSContext *cx, jsval *val, unsigned lineno, JSScript **script, jsbytecode **pc ) {

	if ( JL_ValueIsCallable(cx, *val) ) {

		*script = JS_GetFunctionScript(cx, JS_ValueToFunction(cx, *val));
		if ( *script == NULL )
			return true;
		lineno += JS_GetScriptBaseLineNumber(cx, *script);
	} else
/*
	if ( !JSVAL_IS_PRIMITIVE(*val) && JL_IsScript(cx, JSVAL_TO_OBJECT(*val)) ) {

		*script = (JSScript *) JL_GetPrivate(JSVAL_TO_OBJECT(*val));
		if ( *script == NULL )
			return true;
		lineno += JS_GetScriptBaseLineNumber(cx, *script);
	} else
*/
	{

		jl::Queue *scriptFileList = &((ModulePrivate*)JL_GetModulePrivate(cx, _moduleId))->scriptFileList;

		JLData fileName;
		JL_CHK( jl::getValue(cx, *val, &fileName) );
		*script = ScriptByLocation(cx, scriptFileList, fileName, lineno);
		if ( *script == NULL )
			return true;
	}
	*pc = JS_LineNumberToPC(cx, *script, lineno);
	return true;
	JL_BAD;
}


void SourceHandler(const char *filename, unsigned lineno, jschar *str, size_t length, void **listenerTSData, void *closure) {
}



bool
ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	JL_CHK( InitJslibsModule(cx, id)  );

	ModulePrivate *mpv;
	mpv = (ModulePrivate*)jl_malloc(sizeof(ModulePrivate));
	JL_ASSERT_ALLOC( mpv );
	JL_CHKM( JL_SetModulePrivate(cx, _moduleId, mpv), E_MODULE, E_INIT ); // "Module id already in use."

	mpv->JLID_onNewScript = JL_StringToJsid(cx, L("onNewScript")); // see NewScriptHook

	jl::QueueInitialize(&mpv->scriptFileList);

	// record the caller's scripts (at least).
	//for ( JSStackFrame *fp = JL_CurrentStackFrame(cx); fp; fp = fp->prev() ) { // cf. JS_FrameIterator
	JSStackFrame *fp;
	fp = NULL;
	while ( (fp = JS_FrameIterator(cx, &fp)) ) {

		JSScript *script = JS_GetFrameScript(cx, fp);
		if ( !script ) // !JS_IsNativeFrame ?
			continue;
		const char *filename = JS_GetScriptFilename(cx, script);
		if ( !filename )
			continue;
		unsigned lineno = JS_GetScriptBaseLineNumber(cx, script);
		JSFunction *fun = JS_GetFrameFunction(cx, fp);
		NewScriptHook(cx, filename, lineno, script, fun, obj);
	}

	// records script creation/destruction from this point.
	JS_SetNewScriptHook(JL_GetRuntime(cx), NewScriptHook, obj); // obj is moduleObject in NewScriptHook()
	JS_SetDestroyScriptHook(JL_GetRuntime(cx), DestroyScriptHook, obj); // obj is moduleObject in NewScriptHook()

//	JS_SetSourceHandler(JL_GetRuntime(cx), SourceHandler, NULL);

	INIT_STATIC();
	INIT_CLASS( Debugger );

	return true;
	JL_BAD;
}


bool
ModuleRelease(JSContext *cx) {

	JS_SetNewScriptHookProc(JL_GetRuntime(cx), NULL, NULL);
	JS_SetDestroyScriptHookProc(JL_GetRuntime(cx), NULL, NULL);

	if ( jl::Host::getHost(JL_GetRuntime(cx))->canSkipCleanup ) // do not cleanup in unsafe mode.
		return true;

	jl::Queue *scriptFileList = &((ModulePrivate*)JL_GetModulePrivate(cx, _moduleId))->scriptFileList;

	while ( !jl::QueueIsEmpty(scriptFileList) ) {

		jl::Queue *scriptList = (jl::Queue*)jl::QueuePop(scriptFileList);
		jl::QueueDestruct(scriptList);
	}

	jl_free(JL_GetModulePrivate(cx, _moduleId));

	return true;
}
