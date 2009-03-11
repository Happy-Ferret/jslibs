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

#define OBJ_PROP_FLAGS (JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)


extern jl::Queue *scriptFileList;

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

// Action
#define CONTINUE 1
#define STEP 2
#define STEP_OVER 3
#define STEP_THROUGH 4
#define STEP_OUT 5

// Reason
#define FROM_BREAKPOINT 1
#define FROM_STEP 2
#define FROM_THROW 3
#define FROM_ERROR 4
#define FROM_DEBUGGER 5


struct Private {

	// previous break state
	int stackFrameIndex;
	JSStackFrame *frame;
	JSStackFrame *pframe;
	JSScript *script;
	uintN lineno;
};

unsigned int StackSize(const JSStackFrame *frame) {

	unsigned int length;
	for ( length = 0; frame; frame = frame->down, length++ );
	return length; // 0 is the first frame
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



static JSTrapStatus BreakHandler(JSContext *cx, JSObject *obj, JSScript *script, jsbytecode *pc, int breakOrigin);

static JSTrapStatus DebuggerKeyword(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	return BreakHandler(cx, (JSObject*)closure, script, pc, FROM_DEBUGGER);
}

static JSTrapStatus Step(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	Private *pv = (Private*)JS_GetPrivate(cx, (JSObject*)closure);

	if ( script == pv->script && JS_PCToLineNumber(cx, script, pc) == pv->lineno )
		return JSTRAP_CONTINUE;

	if ( *pc == JSOP_DEFLOCALFUN )
		return JSTRAP_CONTINUE;

	return BreakHandler(cx, (JSObject*)closure, script, pc, FROM_STEP);
}


static JSTrapStatus StepOver(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	Private *pv = (Private*)JS_GetPrivate(cx, (JSObject*)closure);
	JSStackFrame *frame = JS_GetScriptedCaller(cx, NULL);

	if ( script == pv->script && JS_PCToLineNumber(cx, script, pc) == pv->lineno )
		return JSTRAP_CONTINUE;

	if ( frame != pv->frame && frame != pv->pframe )
		return JSTRAP_CONTINUE;

	if ( *pc == JSOP_DEFLOCALFUN ) // or JOF_DECLARING //	type = JOF_TYPE(cs->format);
		return JSTRAP_CONTINUE;

	return BreakHandler(cx, (JSObject*)closure, script, pc, FROM_STEP);
}


static JSTrapStatus StepOut(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	Private *pv = (Private*)JS_GetPrivate(cx, (JSObject*)closure);
	JSStackFrame *frame = JS_GetScriptedCaller(cx, NULL);

	if ( script == pv->script && JS_PCToLineNumber(cx, script, pc) == pv->lineno )
		return JSTRAP_CONTINUE;

	if ( frame != pv->pframe )
		return JSTRAP_CONTINUE;

	if ( *pc == JSOP_DEFLOCALFUN ) // or JOF_DECLARING //	type = JOF_TYPE(cs->format);
		return JSTRAP_CONTINUE;

	return BreakHandler(cx, (JSObject*)closure, script, pc, FROM_STEP);
}

static JSTrapStatus StepThrough(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	Private *pv = (Private*)JS_GetPrivate(cx, (JSObject*)closure);
	JSStackFrame *frame = JS_GetScriptedCaller(cx, NULL);

	if ( script == pv->script && JS_PCToLineNumber(cx, script, pc) <= pv->lineno )
		return JSTRAP_CONTINUE;

	if ( frame != pv->frame && frame != pv->pframe )
		return JSTRAP_CONTINUE;

	if ( *pc == JSOP_DEFLOCALFUN ) // or JOF_DECLARING //	type = JOF_TYPE(cs->format);
		return JSTRAP_CONTINUE;

	return BreakHandler(cx, (JSObject*)closure, script, pc, FROM_STEP);
}

static JSTrapStatus TrapHandler(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	return BreakHandler(cx, (JSObject*)closure, script, pc, FROM_BREAKPOINT);
}

JSBool DebugErrorHookHandler(JSContext *cx, const char *message, JSErrorReport *report, void *closure) {

	JSStackFrame *frame = JS_GetScriptedCaller(cx, NULL);
	if ( frame )
		BreakHandler(cx, (JSObject*)closure, JS_GetFrameScript(cx, frame), JS_GetFramePC(cx, frame), FROM_ERROR);
	return JS_TRUE;
}


static JSTrapStatus ThrowHookHandler(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure) {

	return BreakHandler(cx, (JSObject*)closure, script, pc, FROM_THROW);
}


static JSTrapStatus BreakHandler(JSContext *cx, JSObject *obj, JSScript *script, jsbytecode *pc, int breakOrigin) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);

	jsval fval;
	J_CHK( JS_GetProperty(cx, obj, "onBreak", &fval) );
	if ( !JsvalIsFunction(cx, fval) ) // nothing to do
		return JSTRAP_CONTINUE;

	JSRuntime *rt;
	rt = JS_GetRuntime(cx);
	JSStackFrame *frame;
	frame = JS_GetScriptedCaller(cx, NULL);
	int stackFrameIndex;
	stackFrameIndex = StackSize(frame)-1;
	int lineno;
	lineno = JS_PCToLineNumber(cx, script, pc);


	jsval argv[8];
	argv[0] = JSVAL_NULL;
	J_CHK( StringToJsval(cx, JS_GetScriptFilename(cx, script), &argv[1]) );
	argv[2] = INT_TO_JSVAL( lineno );
	argv[3] = OBJECT_TO_JSVAL( JS_GetFrameScopeChain(cx, frame) );
	argv[4] = INT_TO_JSVAL( breakOrigin );
	argv[5] = INT_TO_JSVAL( stackFrameIndex );

	JSBool hasException;
	hasException = JS_IsExceptionPending(cx);

	if ( hasException ) {

		argv[6] = JSVAL_TRUE;
		J_CHK( JS_GetPendingException(cx, &argv[7]) );
		JS_ClearPendingException(cx);
	} else {

		argv[6] = JSVAL_FALSE;
		argv[7] = JSVAL_VOID;
	}

	JSDebugHooks noHooks, *prevHooks;
	memset(&noHooks, 0, sizeof(JSDebugHooks));
	prevHooks = JS_SetContextDebugHooks(cx, &noHooks); // avoid reentrant case.

	JSTempValueRooter tvr;
	JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr);
	JSBool status;

	// (TBD) should I set the JSFRAME_DEBUGGER flag to the current frame ??? (see JS_EvaluateInStackFrame)
	status = JS_CallFunctionValue(cx, obj, fval, COUNTOF(argv)-1, argv+1, &argv[0]);
	int action;
	J_CHK( JsvalToInt(cx, argv[0], &action) );
	JS_POP_TEMP_ROOT(cx, &tvr);

	JS_SetContextDebugHooks(cx, prevHooks);

	if ( hasException ) // restore the exception
		JS_SetPendingException(cx, argv[7]); // (TBD) should return JSTRAP_ERROR ???
	J_CHK( status );

	// store the current state
	pv->stackFrameIndex = stackFrameIndex;
	pv->script = script;
	pv->lineno = lineno;
	pv->frame = frame;
	pv->pframe = frame->down ? JS_GetScriptedCaller(cx, frame->down) : NULL;

	switch (action) {

		case CONTINUE:
			JS_ClearInterrupt(rt, NULL, NULL);
			break;
		case STEP:
			JS_SetInterrupt(rt, Step, obj);
			break;
		case STEP_OVER:
			JS_SetInterrupt(rt, StepOver, obj);
			break;
		case STEP_THROUGH:
			JS_SetInterrupt(rt, StepThrough, obj);
			break;
		case STEP_OUT:
			JS_SetInterrupt(rt, StepOut, obj);
			break;
	}

	return JSTRAP_CONTINUE; // http://www.google.com/codesearch/p?hl=en#SPZdyP79RtQ/trunk/third_party/spidermonkey/js/src/jsinterp.c&q=JSTRAP_RETURN&l=742
