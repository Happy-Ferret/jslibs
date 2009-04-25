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

#include "../common/stack.h"

#define OBJ_PROP_FLAGS (JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)


extern jl::Queue *scriptFileList;

static const JSCodeSpec jsCodeSpec[] = {
	#define OPDEF(op,val,name,token,length,nuses,ndefs,prec,format) {length,nuses,ndefs,prec,format},
	#include "jsopcode.tbl"
	#undef OPDEF
};


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 2290 $
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


struct Private {

	void *excludedFiles;
	JSDebugHooks *debugHooks; // current hooks cannot be changed while onBreak is being called.
	// previous break state
	unsigned int stackFrameIndex;
	JSStackFrame *frame;
	JSStackFrame *pframe;
	JSScript *script;
	uintN lineno;
};


inline JSStackFrame* CurrentStackFrame(JSContext *cx) {

	JSStackFrame *fp = NULL;
	return JS_FrameIterator(cx, &fp);
}

inline unsigned int StackSize(JSContext *cx, const JSStackFrame *frame) {

	unsigned int length = 0;
	for ( JSStackFrame *fp = CurrentStackFrame(cx); fp; JS_FrameIterator(cx, &fp) )
		length++;
	return length; // 0 is the first frame
}


JSStackFrame *StackFrameByIndex(JSContext *cx, unsigned int frameIndex) {

	JSStackFrame *fp = CurrentStackFrame(cx);
	unsigned int currentFrameIndex;
	currentFrameIndex = StackSize(cx, fp)-1;

	if ( frameIndex > currentFrameIndex )
		return NULL;

	// select the right frame
	while ( fp && currentFrameIndex > frameIndex ) {

		//frame = frame->down;
		JS_FrameIterator(cx, &fp);
		currentFrameIndex--;
	}
	return fp;
}


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

		if ( lineno >= script->lineno && lineno <= script->lineno + extent )
			break;
		// else the last script in the list (depth 0) will be selected
	}

	return script;
}


static JSTrapStatus BreakHandler(JSContext *cx, JSObject *obj, JSStackFrame *fp, BreakReason breakOrigin);


static JSTrapStatus TrapHandler(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	return BreakHandler(cx, (JSObject*)closure, CurrentStackFrame(cx), FROM_BREAKPOINT);
}


JSBool DebugErrorHookHandler(JSContext *cx, const char *message, JSErrorReport *report, void *closure) {

	JSStackFrame *fp = CurrentStackFrame(cx);
	if ( !fp )
		return JS_TRUE;
	JSTrapStatus status = BreakHandler(cx, (JSObject*)closure, fp, FROM_ERROR);
	return status == JSTRAP_ERROR ? JS_FALSE : JS_TRUE; // (TBD) check return value management
}


static JSTrapStatus ThrowHookHandler(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	return BreakHandler(cx, (JSObject*)closure, CurrentStackFrame(cx), FROM_THROW);
}


static void* ExecuteHookHandler(JSContext *cx, JSStackFrame *fp, JSBool before, JSBool *ok, void *closure) {

//	if ( JS_IsNativeFrame(cx, fp) )
//		return NULL;
//	if ( before ) return closure;
	JSBool status = BreakHandler(cx, (JSObject*)closure, fp, FROM_EXECUTE); // (TBD) manage return value
	return NULL; // hookData for the "after" stage.
}


static JSTrapStatus DebuggerKeyword(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	return BreakHandler(cx, (JSObject*)closure, CurrentStackFrame(cx), FROM_DEBUGGER);
}


static JSTrapStatus Step(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	Private *pv = (Private*)JS_GetPrivate(cx, (JSObject*)closure);
	if ( script == pv->script && JS_PCToLineNumber(cx, script, pc) == pv->lineno )
		return JSTRAP_CONTINUE;
	if ( jsCodeSpec[*pc].format & JOF_DECLARING )
		return JSTRAP_CONTINUE;
	return BreakHandler(cx, (JSObject*)closure, CurrentStackFrame(cx), FROM_STEP);
}


