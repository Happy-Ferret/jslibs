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
#include "jsdbgapi.h"

/*
const JSCodeSpec jsCodeSpec[] = {
#define OPDEF(op,val,name,token,length,nuses,ndefs,prec,format) \
    {length,nuses,ndefs,prec,format},
#include "jsopcode.tbl"
#undef OPDEF
};
*/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 2290 $
**/
BEGIN_CLASS( Debugger )

#define CONTINUE 1
#define STEP 2
#define STEP_OVER 3
#define STEP_OUT 4

// Reason
#define FROM_BREAKPOINT 1
#define FROM_STEP 2
#define FROM_THROW 3
#define FROM_ERROR 4


struct Private {

	JSStackFrame *stopAtFrame;
	jl::Queue *scriptFileList;
};

int GetFrameDepth(const JSStackFrame *frame) {

	int frameDepth;
	for ( frameDepth = 0; frame; frame = frame->down, frameDepth++ );
	return frameDepth;
}

void NewScriptHook(JSContext *cx, const char *filename, uintN lineno, JSScript *script, JSFunction *fun, void *callerdata) {

	Private *pv = (Private*)callerdata;

	jl::QueueCell *it;
	jl::Queue *scriptList = NULL;

	// find the right script filename
	for ( it = jl::QueueBegin(pv->scriptFileList); it; it = jl::QueueNext(it) ) {

		scriptList = (jl::Queue*)jl::QueueGetData(it);
		JSScript *s = (JSScript*)jl::QueueGetData(jl::QueueBegin(scriptList));
		if ( strcmp(filename, s->filename) == 0 )
			break;
	}

	if ( it == NULL ) { // if not found, create one
	
		scriptList = jl::QueueConstruct();
		jl::QueueUnshift(pv->scriptFileList, scriptList);
		jl::QueuePush(scriptList, script);
	} else { // add the script at the right place in the queue

		for ( it = jl::QueueBegin(scriptList); it; it = jl::QueueNext(it) ) {

			JSScript *s = (JSScript*)jl::QueueGetData(it);
			if ( script->staticDepth > s->staticDepth )
				jl::QueueInsertCell(scriptList, it, script);
		}
		if ( it == NULL )
			jl::QueueUnshift(scriptList, script);
	}
bad:
	return;
}

void DestroyScriptHook(JSContext *cx, JSScript *script, void *callerdata) {

	Private *pv = (Private*)callerdata;

	jl::QueueCell *it, *it1;
	jl::Queue *scriptList = NULL;

	// find the right script filename
	for ( it = jl::QueueBegin(pv->scriptFileList); it; it = jl::QueueNext(it) ) {

		scriptList = (jl::Queue*)jl::QueueGetData(it);
		for ( it1 = jl::QueueBegin(scriptList); it1; it1 = jl::QueueNext(it1) ) {
		
			JSScript *s = (JSScript*)jl::QueueGetData(it1);
			if ( s == script ) {

				jl::QueueRemoveCell(scriptList, it1);
				break;
			}
		}

		if ( jl::QueueIsEmpty(scriptList) ) {
			
			jl::QueueRemoveCell(pv->scriptFileList, it);
			break;
		}
	}

bad:
	return;
}




static JSTrapStatus BreakHandler(JSContext *cx, JSObject *obj, JSScript *script, jsbytecode *pc, int breakOrigin);

static JSTrapStatus InterruptHandler(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	JSObject *debuggerObj = (JSObject*)closure;
	Private *pv = (Private*)JS_GetPrivate(cx, debuggerObj);
	JSStackFrame *currentFrame = JS_GetScriptedCaller(cx, NULL);

	if ( pv->stopAtFrame != NULL && pv->stopAtFrame != currentFrame )
		return JSTRAP_CONTINUE;

	JS_ClearInterrupt(JS_GetRuntime(cx), NULL, NULL); // avoid nested calls
	return BreakHandler(cx, debuggerObj, script, pc, FROM_STEP);
}


static JSTrapStatus TrapHandler(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	JSObject *debuggerObj = (JSObject*)closure;
	JS_ClearInterrupt(JS_GetRuntime(cx), NULL, NULL); // avoid nested calls
	return BreakHandler(cx, debuggerObj, script, pc, FROM_BREAKPOINT);
}


JSBool DebugErrorHookHandler(JSContext *cx, const char *message, JSErrorReport *report, void *closure) {

	JSObject *debuggerObj = (JSObject*)closure;
	JS_ClearInterrupt(JS_GetRuntime(cx), NULL, NULL); // avoid nested calls
	JSStackFrame *frame;
	frame = JS_GetScriptedCaller(cx, NULL);
	if ( frame )
		BreakHandler(cx, debuggerObj, JS_GetFrameScript(cx, frame), JS_GetFramePC(cx, frame), FROM_ERROR);
	return JS_TRUE;
}


