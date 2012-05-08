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

/*
static const JSCodeSpec jsCodeSpec[] = {
	#define OPDEF(op,val,name,token,length,nuses,ndefs,prec,format) {length,nuses,ndefs,prec,format},
	#include <jsopcode.tbl>
	#undef OPDEF
};
*/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Debugger )

// next action
enum NextAction {
	DO_CONTINUE,
	DO_STEP,
	DO_STEP_OVER,
	DO_STEP_THROUGH,
	DO_STEP_OUT,
};

// break reason
enum BreakReason {
	FROM_INTERRUPT,
	FROM_STEP,
	FROM_STEP_OVER,
	FROM_STEP_THROUGH,
	FROM_STEP_OUT,
	FROM_BREAKPOINT,
	FROM_THROW,
	FROM_ERROR,
	FROM_DEBUGGER,
	FROM_EXECUTE,
	FROM_CALL,
};


struct DebuggerPrivate {

	size_t interruptCounter;
	size_t interruptCounterLimit;
	void *excludedFiles;
	JSDebugHooks *debugHooks; // because current hooks cannot be changed while onBreak is being called.
	// previous break state
	uint32_t stackFrameIndex;
	JSStackFrame *frame;
	JSStackFrame *pframe;
	JSScript *script;
	unsigned lineno;
};


bool IsExcludedFile( void **excludedFiles, const char *filename ) {

	for ( void *it = NULL; jl::StackIterate(excludedFiles, &it); )
		if ( strcmp((char*)jl::StackData(&it), filename) == 0 )
			return true;
	return false;
}

void CleanExcludedFileList( void **excludedFiles ) {

	jl::StackFreeContent(excludedFiles);
	*excludedFiles = NULL;
}

void AddExcludedFile( void **excludedFiles, char *filename ) {

	jl::StackPush(excludedFiles, (char*)filename);
}


JSTrapStatus BreakHandler(JSContext *cx, JSObject *obj, JSStackFrame *fp, BreakReason breakOrigin);


JSTrapStatus InterruptCounterHandler(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate((JSObject*)closure);
	if ( --pv->interruptCounter != 0 )
		return JSTRAP_CONTINUE;
	return BreakHandler(cx, (JSObject*)closure, JL_CurrentStackFrame(cx), FROM_INTERRUPT);
}


JSTrapStatus TrapHandler(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, jsval closure) {

	return BreakHandler(cx, JSVAL_TO_OBJECT(closure), JL_CurrentStackFrame(cx), FROM_BREAKPOINT);
}


JSBool DebugErrorHookHandler(JSContext *cx, const char *message, JSErrorReport *report, void *closure) {

	JSStackFrame *fp = JL_CurrentStackFrame(cx);
	if ( !fp || !JS_GetFrameScript(cx, fp) )
		return JS_TRUE;
	JSTrapStatus status = BreakHandler(cx, (JSObject*)closure, fp, FROM_ERROR);
	return status == JSTRAP_ERROR ? JS_FALSE : JS_TRUE; // (TBD) check return value management
}


JSTrapStatus ThrowHookHandler(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	return BreakHandler(cx, (JSObject*)closure, JL_CurrentStackFrame(cx), FROM_THROW);
}


void* ExecuteHookHandler(JSContext *cx, JSStackFrame *fp, JSBool before, JSBool *ok, void *closure) {

//	if ( JS_IsNativeFrame(cx, fp) )
//		return NULL;
//	if ( before ) return closure;
	BreakHandler(cx, (JSObject*)closure, fp, FROM_EXECUTE); // (TBD) manage return value !
	return NULL; // hookData for the "after" stage.
}

void* FirstExecuteHookHandler(JSContext *cx, JSStackFrame *fp, JSBool before, JSBool *ok, void *closure) {

	BreakHandler(cx, (JSObject*)closure, fp, FROM_EXECUTE); // (TBD) manage return value !
	JS_SetExecuteHook(JL_GetRuntime(cx), NULL, NULL);
//	if ( status == JSTRAP_ERROR )
//		*ok == JS_FALSE; // ok is NULL when before is true !!!
	return NULL;
}

JSTrapStatus DebuggerKeyword(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	return BreakHandler(cx, (JSObject*)closure, JL_CurrentStackFrame(cx), FROM_DEBUGGER);
}


