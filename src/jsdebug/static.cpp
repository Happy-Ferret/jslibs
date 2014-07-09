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
#include <jslibsModule.h>

#include <jsdbgapi.h>


#ifdef VALGRIND
#include "/usr/include/valgrind/valgrind.h"
#include "/usr/include/valgrind/memcheck.h"
#endif // VALGRIND



int _puts(JSContext *cx, const char *str) {

	jsval stdoutFunction;

	JL_CHK( JS_GetPropertyById(cx, JL_GetHostPrivate(cx)->hostObject, JLID(cx, stdout), &stdoutFunction) );

	if ( JL_ValueIsCallable(cx, stdoutFunction) ) {

		int len = (int)strlen(str);
		JSString *jsstr = JS_NewStringCopyN(cx, str, len);
		if ( jsstr == NULL )
			return EOF;
	//	jsstr = JS_ConcatStrings(cx, jsstr, JS_NewStringCopyZ(cx, "\n"));

		jsval tmp;
		bool status = JS_CallFunctionValue(cx, JL_GetGlobal(cx), stdoutFunction, 1, &tmp, &tmp);
		if ( status == true )
			return len;
	}

bad:
	return EOF;
}

/*
int _printf(JSContext *cx, const char * format, ...) {

  char buffer[65535];
  va_list args;
  va_start (args, format);
  int res = vsprintf(buffer, format, args);
//  perror (buffer);
  va_end (args);
  if ( res < 0 )
	  return -1;
  _puts(cx, buffer);
  return res;
}
*/


/**doc fileIndex:topmost **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$

Debug and introspection tools.$LF
$H guidelines
 Your program should run without using this module.
**/
BEGIN_STATIC

/**doc
=== Static functions ===
**/

/*
void
DumpScope(JSContext *cx, JSObject *obj)
{
    unsigned i;
    JSScope *scope;
    JSScopeProperty *sprop;
    jsval v;
    JSString *str;

    i = 0;
    scope = OBJ_SCOPE(obj);
    for (sprop = SCOPE_LAST_PROP(scope); sprop; sprop = sprop->parent) {
        if (SCOPE_HAD_MIDDLE_DELETE(scope) && !SCOPE_HAS_PROPERTY(scope, sprop))
            continue;
        _printf(cx, "%3u %p ", i, (void *)sprop);

        v = ID_TO_VALUE(sprop->id);
        if (JSID_IS_INT(sprop->id)) {
            _printf(cx, "[%ld]", (long)v.toInt32());
        } else {
            if (JSID_IS_ATOM(sprop->id)) {
                str = JSVAL_TO_STRING(v);
            } else {
                JS_ASSERT(JSID_IS_OBJECT(sprop->id));
                str = JS::ToString(cx, v);
                _puts(cx, "object ");
            }
            if (!str)
                _puts(cx, "<error>");
				else {
#ifndef DEBUG

//	JS_ReportError( cx, ("Available in DEBUG mode only." RT_CODE_LOCATION) );
	_puts(cx, "Available in DEBUG mode only.");

#else //DEBUG
					char buffer[65535];
					size_t count = js_PutEscapedStringImpl(buffer, sizeof(buffer), NULL, str, '"'); // js_FileEscapedString(fp, str, '"');
					buffer[count] = '\0';
					_puts(cx, buffer);
#endif //DEBUG

				}
        }
        _printf(cx, " slot %lu flags %x shortid %d\n",
                (unsigned long)sprop->slot, sprop->flags, sprop->shortid);
    }
}
*/




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
#ifdef DEBUG

/ **doc
$TOC_MEMBER $INAME
 $INAME( [ filename [, startThing [, thingToFind [, maxDepth [, thingToIgnore] ] ] ] ] )
 $H note
  This function in only available in DEBUG mode.
** /
DEFINE_FUNCTION( dumpHeap )
{

    JLData fileName;
    jsval v;
    void* startThing;
    uint32 startTraceKind;
    const char *badTraceArg;
    void *thingToFind;
    size_t maxDepth;
    void *thingToIgnore;
    FILE *dumpFile;
    bool ok;

    if (argc > 0) {
        v = JS_ARGV(cx, vp)[0];
        if ( !JSVAL_IS_NULL( v ) ) {
            JSString *str;

            str = JS::ToString(cx, v);
            if (!str)
                return false;
            JS_ARGV(cx, vp)[0] = STRING_TO_JSVAL(str);
				fileName = JLData(cx, str);
//            fileName = JL_GetStringBytesZ(cx, str);
//				if ( fileName == NULL )
//					fileName = "";
        }
    }

    startThing = NULL;
    startTraceKind = 0;
    if (argc > 1) {
        v = JS_ARGV(cx, vp)[1];
        if (JSVAL_IS_TRACEABLE(v)) {
            startThing = JSVAL_TO_TRACEABLE(v);
            startTraceKind = JSVAL_TRACE_KIND(v);
        } else if ( !JSVAL_IS_NULL( v ) ) {
            badTraceArg = "start";
            goto not_traceable_arg;
        }
    }

    thingToFind = NULL;
    if (argc > 2) {
        v = JS_ARGV(cx, vp)[2];
        if (JSVAL_IS_TRACEABLE(v)) {
            thingToFind = JSVAL_TO_TRACEABLE(v);
        } else if (v != JSVAL_NULL) {
            badTraceArg = "toFind";
            goto not_traceable_arg;
        }
    }

    maxDepth = (size_t)-1;
    if (argc > 3) {
        v = JS_ARGV(cx, vp)[3];
        if ( !JSVAL_IS_NULL( v ) ) {
            uint32 depth;

            if (!JS_ValueToECMAUint32(cx, v, &depth))
                return false;
            maxDepth = depth;
        }
    }

    thingToIgnore = NULL;
    if (argc > 4) {
        v = JS_ARGV(cx, vp)[4];
        if (JSVAL_IS_TRACEABLE(v)) {
            thingToIgnore = JSVAL_TO_TRACEABLE(v);
        } else if ( !JSVAL_IS_NULL( v ) ) {
            badTraceArg = "toIgnore";
            goto not_traceable_arg;
        }
    }

	 if (!fileName.IsSet()) {
        dumpFile = stdout;
    } else {
        dumpFile = fopen(fileName, "w");
        if (!dumpFile) {
            JS_ReportError(cx, "can't open %s: %s", fileName, strerror(errno));
            return false;
        }
    }

    ok = JS_DumpHeap(cx, dumpFile, startThing, startTraceKind, thingToFind,
                     maxDepth, thingToIgnore);
    if (dumpFile != stdout)
        fclose(dumpFile);
    return ok;

  not_traceable_arg:
    JS_ReportError(cx, "argument '%s' is not null or a heap-allocated thing",
                   badTraceArg);

	JL_RVAL.setUndefined();
	return false;
}