static JSTrapStatus ThrowHookHandler(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	JSObject *debuggerObj = (JSObject*)closure;
	JS_ClearInterrupt(JS_GetRuntime(cx), NULL, NULL); // avoid nested calls
	return BreakHandler(cx, debuggerObj, script, pc, FROM_THROW);
}



static JSTrapStatus BreakHandler(JSContext *cx, JSObject *obj, JSScript *script, jsbytecode *pc, int breakOrigin) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);

	jsval fval;
	J_CHK( JS_GetProperty(cx, obj, "onBreak", &fval) );
	if ( !JsvalIsFunction(cx, fval) ) // nothing to do
		return JSTRAP_CONTINUE;

	JSStackFrame *frame;
	frame = JS_GetScriptedCaller(cx, NULL);

	jsval argv[5];
	J_CHK( StringToJsval(cx, JS_GetScriptFilename(cx, JS_GetFrameScript(cx, frame)), &argv[0]) );
	argv[1] = INT_TO_JSVAL(JS_PCToLineNumber(cx, JS_GetFrameScript(cx, frame), pc));
	argv[2] = OBJECT_TO_JSVAL( JS_GetFrameScopeChain(cx, frame) );
	argv[3] = INT_TO_JSVAL( breakOrigin );
	argv[4] = INT_TO_JSVAL( GetFrameDepth(frame) );

	jsval trapRval;
	J_CHK( JS_CallFunctionValue(cx, obj, fval, COUNTOF(argv), argv, &trapRval) );

	int t = JSVAL_TO_INT( trapRval );

	switch (trapRval) {
		
		case INT_TO_JSVAL(CONTINUE):

			pv->stopAtFrame = NULL;
			return JSTRAP_CONTINUE;
			break;
		case INT_TO_JSVAL(STEP):
			
			pv->stopAtFrame = NULL; // do not stop at a given frame
			J_CHK( JS_SetInterrupt(JS_GetRuntime(cx), InterruptHandler, (void*)OBJECT_TO_JSVAL(obj)) );
			break;
		case INT_TO_JSVAL(STEP_OVER):
		
			pv->stopAtFrame = JS_GetScriptedCaller(cx, NULL); // stop next time we are in this frame.
			J_CHK( JS_SetInterrupt(JS_GetRuntime(cx), InterruptHandler, (void*)OBJECT_TO_JSVAL(obj)) );
	/*
			//jsbytecode *nextPc = JS_LineNumberToPC(cx, script, JS_PCToLineNumber(cx, script, pc+1) + 1);
	//		jsbytecode *nextPc = JS_GetFramePC(cx, caller);
	//		JSOp op = JS_GetTrapOpcode(cx, caller->script, nextPc);
	//		nextPc += jsCodeSpec[op].length;
	//		JS_SetTrap(cx, caller->script, nextPc, InterruptHandler, NULL);
			JSOp op = JS_GetTrapOpcode(cx, caller->script, nextPc);
			JS_SetInterrupt(JS_GetRuntime(cx), InterruptHandler, NULL);
			jsbytecode *nextPc = JS_GetFramePC(cx, caller);
			JSOp op = JS_GetTrapOpcode(cx, caller->script, nextPc);
			nextPc += jsCodeSpec[op].length;
			JS_SetTrap(cx, caller->script, nextPc, InterruptHandler, NULL);
	*/
			break;
		case INT_TO_JSVAL(STEP_OUT):
			
			pv->stopAtFrame = JS_GetScriptedCaller(cx, frame->down); // stop when we reach the parent's frame.
			J_CHK( JS_SetInterrupt(JS_GetRuntime(cx), InterruptHandler, (void*)OBJECT_TO_JSVAL(obj)) );
	/*
			JSStackFrame *frame;
			frame = JS_GetScriptedCaller(cx, NULL);
			// check if there is a frame to return to, else continue
			if ( !frame->down )
				return JSTRAP_CONTINUE;
			frame = JS_GetScriptedCaller(cx, frame->down);
			JSScript *nextScript = JS_GetFrameScript(cx, frame);
			jsbytecode *nextPc = JS_GetFramePC(cx, frame);
	//		uintN lineno = JS_PCToLineNumber(cx, nextScript, nextPc);
	//		uintN lastlineno = JS_GetScriptBaseLineNumber(cx, nextScript) + JS_GetScriptLineExtent(cx, nextScript);
	//		if ( lineno + 1 >= lastlineno )
	//			return JSTRAP_CONTINUE;
	//		nextPc = JS_LineNumberToPC(cx, nextScript, lineno +1 );
	//		JS_ClearTrap(cx, nextScript, nextPc, NULL, NULL);
			// set the next trap on the next opcode
			JSOp op = JS_GetTrapOpcode(cx, nextScript, nextPc);
			nextPc += jsCodeSpec[op].length;
			JSBool status = JS_SetTrap(cx, nextScript, nextPc, InterruptHandler, (void*)PRIVATE_TO_JSVAL(NULL));
	*/
			break;
	}

	return JSTRAP_CONTINUE; // http://www.google.com/codesearch/p?hl=en#SPZdyP79RtQ/trunk/third_party/spidermonkey/js/src/jsinterp.c&q=JSTRAP_RETURN&l=742