JSTrapStatus Step(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate((JSObject*)closure);
	if ( script == pv->script && JS_PCToLineNumber(cx, script, pc) == pv->lineno )
		return JSTRAP_CONTINUE;
//	if ( jsCodeSpec[*pc].format & JOF_DECLARING )
//		return JSTRAP_CONTINUE;
	return BreakHandler(cx, (JSObject*)closure, JL_CurrentStackFrame(cx), FROM_STEP);
}


JSTrapStatus StepOver(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate((JSObject*)closure);
	if ( script == pv->script && JS_PCToLineNumber(cx, script, pc) == pv->lineno )
		return JSTRAP_CONTINUE;
	JSStackFrame *fp = JL_CurrentStackFrame(cx);
	if ( fp != pv->frame && fp != pv->pframe )
		return JSTRAP_CONTINUE;
	return BreakHandler(cx, (JSObject*)closure, fp, FROM_STEP_OVER);
}


JSTrapStatus StepOut(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	// (TBD) manage return value with JS_GetFrameReturnValue()
	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate((JSObject*)closure);
	JSStackFrame *fp = JL_CurrentStackFrame(cx);
	if ( fp != pv->pframe )
		return JSTRAP_CONTINUE;
	return BreakHandler(cx, (JSObject*)closure, fp, FROM_STEP_OUT);
}


JSTrapStatus StepThrough(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate((JSObject*)closure);
	if ( script == pv->script && JS_PCToLineNumber(cx, script, pc) <= pv->lineno )
		return JSTRAP_CONTINUE;

	JSStackFrame *fp = JL_CurrentStackFrame(cx);
	if ( JL_StackSize(cx, fp)-1 > pv->stackFrameIndex )
		return JSTRAP_CONTINUE;
	return BreakHandler(cx, (JSObject*)closure, fp, FROM_STEP_THROUGH);
}