#else // DEBUG

DEFINE_FUNCTION( dumpHeap ) {

	JL_WARN( E_THISOPERATION, E_NOTSUPPORTED );
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

#endif // DEBUG

*/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



static bool hasGCTrace = false; // (TBD) fix static keyword issue
static JSGCCallback prevGCCallback = NULL; // (TBD) fix static keyword issue
static char GCTraceFileName[PATH_MAX]; // (TBD) fix static keyword issue

bool GCCallTrace(JSContext *cx, JSGCStatus status) {

//	const char *statusStr[4] = { "JSGC_BEGIN", "JSGC_END", "JSGC_MARK_END", "JSGC_FINALIZE_END" };
	if ( status == JSGC_END )
		return true;

	time_t t;
	struct tm *tim;
	t = time(NULL); // for milliseconds, cf. ftime() or clock()
	tim = localtime(&t);

	char timeTmp[256];
	strftime(timeTmp, COUNTOF(timeTmp), "%m/%d %H:%M:%S", tim);

	FILE *dumpFile;

	if ( GCTraceFileName[0] ) {

		dumpFile = fopen(GCTraceFileName, "a");
		if (!dumpFile) {
			JS_ReportError(cx, "can't open %s: %s", GCTraceFileName, strerror(errno));
			return false;
		}
	} else {
		dumpFile = stdout;
	}

	// (TBD) JM
//	if ( status == JSGC_BEGIN )
//		fprintf( dumpFile, "%s - gcByte:%lu gcMallocBytes:%lu ... ", timeTmp, (unsigned long)cx->runtime->gcBytes, (unsigned long)cx->runtime->gcMallocBytes );

	// (TBD) JM
//	if ( status == JSGC_END )
//		fprintf( dumpFile, "gcByte:%lu gcMallocBytes:%lu  \n", (unsigned long)cx->runtime->gcBytes, (unsigned long)cx->runtime->gcMallocBytes );

	if ( dumpFile != stdout )
		fclose(dumpFile);

	return true;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**doc
=== Static properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Number of times when GC was invoked.
**/
DEFINE_PROPERTY_GETTER( gcNumber ) {

	JL_IGNORE( id, obj );

	vp.setNumber(JS_GetGCParameter(JL_GetRuntime(cx), JSGC_NUMBER));
	return true;
}


/* *doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the amount of bytes mallocated by the JavaScript engine. It is incremented each time the JavaScript engine allocates memory.
**/
/*
DEFINE_PROPERTY( gcMallocBytes ) {

	JSRuntime *rt = JL_GetRuntime(cx);

	// (TBD) JM
	
	return JL_NewNumberValue(cx, JS_GetGCParameter(rt, JSGC_MAX_MALLOC_BYTES) - rt->gcMallocBytes, vp);
}
*/

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  It is the total amount of memory that the GC uses now and right after the last GC.
**/
DEFINE_PROPERTY_GETTER( gcBytes ) {

	JL_IGNORE( id, obj );

	return JL_NewNumberValue(cx, JS_GetGCParameter(JL_GetRuntime(cx), JSGC_BYTES), vp);
}

/*
makes the garbage collector extremely aggressive, which will
help you find any GC hazards much more quickly. This is the same as
-DWAY_TOO_MUCH_GC and friends, except that it can be manipulated at runtime
(gdb: set rt->gcZeal = 2).
...
in about:config javascript.options.gczeal = 2 (or 1, or 0, to disable).
*/
/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Enable GC zeal, a testing and debugging feature that helps find GC-related bugs in JSAPI applications.
 $H note
  This function in only available in DEBUG mode.
**/
DEFINE_PROPERTY_SETTER( gcZeal ) {

	JL_IGNORE( strict );

#ifdef JS_GC_ZEAL

	uint8_t zeal;
	JL_CHKM( jl::getValue(cx, *vp, &zeal), E_VALUE, E_INVALID );
	JS_SetGCZeal(cx, zeal, 1); // JS_DEFAULT_ZEAL_FREQ
	return jl::StoreProperty(cx, obj, id, vp, false); // make the value available for default getter

#else // JS_GC_ZEAL

	JL_WARN( E_THISOPERATION, E_NOTSUPPORTED );
	*vp = JSVAL_VOID;
	return true;

#endif // JS_GC_ZEAL

	JL_BAD;
}


// undocumented
DEFINE_FUNCTION( disableJIT ) {

	JL_IGNORE( argc );

	JS_SetOptions(cx, JS_GetOptions(cx) & ~(/*JSOPTION_JIT|*/JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE));

	JL_RVAL.setUndefined();
	return true;
}


// undocumented
DEFINE_FUNCTION( objectGCId ) {

	JL_ASSERT_ARGC(1);

	if ( JSVAL_IS_PRIMITIVE(JL_ARG(1)) ) {
		
		*JL_RVAL = JSVAL_ZERO;
	} else {

		jsid id;
		JL_CHK( JS_GetObjectId(cx, JSVAL_TO_OBJECT(JL_ARG(1)), &id) );
		JL_CHK( JL_NativeToJsval(cx, JSID_BITS(id), JL_RVAL) );
	}
	return true;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  TBD
**/
DEFINE_FUNCTION( getObjectPrivate ) {

		JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_OBJECT(1);

	if ( !(JL_GetClass(obj)->flags & JSCLASS_HAS_PRIVATE) ) {

		JL_RVAL.setUndefined();
		return true;
	}
	unsigned long n;
	n = (unsigned long)JL_GetPrivate(JSVAL_TO_OBJECT( JL_ARG( 1 ) ));
	JL_CHK( JL_NewNumberValue(cx, (double)n, JL_RVAL) );
	return true;
	JL_BAD;
}



/*
JSScript *
ValueToScript(JSContext *cx, jsval v)
{
    JS::RootedScript script;
    JSFunction *fun;

    if (!JSVAL_IS_PRIMITIVE(v) &&
        JL_GetClass(JSVAL_TO_OBJECT(v)) == &js_ScriptClass) {
        script = (JSScript *) JL_GetPrivate(JSVAL_TO_OBJECT(v));
    } else {
        fun = JS_ValueToFunction(cx, v);
        if (!fun)
            return NULL;
        script = FUN_SCRIPT(fun);
    }
    return script;
}

bool
GetTrapArgs(JSContext *cx, unsigned argc, jsval *argv, JSScript **scriptp,
            int32 *ip)
{
    jsval v;
    unsigned intarg;
    JS::RootedScript script;

    *scriptp = JS_GetScriptedCaller(cx, NULL)->script;
    *ip = 0;
    if (argc != 0) {
        v = argv[0];
        intarg = 0;
        if (!JSVAL_IS_PRIMITIVE(v) &&
            (JL_GetClass(JSVAL_TO_OBJECT(v)) == &js_FunctionClass ||
             JL_GetClass(JSVAL_TO_OBJECT(v)) == &js_ScriptClass)) {
            script = ValueToScript(cx, v);
            if (!script)
                return false;
            *scriptp = script;
            intarg++;
        }
        if (argc > intarg) {
            if (!JS::ToInt32(cx, argv[intarg], ip))
                return false;
        }
    }
    return true;
}

JSTrapStatus
TrapHandler(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval,
            void *closure)
{
    JSString *str;
    JSStackFrame *caller;

    str = (JSString *) closure;
    caller = JS_GetScriptedCaller(cx, NULL);
    if (!JS_EvaluateScript(cx, caller->scopeChain,
                           JL_GetStringBytes(str), JS_GetStringLength(str),
                           caller->script->filename, caller->script->lineno,
                           rval)) {
        return JSTRAP_ERROR;
    }
    if (!JSVAL_IS_VOID(*rval))
        return JSTRAP_RETURN;
    return JSTRAP_CONTINUE;
}

/ **doc
$TOC_MEMBER $INAME
 $VOID $INAME( ??? )
  TBD
** /
DEFINE_FUNCTION( trap )
{
    JSString *str;
    JS::RootedScript script;
    int32 i;

	 JL_ASSERT_ARGC_MIN( 1 );

    argc--;
    str = JS::ToString(cx, argv[argc]);
    if (!str)
        return false;
    argv[argc] = STRING_TO_JSVAL(str);
    if (!GetTrapArgs(cx, argc, argv, &script, &i))
        return false;
    return JS_SetTrap(cx, script, script->code + i, TrapHandler, str);
	JL_BAD;
}

/ **doc
$TOC_MEMBER $INAME
 $INAME( ??? )
  TBD
** /
DEFINE_FUNCTION( untrap )
{
    JS::RootedScript script;
    int32 i;

    if (!GetTrapArgs(cx, argc, argv, &script, &i))
        return false;
    JS_ClearTrap(cx, script, script->code + i, NULL, NULL);
    return true;
}

/ **doc
$TOC_MEMBER $INAME
 $INAME( ??? )
  TBD
** /
DEFINE_FUNCTION( lineToPC )
{
    JS::RootedScript script;
    int32 i;
    unsigned lineno;
    jsbytecode *pc;

	 JL_ASSERT_ARGC_MIN(1);

	 script = JS_GetScriptedCaller(cx, NULL)->script;
    if (!GetTrapArgs(cx, argc, argv, &script, &i))
        return false;
    lineno = (i == 0) ? script->lineno : (unsigned)i;
    pc = JS_LineNumberToPC(cx, script, lineno);
    if (!pc)
        return false;
    *rval = INT_TO_JSVAL(pc - script->code);
    return true;
	 JL_BAD;
}

/ **doc
$TOC_MEMBER $INAME
 $INAME( ??? )
  TBD
** /
DEFINE_FUNCTION( pCToLine )
{
    JS::RootedScript script;
    int32 i;
    unsigned lineno;

    if (!GetTrapArgs(cx, argc, argv, &script, &i))
        return false;
    lineno = JS_PCToLineNumber(cx, script, script->code + i);
    if (!lineno)
        return false;
    *rval = INT_TO_JSVAL(lineno);
    return true;
}
*/

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME $READONLY
  Is the list of all detected and active scripts.
**/
DEFINE_PROPERTY_GETTER( scriptFilenameList ) {

	JL_IGNORE( id, obj );

	JSObject *arr = JS_NewArrayObject(cx, 0);
	JL_CHK(arr);
	*vp = OBJECT_TO_JSVAL(arr);

	int index;
	index = 0;

	jl::Queue *scriptFileList;
	scriptFileList = &((ModulePrivate*)JL_GetModulePrivate(cx, _moduleId))->scriptFileList;

	for ( jl::QueueCell *it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

		jl::Queue *scriptList = (jl::Queue*)jl::QueueGetData(it);
		JSScript *s = (JSScript*)jl::QueueGetData(jl::QueueBegin(scriptList));

		jsval filename;
		JL_CHK( JL_NativeToJsval(cx, JS_GetScriptFilename(cx, s), &filename) );
		JL_CHK( JL_SetElement(cx, arr, index, &filename) );
		++index;
	}
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT | $UNDEF $INAME( filename, lineno )
 $INT | $UNDEF $INAME( function, relativeLineno )
  Transform a random line number into an actual line number or $UNDEFINED if the file number cannot be reached.
  $H example
  {{{
  1.  var i = 0;
  2.
  3.  i++;

  getActualLineno('test.js', 2); // returns: 3
  getActualLineno('nofile.js', 2); // returns: undefined
  }}}
**/
DEFINE_FUNCTION( getActualLineno ) {

	JL_ASSERT_ARGC_MIN( 2 );

	unsigned lineno;
	JL_CHK( jl::getValue(cx, JL_ARG(2), &lineno) );

	JS::RootedScript script;
	jsbytecode *pc;
	JL_CHK( GetScriptLocation(cx, &JL_ARG(1), lineno, &script, &pc) );
	if ( script == NULL ) {

		JL_RVAL.setUndefined();
		return true;
	}
	*JL_RVAL = INT_TO_JSVAL(JS_PCToLineNumber(cx, script, pc));
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME $READONLY
  Is the number of stack frames. 0 is the older stack frame index. The current stack frame index is (stackSize-1).
**/
DEFINE_PROPERTY_GETTER( stackSize ) {

	JL_IGNORE( id, obj );

	return JL_NativeToJsval(cx, JL_StackSize(cx, JL_CurrentStackFrame(cx)), vp);
}


/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME( frameLevel )
  Returns an object that describes the given stack frame index.
  0 is the older stack frame index. The current (last) stack frame index is (stackSize-1). See Locate() function for more details. $LF
  The object contains the following properties:
  * filename: filename of the script being executed.
  * lineno: line number of the script being executed.
  * callee: function being executed.
  * baseLineNumber: first line number of the function being executed.
  * lineExtent: number of lines of the function being executed.
  * scope: scope object.
  * this: this object.
  * argv: argument array of the function being executed.
  * rval: return value.
  * isNative: the frame is running native code.
  * isConstructing: frame is for a constructor invocation.
  * isEval: frame for eval.
  * isAssigning: a complex op is currently assigning to a property.
**/
DEFINE_FUNCTION( stackFrameInfo ) {

	jsval tmp;
	JL_ASSERT_ARGC_MIN( 1 );

	unsigned int frameIndex;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &frameIndex) );

	JSStackFrame *fp;
	fp = JL_StackFrameByIndex(cx, frameIndex);
	if ( fp == NULL ) {

		JL_RVAL.setUndefined();
		return true;
	}

	JSObject *frameInfo;
	frameInfo = JL_NewObj(cx);
	JL_CHK( frameInfo );
	*JL_RVAL = OBJECT_TO_JSVAL(frameInfo);

	JS::RootedScript script;
	script = JS_GetFrameScript(cx, fp);
	if ( script )
		JL_CHK( JL_NativeToJsval(cx, JS_GetScriptFilename(cx, script), &tmp) );
	else
		tmp = JSVAL_VOID;
	JL_CHK( JS_SetProperty(cx, frameInfo, "filename", &tmp) );

	jsbytecode *pc;
	pc = JS_GetFramePC(cx, fp);
	tmp = INT_TO_JSVAL(JS_PCToLineNumber(cx, script, pc));
	JL_CHK( JS_SetProperty(cx, frameInfo, "lineno", &tmp) );

	JSObject *callee;
	callee = JS_GetFrameFunctionObject(cx, fp);
	tmp = callee ? OBJECT_TO_JSVAL(callee) : JSVAL_VOID;
	JL_CHK( JS_SetProperty(cx, frameInfo, "callee", &tmp) );

	tmp = script ? INT_TO_JSVAL( JS_GetScriptBaseLineNumber(cx, script) ) : JSVAL_VOID;
	JL_CHK( JS_SetProperty(cx, frameInfo, "baseLineNumber", &tmp) );

	tmp = script ? INT_TO_JSVAL( JS_GetScriptLineExtent(cx, script) ) : JSVAL_VOID;
	JL_CHK( JS_SetProperty(cx, frameInfo, "lineExtent", &tmp) );

	tmp = OBJECT_TO_JSVAL(JS_GetFrameScopeChain(cx, fp)); // Assertion failure: FUN_INTERPRETED(this). (TBD) check if fp->fun is not a native function ?
	JL_CHK( JS_SetProperty(cx, frameInfo, "scope", &tmp) );

//	JL_CHK( JS_DefineProperty(cx, frameInfo, "variables", fp->varobj ? OBJECT_TO_JSVAL(fp->varobj) : JSVAL_VOID, NULL, NULL, JSPROP_ENUMERATE) );

	JL_CHK( JS_GetFrameThis(cx, fp, &tmp) );
	JL_CHK( JS_SetProperty(cx, frameInfo, "this", &tmp) );

/*
	if ( js::Valueify(fp)->hasArgs() ) {

		JSObject *arguments;
		arguments = JS_NewArrayObject(cx, js::Valueify(fp)->numFormalArgs(), js::Jsvalify(js::Valueify(fp)->formalArgs()));
		JL_CHK( arguments );
		tmp = OBJECT_TO_JSVAL(arguments);
	} else {

		tmp = JSVAL_VOID;
	}
	JL_CHK( JS_SetProperty(cx, frameInfo, "argv", &tmp) );
*/

	tmp = JS_GetFrameReturnValue(cx, fp);
	JL_CHK( JS_SetProperty(cx, frameInfo, "rval", &tmp) );

	// JS_IsNativeFrame
	//tmp = fp->isFunctionFrame() ? JSVAL_FALSE : JSVAL_TRUE; // (TBD) check if isFunctionFrame() <=> !isNative

//	tmp = BOOLEAN_TO_JSVAL( !js::Valueify(fp)->isFunctionFrame() ); // (TBD) check if isFunctionFrame() <=> !isNative
	tmp = BOOLEAN_TO_JSVAL( !JS_IsScriptFrame(cx, fp) );
	JL_CHK( JS_SetProperty(cx, frameInfo, "isNative", &tmp) );

	//tmp = BOOLEAN_TO_JSVAL(js::Valueify(fp)->isConstructing());
	tmp = BOOLEAN_TO_JSVAL(JS_IsConstructorFrame(cx, fp));
	JL_CHK( JS_SetProperty(cx, frameInfo, "isConstructing", &tmp) );

	//tmp = BOOLEAN_TO_JSVAL(js::Valueify(fp)->isEvalFrame());
	//JL_CHK( JS_SetProperty(cx, frameInfo, "isEval", &tmp) );

// not available any more. see. https://bugzilla.mozilla.org/show_bug.cgi?id=458421
//	tmp = BOOLEAN_TO_JSVAL(js::Valueify(fp)->isAssigning());
//	JL_CHK( JS_SetProperty(cx, frameInfo, "isAssigning", &tmp) );

//	JL_CHK( JS_DefineProperty(cx, frameInfo, "opnd", fp->regs->sp[-1], NULL, NULL, JSPROP_ENUMERATE) );
//	char * s = JL_GetStringBytes(JS::ToString(cx, fp->regs->sp[-1]));

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME( code, frameLevel )
 Evaluates code in the given stack frame.
 0 is the older stack frame index. The current (last) stack frame index is (stackSize-1). See Locate() function for more details.
**/
DEFINE_FUNCTION( evalInStackFrame ) {

	JL_ASSERT_ARGC_MIN( 2 );
	JL_ASSERT_ARG_IS_STRING(1);

	unsigned int frameIndex;
	JL_CHK( jl::getValue(cx, JL_ARG(2), &frameIndex) );

	JSStackFrame *fp;
	fp = JL_StackFrameByIndex(cx, frameIndex);

	if ( fp == NULL ) {

		JL_RVAL.setUndefined();
		return true;
	}

	JS::RootedScript script;
	script = JS_GetFrameScript(cx, fp);

	jsbytecode *pc;
	pc = JS_GetFramePC(cx, fp);

	JSString *jsstr;
	jsstr = JL_ARG(1).toString();

	size_t strlen;
	const jschar *str;
	str = JS_GetStringCharsAndLength(cx, jsstr, &strlen);

	JL_CHK( JS_EvaluateUCInStackFrame(cx, fp, str, (unsigned)strlen, JS_GetScriptFilename(cx, script), JS_PCToLineNumber(cx, script, pc), JL_RVAL) );

	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME( [frameIndex] );
  Returns the current script name and line number.
  0 is the older (the first) stack frame index. The current stack frame index is Locate() or Locate(stackSize-1).
  Negative numbers are interpreted as starting from the current stack frame.$LF
  The function returns $UNDEFINED if the given stack frame is not defined.
  $H note
   `Locate(-1) == Locate(stackSize-2)`
  $H example
  (file debug.js)
  {{{
   1 loadModule('jsstd');
   2 loadModule('jsdebug');
   3
   4 function test2() {
   5
   6 print( stackSize, ' - ', locate(stackSize-1), '\n' ); // prints: 3 - debug.js,6
   7 print( locate(-1), ' - ', locate(-2), ' - ', locate(-3), '\n' ); // prints: debug.js,13 - debug.js,16 - undefined
   8 print( locate(0), ' - ', locate(1), ' - ', locate(2), '\n' ); // prints: debug.js,16 - debug.js,13 - debug.js,8
   9 }
  10
  11 function test() {
  12
  13  test2();
  14 }
  15
  16 test();
  }}}
**/
DEFINE_FUNCTION( locate ) {

	jsval tmp;
	JSStackFrame *fp;
	if ( JL_ARG_ISDEF(1) ) {

		int frame;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &frame) );
		fp = JL_StackFrameByIndex(cx, frame);
	} else {

		fp = JL_CurrentStackFrame(cx);
	}

	if ( fp == NULL ) {

		JL_RVAL.setUndefined();
		return true;
	}

	JS::RootedScript script;
	script = JS_GetFrameScript(cx, fp); // because we are in a fast native function, this frame is ok.
	unsigned lineno;
	lineno = JS_PCToLineNumber(cx, script, JS_GetFramePC(cx, fp));

	JSObject *arrObj;
	arrObj = JS_NewArrayObject(cx, 2);
	*JL_RVAL = OBJECT_TO_JSVAL(arrObj);

	if ( script ) {

		const char *filename;
		filename = JS_GetScriptFilename(cx, script);
		JL_CHK( JL_NativeToJsval(cx, filename, &tmp) );
	} else { // native frame ?

		tmp = JSVAL_VOID;
	}
	JL_CHK( JL_SetElement(cx, arrObj, 0, &tmp) );

	tmp = INT_TO_JSVAL(lineno);
	JL_CHK( JL_SetElement(cx, arrObj, 1, &tmp) );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( value )
  Try to find the definition location of the given value.
**/
DEFINE_FUNCTION( definitionLocation ) {

	jsval values[2];
	JL_ASSERT_ARGC_MIN( 1 );

	JS::RootedScript script;
	script = NULL;

	if ( JL_ValueIsCallable(cx, JL_ARG(1)) ) {

		JSFunction *fun;
		fun = JS_ValueToFunction(cx, JL_ARG(1));
		if ( JS_GetFunctionScript(cx, fun) )
			script = JS_GetFunctionScript(cx, fun);
		goto next;
	}

	if ( !JSVAL_IS_PRIMITIVE( JL_ARG(1) ) ) {

		JSObject* obj;
		obj = JL_GetConstructor(cx, JSVAL_TO_OBJECT( JL_ARG(1) ));
		JSFunction *fun;
		fun = JS_ValueToFunction(cx, OBJECT_TO_JSVAL( obj ) );
		if ( fun ) {

			script = JS_GetFunctionScript(cx, fun);
			if ( script )
				goto next;
		}
	}
/*
	if ( !JSVAL_IS_PRIMITIVE(JL_ARG(1)) && JL_IsScript(cx, JSVAL_TO_OBJECT(JL_ARG(1))) ) {

		JSObject* obj;
		obj = JSVAL_TO_OBJECT(JL_ARG(1));
		script = (JSScript*)JL_GetPrivate(obj);
	}
*/
next:
	if ( !script ) {

		JL_RVAL.setUndefined();
		return true;
	}

	JL_CHK( JL_NativeToJsval(cx, JS_GetScriptFilename(cx, script), &values[0]) );
	JL_CHK( JL_NativeToJsval(cx, JS_GetScriptBaseLineNumber(cx, script), &values[1] ) );
	*JL_RVAL = OBJECT_TO_JSVAL( JS_NewArrayObject(cx, COUNTOF(values), values) );

	return true;
	JL_BAD;
}


/*
S_ASSERT(JSTRY_CATCH == 0);
S_ASSERT(JSTRY_FINALLY == 1);
S_ASSERT(JSTRY_ITER == 2);

static const char* const TryNoteNames[] = { "catch", "finally", "iter" };

bool
TryNotes(JSContext *cx, JSScript *script, FILE *gOutFile)
{
    JSTryNote *tn, *tnlimit;

    if (script->trynotesOffset == 0)
        return true;

    tn = script->trynotes()->vector;
    tnlimit = tn + script->trynotes()->length;
    fprintf(gOutFile, "\nException table:\n"
            "kind      stack    start      end\n");
    do {
        JS_ASSERT(tn->kind < JS_ARRAY_LENGTH(TryNoteNames));
        fprintf(gOutFile, " %-7s %6u %8u %8u\n",
                TryNoteNames[tn->kind], tn->stackDepth,
                tn->start, tn->start + tn->length);
    } while (++tn != tnlimit);
    return true;
}
*/

/* *doc
$TOC_MEMBER $INAME
 $TYPE Script $INAME( filename, lineno )
**/
/*
DEFINE_FUNCTION( scriptByLocation ) {

	JL_ASSERT_ARGC(2);

	const char *filename;
	unsigned int lineno;

	JL_CHK( jl::getValue(cx, &JL_ARG(1), &filename) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &lineno) );
	JSScript *script = ScriptByLocation(cx, scriptFileList, filename, lineno);
	JL_CHK( script );
	JSObject *scrobj = JS_GetScriptObject(script);
//	if ( scrobj == NULL )
//		scrobj = JS_NewScriptObject(cx, script); // Doc: https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_NewScriptObject

	if ( scrobj == NULL ) {

		JL_RVAL.setUndefined();
		return true;
	}

	*JL_RVAL = OBJECT_TO_JSVAL(scrobj);
	return true;
	JL_BAD;
}
*/

/*

/ **doc
$TOC_MEMBER $INAME
 $STRING $INAME( filename, lineno )
  Returns the assembly code of the given block location.
 $H beware
  This function is only available in DEBUG mode.
** /
DEFINE_FUNCTION( disassembleScript ) {

#ifdef DEBUG

	jl::Queue *scriptFileList = NULL;

	JLData filename;
	unsigned int lineno;

	JL_ASSERT_ARGC(2);

	JL_CHK( jl::getValue(cx, JL_ARG(1), &filename) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &lineno) );

	scriptFileList = &((ModulePrivate*)JL_GetModulePrivate(cx, _moduleId))->scriptFileList;

	JS::RootedScript script;
	script = ScriptByLocation(cx, scriptFileList, filename, lineno);
	JL_CHK( script );

	int length;

    void *mark = JS_ARENA_MARK(&cx->tempPool);
	js::Sprinter sprinter;
    INIT_SPRINTER(cx, &sprinter, &cx->tempPool, 0);

	jsbytecode *pc, *end;
	unsigned len;
	pc = script->main;
	end = script->code + script->length;
	while (pc < end) {

		len = js_Disassemble1(cx, script, pc, pc - script->code, true, &sprinter);
		if (!len)
			return false;
		pc += len;
	}

    JSString *str = JS_NewStringCopyZ(cx, sprinter.base);
    JS_ARENA_RELEASE(&cx->tempPool, mark);
    if (!str)
        return false;
    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(str));
	return true;

#else // DEBUG

	JL_WARN( E_THISOPERATION, E_NOTSUPPORTED );
	JL_RVAL.setUndefined();
	return true;

#endif // DEBUG

	JL_BAD;
}
*/

/*
/ **doc
$TOC_MEMBER $INAME
 $STRING $INAME( filename, lineno )
  Throw if the calling function failed to JIT
** /
DEFINE_FUNCTION( assertJit ) {

#ifdef JS_METHODJIT
	if (JS_GetOptions(cx) & JSOPTION_METHODJIT) {

		if ( !cx->fp()->script()->getJIT(cx->fp()->isConstructing()) ) {

			JL_ERR( E_STR("JIT"), E_DISABLED );
		}
	}
#endif

	JS_SET_RVAL(cx, vp, JSVAL_VOID);
	return true;
	JL_BAD;
}
*/



DEFINE_FUNCTION( debugOutput ) {

	JL_ASSERT_ARGC(1);

#if defined(_MSC_VER) && defined(DEBUG)
	{
	JLData str;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &str) );
	OutputDebugString(str); // (TBD) not thread-safe, use a critical section
	}
	*JL_RVAL = JSVAL_TRUE;
	return true;
#else
	JL_WARN( E_THISOPERATION, E_NOTSUPPORTED );
#endif

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


#ifdef VALGRIND

// http://valgrind.org/docs/manual/mc-manual.html#mc-manual.clientreqs

// undocumented
DEFINE_FUNCTION( createLeak ) {

	malloc(1234);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


// undocumented
DEFINE_FUNCTION( VALGRIND_COUNT_ERRORS ) {

	// Returns the number of errors found so far by Valgrind. Can be useful in test harness code when combined with the --log-fd=-1 option; this runs Valgrind silently,
	// but the client program can detect when errors occur. Only useful for tools that report errors, e.g. it's useful for Memcheck,
	// but for Cachegrind it will always return zero because Cachegrind doesn't report errors.
	*JL_RVAL = INT_TO_JSVAL( VALGRIND_COUNT_ERRORS );
	return true;
	JL_BAD;
}


// undocumented
DEFINE_FUNCTION( VALGRIND_DO_LEAK_CHECK ) {

	// does a full memory leak check (like --leak-check=full) right now.
	// This is useful for incrementally checking for leaks between arbitrary places in the program's execution. It has no return value.
	VALGRIND_DO_LEAK_CHECK;
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

// undocumented
DEFINE_FUNCTION( VALGRIND_DO_QUICK_LEAK_CHECK ) {

	// like VALGRIND_DO_LEAK_CHECK, except it produces only a leak summary (like --leak-check=summary). It has no return value.
	VALGRIND_DO_QUICK_LEAK_CHECK;
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

// undocumented
DEFINE_FUNCTION( VALGRIND_COUNT_LEAKS ) {

	int leaked, dubious, reachable, suppressed;

	// fills in the four arguments with the number of bytes of memory found by the previous leak check to be leaked (i.e. the sum of direct leaks and indirect leaks),
	// dubious, reachable and suppressed. This is useful in test harness code, after calling VALGRIND_DO_LEAK_CHECK or VALGRIND_DO_QUICK_LEAK_CHECK.
	VALGRIND_COUNT_LEAKS(leaked, dubious, reachable, suppressed);

	JSObject *arrayObj = JS_NewArrayObject(cx, 4, NULL);
	*JL_RVAL = OBJECT_TO_JSVAL(arrayObj);
	jsval tmp;
	tmp = INT_TO_JSVAL(leaked);
	JL_SetElement(cx, arrayObj, 0, &tmp);
	tmp = INT_TO_JSVAL(dubious);
	JL_SetElement(cx, arrayObj, 1, &tmp);
	tmp = INT_TO_JSVAL(reachable);
	JL_SetElement(cx, arrayObj, 2, &tmp);
	tmp = INT_TO_JSVAL(suppressed);
	JL_SetElement(cx, arrayObj, 3, &tmp);
	return true;
	JL_BAD;
}

#endif // VALGRIND


DEFINE_FUNCTION( debugBreak ) {

	JL_Break();
	JL_RVAL.setUndefined();
	return true;
}


DEFINE_FUNCTION( crashGuard ) {

		JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_CALLABLE(1);

#if defined(WIN)
	EXCEPTION_POINTERS * eps = 0;
	__try {

		JS_CallFunctionValue(cx, JL_OBJ, JL_ARG(1), 0, NULL, JL_RVAL);
		*JL_RVAL = JSVAL_TRUE;

//	} __except (eps = GetExceptionInformation(), ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)) {
	} __except (eps = GetExceptionInformation(), EXCEPTION_EXECUTE_HANDLER) {
		
		*JL_RVAL = JSVAL_FALSE;
	}
#elif defined UNIX

	JS_CallFunctionValue(cx, JL_OBJ, JL_ARG(1), 0, NULL, JL_RVAL);
	*JL_RVAL = JSVAL_TRUE;

#else

	#error NOT IMPLEMENTED YET	// (TBD)

#endif

	return true;
	JL_BAD;
}


DEFINE_FUNCTION( setPerfTestMode ) {

	JL_RVAL.setUndefined();

#if defined(WIN32)

	HANDLE currentProcess = ::GetCurrentProcess();
	HANDLE currentThread = ::GetCurrentThread();
	DWORD_PTR processAffinityMask, systemAffinityMask;
	// detect thread availability
	JL_CHK( ::GetProcessAffinityMask(currentProcess, &processAffinityMask, &systemAffinityMask) );
	// select a thread
	JL_CHK( ::SetThreadAffinityMask(currentThread, jl::LeastSignificantBit(processAffinityMask)) );
	//   do not set time critical because this will set time critical for this thread only !
	JL_CHK( ::SetPriorityClass(currentProcess, REALTIME_PRIORITY_CLASS) );
	JL_CHK( ::SetProcessPriorityBoost(currentProcess, TRUE) ); // disable dynamic boosting

#endif // WIN32

	return true;
	JL_BAD;
}


#ifdef DEBUG
DEFINE_FUNCTION( testDebug ) {

/*
	// see https://bugzilla.mozilla.org/show_bug.cgi?id=488924
	JSObject *o = JS_NewArrayObject(cx, 0, NULL);
	JSScopeProperty *jssp;
	jssp = NULL;
	JS_PropertyIterator(o, &jssp);
*/
//	if ( JL_IsRValOptional(cx, _TestDebug) )
//		printf("OPTIONAL\n");

	jsid id;
	JSObject *obj = JL_NewObj(cx);
	JS_ValueToId(cx, OBJECT_TO_JSVAL(obj), &id);
	bool isobjid = JSID_IS_OBJECT(id);


	JL_RVAL.setUndefined();
	return true;
}

DEFINE_FUNCTION( test2Debug ) {

	JL_RVAL.setUndefined();
	return true;

//	//	jsval arg = JSVAL_ONE;
//	return JS_CallFunctionValue(cx, JL_OBJ, JL_ARG(1), 1, &arg, JL_RVAL );
}

#endif // DEBUG



CONFIGURE_STATIC

	REVISION(jl::SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( getObjectPrivate )
//		FUNCTION( dumpStats )

//		FUNCTION( scriptByLocation )
//		FUNCTION( disassembleScript )

//		FUNCTION( assertJit )

//		FUNCTION( trap )
//		FUNCTION( untrap )
//		FUNCTION( lineToPC )
//		FUNCTION( pCToLine )

		FUNCTION( getActualLineno )
		FUNCTION( locate )
		FUNCTION( definitionLocation )
		FUNCTION( stackFrameInfo )
		FUNCTION( evalInStackFrame )
		FUNCTION_ARGC( debugOutput, 1 )
		FUNCTION( disableJIT )
		FUNCTION( objectGCId )
	#ifdef VALGRIND
		FUNCTION( createLeak )
		FUNCTION( VALGRIND_COUNT_ERRORS )
		FUNCTION( VALGRIND_DO_QUICK_LEAK_CHECK )
		FUNCTION( VALGRIND_DO_LEAK_CHECK )
		FUNCTION( VALGRIND_COUNT_LEAKS )
	#endif // VALGRIND

//		FUNCTION( dumpHeap )
		FUNCTION( debugBreak )
		FUNCTION( crashGuard )
		FUNCTION( setPerfTestMode )

	// for internal tests
	#ifdef DEBUG
		FUNCTION( testDebug )
		FUNCTION( test2Debug )
	#endif // DEBUG
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( scriptFilenameList )
		PROPERTY_GETTER( stackSize )
		PROPERTY_SETTER( gcZeal )

		PROPERTY_GETTER( gcNumber )
//		PROPERTY_GETTER( gcMallocBytes )
		PROPERTY_GETTER( gcBytes )

	END_STATIC_PROPERTY_SPEC

END_STATIC