bad:
	return JSTRAP_ERROR;
}



DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv ) {

		JSTrapHandler tmpTrapHandler;
		void *tmpTrapClosure;
		JS_ClearInterrupt(JS_GetRuntime(cx), &tmpTrapHandler, &tmpTrapClosure); // avoid nested calls
		// restore the interrupt if it is not ours.
		if ( tmpTrapHandler != NULL && tmpTrapHandler != InterruptHandler && tmpTrapClosure != (void*)OBJECT_TO_JSVAL(obj) ) {

			JS_SetInterrupt(JS_GetRuntime(cx), tmpTrapHandler, tmpTrapClosure);
		}

		JS_SetNewScriptHookProc(JS_GetRuntime(cx), NULL, NULL);
		JS_SetDestroyScriptHookProc(JS_GetRuntime(cx), NULL, NULL);

		for ( jl::QueueCell *it = jl::QueueBegin(pv->scriptFileList); it; it = jl::QueueNext(it) ) {

			jl::Queue *scriptList = (jl::Queue*)jl::QueueGetData(it);
			jl::QueueDestruct(scriptList);
		}
		jl::QueueDestruct(pv->scriptFileList);

		JS_SetDebuggerHandler(JS_GetRuntime(cx), NULL, NULL);
		JS_SetThrowHook( JS_GetRuntime(cx), NULL, NULL);
		JS_SetDebugErrorHook( JS_GetRuntime(cx), NULL, NULL);

		free(pv);
	}
}


DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

	Private *pv = (Private*)malloc(sizeof(Private));
	J_S_ASSERT_ALLOC(pv);
	memset(pv, 0, sizeof(Private));
	J_CHK( JS_SetPrivate(cx, obj, pv) );
	J_CHK( JS_SetDebuggerHandler(JS_GetRuntime(cx), InterruptHandler, (void*)OBJECT_TO_JSVAL(obj)) );
	J_CHK( JS_SetThrowHook( JS_GetRuntime(cx), ThrowHookHandler, (void*)OBJECT_TO_JSVAL(obj)) );
	J_CHK( JS_SetDebugErrorHook( JS_GetRuntime(cx), DebugErrorHookHandler, (void*)OBJECT_TO_JSVAL(obj)) );

	pv->scriptFileList = jl::QueueConstruct();

	JS_SetNewScriptHookProc(JS_GetRuntime(cx), NewScriptHook, pv);
	JS_SetDestroyScriptHookProc(JS_GetRuntime(cx), DestroyScriptHook, pv);

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( ToggleBreakpoint ) {

	J_S_ASSERT_ARG_MIN( 3 );

	bool polarity;
	J_CHK( JsvalToBool(cx, J_ARG(1), &polarity) );

	const char *filename;
	J_CHK( JsvalToString(cx, &J_ARG(2), &filename) );

	uintN lineno;
	J_CHK( JsvalToUInt(cx, J_ARG(3), &lineno) );

	// find the right script object
	Private *pv = (Private*)JS_GetPrivate(cx, obj);

	jl::QueueCell *it;
	jl::Queue *scriptList = NULL;

	// find the right script filename
	for ( it = jl::QueueBegin(pv->scriptFileList); it; it = jl::QueueNext(it) ) {

		scriptList = (jl::Queue*)jl::QueueGetData(it);
		JSScript *s = (JSScript*)jl::QueueGetData(jl::QueueBegin(scriptList));

		if ( strcmp(filename, s->filename) == 0 )
			break;
	}

	if ( it == NULL )
		J_REPORT_ERROR("script not found");

	JSScript *script = NULL;
	for ( it = jl::QueueBegin(scriptList); it; it = jl::QueueNext(it) ) {

		script = (JSScript*)jl::QueueGetData(it);
		uintN extent = JS_GetScriptLineExtent(cx, script);

		if ( lineno >= script->lineno && lineno <= script->lineno + extent )
			break;
		// else the last script in the list (depth 0) will be selected
	}

	J_S_ASSERT( script != NULL, "script not found");

	jsbytecode *pc;
	pc = JS_LineNumberToPC(cx, script, lineno);

	if ( polarity ) {

		J_CHK( JS_SetTrap(cx, script, pc, TrapHandler, (void*)OBJECT_TO_JSVAL(obj)) );
	} else {

		JSTrapHandler tmpHandler;
		void *tmpClosure;
		JS_ClearTrap(cx, script, pc, &tmpHandler, &tmpClosure);
		// restore the trap if it is not ours.
		if ( tmpHandler != NULL && ( tmpHandler != TrapHandler || tmpClosure != (void*)OBJECT_TO_JSVAL(obj)) )
			J_CHK( JS_SetTrap(cx, script, pc, tmpHandler, tmpClosure) );
	}

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( Stack ) {

	J_S_ASSERT_ARG_MIN( 1 );

	int level;
	J_CHK( JsvalToInt(cx, J_ARG(1), &level) );


	JSStackFrame *frame;
	frame = JS_GetScriptedCaller(cx, NULL); // the current frame
	int currentDepth = GetFrameDepth(frame);

	J_S_ASSERT( level >= 0 && level <= currentDepth, "Invalid frame level." );

	// select the right frame
	while ( currentDepth > level ) {
		
		frame = frame->down;
		currentDepth--;
	}


	JSObject *stackItem = JS_NewObject(cx, NULL, NULL, NULL);
	J_S_ASSERT_ALLOC( stackItem );

	*J_RVAL = OBJECT_TO_JSVAL(stackItem);

	JSScript *script = JS_GetFrameScript(cx, frame);
	jsbytecode *pc = JS_GetFramePC(cx, frame);

	jsval tmp;

	JSObject *scope = JS_GetFrameScopeChain(cx, frame);

	J_CHK( StringToJsval(cx, JS_GetScriptFilename(cx, script), &tmp) );
	J_CHK( JS_DefineProperty(cx, stackItem, "filename", tmp, NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT) );
	J_CHK( JS_DefineProperty(cx, stackItem, "lineno", INT_TO_JSVAL(JS_PCToLineNumber(cx, script, pc)), NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT) );
	J_CHK( JS_DefineProperty(cx, stackItem, "callee", OBJECT_TO_JSVAL(JS_GetFrameCalleeObject(cx, frame)), NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT) );
	J_CHK( JS_DefineProperty(cx, stackItem, "scope", OBJECT_TO_JSVAL(scope), NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT) );
	J_CHK( JS_DefineProperty(cx, stackItem, "this", OBJECT_TO_JSVAL(JS_GetFrameThis(cx, frame)), NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT) );
	J_CHK( JS_DefineProperty(cx, stackItem, "isNative", BOOLEAN_TO_JSVAL(JS_IsNativeFrame(cx, frame)), NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( scriptList ) {

	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
	J_S_ASSERT_ALLOC(arr);
	*vp = OBJECT_TO_JSVAL(arr);

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	int index = 0;
	for ( jl::QueueCell *it = jl::QueueBegin(pv->scriptFileList); it; it = jl::QueueNext(it) ) {

		jl::Queue *scriptList = (jl::Queue*)jl::QueueGetData(it);
		JSScript *s = (JSScript*)jl::QueueGetData(jl::QueueBegin(scriptList));
		
		jsval filename;
		J_CHK( StringToJsval(cx, s->filename, &filename) );
		J_CHK( JS_SetElement(cx, arr, index, &filename) );
		index++;
	}
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision: 2290 $"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( ToggleBreakpoint )
		FUNCTION( Stack )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( scriptList )
	END_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER_SINGLE( CONTINUE )
		CONST_INTEGER_SINGLE( STEP )
		CONST_INTEGER_SINGLE( STEP_OVER )
		CONST_INTEGER_SINGLE( STEP_OUT )

		CONST_INTEGER_SINGLE( FROM_BREAKPOINT )
		CONST_INTEGER_SINGLE( FROM_STEP )
		CONST_INTEGER_SINGLE( FROM_THROW )
		CONST_INTEGER_SINGLE( FROM_ERROR )
	END_CONST_INTEGER_SPEC

END_CLASS