JSTrapStatus BreakHandler(JSContext *cx, JSObject *obj, JSStackFrame *fp, BreakReason breakOrigin) {

	jsval exception;
	jsval fval;
	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JSScript *script;
	script = JS_GetFrameScript(cx, fp);
	const char *filename;
	filename = script ? JS_GetScriptFilename(cx, script) : "<no_file>";
	if ( pv->excludedFiles && script && IsExcludedFile(&pv->excludedFiles, filename) )
		return JSTRAP_CONTINUE;

	if ( JS_GetProperty(cx, obj, "onBreak", &fval) == JS_FALSE )
		return JSTRAP_ERROR;
	if ( !JL_ValueIsCallable(cx, fval) ) // nothing to do
		return JSTRAP_CONTINUE;

	IFDEBUG( exception = JSVAL_VOID ); // avoid "potentially uninitialized local variable" warning

	JSBool hasException;
	hasException = JL_IsExceptionPending(cx);
	if ( hasException ) {

		JL_CHK( JS_GetPendingException(cx, &exception) );
		JS_ClearPendingException(cx);
	}
	
	JSRuntime *rt;
	rt = JL_GetRuntime(cx);
	unsigned lineno;
	lineno = JS_PCToLineNumber(cx, script, JS_GetFramePC(cx, fp));
	uint32_t stackFrameIndex;
	stackFrameIndex = JL_StackSize(cx, fp)-1;

	{
		jsval argv[] = {
			JSVAL_NULL, // argv[0] is reserved for the rval
			JSVAL_NULL,
			INT_TO_JSVAL( lineno ),
			INT_TO_JSVAL( breakOrigin ),
			INT_TO_JSVAL( stackFrameIndex ),
			BOOLEAN_TO_JSVAL(hasException),
			hasException ? exception : JSVAL_VOID,
			//breakOrigin == FROM_STEP_OUT ? fp->regs->sp[-1] : JSVAL_VOID; // (TBD) try to get the functions's rval. ask in the mailing list.
			JSVAL_NULL /*,
			BOOLEAN_TO_JSVAL(script && JS_GetFramePC(cx, fp) == script->code) // is entering function
			*/
		};
		JL_CHK( JL_NativeToJsval(cx, filename, &argv[1]) );
		
		// defer script's hooks assignment
		JSDebugHooks prevHooks;
		prevHooks = *JS_GetGlobalDebugHooks(rt); // save hooks
		pv->debugHooks = &prevHooks; // beware: reference to a local variable, dont return with restoring the previous value !

		// no hooks while onBreak is being called
		JL_CHK( JS_SetInterrupt(rt, NULL, NULL) ); // case: break on exception, continue, step
		JL_CHK( JS_SetDebuggerHandler(rt, NULL, NULL) );
		JL_CHK( JS_SetDebugErrorHook(rt, NULL, NULL) );
		JL_CHK( JS_SetThrowHook(rt, NULL, NULL) );
		JL_CHK( JS_SetExecuteHook(rt, NULL, NULL) );
		JS_SetNewScriptHookProc(rt, NULL, NULL); // beware: never remove JS_SetDestroyScriptHookProc hook !!!

		JSBool status;
		status = JS_CallFunctionValue(cx, obj, fval, COUNTOF(argv)-1, argv+1, &argv[0]);

		// apply the previous hooks that could be changed.
		JS_ASSERT( pv->debugHooks );
		JL_CHK( JS_SetInterrupt(rt, pv->debugHooks->interruptHook, pv->debugHooks->interruptHookData) );
		JL_CHK( JS_SetDebuggerHandler(rt, pv->debugHooks->debuggerHandler, pv->debugHooks->debuggerHandlerData) );
		JL_CHK( JS_SetDebugErrorHook(rt, pv->debugHooks->debugErrorHook, pv->debugHooks->debugErrorHookData) );
		JL_CHK( JS_SetThrowHook(rt, pv->debugHooks->throwHook, pv->debugHooks->throwHookData) );
		JL_CHK( JS_SetExecuteHook(rt, pv->debugHooks->executeHook, pv->debugHooks->executeHookData) );
		JS_SetNewScriptHookProc(rt, pv->debugHooks->newScriptHook, pv->debugHooks->newScriptHookData);

		pv->debugHooks = NULL;

		JL_CHK( status );

		JL_ASSERT_IS_INTEGER(argv[0], "onBreak return");

		if ( hasException ) // restore the exception
			JS_SetPendingException(cx, exception); // (TBD) should return JSTRAP_ERROR ???

		// store the current state
		pv->stackFrameIndex = stackFrameIndex;
		pv->script = script;
		pv->lineno = lineno;
		pv->frame = fp;
		pv->pframe = fp;
		JS_FrameIterator(cx, &pv->pframe); // gets the parent frame

		switch (JSVAL_TO_INT( argv[0] )) {

			case DO_CONTINUE:
				if ( pv->interruptCounterLimit ) {

					pv->interruptCounter = pv->interruptCounterLimit;
					JL_CHK( JS_SetInterrupt(rt, InterruptCounterHandler, obj) );
				} else {

					JL_CHK( JS_SetInterrupt(rt, NULL, NULL) );
				}
				break;
			case DO_STEP:
				JS_SetInterrupt(rt, Step, obj);
				break;
			case DO_STEP_OVER:
				JS_SetInterrupt(rt, StepOver, obj);
				break;
			case DO_STEP_THROUGH:
				JS_SetInterrupt(rt, StepThrough, obj);
				break;
			case DO_STEP_OUT:
				JS_SetInterrupt(rt, StepOut, obj);
				break;
		}
	}

	return JSTRAP_CONTINUE; // http://www.google.com/codesearch/p?hl=en#SPZdyP79RtQ/trunk/third_party/spidermonkey/js/src/jsinterp.c&q=JSTRAP_RETURN&l=742
bad:
	return JSTRAP_ERROR;
}


DEFINE_FINALIZE() {

	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate(obj);
	if ( !pv )
		return;
	JSRuntime *rt = fop->runtime();
	JL_CHK( JS_SetInterrupt(rt, NULL, NULL) );
	JL_CHK( JS_SetDebuggerHandler(rt, NULL, NULL) );
	JL_CHK( JS_SetDebugErrorHook(rt, NULL, NULL) );
	JL_CHK( JS_SetThrowHook(rt, NULL,NULL) );
	JL_CHK( JS_SetExecuteHook(rt, NULL, NULL) );

	if ( JL_GetHostPrivate(fop->runtime())->canSkipCleanup )
		return;

	if ( pv->excludedFiles )
		CleanExcludedFileList(&pv->excludedFiles);
	JS_freeop(fop, pv);
	return;
bad:
	// report a warning ?
	return;
}