bad:
	return JSTRAP_ERROR;
}



DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv ) {

		JSDebugHooks *hooks = JS_GetGlobalDebugHooks(JS_GetRuntime(cx));
		memset(hooks, 0, sizeof(JSDebugHooks));
		free(pv);
	}
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
	JS_SetDebuggerHandler(JS_GetRuntime(cx), DebuggerKeyword, obj);
	JS_SetDebugErrorHook( JS_GetRuntime(cx), DebugErrorHookHandler, obj);
//	JS_SetThrowHook( JS_GetRuntime(cx), ThrowHookHandler, obj);
	return JS_TRUE;
	JL_BAD;
}


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
 $INT | $BOOL $INAME( polarity, filename, lineno );
**/
DEFINE_FUNCTION_FAST( ToggleBreakpoint ) {

	J_S_ASSERT_ARG_MIN( 3 );

	bool polarity;
	J_CHK( JsvalToBool(cx, J_FARG(1), &polarity) );

	const char *filename;
	J_CHK( JsvalToString(cx, &J_FARG(2), &filename) );

	uintN lineno;
	J_CHK( JsvalToUInt(cx, J_FARG(3), &lineno) );

	Private *pv;
	pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
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
 $INT | $BOOL $INAME();
**/
DEFINE_FUNCTION( ClearBreakpoints ) {

	JS_ClearAllTraps(cx);
	return JS_TRUE;
}



/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME( frameLevel );
**/
DEFINE_FUNCTION( StackFrame ) {

	J_S_ASSERT_ARG_MIN( 1 );

	unsigned int frameIndex;
	J_CHK( JsvalToUInt(cx, J_ARG(1), &frameIndex) );

	JSStackFrame *frame;
	frame = JS_GetScriptedCaller(cx, NULL); // the current frame
	unsigned int currentFrameIndex;
	currentFrameIndex = StackSize(frame)-1;

	if ( frameIndex > currentFrameIndex ) {

		*J_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	// select the right frame
	while ( currentFrameIndex > frameIndex ) {

		frame = frame->down;
		currentFrameIndex--;
	}

	JSObject *stackItem;
	stackItem = JS_NewObject(cx, NULL, NULL, NULL);
	J_S_ASSERT_ALLOC( stackItem );
	*J_RVAL = OBJECT_TO_JSVAL( stackItem );

	JSScript *script;
	script = JS_GetFrameScript(cx, frame);
	jsbytecode *pc;
	pc = JS_GetFramePC(cx, frame);

	jsval tmp;
	J_CHK( StringToJsval(cx, JS_GetScriptFilename(cx, script), &tmp) );
	J_CHK( JS_DefineProperty(cx, stackItem, "filename", tmp, NULL, NULL, OBJ_PROP_FLAGS) );

	J_CHK( JS_DefineProperty(cx, stackItem, "lineno", INT_TO_JSVAL(JS_PCToLineNumber(cx, script, pc)), NULL, NULL, OBJ_PROP_FLAGS) );

	JSObject *callee = JS_GetFrameCalleeObject(cx, frame);
	J_CHK( JS_DefineProperty(cx, stackItem, "callee", callee ? OBJECT_TO_JSVAL(callee) : JSVAL_VOID, NULL, NULL, OBJ_PROP_FLAGS) );

	JSObject *scope;
	scope = JS_GetFrameScopeChain(cx, frame);
	J_CHK( JS_DefineProperty(cx, stackItem, "scope", OBJECT_TO_JSVAL(scope), NULL, NULL, OBJ_PROP_FLAGS) );

	J_CHK( JS_DefineProperty(cx, stackItem, "this", OBJECT_TO_JSVAL(JS_GetFrameThis(cx, frame)), NULL, NULL, OBJ_PROP_FLAGS) );
	J_CHK( JS_DefineProperty(cx, stackItem, "argc", INT_TO_JSVAL(frame->argc), NULL, NULL, OBJ_PROP_FLAGS) );
	J_CHK( JS_DefineProperty(cx, stackItem, "argv", frame->argv ? *frame->argv : JSVAL_VOID, NULL, NULL, OBJ_PROP_FLAGS) );
	J_CHK( JS_DefineProperty(cx, stackItem, "rval", JS_GetFrameReturnValue(cx, frame), NULL, NULL, OBJ_PROP_FLAGS) );

	J_CHK( JS_DefineProperty(cx, stackItem, "isNative", BOOLEAN_TO_JSVAL(JS_IsNativeFrame(cx, frame)), NULL, NULL, OBJ_PROP_FLAGS) );
	J_CHK( JS_DefineProperty(cx, stackItem, "isEval", frame->flags & JSFRAME_EVAL ? JSVAL_TRUE : JSVAL_FALSE, NULL, NULL, OBJ_PROP_FLAGS) );
	J_CHK( JS_DefineProperty(cx, stackItem, "isConstructing", frame->flags & JSFRAME_CONSTRUCTING ? JSVAL_TRUE : JSVAL_FALSE, NULL, NULL, OBJ_PROP_FLAGS) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
**/
DEFINE_PROPERTY( scriptList ) {

	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
	J_S_ASSERT_ALLOC(arr);
	*vp = OBJECT_TO_JSVAL(arr);

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
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

	JSStackFrame *frame;
	frame = JS_GetScriptedCaller(cx, NULL); // the current frame
	J_CHK( IntToJsval(cx, StackSize(frame), vp) );
	return JS_TRUE;
	JL_BAD;
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




CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision: 2290 $"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( GetActualLineno )
		FUNCTION_FAST( ToggleBreakpoint )
		FUNCTION( ClearBreakpoints )
		FUNCTION( StackFrame )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( scriptList )
		PROPERTY_READ( stackSize )


//		PROPERTY_READ( breakpointList )
//		PROPERTY_READ( pendingException )
	END_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER_SINGLE( CONTINUE )
		CONST_INTEGER_SINGLE( STEP )
		CONST_INTEGER_SINGLE( STEP_OVER )
		CONST_INTEGER_SINGLE( STEP_THROUGH )
		CONST_INTEGER_SINGLE( STEP_OUT )

		CONST_INTEGER_SINGLE( FROM_BREAKPOINT )
		CONST_INTEGER_SINGLE( FROM_STEP )
		CONST_INTEGER_SINGLE( FROM_THROW )
		CONST_INTEGER_SINGLE( FROM_ERROR )
		CONST_INTEGER_SINGLE( FROM_DEBUGGER )
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
