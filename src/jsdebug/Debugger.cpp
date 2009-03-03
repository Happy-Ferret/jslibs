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


const JSCodeSpec jsCodeSpec[] = {
#define OPDEF(op,val,name,token,length,nuses,ndefs,prec,format) \
    {length,nuses,ndefs,prec,format},
#include "jsopcode.tbl"
#undef OPDEF
};


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 2290 $
**/
BEGIN_CLASS( Debugger )

struct Private {

	JSStackFrame *stopAtFrame;
	jl::Queue *scriptList;
};


static JSTrapStatus InterruptHandler(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	JSObject *debuggerObj = (JSObject*)closure;
	Private *pv = (Private*)JS_GetPrivate(cx, debuggerObj);
	JSStackFrame *currentFrame = JS_GetScriptedCaller(cx, NULL);

	if ( pv->stopAtFrame != NULL && pv->stopAtFrame != currentFrame )
		return JSTRAP_CONTINUE;

	JSTrapHandler tmpTrapHandler;
	void *tmpTrapClosure;
	JS_ClearInterrupt(JS_GetRuntime(cx), &tmpTrapHandler, &tmpTrapClosure); // avoid nested calls
	// restore the interrupt if it is not ours.
	if ( tmpTrapHandler != NULL && tmpTrapHandler != InterruptHandler )
		JS_SetInterrupt(JS_GetRuntime(cx), tmpTrapHandler, tmpTrapClosure);

	JSTrapHandler tmpHandler;
	void *tmpClosure;
	JS_ClearTrap(cx, script, pc, &tmpHandler, &tmpClosure);
	// restore the trap if it is not ours.
	if ( tmpHandler != NULL && tmpHandler != InterruptHandler )
		JS_SetTrap(cx, script, pc, tmpHandler, tmpClosure);

	jsval fval;
	J_CHK( JS_GetProperty(cx, debuggerObj, "onInterrupt", &fval) );
	if ( !JsvalIsFunction(cx, fval) )
		return JSTRAP_CONTINUE;


	JSStackFrame *frame;
	frame = JS_GetScriptedCaller(cx, NULL);

	jsval argv[3];
	J_CHK( StringToJsval(cx, frame->script->filename, &argv[0]) );
	argv[1] = INT_TO_JSVAL(JS_PCToLineNumber(cx, frame->script, pc));
	argv[2] = OBJECT_TO_JSVAL( frame->scopeChain );

	jsval trapRval;
	JSBool status = JS_CallFunctionValue(cx, debuggerObj, fval, COUNTOF(argv), argv, &trapRval);

	if ( JSVAL_IS_VOID(trapRval) )
		return JSTRAP_CONTINUE;


	if ( trapRval == INT_TO_JSVAL(1) ) { // step into

		// do not stop
		pv->stopAtFrame = NULL;
		J_CHK( JS_SetInterrupt(JS_GetRuntime(cx), InterruptHandler, (void*)OBJECT_TO_JSVAL(debuggerObj)) );
	}

	if ( trapRval == INT_TO_JSVAL(2) ) { // step over
	
		// stop next time we are in this frame.
		pv->stopAtFrame = JS_GetScriptedCaller(cx, NULL);
		J_CHK( JS_SetInterrupt(JS_GetRuntime(cx), InterruptHandler, (void*)OBJECT_TO_JSVAL(debuggerObj)) );

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
	}

	if ( trapRval == INT_TO_JSVAL(3) ) { // step out
		
		// stop when we reach the parent's frame.
		pv->stopAtFrame = JS_GetScriptedCaller(cx, JS_GetScriptedCaller(cx, NULL)->down);
		J_CHK( JS_SetInterrupt(JS_GetRuntime(cx), InterruptHandler, (void*)OBJECT_TO_JSVAL(debuggerObj)) );

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
	}

	return JSTRAP_CONTINUE; // http://www.google.com/codesearch/p?hl=en#SPZdyP79RtQ/trunk/third_party/spidermonkey/js/src/jsinterp.c&q=JSTRAP_RETURN&l=742
bad:
	return JSTRAP_ERROR;
}


void NewScriptHook(JSContext *cx, const char *filename, uintN lineno, JSScript *script, JSFunction *fun, void *callerdata) {

	JSObject *debuggerObj = (JSObject*)callerdata;
	Private *pv = (Private*)JS_GetPrivate(cx, debuggerObj);

	jsval fval;
	J_CHK( JS_GetProperty(cx, debuggerObj, "onNewScript", &fval) );
	if ( !JsvalIsFunction(cx, fval) )
		return;

	jsval argv[6];
	J_CHK( StringToJsval(cx, filename, &argv[0]) );
	argv[1] = INT_TO_JSVAL(lineno);
	argv[2] = INT_TO_JSVAL(JS_GetScriptLineExtent(cx, script));
	argv[3] = INT_TO_JSVAL(script->staticDepth);
	argv[4] = fun ? STRING_TO_JSVAL(JS_GetFunctionId(fun)) : JSVAL_VOID;

	J_S_ASSERT(INT_FITS_IN_JSVAL(script));
	argv[5] = PRIVATE_TO_JSVAL(script);

/*
	JSObject *scriptObj = JS_GetScriptObject(script);
	if ( scriptObj == NULL )
		scriptObj = JS_NewScriptObject(cx, script);
	argv[5] = OBJECT_TO_JSVAL(scriptObj);

	J_S_ASSERT( script == JS_GetPrivate(cx, scriptObj) );
*/	
	



	// (TBD) protect argv from GC

	jsval trapRval;
	JSBool status = JS_CallFunctionValue(cx, debuggerObj, fval, COUNTOF(argv), argv, &trapRval);

bad:
	return;
}

void DestroyScriptHook(JSContext *cx, JSScript *script, void *callerdata) {
/*
	JSObject *debuggerObj = (JSObject*)callerdata;
	Private *pv = (Private*)JS_GetPrivate(cx, debuggerObj);

	jsval fval;
	J_CHK( JS_GetProperty(cx, debuggerObj, "onDestroyScript", &fval) );
	if ( !JsvalIsFunction(cx, fval) )
		return;
	// (TBD)
bad:
	return;
*/
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
	JS_SetNewScriptHookProc(JS_GetRuntime(cx), NewScriptHook, (void*)OBJECT_TO_JSVAL(obj));
	JS_SetDestroyScriptHookProc(JS_GetRuntime(cx), DestroyScriptHook, (void*)OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
	JL_BAD;
}


/*	
	JSString *str;
    JSStackFrame *caller;

	 char *s = "Print(123)";
    
    caller = JS_GetScriptedCaller(cx, NULL);
    if (!JS_EvaluateScript(cx, caller->scopeChain,
                           s, strlen(s),
                           caller->script->filename, caller->script->lineno,
                           rval)) {
        return JSTRAP_ERROR;
    }
    if (!JSVAL_IS_VOID(*rval))
        return JSTRAP_RETURN;
  
*/



DEFINE_FUNCTION( SetBreakpoint ) {

	J_S_ASSERT_ARG_MIN( 2 );



	int line;
	J_CHK( JsvalToInt(cx, argv[1], &line) );




	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision: 2290 $"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( SetBreakpoint )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

END_CLASS