/**doc
$TOC_MEMBER $INAME
 $INAME()
  Creates a new debugger object.
  When the program have to break because of a breakpoint, an error, a step, ... , the debugger calls its user-defined .onBreak function with the following arguments:
  * filename
  * lineno
  * breakOrigin
  * stackFrameIndex
  * hasException
  * exception
  * rval
  * isEnteringFunction

  For further information about the break context, you can call StackFrame(stackFrameIndex).

  $H example
  {{{
  var dbg = new Debugger();
  dbg.onBreak = function(filename, lineno, breakOrigin, stackFrameIndex, hasException, exception, rval, isEnteringFunction) {

   print( 'break at '+ filename +':'+ lineno + ' because '+breakOrigin ,'\n' );
  }}}
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	DebuggerPrivate *pv;
	pv = (DebuggerPrivate*)JS_malloc(cx, sizeof(DebuggerPrivate));
	JL_CHK( pv );
	memset(pv, 0, sizeof(DebuggerPrivate));

	JL_SetPrivate( obj, pv);

	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Methods ===
**/


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( polarity, filename, lineno )
 $INT $INAME( polarity, function, relativeLineno )
  Add or remove a breakpoint on the given filename:line or function.
  The function returns the actual line number where the breakpoint has been added or removed.
  If the file connot be reached, the function call failed with an "Invalid location" error.
  An existing breakpoint can be overwritten by a new breakpoint.
  Removing an nonexistent breakpoint does not matter.