static JSTrapStatus StepOver(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	Private *pv = (Private*)JS_GetPrivate(cx, (JSObject*)closure);
	if ( script == pv->script && JS_PCToLineNumber(cx, script, pc) == pv->lineno )
		return JSTRAP_CONTINUE;
	JSStackFrame *fp = CurrentStackFrame(cx);
	if ( fp != pv->frame && fp != pv->pframe )
		return JSTRAP_CONTINUE;
	return BreakHandler(cx, (JSObject*)closure, fp, FROM_STEP_OVER);
}


static JSTrapStatus StepOut(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	Private *pv = (Private*)JS_GetPrivate(cx, (JSObject*)closure);
	JSStackFrame *fp = CurrentStackFrame(cx);
	if ( fp != pv->pframe )
		return JSTRAP_CONTINUE;
	return BreakHandler(cx, (JSObject*)closure, fp, FROM_STEP_OUT);
}


static JSTrapStatus StepThrough(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	Private *pv = (Private*)JS_GetPrivate(cx, (JSObject*)closure);
	if ( script == pv->script && JS_PCToLineNumber(cx, script, pc) <= pv->lineno )
		return JSTRAP_CONTINUE;
	JSStackFrame *fp = CurrentStackFrame(cx);
	if ( StackSize(cx, fp)-1 > pv->stackFrameIndex )
		return JSTRAP_CONTINUE;
	return BreakHandler(cx, (JSObject*)closure, fp, FROM_STEP_THROUGH);
}


static JSTrapStatus BreakHandler(JSContext *cx, JSObject *obj, JSStackFrame *fp, BreakReason breakOrigin) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);

	JSScript *script = JS_GetFrameScript(cx, fp);
	const char *filename;
	filename = JS_GetScriptFilename(cx, script);
	if ( pv->excludedFiles && IsExcludedFile(&pv->excludedFiles, filename) )
		return JSTRAP_CONTINUE;

	jsval fval;
	if ( JS_GetProperty(cx, obj, "onBreak", &fval) == JS_FALSE )
		return JSTRAP_ERROR;
	if ( !JsvalIsFunction(cx, fval) ) // nothing to do
		return JSTRAP_CONTINUE;

	jsval argv[11] = { JSVAL_NULL };
	JSTempValueRooter tvr;
	JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr);

	jsval exception;
	JSBool hasException = JS_IsExceptionPending(cx);
	if ( hasException ) {

		J_CHK( JS_GetPendingException(cx, &exception) );
		JS_ClearPendingException(cx);
	}

	JSRuntime *rt = JS_GetRuntime(cx);
	uintN lineno = JS_PCToLineNumber(cx, script, JS_GetFramePC(cx, fp));
	unsigned int stackFrameIndex = StackSize(cx, fp)-1;

	J_CHK( StringToJsval(cx, filename, &argv[1]) );
	argv[2] = INT_TO_JSVAL( lineno );
	argv[3] = OBJECT_TO_JSVAL( JS_GetFrameScopeChain(cx, fp) );
	argv[4] = OBJECT_TO_JSVAL( JS_GetFrameFunctionObject(cx, fp) );
	argv[5] = INT_TO_JSVAL( breakOrigin );
	argv[6] = INT_TO_JSVAL( stackFrameIndex );
	argv[7] = hasException ? JSVAL_TRUE : JSVAL_FALSE;
	argv[8] = hasException ? exception : JSVAL_VOID;
	argv[9] = breakOrigin == FROM_STEP_OUT ? fp->regs->sp[-1] : JSVAL_VOID; // try to the the functions's rval
	argv[10] = JS_GetFramePC(cx, fp) == script->code ? JSVAL_TRUE : JSVAL_FALSE; // is entering function

	JSDebugHooks prevHooks;
	prevHooks = *JS_GetGlobalDebugHooks(rt); // save hooks
	pv->debugHooks = &prevHooks; // beware: reference to a local variable, dont return with restoring the previous value !

	// no hooks while onBreak is being called
	JS_SetInterrupt(rt, NULL, NULL); // case: break on exception, continue, step
	JS_SetDebuggerHandler(rt, NULL, NULL);
	JS_SetDebugErrorHook(rt, NULL, NULL);
	JS_SetThrowHook(rt, NULL, NULL);
	JS_SetExecuteHook(rt, NULL, NULL);
	JS_SetNewScriptHookProc(rt, NULL, NULL); // beware: never remove JS_SetDestroyScriptHookProc hook !!!

	JSBool status;
	status = JS_CallFunctionValue(cx, obj, fval, COUNTOF(argv)-1, argv+1, &argv[0]);

	pv->debugHooks = JS_GetGlobalDebugHooks(rt); // restore debug hooks
	*JS_GetGlobalDebugHooks(rt) = prevHooks;

	J_CHK( status );

	J_S_ASSERT_INT( argv[0] );

	if ( hasException ) // restore the exception
		JS_SetPendingException(cx, exception); // (TBD) should return JSTRAP_ERROR ???

	// store the current state
	pv->stackFrameIndex = stackFrameIndex;
	pv->script = script;
	pv->lineno = lineno;
	pv->frame = fp;
	pv->pframe = fp;
	JS_FrameIterator(cx, &pv->pframe);

	switch (JSVAL_TO_INT( argv[0] )) {

		case DO_CONTINUE:
			JS_SetInterrupt(rt, NULL, NULL);
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

	JS_POP_TEMP_ROOT(cx, &tvr);
	return JSTRAP_CONTINUE; // http://www.google.com/codesearch/p?hl=en#SPZdyP79RtQ/trunk/third_party/spidermonkey/js/src/jsinterp.c&q=JSTRAP_RETURN&l=742
bad:
	JS_POP_TEMP_ROOT(cx, &tvr);
	return JSTRAP_ERROR;
}


DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( !pv )
		return;

	JSRuntime *rt = JS_GetRuntime(cx);
	JS_SetDebuggerHandler(rt, NULL, NULL);
	JS_SetDebugErrorHook(rt, NULL, NULL);
	JS_SetThrowHook(rt, NULL,NULL);
	JS_SetExecuteHook(rt, NULL, NULL);

	if ( pv->excludedFiles )
		CleanExcludedFileList(&pv->excludedFiles);

	free(pv);
}

/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME();
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

	Private *pv;
	pv = (Private*)malloc(sizeof(Private));
	J_S_ASSERT_ALLOC(pv);
	memset(pv, 0, sizeof(Private));
	J_CHK( JS_SetPrivate(cx, obj, pv) );
	pv->debugHooks = JS_GetGlobalDebugHooks(JS_GetRuntime(cx));

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT | $BOOL $INAME( filename, lineno );
**/
DEFINE_FUNCTION_FAST( GetActualLineno ) {

	const char *filename;
	J_CHK( JsvalToString(cx, &J_FARG(1), &filename) );

	uintN lineno;
	J_CHK( JsvalToUInt(cx, J_FARG(2), &lineno) );

	JSScript *script;
	script = ScriptByLocation(cx, scriptFileList, filename, lineno);
	if ( script == NULL ) {

		*J_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	*J_FRVAL = INT_TO_JSVAL(JS_PCToLineNumber(cx, script, JS_LineNumberToPC(cx, script, lineno)));
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( polarity, filename, lineno );
**/
DEFINE_FUNCTION_FAST( ToggleBreakpoint ) {

	J_S_ASSERT_ARG_MIN( 3 );

	bool polarity;
	J_CHK( JsvalToBool(cx, J_FARG(1), &polarity) );

	const char *filename;
	J_CHK( JsvalToString(cx, &J_FARG(2), &filename) );

	uintN lineno;
	J_CHK( JsvalToUInt(cx, J_FARG(3), &lineno) );

	JSScript *script;
	script = ScriptByLocation(cx, scriptFileList, filename, lineno);
	//J_S_ASSERT( script != NULL, "Invalid breakpoint location.");
	if ( script == NULL )
		J_REPORT_ERROR_2("Invalid location (%s:%d)", filename, lineno);

	jsbytecode *pc;
	pc = JS_LineNumberToPC(cx, script, lineno);
	lineno = JS_PCToLineNumber(cx, script, pc);

	JSTrapHandler prevHandler;
	void *prevClosure;
	JS_ClearTrap(cx, script, pc, &prevHandler, &prevClosure);

	if ( polarity ) {
		J_CHK( JS_SetTrap(cx, script, pc, TrapHandler, J_FOBJ) );
	} else {
		if ( prevHandler != TrapHandler || prevClosure != J_FOBJ )
			J_CHK( JS_SetTrap(cx, script, pc, TrapHandler, J_FOBJ) );
	}

	*J_FRVAL = INT_TO_JSVAL(lineno);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( filename, lineno );
**/
DEFINE_FUNCTION_FAST( HasBreakpoint ) {

	J_S_ASSERT_ARG_MIN( 2 );
	const char *filename;
	J_CHK( JsvalToString(cx, &J_FARG(1), &filename) );
	uintN lineno;
	J_CHK( JsvalToUInt(cx, J_FARG(2), &lineno) );
	JSScript *script;
	script = ScriptByLocation(cx, scriptFileList, filename, lineno);
	if ( script == NULL )
		J_REPORT_ERROR_2("Invalid location (%s:%d)", filename, lineno);

	jsbytecode *pc;
	pc = JS_LineNumberToPC(cx, script, lineno);
	lineno = JS_PCToLineNumber(cx, script, pc);

	JSTrapHandler prevHandler;
	void *prevClosure;
	JS_ClearTrap(cx, script, pc, &prevHandler, &prevClosure);
	if ( prevHandler ) {

		J_CHK( JS_SetTrap(cx, script, pc, prevHandler, prevClosure) );
		*J_FRVAL = JSVAL_TRUE;
	} else {

		*J_FRVAL = JSVAL_FALSE;
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT | $BOOL $INAME();
**/
DEFINE_FUNCTION_FAST( ClearBreakpoints ) {

	JS_ClearAllTraps(cx);
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME( frameLevel );
**/
DEFINE_FUNCTION_FAST( StackFrame ) {

	J_S_ASSERT_ARG_MIN( 1 );

	unsigned int frameIndex;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &frameIndex) );

	JSStackFrame *frame;
	frame = StackFrameByIndex(cx, frameIndex);

	if ( frame == NULL ) {

		*J_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	JSObject *stackItem;
	stackItem = JS_NewObject(cx, NULL, NULL, NULL);
	J_S_ASSERT_ALLOC( stackItem );
	*J_FRVAL = OBJECT_TO_JSVAL( stackItem );

	JSScript *script;
	script = JS_GetFrameScript(cx, frame);
	jsbytecode *pc;
	pc = JS_GetFramePC(cx, frame);

	jsval tmp;
	J_CHK( StringToJsval(cx, JS_GetScriptFilename(cx, script), &tmp) );
	J_CHK( JS_DefineProperty(cx, stackItem, "filename", tmp, NULL, NULL, OBJ_PROP_FLAGS) );

	J_CHK( JS_DefineProperty(cx, stackItem, "lineno", INT_TO_JSVAL(JS_PCToLineNumber(cx, script, pc)), NULL, NULL, OBJ_PROP_FLAGS) );

	JSObject *callee;
	callee = JS_GetFrameCalleeObject(cx, frame);
	J_CHK( JS_DefineProperty(cx, stackItem, "callee", callee ? OBJECT_TO_JSVAL(callee) : JSVAL_VOID, NULL, NULL, OBJ_PROP_FLAGS) );
	J_CHK( JS_DefineProperty(cx, stackItem, "baseLineNumber", INT_TO_JSVAL( JS_GetScriptBaseLineNumber(cx, script) ), NULL, NULL, OBJ_PROP_FLAGS) );
	J_CHK( JS_DefineProperty(cx, stackItem, "lineExtent", INT_TO_JSVAL( JS_GetScriptLineExtent(cx, script) ), NULL, NULL, OBJ_PROP_FLAGS) );

	J_CHK( JS_DefineProperty(cx, stackItem, "scope", OBJECT_TO_JSVAL(JS_GetFrameScopeChain(cx, frame)), NULL, NULL, OBJ_PROP_FLAGS) );
	J_CHK( JS_DefineProperty(cx, stackItem, "variables", frame->varobj ? OBJECT_TO_JSVAL(frame->varobj) : JSVAL_VOID, NULL, NULL, OBJ_PROP_FLAGS) );

	J_CHK( JS_DefineProperty(cx, stackItem, "this", OBJECT_TO_JSVAL(JS_GetFrameThis(cx, frame)), NULL, NULL, OBJ_PROP_FLAGS) );

	if ( frame->argv ) {

		JSObject *arguments;
		arguments = JS_NewArrayObject(cx, frame->argc, frame->argv);
		J_CHK( JS_DefineProperty(cx, stackItem, "argv", OBJECT_TO_JSVAL(arguments), NULL, NULL, OBJ_PROP_FLAGS) );
	} else {

		J_CHK( JS_DefineProperty(cx, stackItem, "argv", JSVAL_VOID, NULL, NULL, OBJ_PROP_FLAGS) );
	}
	J_CHK( JS_DefineProperty(cx, stackItem, "rval", JS_GetFrameReturnValue(cx, frame), NULL, NULL, OBJ_PROP_FLAGS) );

	J_CHK( JS_DefineProperty(cx, stackItem, "isNative", BOOLEAN_TO_JSVAL(JS_IsNativeFrame(cx, frame)), NULL, NULL, OBJ_PROP_FLAGS) );
	J_CHK( JS_DefineProperty(cx, stackItem, "isConstructing", JS_IsConstructorFrame(cx, frame) ? JSVAL_TRUE : JSVAL_FALSE, NULL, NULL, OBJ_PROP_FLAGS) );
	J_CHK( JS_DefineProperty(cx, stackItem, "isEval", frame->flags & JSFRAME_EVAL ? JSVAL_TRUE : JSVAL_FALSE, NULL, NULL, OBJ_PROP_FLAGS) );
	J_CHK( JS_DefineProperty(cx, stackItem, "isAssigning", frame->flags & JSFRAME_ASSIGNING ? JSVAL_TRUE : JSVAL_FALSE, NULL, NULL, OBJ_PROP_FLAGS) );

//	J_CHK( JS_DefineProperty(cx, stackItem, "opnd", frame->regs->sp[-1], NULL, NULL, OBJ_PROP_FLAGS) );
//	char * s = JS_GetStringBytes(JS_ValueToString(cx, frame->regs->sp[-1]));

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME( code, frameLevel );
**/
DEFINE_FUNCTION_FAST( EvalInStackFrame ) {

	J_S_ASSERT_ARG_MIN( 2 );

	J_S_ASSERT_STRING( J_FARG(1) );

	unsigned int frameIndex;
	J_CHK( JsvalToUInt(cx, J_FARG(2), &frameIndex) );

	JSStackFrame *frame;
	frame = StackFrameByIndex(cx, frameIndex);

	if ( frame == NULL ) {

		*J_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	JSScript *script;
	script = JS_GetFrameScript(cx, frame);

	jsbytecode *pc;
	pc = JS_GetFramePC(cx, frame);

	JSString *jsstr;
	jsstr = JSVAL_TO_STRING( J_FARG(1) );
	J_CHK( JS_EvaluateUCInStackFrame(cx, frame, JS_GetStringChars(jsstr), JS_GetStringLength(jsstr), JS_GetScriptFilename(cx, script), JS_PCToLineNumber(cx, script, pc), J_FRVAL) );

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME( function );
**/
DEFINE_FUNCTION_FAST( DefinitionLocation ) {

	J_S_ASSERT_ARG_MIN( 1 );

	JSScript *script;
	script = NULL;

	if ( JsvalIsFunction(cx, J_FARG(1)) ) {

		JSFunction *fun;
		fun = JS_ValueToFunction(cx, J_FARG(1));
		if ( FUN_SCRIPT(fun) )
			script = JS_GetFunctionScript(cx, fun);
		goto next;
	}

	if ( !JSVAL_IS_PRIMITIVE( J_FARG(1) ) ) {

		JSObject* obj;
		obj = JS_GetConstructor(cx, JSVAL_TO_OBJECT( J_FARG(1) ));
		JSFunction *fun;
		fun = JS_ValueToFunction(cx, OBJECT_TO_JSVAL( obj ) );
		if ( fun ) {

			script = JS_GetFunctionScript(cx, fun);
			if ( script )
				goto next;
		}
	}

	if ( JsvalIsScript(cx, J_FARG(1)) ) {

		JSObject* obj;
		obj = JSVAL_TO_OBJECT(J_FARG(1));
		script = (JSScript*)JS_GetPrivate(cx, obj);
	}


next:
	if ( !script ) {

		*J_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	jsval values[2];
	J_CHK( StringToJsval(cx, script->filename, &values[0]) );
	IntToJsval(cx, script->lineno, &values[1] );
	*J_FRVAL = OBJECT_TO_JSVAL( JS_NewArrayObject(cx, COUNTOF(values), values) );

	return JS_TRUE;
	JL_BAD;
}



/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
**/
DEFINE_PROPERTY( scriptList ) {

	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
	J_S_ASSERT_ALLOC(arr);
	*vp = OBJECT_TO_JSVAL(arr);

	int index = 0;
	for ( jl::QueueCell *it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

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


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
**/
DEFINE_PROPERTY( stackSize ) {

	JSStackFrame *fp = NULL;
	JS_FrameIterator(cx, &fp);
	return IntToJsval(cx, StackSize(cx, fp), vp);
}


/*
DEFINE_PROPERTY( breakpointList ) {

	JSRuntime *rt = JS_GetRuntime(cx);
	JSTrap *trap;

    for (trap = (JSTrap *)rt->trapList.next; &trap->links != &rt->trapList; trap = (JSTrap *)trap->links.next) {

		 if ( trap->handler == TrapHandler ) {
		 }
//		 if (trap->script == script && trap->pc == pc)
//            return trap;
    }
}
*/

/** doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
**/
/*
DEFINE_PROPERTY( pendingException ) {

	*vp = cx->exception;
	return JS_TRUE;
}
*/

/*
static JSBool
Disassemble(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSBool lines;
    uintN i;
    JSScript *script;

    if (argc > 0 &&
        JSVAL_IS_STRING(argv[0]) &&
        !strcmp(JS_GetStringBytes(JSVAL_TO_STRING(argv[0])), "-l")) {
        lines = JS_TRUE;
        argv++, argc--;
    } else {
        lines = JS_FALSE;
    }
    for (i = 0; i < argc; i++) {
        script = ValueToScript(cx, argv[i]);
        if (!script)
            return JS_FALSE;
        if (VALUE_IS_FUNCTION(cx, argv[i])) {
            JSFunction *fun = JS_ValueToFunction(cx, argv[i]);
            if (fun && (fun->flags & JSFUN_FLAGS_MASK)) {
                uint16 flags = fun->flags;
                fputs("flags:", stdout);

#define SHOW_FLAG(flag) if (flags & JSFUN_##flag) fputs(" " #flag, stdout);

                SHOW_FLAG(LAMBDA);
                SHOW_FLAG(SETTER);
                SHOW_FLAG(GETTER);
                SHOW_FLAG(BOUND_METHOD);
                SHOW_FLAG(HEAVYWEIGHT);
                SHOW_FLAG(THISP_STRING);
                SHOW_FLAG(THISP_NUMBER);
                SHOW_FLAG(THISP_BOOLEAN);
                SHOW_FLAG(EXPR_CLOSURE);
                SHOW_FLAG(INTERPRETED);

#undef SHOW_FLAG
                putchar('\n');
            }
        }

        if (!js_Disassemble(cx, script, lines, stdout))
            return JS_FALSE;
        SrcNotes(cx, script);
        TryNotes(cx, script);
    }
    return JS_TRUE;
}
*/


DEFINE_PROPERTY( breakOnError ) {

	bool b;
	J_CHK( JsvalToBool(cx, *vp, &b) );
	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	pv->debugHooks->debugErrorHook = b ? DebugErrorHookHandler : NULL;
	pv->debugHooks->debugErrorHookData = b ? obj : NULL;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( breakOnException ) {

	bool b;
	J_CHK( JsvalToBool(cx, *vp, &b) );
	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	pv->debugHooks->throwHook = b ? ThrowHookHandler : NULL;
	pv->debugHooks->throwHookData = b ? obj : NULL;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( breakOnDebuggerKeyword ) {

	bool b;
	J_CHK( JsvalToBool(cx, *vp, &b) );
	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	pv->debugHooks->debuggerHandler = b ? DebuggerKeyword : NULL;
	pv->debugHooks->debuggerHandlerData = b ? obj : NULL;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( breakOnExecute ) {

	bool b;
	J_CHK( JsvalToBool(cx, *vp, &b) );
	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	pv->debugHooks->executeHook = b ? ExecuteHookHandler : NULL;
	pv->debugHooks->executeHookData = b ? obj : NULL;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( excludedFileList ) {

	J_S_ASSERT_ARRAY( *vp );

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);

	JSObject *arrayObject = JSVAL_TO_OBJECT( *vp );

	if ( pv->excludedFiles )
		CleanExcludedFileList(&pv->excludedFiles);

	jsuint length;
	J_CHK( JS_GetArrayLength(cx, arrayObject, &length) );

	jsval tmp;
	const char *buffer;
	char *filename;
	for ( jsuint i = 0; i < length; ++i ) {

		J_CHK( JS_GetElement(cx, arrayObject, i, &tmp ) );
		J_CHK( JsvalToString(cx, &tmp, &buffer) );
		filename = strdup(buffer);
		J_S_ASSERT_ALLOC( filename );
		AddExcludedFile(&pv->excludedFiles, filename);
	}

	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( currentFilename ) {

	JSStackFrame *fp = NULL;
	JS_FrameIterator(cx, &fp);
	JSScript *script = JS_GetFrameScript(cx, fp);
	const char *filename;
	filename = JS_GetScriptFilename(cx, script);
	J_CHK( StringToJsval(cx, filename, vp) );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision: 2290 $"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( GetActualLineno )
		FUNCTION_FAST( ToggleBreakpoint )
		FUNCTION_FAST( HasBreakpoint )
		FUNCTION_FAST( ClearBreakpoints )
		FUNCTION_FAST( StackFrame )
		FUNCTION_FAST( EvalInStackFrame )
		FUNCTION_FAST( DefinitionLocation )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( scriptList )
		PROPERTY_READ( stackSize )
		PROPERTY_WRITE_STORE( excludedFileList )
		PROPERTY_WRITE_STORE( breakOnError )
		PROPERTY_WRITE_STORE( breakOnException )
		PROPERTY_WRITE_STORE( breakOnDebuggerKeyword )
		PROPERTY_WRITE_STORE( breakOnExecute )
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ( currentFilename )
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER_SINGLE( DO_CONTINUE )
		CONST_INTEGER_SINGLE( DO_STEP )
		CONST_INTEGER_SINGLE( DO_STEP_OVER )
		CONST_INTEGER_SINGLE( DO_STEP_THROUGH )
		CONST_INTEGER_SINGLE( DO_STEP_OUT )
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
	END_CONST_INTEGER_SPEC

END_CLASS


	//	case STEP_OVER:
	///*
	//		//jsbytecode *nextPc = JS_LineNumberToPC(cx, script, JS_PCToLineNumber(cx, script, pc+1) + 1);
	////		jsbytecode *nextPc = JS_GetFramePC(cx, caller);
	////		JSOp op = JS_GetTrapOpcode(cx, caller->script, nextPc);
	////		nextPc += jsCodeSpec[op].length;
	////		JS_SetTrap(cx, caller->script, nextPc, InterruptHandler, NULL);
	//		JSOp op = JS_GetTrapOpcode(cx, caller->script, nextPc);
	//		JS_SetInterrupt(JS_GetRuntime(cx), InterruptHandler, NULL);
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
	////		uintN lineno = JS_PCToLineNumber(cx, nextScript, nextPc);
	////		uintN lastlineno = JS_GetScriptBaseLineNumber(cx, nextScript) + JS_GetScriptLineExtent(cx, nextScript);
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
