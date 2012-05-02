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

#ifdef XP_WIN
	#pragma comment(lib,"Psapi.lib") // need for currentMemoryUsage()
	#include <Psapi.h>
	#pragma comment(lib,"pdh.lib") // need for performance counters usage
	#include <pdh.h>
#endif // XP_WIN


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
		JSBool status = JS_CallFunctionValue(cx, JL_GetGlobal(cx), stdoutFunction, 1, &tmp, &tmp);
		if ( status == JS_TRUE )
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
            _printf(cx, "[%ld]", (long)JSVAL_TO_INT(v));
        } else {
            if (JSID_IS_ATOM(sprop->id)) {
                str = JSVAL_TO_STRING(v);
            } else {
                JS_ASSERT(JSID_IS_OBJECT(sprop->id));
                str = js_ValueToString(cx, v);
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
    JSBool ok;

    if (argc > 0) {
        v = JS_ARGV(cx, vp)[0];
        if ( !JSVAL_IS_NULL( v ) ) {
            JSString *str;

            str = JS_ValueToString(cx, v);
            if (!str)
                return JS_FALSE;
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
                return JS_FALSE;
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
            return JS_FALSE;
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

	*JL_RVAL = JSVAL_VOID;
	return JS_FALSE;
}

#else // DEBUG

DEFINE_FUNCTION( dumpHeap ) {

	JL_WARN( E_THISOPERATION, E_NOTSUPPORTED );
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

#endif // DEBUG

*/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



static bool hasGCTrace = false; // (TBD) fix static keyword issue
static JSGCCallback prevGCCallback = NULL; // (TBD) fix static keyword issue
static char GCTraceFileName[PATH_MAX]; // (TBD) fix static keyword issue

JSBool GCCallTrace(JSContext *cx, JSGCStatus status) {

//	const char *statusStr[4] = { "JSGC_BEGIN", "JSGC_END", "JSGC_MARK_END", "JSGC_FINALIZE_END" };
	if ( status == JSGC_END )
		return JS_TRUE;

	time_t t;
	struct tm *tim;
	t = time(NULL); // for milliseconds, cf. ftime() or clock()
	tim = localtime(&t);

	char timeTmp[256];
	strftime( timeTmp, sizeof(timeTmp), "%m/%d %H:%M:%S", tim);

	FILE *dumpFile;

	if ( GCTraceFileName[0] ) {

		dumpFile = fopen(GCTraceFileName, "a");
		if (!dumpFile) {
			JS_ReportError(cx, "can't open %s: %s", GCTraceFileName, strerror(errno));
			return JS_FALSE;
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

	return JS_TRUE;
}


#ifdef XP_UNIX
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

struct LinuxProcInfo {
	int pid; // %d
	char comm[400]; // %s
	char state; // %c
	int ppid; // %d
	int pgrp; // %d
	int session; // %d
	int tty; // %d
	int tpgid; // %d
	unsigned int flags; // %u
	unsigned int minflt; // %u
	unsigned int cminflt; // %u
	unsigned int majflt; // %u
	unsigned int cmajflt; // %u
	int utime; // %d
	int stime; // %d
	int cutime; // %d
	int cstime; // %d
	int counter; // %d
	int priority; // %d
	unsigned int timeout; // %u
	unsigned int itrealvalue; // %u
	int starttime; // %d
	unsigned int vsize; // %u
	unsigned int rss; // %u
	unsigned int rlim; // %u
	unsigned int startcode; // %u
	unsigned int endcode; // %u
	unsigned int startstack; // %u
	unsigned int kstkesp; // %u
	unsigned int kstkeip; // %u
	int signal; // %d
	int blocked; // %d
	int sigignore; // %d
	int sigcatch; // %d
	unsigned int wchan; // %u
};

bool GetProcInfo( pid_t pid, LinuxProcInfo *pinfo ) {

	char path[128];
	char data[512];
	snprintf(path, sizeof(path), "/proc/%d/stat", pid);
	int fd = open(path, O_RDONLY);
	//lseek(fd,0,SEEK_SET);
	int rd = read(fd, data, sizeof(data));
	close(fd);
	data[rd] = '\0';

	sscanf(data, "%d %s %c %d %d %d %d %d %u %u %u %u %u %d %d %d %d %d %d %u %u %d %u %u %u %u %u %u %u %u %d %d %d %d %u",
		&pinfo->pid, // %d
		pinfo->comm, // %s
		&pinfo->state, // %c
		&pinfo->ppid, // %d
		&pinfo->pgrp, // %d
		&pinfo->session, // %d
		&pinfo->tty, // %d
		&pinfo->tpgid, // %d
		&pinfo->flags, // %u
		&pinfo->minflt, // %u
		&pinfo->cminflt, // %u
		&pinfo->majflt, // %u
		&pinfo->cmajflt, // %u
		&pinfo->utime, // %d
		&pinfo->stime, // %d
		&pinfo->cutime, // %d
		&pinfo->cstime, // %d
		&pinfo->counter, // %d
		&pinfo->priority, // %d
		&pinfo->timeout, // %u
		&pinfo->itrealvalue, // %u
		&pinfo->starttime, // %d
		&pinfo->vsize, // %u
		&pinfo->rss, // %u
		&pinfo->rlim, // %u
		&pinfo->startcode, // %u
		&pinfo->endcode, // %u
		&pinfo->startstack, // %u
		&pinfo->kstkesp, // %u
		&pinfo->kstkeip, // %u
		&pinfo->signal, // %d
		&pinfo->blocked, // %d
		&pinfo->sigignore, // %d
		&pinfo->sigcatch, // %d
		&pinfo->wchan // %u
	);
	return true;
}

#endif // XP_UNIX



/**doc
$TOC_MEMBER $INAME
 $INAME
  TBD
**/
DEFINE_PROPERTY_GETTER( currentMemoryUsage ) {

	JL_IGNORE( id, obj );

	uint32_t bytes;

#if defined(XP_WIN)
	// SIZE_T is compatible with uint32
	HANDLE hProcess = GetCurrentProcess(); // doc: (HANDLE)-1, that is interpreted as the current process handle
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo( hProcess, (PPROCESS_MEMORY_COUNTERS)&pmc, sizeof(pmc) ); // MEM_PRIVATE
//	bytes = pmc.PrivateUsage; // doc: The current amount of memory that cannot be shared with other processes, in bytes. Private bytes include memory that is committed and marked MEM_PRIVATE, data that is not mapped, and executable pages that have been written to.
	bytes = pmc.WorkingSetSize; // same value as "windows task manager" "mem usage"
#elif defined(XP_UNIX)
	LinuxProcInfo pinfo;
	GetProcInfo(getpid(), &pinfo);
	bytes = pinfo.vsize;
#else

	JL_WARN( E_API, E_NOTIMPLEMENTED );

#endif

	JL_CHK( JL_NewNumberValue(cx, bytes, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME
  TBD
**/
DEFINE_PROPERTY_GETTER( peakMemoryUsage ) {

	JL_IGNORE( id, obj );

#if defined(XP_WIN)

	uint32_t bytes;
/*
	DWORD  dwMin, dwMax;
	HANDLE hProcess;
	hProcess = GetCurrentProcess();
	if (!GetProcessWorkingSetSize(hProcess, &dwMin, &dwMax))
		JL_REPORT_ERROR("GetProcessWorkingSetSize failed (%d)\n", GetLastError());
//	printf("Minimum working set: %lu Kbytes\n", dwMin/1024);
//	printf("Maximum working set: %lu Kbytes\n", dwMax/1024);
	bytes = dwMax;
	//cf. GetProcessWorkingSetSizeEx
*/
	HANDLE hProcess = GetCurrentProcess(); // doc: (HANDLE)-1, that is interpreted as the current process handle
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo( hProcess, (PPROCESS_MEMORY_COUNTERS)&pmc, sizeof(pmc) ); // MEM_PRIVATE
	bytes = pmc.PeakWorkingSetSize; // same value as "windows task manager" "peak mem usage"
	JL_CHK( JL_NewNumberValue(cx, bytes, vp) );
	return JS_TRUE;
#else

	JL_WARN( E_API, E_NOTIMPLEMENTED );

#endif

	*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME
  TBD
**/
DEFINE_PROPERTY_GETTER( privateMemoryUsage ) {

	JL_IGNORE( id, obj );

#if defined(XP_WIN)

	// see also. http://www.codeproject.com/KB/cpp/XPWSPrivate.aspx (Calculate Memory (Working Set - Private) Programmatically in Windows XP/2000)

	// SIZE_T is compatible with uint32
	HANDLE hProcess = GetCurrentProcess(); // doc: (HANDLE)-1, that is interpreted as the current process handle
	PROCESS_MEMORY_COUNTERS_EX pmc;
	pmc.cb = sizeof(pmc);
	BOOL status = GetProcessMemoryInfo( hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc) ); // MEM_PRIVATE
	if ( !status )
		return JL_ThrowOSError(cx);

	//	pmc.PrivateUsage:
	// doc: The current amount of memory that cannot be shared with other processes, in bytes. Private bytes include memory that is committed and marked MEM_PRIVATE,
	//      data that is not mapped, and executable pages that have been written to.
	
	// pmc.WorkingSetSize:
	//   same value as "windows task manager" "mem usage"

	return JL_NewNumberValue(cx, pmc.PrivateUsage, vp);
#else

	JL_WARN( E_API, E_NOTIMPLEMENTED );

	*vp = JSVAL_VOID;
	return JS_TRUE;

#endif
	JL_BAD;
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

	return JL_NewNumberValue(cx, JS_GetGCParameter(JL_GetRuntime(cx), JSGC_NUMBER), vp);
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
	JL_CHKM( JL_JsvalToNative(cx, *vp, &zeal), E_VALUE, E_INVALID );
	JS_SetGCZeal(cx, zeal, 1, JS_FALSE); // JS_DEFAULT_ZEAL_FREQ
	return jl::StoreProperty(cx, obj, id, vp, false); // make the value available for default getter

#else // JS_GC_ZEAL

	JL_WARN( E_THISOPERATION, E_NOTSUPPORTED );
	*vp = JSVAL_VOID;
	return JS_TRUE;

#endif // JS_GC_ZEAL

	JL_BAD;
}


// undocumented
DEFINE_FUNCTION( disableJIT ) {

	JL_IGNORE( argc );

	JS_SetOptions(cx, JS_GetOptions(cx) & ~(/*JSOPTION_JIT|*/JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE));

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
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
	return JS_TRUE;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  TBD
**/
DEFINE_FUNCTION( getObjectPrivate ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_OBJECT(1);

	if ( !(JL_GetClass(obj)->flags & JSCLASS_HAS_PRIVATE) ) {

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}
	unsigned long n;
	n = (unsigned long)JL_GetPrivate(JSVAL_TO_OBJECT( JL_ARG( 1 ) ));
	JL_CHK( JL_NewNumberValue(cx, (double)n, JL_RVAL) );
	return JS_TRUE;
	JL_BAD;
}



/*
JSScript *
ValueToScript(JSContext *cx, jsval v)
{
    JSScript *script;
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

JSBool
GetTrapArgs(JSContext *cx, unsigned argc, jsval *argv, JSScript **scriptp,
            int32 *ip)
{
    jsval v;
    unsigned intarg;
    JSScript *script;

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
                return JS_FALSE;
            *scriptp = script;
            intarg++;
        }
        if (argc > intarg) {
            if (!JS_ValueToInt32(cx, argv[intarg], ip))
                return JS_FALSE;
        }
    }
    return JS_TRUE;
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
    JSScript *script;
    int32 i;

	 JL_ASSERT_ARGC_MIN( 1 );

    argc--;
    str = JS_ValueToString(cx, argv[argc]);
    if (!str)
        return JS_FALSE;
    argv[argc] = STRING_TO_JSVAL(str);
    if (!GetTrapArgs(cx, argc, argv, &script, &i))
        return JS_FALSE;
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
    JSScript *script;
    int32 i;

    if (!GetTrapArgs(cx, argc, argv, &script, &i))
        return JS_FALSE;
    JS_ClearTrap(cx, script, script->code + i, NULL, NULL);
    return JS_TRUE;
}

/ **doc
$TOC_MEMBER $INAME
 $INAME( ??? )
  TBD
** /
DEFINE_FUNCTION( lineToPC )
{
    JSScript *script;
    int32 i;
    unsigned lineno;
    jsbytecode *pc;

	 JL_ASSERT_ARGC_MIN(1);

	 script = JS_GetScriptedCaller(cx, NULL)->script;
    if (!GetTrapArgs(cx, argc, argv, &script, &i))
        return JS_FALSE;
    lineno = (i == 0) ? script->lineno : (unsigned)i;
    pc = JS_LineNumberToPC(cx, script, lineno);
    if (!pc)
        return JS_FALSE;
    *rval = INT_TO_JSVAL(pc - script->code);
    return JS_TRUE;
	 JL_BAD;
}

/ **doc
$TOC_MEMBER $INAME
 $INAME( ??? )
  TBD
** /
DEFINE_FUNCTION( pCToLine )
{
    JSScript *script;
    int32 i;
    unsigned lineno;

    if (!GetTrapArgs(cx, argc, argv, &script, &i))
        return JS_FALSE;
    lineno = JS_PCToLineNumber(cx, script, script->code + i);
    if (!lineno)
        return JS_FALSE;
    *rval = INT_TO_JSVAL(lineno);
    return JS_TRUE;
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

	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
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
	return JS_TRUE;
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
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &lineno) );

	JSScript *script;
	jsbytecode *pc;
	JL_CHK( GetScriptLocation(cx, &JL_ARG(1), lineno, &script, &pc) );
	if ( script == NULL ) {

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}
	*JL_RVAL = INT_TO_JSVAL(JS_PCToLineNumber(cx, script, pc));
	return JS_TRUE;
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
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &frameIndex) );

	JSStackFrame *fp;
	fp = JL_StackFrameByIndex(cx, frameIndex);
	if ( fp == NULL ) {

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	JSObject *frameInfo;
	frameInfo = JL_NewObj(cx);
	JL_CHK( frameInfo );
	*JL_RVAL = OBJECT_TO_JSVAL(frameInfo);

	JSScript *script;
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
//	char * s = JL_GetStringBytes(JS_ValueToString(cx, fp->regs->sp[-1]));

	return JS_TRUE;
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
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &frameIndex) );

	JSStackFrame *fp;
	fp = JL_StackFrameByIndex(cx, frameIndex);

	if ( fp == NULL ) {

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	JSScript *script;
	script = JS_GetFrameScript(cx, fp);

	jsbytecode *pc;
	pc = JS_GetFramePC(cx, fp);

	JSString *jsstr;
	jsstr = JSVAL_TO_STRING( JL_ARG(1) );

	size_t strlen;
	const jschar *str;
	str = JS_GetStringCharsAndLength(cx, jsstr, &strlen);

	JL_CHK( JS_EvaluateUCInStackFrame(cx, fp, str, (unsigned)strlen, JS_GetScriptFilename(cx, script), JS_PCToLineNumber(cx, script, pc), JL_RVAL) );

	return JS_TRUE;
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
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &frame) );
		fp = JL_StackFrameByIndex(cx, frame);
	} else {

		fp = JL_CurrentStackFrame(cx);
	}

	if ( fp == NULL ) {

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	JSScript *script;
	script = JS_GetFrameScript(cx, fp); // because we are in a fast native function, this frame is ok.
	unsigned lineno;
	lineno = JS_PCToLineNumber(cx, script, JS_GetFramePC(cx, fp));

	JSObject *arrObj;
	arrObj = JS_NewArrayObject(cx, 2, NULL);
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

	return JS_TRUE;
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

	JSScript *script;
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

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	JL_CHK( JL_NativeToJsval(cx, JS_GetScriptFilename(cx, script), &values[0]) );
	JL_CHK( JL_NativeToJsval(cx, JS_GetScriptBaseLineNumber(cx, script), &values[1] ) );
	*JL_RVAL = OBJECT_TO_JSVAL( JS_NewArrayObject(cx, COUNTOF(values), values) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( object [, followPrototypeChain = false ] )
  Returns an array of properties name.
**/
DEFINE_FUNCTION( propertiesList ) {

	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_OBJECT(1);

	JSObject *srcObj;
	srcObj = JSVAL_TO_OBJECT( JL_ARG(1) );

	if ( !JS_IsNative(srcObj) ) { // (TBD) remove this workaround to bz#522101 / bz#488924

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	bool followPrototypeChain;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &followPrototypeChain) );
	else
		followPrototypeChain = false;

	JSObject *arrayObject;
	arrayObject = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK( arrayObject );
	*JL_RVAL = OBJECT_TO_JSVAL( arrayObject );

	int index;
	index = 0;
	
	JSScopeProperty *jssp;

	while ( srcObj ) {

		jssp = NULL;
		//about bz#522101 / bz#488924
		//<jorendorff>	I was going to say, something like if (OBJ_IS_DENSE_ARRAY(cx, obj)) { if (!js_MakeArraySlow(cx, obj)) return NULL; }

		// see Bug 688571 - JS_PropertyIterator is broken
		while ( JS_PropertyIterator(srcObj, &jssp) ) {

			//jsid id = ((js::Shape*)jssp)->propid;
			JSPropertyDesc pd;
			JL_CHK( JS_GetPropertyDesc(cx, srcObj, jssp, &pd) );
			//JL_CHK( JS_IdToValue(cx, pd.id, &tmp) );
			JL_CHK( JL_SetElement(cx, arrayObject, index, &pd.id) );
			index++;
		}

		if ( !followPrototypeChain )
			break;

		srcObj = JL_GetPrototype(cx, srcObj);
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( object [, followPrototypeChain = false ] )
  Returns an array of properties information.
**/
DEFINE_FUNCTION( propertiesInfo ) {

	jsval tmp;
	JSPropertyDesc desc;
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_OBJECT(1);

	JSObject *srcObj;
	srcObj = JSVAL_TO_OBJECT( JL_ARG(1) );

	if ( !JS_IsNative(srcObj) ) { // (TBD) remove this workaround to bz#522101 / bz#488924

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	bool followPrototypeChain;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &followPrototypeChain) );
	else
		followPrototypeChain = false;

//	JSObject *infoObject;
//	infoObject = JL_NewProtolessObj(cx);
//	*JL_RVAL = OBJECT_TO_JSVAL( infoObject );

	JSObject *arrayObject;
	arrayObject = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK( arrayObject );
	*JL_RVAL = OBJECT_TO_JSVAL( arrayObject );

	int index;
	index = 0;
	int prototypeLevel;
	prototypeLevel = 0;

	JSScopeProperty *jssp;

	while ( srcObj ) {

		jssp = NULL;
		//about bz#522101 / bz#488924
		//<jorendorff>	I was going to say, something like if (OBJ_IS_DENSE_ARRAY(cx, obj)) { if (!js_MakeArraySlow(cx, obj)) return NULL; }

		// see Bug 688571 - JS_PropertyIterator is broken
		while ( JS_PropertyIterator(srcObj, &jssp) ) {

			JL_CHK( JS_GetPropertyDesc(cx, srcObj, jssp, &desc) );

			JSObject *descObj = JL_NewObj(cx);
			tmp = OBJECT_TO_JSVAL(descObj);
			JL_CHK( JL_SetElement(cx, arrayObject, index, &tmp) );

//			JL_CHK( JS_IdToValue(cx, jssp->id, &tmp) );
			JL_CHK( JS_SetProperty(cx, descObj, "name", &desc.id) );

			tmp = desc.value;
			if ( desc.flags & JSPD_EXCEPTION ) // doc. exception occurred fetching the property, value is exception.
				JL_CHK( JS_SetProperty(cx, descObj, "exception", &tmp) );
			else
				JL_CHK( JS_SetProperty(cx, descObj, "value", &tmp) );

//			tmp = BOOLEAN_TO_JSVAL( ((js::Shape*)jssp)->hasGetterValue() );
//			JL_CHK( JS_SetProperty(cx, descObj, "getter", &tmp) );

//			tmp = BOOLEAN_TO_JSVAL( ((js::Shape*)jssp)->hasSetterValue() );
//			JL_CHK( JS_SetProperty(cx, descObj, "setter", &tmp) );

			tmp = BOOLEAN_TO_JSVAL( desc.flags & JSPD_VARIABLE ); // doc. local variable in function
			JL_CHK( JS_SetProperty(cx, descObj, "variable", &tmp) );

			tmp = BOOLEAN_TO_JSVAL( desc.flags & JSPD_ARGUMENT ); // doc. argument to function
			JL_CHK( JS_SetProperty(cx, descObj, "argument", &tmp) );

			tmp = BOOLEAN_TO_JSVAL( desc.flags & JSPD_ENUMERATE ); // visible to for/in loop
			JL_CHK( JS_SetProperty(cx, descObj, "enumerate", &tmp) );
//			JL_CHK( SetPropertyBool(cx, descObj, "enumerate", desc.flags & JSPD_ENUMERATE) );

			tmp = BOOLEAN_TO_JSVAL( desc.flags & JSPD_READONLY );
			JL_CHK( JS_SetProperty(cx, descObj, "readonly", &tmp) );

			tmp = BOOLEAN_TO_JSVAL( desc.flags & JSPD_PERMANENT );
			JL_CHK( JS_SetProperty(cx, descObj, "permanent", &tmp) );

//			tmp = jssp->setter() != NULL || jssp->getter() != NULL ? JSVAL_TRUE : JSVAL_FALSE;
//			tmp = BOOLEAN_TO_JSVAL(  ((js::Shape*)jssp)->isNative() );
//			JL_CHK( JS_SetProperty(cx, descObj, "native", &tmp) );

			tmp = INT_TO_JSVAL(prototypeLevel);
			JL_CHK( JS_SetProperty(cx, descObj, "prototypeLevel", &tmp) );

			tmp = OBJECT_TO_JSVAL(srcObj);
			JL_CHK( JS_SetProperty(cx, descObj, "object", &tmp) );

			index++;
		}

		if ( !followPrototypeChain )
			break;

		srcObj = JL_GetPrototype(cx, srcObj);
		prototypeLevel++;
	}

	return JS_TRUE;
	JL_BAD;
}


/*
S_ASSERT(JSTRY_CATCH == 0);
S_ASSERT(JSTRY_FINALLY == 1);
S_ASSERT(JSTRY_ITER == 2);

static const char* const TryNoteNames[] = { "catch", "finally", "iter" };

JSBool
TryNotes(JSContext *cx, JSScript *script, FILE *gOutFile)
{
    JSTryNote *tn, *tnlimit;

    if (script->trynotesOffset == 0)
        return JS_TRUE;

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
    return JS_TRUE;
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

	JL_CHK( JL_JsvalToNative(cx, &JL_ARG(1), &filename) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &lineno) );
	JSScript *script = ScriptByLocation(cx, scriptFileList, filename, lineno);
	JL_CHK( script );
	JSObject *scrobj = JS_GetScriptObject(script);
//	if ( scrobj == NULL )
//		scrobj = JS_NewScriptObject(cx, script); // Doc: https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_NewScriptObject

	if ( scrobj == NULL ) {

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	*JL_RVAL = OBJECT_TO_JSVAL(scrobj);
	return JS_TRUE;
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

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &filename) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &lineno) );

	scriptFileList = &((ModulePrivate*)JL_GetModulePrivate(cx, _moduleId))->scriptFileList;

	JSScript *script;
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

		len = js_Disassemble1(cx, script, pc, pc - script->code, JS_TRUE, &sprinter);
		if (!len)
			return JS_FALSE;
		pc += len;
	}

    JSString *str = JS_NewStringCopyZ(cx, sprinter.base);
    JS_ARENA_RELEASE(&cx->tempPool, mark);
    if (!str)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(str));
	return JS_TRUE;

#else // DEBUG

	JL_WARN( E_THISOPERATION, E_NOTSUPPORTED );
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;

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
	return JS_TRUE;
	JL_BAD;
}
*/


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the amount of CPU time (milliseconds) that the process has executed.
**/
DEFINE_PROPERTY_GETTER( processTime ) {

	JL_IGNORE( id, obj );

#if defined(XP_WIN)

	__int64 creationTime, exitTime, kernelTime, userTime;
	BOOL status = GetThreadTimes(GetCurrentThread(), (FILETIME *)&creationTime, (FILETIME *)&exitTime, (FILETIME *)&kernelTime, (FILETIME *)&userTime);
//	BOOL status = GetProcessTimes(GetCurrentProcess(), (FILETIME *)&creationTime, (FILETIME *)&exitTime, (FILETIME *)&kernelTime, (FILETIME *)&userTime);
	if ( !status )
		return JL_ThrowOSError(cx);
	return JL_NativeToJsval(cx, (kernelTime + userTime) / (double)10000 , vp);

#else

	JL_WARN( E_API, E_NOTIMPLEMENTED );

#endif

	*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the current CPU usage in percent.
**/
DEFINE_PROPERTY_GETTER( cpuLoad ) {

	JL_IGNORE( id, obj );

#if defined(XP_WIN)

  static PDH_STATUS status;
  static PDH_FMT_COUNTERVALUE value;
  static HQUERY query;
  static HCOUNTER counter;
  static DWORD ret;
  static bool runonce = true;

//  char errorMessage[1024];

	if ( runonce ) {

		status = PdhOpenQuery(NULL, 0, &query);
		if ( status != ERROR_SUCCESS )
			return JL_ThrowOSError(cx);
			PdhAddCounter(query, TEXT("\\Processor(_Total)\\% Processor Time"), 0, &counter); // A total of ALL CPU's in the system
		PdhCollectQueryData(query); // No error checking here
		runonce = false;
	}

	status = PdhCollectQueryData(query);
	if ( status != ERROR_SUCCESS )
		return JL_ThrowOSError(cx);

	status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, &ret, &value);
	if ( status != ERROR_SUCCESS )
		return JL_ThrowOSError(cx);

	return JL_NativeToJsval(cx, value.doubleValue, vp);

#else

	JL_WARN( E_API, E_NOTIMPLEMENTED );

#endif

	*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( debugOutput ) {

	JL_ASSERT_ARGC(1);

#if defined(_MSC_VER) && defined(DEBUG)
	{
	JLData str;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	OutputDebugString(str); // (TBD) not thread-safe, use a critical section
	}
	*JL_RVAL = JSVAL_TRUE;
	return JS_TRUE;
#else
	JL_WARN( E_THISOPERATION, E_NOTSUPPORTED );
#endif

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


#ifdef VALGRIND

// http://valgrind.org/docs/manual/mc-manual.html#mc-manual.clientreqs

// undocumented
DEFINE_FUNCTION( createLeak ) {

	malloc(1234);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


// undocumented
DEFINE_FUNCTION( VALGRIND_COUNT_ERRORS ) {

	// Returns the number of errors found so far by Valgrind. Can be useful in test harness code when combined with the --log-fd=-1 option; this runs Valgrind silently,
	// but the client program can detect when errors occur. Only useful for tools that report errors, e.g. it's useful for Memcheck,
	// but for Cachegrind it will always return zero because Cachegrind doesn't report errors.
	*JL_RVAL = INT_TO_JSVAL( VALGRIND_COUNT_ERRORS );
	return JS_TRUE;
	JL_BAD;
}


// undocumented
DEFINE_FUNCTION( VALGRIND_DO_LEAK_CHECK ) {

	// does a full memory leak check (like --leak-check=full) right now.
	// This is useful for incrementally checking for leaks between arbitrary places in the program's execution. It has no return value.
	VALGRIND_DO_LEAK_CHECK;
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

// undocumented
DEFINE_FUNCTION( VALGRIND_DO_QUICK_LEAK_CHECK ) {

	// like VALGRIND_DO_LEAK_CHECK, except it produces only a leak summary (like --leak-check=summary). It has no return value.
	VALGRIND_DO_QUICK_LEAK_CHECK;
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
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
	return JS_TRUE;
	JL_BAD;
}

#endif // VALGRIND


DEFINE_FUNCTION( debugBreak ) {

	JL_Break();
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION( crashGuard ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_CALLABLE(1);

#if defined XP_WIN
	EXCEPTION_POINTERS * eps = 0;
	__try {

		JS_CallFunctionValue(cx, JL_OBJ, JL_ARG(1), 0, NULL, JL_RVAL);
		*JL_RVAL = JSVAL_TRUE;

//	} __except (eps = GetExceptionInformation(), ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)) {
	} __except (eps = GetExceptionInformation(), EXCEPTION_EXECUTE_HANDLER) {
		
		*JL_RVAL = JSVAL_FALSE;
	}
#elif defined XP_UNIX

	JS_CallFunctionValue(cx, JL_OBJ, JL_ARG(1), 0, NULL, JL_RVAL);
	*JL_RVAL = JSVAL_TRUE;

#else

	#error NOT IMPLEMENTED YET	// (TBD)

#endif

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( setPerfTestMode ) {

	*JL_RVAL = JSVAL_VOID;

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

	return JS_TRUE;
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
	JSBool isobjid = JSID_IS_OBJECT(id);


	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
}

DEFINE_FUNCTION( test2Debug ) {

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;

//	JL_DEFINE_FUNCTION_OBJ;
//	jsval arg = JSVAL_ONE;
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
		FUNCTION_ARGC( propertiesList, 1 )
		FUNCTION_ARGC( propertiesInfo, 1 )
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
		PROPERTY_GETTER( currentMemoryUsage )
		PROPERTY_GETTER( peakMemoryUsage )
		PROPERTY_GETTER( privateMemoryUsage )
		PROPERTY_GETTER( processTime )
		PROPERTY_GETTER( cpuLoad )
	END_STATIC_PROPERTY_SPEC

END_STATIC