**/
DEFINE_FUNCTION( toggleBreakpoint ) {

	jsval prevClosure;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN( 3 );

	bool polarity;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &polarity) );

	unsigned lineno;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &lineno) );

	JSScript *script;
	jsbytecode *pc;
	JL_CHK( GetScriptLocation(cx, &JL_ARG(2), lineno, &script, &pc) );
	if ( script == NULL ) {

		JL_ERR( E_ARG, E_NUM(2), E_SEP, E_LOCATION, E_INVALID );
		*JL_RVAL = JSVAL_ZERO;
		return JS_TRUE;
	}

	JSTrapHandler prevHandler;
	JS_ClearTrap(cx, script, pc, &prevHandler, &prevClosure);

	if ( polarity ) {
		JL_CHK( JS_SetTrap(cx, script, pc, TrapHandler, OBJECT_TO_JSVAL(obj)) );
	} else {
		if ( prevHandler != TrapHandler || prevClosure != OBJECT_TO_JSVAL(obj) )
			JL_CHK( JS_SetTrap(cx, script, pc, TrapHandler, OBJECT_TO_JSVAL(obj)) );
	}

	*JL_RVAL = INT_TO_JSVAL( JS_PCToLineNumber(cx, script, pc) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( filename, lineno )
 $BOOL $INAME( function, relativeLineno )
  Returns $TRUE if the given line (actual line number) has a breakpoint, otherwise returns $FALSE.
**/
DEFINE_FUNCTION( hasBreakpoint ) {

	jsval prevClosure;
	JL_ASSERT_ARGC_MIN( 2 );

	unsigned lineno;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &lineno) );

	JSScript *script;
	jsbytecode *pc;
	JL_CHK( GetScriptLocation(cx, &JL_ARG(1), lineno, &script, &pc) );
	if ( script == NULL ) {

		JL_ERR( E_ARG, E_NUM(1), E_SEP, E_LOCATION, E_INVALID );
		*JL_RVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	JSTrapHandler prevHandler;
	JS_ClearTrap(cx, script, pc, &prevHandler, &prevClosure);
	if ( prevHandler ) {

		JL_CHK( JS_SetTrap(cx, script, pc, prevHandler, prevClosure) );
		*JL_RVAL = JSVAL_TRUE;
	} else {

		*JL_RVAL = JSVAL_FALSE;
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Unconditionally remove all breakpoints.
**/
DEFINE_FUNCTION( clearBreakpoints ) {

	//JS_ClearAllTraps(cx);
	JS_ClearAllTrapsForCompartment(cx);
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
}




/**doc
=== Properties ===
**/



/**doc
$TOC_MEMBER $INAME
 $INT | $UNDEF $INAME
**/
DEFINE_PROPERTY_SETTER( interruptCounterLimit ) {

	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	if ( JSVAL_IS_VOID(*vp) ) {

		pv->interruptCounterLimit = 0;
	} else {

		JL_ASSERT_IS_INTEGER(*vp, "");
		JL_CHK( JL_JsvalToNative(cx, *vp, &pv->interruptCounterLimit) );
	}

	JSRuntime *rt;
	rt = JL_GetRuntime(cx);
	if ( pv->interruptCounterLimit == 0 && (pv->debugHooks ? pv->debugHooks->interruptHook : JS_GetGlobalDebugHooks(rt)->interruptHook) == InterruptCounterHandler ) {

		// cancel the current one.
		if ( pv->debugHooks ) { // defered hook assignment

			pv->debugHooks->interruptHook = NULL;
			pv->debugHooks->interruptHookData = NULL;
		} else {

			JL_CHK( JS_SetInterrupt(rt, NULL, NULL) );
		}
	}

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
  Sets whether the debugger breaks on errors.
**/
DEFINE_PROPERTY_SETTER( breakOnError ) {

	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	bool b;
	JL_CHK( JL_JsvalToNative(cx, *vp, &b) );

	if ( pv->debugHooks ) { // defered hook assignment

		pv->debugHooks->debugErrorHook = b ? DebugErrorHookHandler : NULL;
		pv->debugHooks->debugErrorHookData = b ? obj : NULL;
	} else {

		JL_CHK( JS_SetDebugErrorHook(JL_GetRuntime(cx), b ? DebugErrorHookHandler : NULL, b ? obj : NULL) );
	}

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
  Sets whether the debugger breaks on exceptions (throw).
**/
DEFINE_PROPERTY_SETTER( breakOnException ) {

	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	bool b;
	JL_CHK( JL_JsvalToNative(cx, *vp, &b) );

	if ( pv->debugHooks ) { // defered hook assignment

		pv->debugHooks->throwHook = b ? ThrowHookHandler : NULL;
		pv->debugHooks->throwHookData = b ? obj : NULL;
	} else {

		JL_CHK( JS_SetThrowHook(JL_GetRuntime(cx), b ? ThrowHookHandler : NULL, b ? obj : NULL) );
	}

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
  Sets whether the debugger breaks when encounters the 'debugger' keyword.
**/
DEFINE_PROPERTY_SETTER( breakOnDebuggerKeyword ) {

	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	bool b;
	JL_CHK( JL_JsvalToNative(cx, *vp, &b) );

	if ( pv->debugHooks ) { // defered hook assignment

		pv->debugHooks->debuggerHandler = b ? DebuggerKeyword : NULL;
		pv->debugHooks->debuggerHandlerData = b ? obj : NULL;
	} else {

		JL_CHK( JS_SetDebuggerHandler(JL_GetRuntime(cx), b ? DebuggerKeyword : NULL,  b ? obj : NULL) );
	}

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
  Sets whether the debugger breaks when a script is about to be executed.
**/
DEFINE_PROPERTY_SETTER( breakOnExecute ) {

	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	bool b;
	JL_CHK( JL_JsvalToNative(cx, *vp, &b) );

	if ( pv->debugHooks ) { // defered hook assignment

		pv->debugHooks->executeHook = b ? ExecuteHookHandler : NULL;
		pv->debugHooks->executeHookData = b ? obj : NULL;
	} else {

		JL_CHK( JS_SetExecuteHook(JL_GetRuntime(cx), b ? ExecuteHookHandler : NULL, b ? obj : NULL) );
	}

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
  Sets whether the debugger breaks on the first script execution.
**/
DEFINE_PROPERTY_SETTER( breakOnFirstExecute ) {

	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	bool b;
	JL_CHK( JL_JsvalToNative(cx, *vp, &b) );

	if ( pv->debugHooks ) { // defered hook assignment

		pv->debugHooks->executeHook = b ? FirstExecuteHookHandler : NULL;
		pv->debugHooks->executeHookData = b ? obj : NULL;
	} else {

		JL_CHK( JS_SetExecuteHook(JL_GetRuntime(cx), b ? FirstExecuteHookHandler : NULL, b ? obj : NULL) );
	}

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
  Is the list of filename where the debugger never breaks.
**/
DEFINE_PROPERTY_SETTER( excludedFileList ) {

	jsval tmp;
	DebuggerPrivate *pv = (DebuggerPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_ASSERT_IS_ARRAY( *vp, "" );

	JSObject *arrayObject;
	arrayObject = JSVAL_TO_OBJECT( *vp );

	if ( pv->excludedFiles )
		CleanExcludedFileList(&pv->excludedFiles);

	unsigned length;
	JL_CHK( JS_GetArrayLength(cx, arrayObject, &length) );

	for ( unsigned i = 0; i < length; ++i ) {

		JLData fileName;
		JL_CHK( JL_GetElement(cx, arrayObject, i, &tmp ) );
		JL_CHK( JL_JsvalToNative(cx, tmp, &fileName) );
		AddExcludedFile(&pv->excludedFiles, fileName.GetStrZOwnership());
	}

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( toggleBreakpoint )
		FUNCTION( hasBreakpoint )
		FUNCTION( clearBreakpoints )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_SETTER( excludedFileList )
		PROPERTY_SETTER( interruptCounterLimit )
		PROPERTY_SETTER( breakOnError )
		PROPERTY_SETTER( breakOnException )
		PROPERTY_SETTER( breakOnDebuggerKeyword )
		PROPERTY_SETTER( breakOnExecute )
		PROPERTY_SETTER( breakOnFirstExecute )
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST
		CONST_INTEGER_SINGLE( DO_CONTINUE )
		CONST_INTEGER_SINGLE( DO_STEP )
		CONST_INTEGER_SINGLE( DO_STEP_OVER )
		CONST_INTEGER_SINGLE( DO_STEP_THROUGH )
		CONST_INTEGER_SINGLE( DO_STEP_OUT )
		CONST_INTEGER_SINGLE( FROM_INTERRUPT )
		CONST_INTEGER_SINGLE( FROM_BREAKPOINT )
		CONST_INTEGER_SINGLE( FROM_STEP )
		CONST_INTEGER_SINGLE( FROM_STEP_OVER )
		CONST_INTEGER_SINGLE( FROM_STEP_THROUGH )
		CONST_INTEGER_SINGLE( FROM_STEP_OUT )
		CONST_INTEGER_SINGLE( FROM_THROW )
		CONST_INTEGER_SINGLE( FROM_ERROR )
		CONST_INTEGER_SINGLE( FROM_DEBUGGER )
		CONST_INTEGER_SINGLE( FROM_EXECUTE )
		CONST_INTEGER_SINGLE( FROM_CALL )
	END_CONST

END_CLASS


	//	case STEP_OVER:
	///*
	//		//jsbytecode *nextPc = JS_LineNumberToPC(cx, script, JS_PCToLineNumber(cx, script, pc+1) + 1);
	////		jsbytecode *nextPc = JS_GetFramePC(cx, caller);
	////		JSOp op = JS_GetTrapOpcode(cx, caller->script, nextPc);
	////		nextPc += jsCodeSpec[op].length;
	////		JS_SetTrap(cx, caller->script, nextPc, InterruptHandler, NULL);
	//		JSOp op = JS_GetTrapOpcode(cx, caller->script, nextPc);
	//		JS_SetInterrupt(JL_GetRuntime(cx), InterruptHandler, NULL);
	//		jsbytecode *nextPc = JS_GetFramePC(cx, caller);
	//		JSOp op = JS_GetTrapOpcode(cx, caller->script, nextPc);
	//		nextPc += jsCodeSpec[op].length;
	//		JS_SetTrap(cx, caller->script, nextPc, InterruptHandler, NULL);
	//*/

	//	case STEP_OUT:

	///*
	//		JSStackFrame *frame;
	//		frame = JS_GetScriptedCaller(cx, NULL);
	//		// check if there is a frame to return to, else continue
	//		if ( !frame->down )
	//			return JSTRAP_CONTINUE;
	//		frame = JS_GetScriptedCaller(cx, frame->down);
	//		JSScript *nextScript = JS_GetFrameScript(cx, frame);
	//		jsbytecode *nextPc = JS_GetFramePC(cx, frame);
	////		unsigned lineno = JS_PCToLineNumber(cx, nextScript, nextPc);
	////		unsigned lastlineno = JS_GetScriptBaseLineNumber(cx, nextScript) + JS_GetScriptLineExtent(cx, nextScript);
	////		if ( lineno + 1 >= lastlineno )
	////			return JSTRAP_CONTINUE;
	////		nextPc = JS_LineNumberToPC(cx, nextScript, lineno +1 );
	////		JS_ClearTrap(cx, nextScript, nextPc, NULL, NULL);
	//		// set the next trap on the next opcode
	//		JSOp op = JS_GetTrapOpcode(cx, nextScript, nextPc);
	//		nextPc += jsCodeSpec[op].length;
	//		JSBool status = JS_SetTrap(cx, nextScript, nextPc, InterruptHandler, (void*)PRIVATE_TO_JSVAL(NULL));
	//*/
	//		break;
