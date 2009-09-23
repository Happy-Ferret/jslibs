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

#ifdef XP_WIN
	#pragma comment(lib,"Psapi.lib") // need for currentMemoryUsage()
	#include <Psapi.h>
	#pragma comment(lib,"pdh.lib") // need for performance counters usage
	#include <pdh.h>
#endif // XP_WIN

#include <errno.h>
#include <time.h>
#include <string.h>


#ifdef XP_WIN
void fpipe( FILE **read, FILE **write ) {

	HANDLE readPipe, writePipe;
	CreatePipe(&readPipe, &writePipe, NULL, 0);

	// doc: The underlying handle is also closed by a call to _close,
	//      so it is not necessary to call the Win32 function CloseHandle on the original handle. 
	int readfd = _open_osfhandle((intptr_t)readPipe, _O_RDONLY);
	int writefd = _open_osfhandle((intptr_t)writePipe, 32768);
	*read = fdopen(readfd, "r");
	*write = fdopen(writefd, "w");
}
#endif // XP_WIN


extern jl::Queue *scriptFileList;
JSBool GetScriptLocation( JSContext *cx, jsval *val, uintN lineno, JSScript **script, jsbytecode **pc ); // defined in main.cpp
JSScript *ScriptByLocation(JSContext *cx, jl::Queue *scriptFileList, const char *filename, unsigned int lineno);


int _puts(JSContext *cx, const char *str) {

	jsval stdoutFunction;
	if ( GetConfigurationValue(cx, NAME_CONFIGURATION_STDOUT, &stdoutFunction) && JsvalIsFunction(cx, stdoutFunction) ) {

		size_t len = strlen(str);
		JSString *jsstr = JS_NewStringCopyN(cx, str, len);
		if ( jsstr == NULL )
			return EOF;
	//	jsstr = JS_ConcatStrings(cx, jsstr, JS_NewStringCopyZ(cx, "\n"));
		JSTempValueRooter tvr;
		JS_PUSH_SINGLE_TEMP_ROOT(cx, STRING_TO_JSVAL(jsstr), &tvr); // protects jsstr against GC
		JSBool status = JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), stdoutFunction, 1, &tvr.u.value, &tvr.u.value);
		JS_POP_TEMP_ROOT(cx, &tvr);
		if ( status == JS_TRUE )
			return len;
	}
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
**/
BEGIN_STATIC

/**doc
=== Static functions ===
**/

/*
static void
DumpScope(JSContext *cx, JSObject *obj)
{
    uintN i;
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


#ifdef DEBUG

/**doc
$TOC_MEMBER $INAME
 $INAME( [ filename [, startThing [, thingToFind [, maxDepth [, thingToIgnore] ] ] ] ] )
 $H note
  This function in only available in DEBUG mode.
**/
DEFINE_FUNCTION_FAST( DumpHeap )
{

    char *fileName;
    jsval v;
    void* startThing;
    uint32 startTraceKind;
    const char *badTraceArg;
    void *thingToFind;
    size_t maxDepth;
    void *thingToIgnore;
    FILE *dumpFile;
    JSBool ok;

    fileName = NULL;
    if (argc > 0) {
        v = JS_ARGV(cx, vp)[0];
        if ( !JSVAL_IS_NULL( v ) ) {
            JSString *str;

            str = JS_ValueToString(cx, v);
            if (!str)
                return JS_FALSE;
            JS_ARGV(cx, vp)[0] = STRING_TO_JSVAL(str);
            fileName = JS_GetStringBytes(str);
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

    if (!fileName) {
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
	 return JS_FALSE;
}
#endif // DEBUG




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



static bool hasGCTrace = false; // (TBD) fix static keyword issue
static JSGCCallback prevGCCallback = NULL; // (TBD) fix static keyword issue
static char GCTraceFileName[PATH_MAX]; // (TBD) fix static keyword issue

JSBool GCCallTrace(JSContext *cx, JSGCStatus status) {

	const char *statusStr[4] = { "JSGC_BEGIN", "JSGC_END", "JSGC_MARK_END", "JSGC_FINALIZE_END" };
	if ( status == JSGC_MARK_END || status == JSGC_FINALIZE_END )
		return JS_TRUE;

	time_t t;
	struct tm *tim;
	t = time(NULL); // for milliseconds, cf. ftime() or clock()
	tim = localtime(&t);

	char timeTmp[256];
	strftime( timeTmp, sizeof(timeTmp), "%m/%d %H:%M:%S", tim);

	FILE *dumpFile;

	if ( GCTraceFileName && GCTraceFileName[0] ) {

		dumpFile = fopen(GCTraceFileName, "a");
		if (!dumpFile) {
			JS_ReportError(cx, "can't open %s: %s", GCTraceFileName, strerror(errno));
			return JS_FALSE;
		}
	} else {
		dumpFile = stdout;
	}

	if ( status == JSGC_BEGIN )
		fprintf( dumpFile, "%s - gcByte:%u gcMallocBytes:%u ... ", timeTmp, cx->runtime->gcBytes, cx->runtime->gcMallocBytes );

	if ( status == JSGC_END )
		fprintf( dumpFile, "gcByte:%u gcMallocBytes:%u  \n", cx->runtime->gcBytes, cx->runtime->gcMallocBytes );

	if ( dumpFile != stdout )
		fclose(dumpFile);

	return JS_TRUE;
}

/**doc
$TOC_MEMBER $INAME
 $INAME( [ filename ] )
  TBD
**/
DEFINE_FUNCTION_FAST( TraceGC )
{

	if ( argc > 0 ) { // start GC dump

		jsval *argv = JS_ARGV(cx, vp);
		JSObject *obj = JS_THIS_OBJECT(cx, vp);

		char *fileName = NULL;

		if ( !JSVAL_IS_NULL( argv[0] ) ) {

			JSString *str;
			str = JS_ValueToString(cx, argv[0]);
			if (!str)
				 return JS_FALSE;
			argv[0] = STRING_TO_JSVAL(str);
			fileName = JS_GetStringBytes(str);
		}

		if (!fileName)
			GCTraceFileName[0] = '\0';
		else
			strcpy( GCTraceFileName, fileName );

		if (!hasGCTrace) {

			prevGCCallback = JS_SetGCCallback(cx, GCCallTrace);  // JS_SetGCCallbackRT(rt, GCCallTrace); ???
			hasGCTrace = true;
		}

	} else { // stop GC dump

		if ( hasGCTrace ) {

			JS_SetGCCallback(cx, prevGCCallback); // (TBD) check this !
			hasGCTrace = false;
		}
	}

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
DEFINE_PROPERTY( currentMemoryUsage ) {

	uint32 bytes;

#ifdef XP_WIN
	// SIZE_T is compatible with uint32
	HANDLE hProcess = GetCurrentProcess(); // doc: (HANDLE)-1, that is interpreted as the current process handle
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo( hProcess, (PPROCESS_MEMORY_COUNTERS)&pmc, sizeof(pmc) ); // MEM_PRIVATE
//	bytes = pmc.PrivateUsage; // doc: The current amount of memory that cannot be shared with other processes, in bytes. Private bytes include memory that is committed and marked MEM_PRIVATE, data that is not mapped, and executable pages that have been written to.
	bytes = pmc.WorkingSetSize; // same value as "windows task manager" "mem usage"
#else
	LinuxProcInfo pinfo;
	GetProcInfo(getpid(), &pinfo);
	bytes = pinfo.vsize;
#endif // XP_WIN

	JL_CHK( JS_NewNumberValue(cx, bytes, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME
  TBD
**/
DEFINE_PROPERTY( peakMemoryUsage ) {

	uint32 bytes;

#ifdef XP_WIN

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
#else

	JL_REPORT_WARNING("peakMemoryUsage() is not implemented yet for this platform.");
	*vp = JSVAL_VOID;
	return JS_TRUE;

#endif // XP_WIN

	JL_CHK( JS_NewNumberValue(cx, bytes, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME
  TBD
**/
DEFINE_PROPERTY( privateMemoryUsage ) {

	uint32 bytes;

#ifdef XP_WIN
	// SIZE_T is compatible with uint32
	HANDLE hProcess = GetCurrentProcess(); // doc: (HANDLE)-1, that is interpreted as the current process handle
	PROCESS_MEMORY_COUNTERS_EX pmc;
	BOOL status = GetProcessMemoryInfo( hProcess, (PPROCESS_MEMORY_COUNTERS)&pmc, sizeof(pmc) ); // MEM_PRIVATE
	JL_CHKM( status, "GetProcessMemoryInfo error." );
// doc. If the function fails, the return value is zero. To get extended error information, call GetLastError.

//	bytes = pmc.PrivateUsage; // doc: The current amount of memory that cannot be shared with other processes, in bytes. Private bytes include memory that is committed and marked MEM_PRIVATE, data that is not mapped, and executable pages that have been written to.
	bytes = pmc.WorkingSetSize; // same value as "windows task manager" "mem usage"
#else

	JL_REPORT_WARNING("privateMemoryUsage() is not implemented yet for this platform.");
	*vp = JSVAL_VOID;
	return JS_TRUE;

#endif // XP_WIN

	JL_CHK( JS_NewNumberValue(cx, bytes, vp) );
	return JS_TRUE;
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
DEFINE_PROPERTY( gcNumber ) {

	uint32 bytes = JS_GetGCParameter(JS_GetRuntime(cx), JSGC_NUMBER);
	return JS_NewNumberValue(cx, bytes, vp);
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the amount of bytes mallocated by the JavaScript engine. It is incremented each time the JavaScript engine allocates memory.
**/
DEFINE_PROPERTY( gcMallocBytes ) {

	uint32 bytes = JS_GetRuntime(cx)->gcMallocBytes;
	return JS_NewNumberValue(cx, bytes, vp);
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  It is the total amount of memory that the GC uses now and right after the last GC.
**/
DEFINE_PROPERTY( gcBytes ) {

	uint32 bytes = JS_GetGCParameter(JS_GetRuntime(cx), JSGC_BYTES);
	return JS_NewNumberValue(cx, bytes, vp);
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
  Level of garbage collection: 0 for normal, 1 for very frequent GC, 2 for extremely frequent GC.
 $H note
  This function in only available in DEBUG mode.
**/
#ifdef JS_GC_ZEAL
DEFINE_PROPERTY( gcZeal ) {

	int zeal;
	JL_CHKM( JsvalToInt(cx, *vp, &zeal), "Invalid value." );
	JS_SetGCZeal(cx, zeal);
	return JS_TRUE;
	JL_BAD;
}
#endif // JS_GC_ZEAL



/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  TBD
**/
DEFINE_FUNCTION_FAST( DisableJIT ) {

	JS_SetOptions(cx, JS_GetOptions(cx) & ~JSOPTION_JIT);
	return JS_TRUE;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  TBD
**/
DEFINE_FUNCTION( GetObjectPrivate ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	JL_S_ASSERT_OBJECT( JL_ARG( 1 ) );

	if ( !(JL_GetClass(obj)->flags & JSCLASS_HAS_PRIVATE) ) {
		
		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}
	unsigned long n;
	n = (unsigned long)JL_GetPrivate(cx, JSVAL_TO_OBJECT( JL_ARG( 1 ) ));
	JL_CHK( JS_NewNumberValue(cx, (double)n, JL_RVAL) );
	return JS_TRUE;
	JL_BAD;
}



/*
static JSScript *
ValueToScript(JSContext *cx, jsval v)
{
    JSScript *script;
    JSFunction *fun;

    if (!JSVAL_IS_PRIMITIVE(v) &&
        JL_GetClass(JSVAL_TO_OBJECT(v)) == &js_ScriptClass) {
        script = (JSScript *) JL_GetPrivate(cx, JSVAL_TO_OBJECT(v));
    } else {
        fun = JS_ValueToFunction(cx, v);
        if (!fun)
            return NULL;
        script = FUN_SCRIPT(fun);
    }
    return script;
}

static JSBool
GetTrapArgs(JSContext *cx, uintN argc, jsval *argv, JSScript **scriptp,
            int32 *ip)
{
    jsval v;
    uintN intarg;
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

static JSTrapStatus
TrapHandler(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval,
            void *closure)
{
    JSString *str;
    JSStackFrame *caller;

    str = (JSString *) closure;
    caller = JS_GetScriptedCaller(cx, NULL);
    if (!JS_EvaluateScript(cx, caller->scopeChain,
                           JS_GetStringBytes(str), JS_GetStringLength(str),
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
DEFINE_FUNCTION( Trap )
{
    JSString *str;
    JSScript *script;
    int32 i;

	 JL_S_ASSERT_ARG_MIN( 1 );

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
DEFINE_FUNCTION( Untrap )
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
DEFINE_FUNCTION( LineToPC )
{
    JSScript *script;
    int32 i;
    uintN lineno;
    jsbytecode *pc;

	 JL_S_ASSERT_ARG_MIN(1);

	 script = JS_GetScriptedCaller(cx, NULL)->script;
    if (!GetTrapArgs(cx, argc, argv, &script, &i))
        return JS_FALSE;
    lineno = (i == 0) ? script->lineno : (uintN)i;
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
DEFINE_FUNCTION( PCToLine )
{
    JSScript *script;
    int32 i;
    uintN lineno;

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
DEFINE_PROPERTY( scriptFilenameList ) {

	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK(arr);
	*vp = OBJECT_TO_JSVAL(arr);

	int index;
	index = 0;
	for ( jl::QueueCell *it = jl::QueueBegin(scriptFileList); it; it = jl::QueueNext(it) ) {

		jl::Queue *scriptList = (jl::Queue*)jl::QueueGetData(it);
		JSScript *s = (JSScript*)jl::QueueGetData(jl::QueueBegin(scriptList));

		jsval filename;
		JL_CHK( StringToJsval(cx, s->filename, &filename) );
		JL_CHK( JS_SetElement(cx, arr, index, &filename) );
		++index;
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME $READONLY
  Is the filename of the script being executed.
  $H note
   The current filename is also available using: `StackFrameInfo(stackSize-1).filename`
**/
DEFINE_PROPERTY( currentFilename ) {

	JSStackFrame *fp = JL_CurrentStackFrame(cx);
	if ( fp == NULL ) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}
	JSScript *script = JS_GetFrameScript(cx, fp);
	if ( script == NULL ) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}

	const char *filename = JS_GetScriptFilename(cx, script);
	JL_CHK( StringToJsval(cx, filename, vp) );
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

  GetActualLineno('test.js', 2); // returns: 3
  GetActualLineno('nofile.js', 2); // returns: undefined
  }}}
**/
DEFINE_FUNCTION_FAST( GetActualLineno ) {

	JL_S_ASSERT_ARG_MIN( 2 );

	uintN lineno;
	JL_CHK( JsvalToUInt(cx, JL_FARG(2), &lineno) );

	JSScript *script;
	jsbytecode *pc;
	JL_CHK( GetScriptLocation(cx, &JL_FARG(1), lineno, &script, &pc) );
	if ( script == NULL ) {

		*JL_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}
	*JL_FRVAL = INT_TO_JSVAL(JS_PCToLineNumber(cx, script, pc));
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME $READONLY
  Is the number of stack frames. 0 is the older stack frame index. The current stack frame index is (stackSize-1).
**/
DEFINE_PROPERTY( stackSize ) {

	return IntToJsval(cx, JL_StackSize(cx, JL_CurrentStackFrame(cx)), vp);
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
DEFINE_FUNCTION_FAST( StackFrameInfo ) {

	JL_S_ASSERT_ARG_MIN( 1 );

	unsigned int frameIndex;
	JL_CHK( JsvalToUInt(cx, JL_FARG(1), &frameIndex) );

	JSStackFrame *fp;
	fp = JL_StackFrameByIndex(cx, frameIndex);
	if ( fp == NULL ) {

		*JL_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	JSObject *frameInfo;
	frameInfo = JS_NewObject(cx, NULL, NULL, NULL);
	JL_CHK( frameInfo );
	*JL_FRVAL = OBJECT_TO_JSVAL(frameInfo);
	jsval tmp;

	JSScript *script;
	script = JS_GetFrameScript(cx, fp);
	if ( script )
		JL_CHK( StringToJsval(cx, JS_GetScriptFilename(cx, script), &tmp) );
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

	tmp = OBJECT_TO_JSVAL(JS_GetFrameScopeChain(cx, fp));
	JL_CHK( JS_SetProperty(cx, frameInfo, "scope", &tmp) );

//	JL_CHK( JS_DefineProperty(cx, frameInfo, "variables", fp->varobj ? OBJECT_TO_JSVAL(fp->varobj) : JSVAL_VOID, NULL, NULL, JSPROP_ENUMERATE) );

	tmp = OBJECT_TO_JSVAL(JS_GetFrameThis(cx, fp));
	JL_CHK( JS_SetProperty(cx, frameInfo, "this", &tmp) );

	if ( fp->argv ) {

		JSObject *arguments;
		arguments = JS_NewArrayObject(cx, fp->argc, fp->argv);
//		arguments = js_GetArgsObject(cx, fp);
		tmp = OBJECT_TO_JSVAL(arguments);
	} else {

		tmp = JSVAL_VOID;
	}
	JL_CHK( JS_SetProperty(cx, frameInfo, "argv", &tmp) );

	tmp = JS_GetFrameReturnValue(cx, fp);
	JL_CHK( JS_SetProperty(cx, frameInfo, "rval", &tmp) );

	tmp = BOOLEAN_TO_JSVAL(JS_IsNativeFrame(cx, fp));
	JL_CHK( JS_SetProperty(cx, frameInfo, "isNative", &tmp) );

	tmp = JS_IsConstructorFrame(cx, fp) ? JSVAL_TRUE : JSVAL_FALSE;
	JL_CHK( JS_SetProperty(cx, frameInfo, "isConstructing", &tmp) );

	tmp = fp->flags & JSFRAME_EVAL ? JSVAL_TRUE : JSVAL_FALSE;
	JL_CHK( JS_SetProperty(cx, frameInfo, "isEval", &tmp) );

	tmp = fp->flags & JSFRAME_ASSIGNING ? JSVAL_TRUE : JSVAL_FALSE;
	JL_CHK( JS_SetProperty(cx, frameInfo, "isAssigning", &tmp) );

//	JL_CHK( JS_DefineProperty(cx, frameInfo, "opnd", fp->regs->sp[-1], NULL, NULL, JSPROP_ENUMERATE) );
//	char * s = JS_GetStringBytes(JS_ValueToString(cx, fp->regs->sp[-1]));

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME( code, frameLevel )
 Evaluates code in the given stack frame.
 0 is the older stack frame index. The current (last) stack frame index is (stackSize-1). See Locate() function for more details.
**/
DEFINE_FUNCTION_FAST( EvalInStackFrame ) {

	JL_S_ASSERT_ARG_MIN( 2 );

	JL_S_ASSERT_STRING( JL_FARG(1) );

	unsigned int frameIndex;
	JL_CHK( JsvalToUInt(cx, JL_FARG(2), &frameIndex) );

	JSStackFrame *fp;
	fp = JL_StackFrameByIndex(cx, frameIndex);

	if ( fp == NULL ) {

		*JL_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	JSScript *script;
	script = JS_GetFrameScript(cx, fp);

	jsbytecode *pc;
	pc = JS_GetFramePC(cx, fp);

	JSString *jsstr;
	jsstr = JSVAL_TO_STRING( JL_FARG(1) );
	JL_CHK( JS_EvaluateUCInStackFrame(cx, fp, JS_GetStringChars(jsstr), JL_GetStringLength(jsstr), JS_GetScriptFilename(cx, script), JS_PCToLineNumber(cx, script, pc), JL_FRVAL) );

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
   1 LoadModule('jsstd');
   2 LoadModule('jsdebug');
   3
   4 function test2() {
   5
   6 Print( stackSize, ' - ', Locate(stackSize-1), '\n' ); // prints: 3 - debug.js,6
   7 Print( Locate(-1), ' - ', Locate(-2), ' - ', Locate(-3), '\n' ); // prints: debug.js,13 - debug.js,16 - undefined
   8 Print( Locate(0), ' - ', Locate(1), ' - ', Locate(2), '\n' ); // prints: debug.js,16 - debug.js,13 - debug.js,8
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
DEFINE_FUNCTION_FAST( Locate ) {

	JSStackFrame *fp;
	if ( JL_FARG_ISDEF(1) ) {

		int frame;
		JL_CHK( JsvalToInt(cx, JL_FARG(1), &frame) );
		fp = JL_StackFrameByIndex(cx, frame);
	} else {

		fp = JL_CurrentStackFrame(cx);
	}

	if ( fp == NULL ) {

		*JL_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	JSScript *script;
	script = JS_GetFrameScript(cx, fp); // because we are in a fast native function, this frame is ok.
	uintN lineno;
	lineno = JS_PCToLineNumber(cx, script, JS_GetFramePC(cx, fp));

	JSObject *arrObj;
	arrObj = JS_NewArrayObject(cx, 2, NULL);
	*JL_FRVAL = OBJECT_TO_JSVAL(arrObj);

	jsval tmp;
	if ( script ) {

		const char *filename;
		filename = JS_GetScriptFilename(cx, script);
		JL_CHK( StringToJsval(cx, filename, &tmp) );
	} else { // native frame ?

		tmp = JSVAL_VOID;
	}
	JL_CHK( JS_SetElement(cx, arrObj, 0, &tmp) );

	tmp = INT_TO_JSVAL(lineno);
	JL_CHK( JS_SetElement(cx, arrObj, 1, &tmp) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( value )
  Try to find the definition location of the given value.
**/
DEFINE_FUNCTION_FAST( DefinitionLocation ) {

	JL_S_ASSERT_ARG_MIN( 1 );

	JSScript *script;
	script = NULL;

	if ( JsvalIsFunction(cx, JL_FARG(1)) ) {

		JSFunction *fun;
		fun = JS_ValueToFunction(cx, JL_FARG(1));
		if ( JS_GetFunctionScript(cx, fun) )
			script = JS_GetFunctionScript(cx, fun);
		goto next;
	}

	if ( !JSVAL_IS_PRIMITIVE( JL_FARG(1) ) ) {

		JSObject* obj;
		obj = JS_GetConstructor(cx, JSVAL_TO_OBJECT( JL_FARG(1) ));
		JSFunction *fun;
		fun = JS_ValueToFunction(cx, OBJECT_TO_JSVAL( obj ) );
		if ( fun ) {

			script = JS_GetFunctionScript(cx, fun);
			if ( script )
				goto next;
		}
	}

	if ( JsvalIsScript(cx, JL_FARG(1)) ) {

		JSObject* obj;
		obj = JSVAL_TO_OBJECT(JL_FARG(1));
		script = (JSScript*)JL_GetPrivate(cx, obj);
	}

next:
	if ( !script ) {

		*JL_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	jsval values[2];
	JL_CHK( StringToJsval(cx, script->filename, &values[0]) );
	IntToJsval(cx, script->lineno, &values[1] );
	*JL_FRVAL = OBJECT_TO_JSVAL( JS_NewArrayObject(cx, COUNTOF(values), values) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( object [, followPrototypeChain = false ] )
  Returns an array of properties name.
**/
DEFINE_FUNCTION_FAST( PropertiesList ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	JL_S_ASSERT_OBJECT( JL_FARG(1) );

	JSObject *srcObj;
	srcObj = JSVAL_TO_OBJECT( JL_FARG(1) );

	bool followPrototypeChain;
	if ( JL_FARG_ISDEF(2) )
		JL_CHK( JsvalToBool(cx, JL_FARG(2), &followPrototypeChain) );
	else
		followPrototypeChain = false;

	JSObject *arrayObject;
	arrayObject = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK( arrayObject );
	*JL_FRVAL = OBJECT_TO_JSVAL( arrayObject );

	jsval tmp;
	int index;
	index = 0;

	JSScopeProperty *jssp;

	while ( srcObj ) {

		jssp = NULL;
		JS_PropertyIterator(srcObj, &jssp);

		while ( jssp ) {

			tmp = ID_TO_VALUE(jssp->id);
			JL_CHK( JS_SetElement(cx, arrayObject, index, &tmp) );
			index++;
			JS_PropertyIterator(srcObj, &jssp);
		}

		srcObj = JS_GetPrototype(cx, srcObj);
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( object [, followPrototypeChain = false ] )
 Returns an array of properties information.
**/
DEFINE_FUNCTION_FAST( PropertiesInfo ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	JL_S_ASSERT_OBJECT( JL_FARG(1) );

	JSObject *srcObj;
	srcObj = JSVAL_TO_OBJECT( JL_FARG(1) );

	bool followPrototypeChain;
	if ( JL_FARG_ISDEF(2) )
		JL_CHK( JsvalToBool(cx, JL_FARG(2), &followPrototypeChain) );
	else
		followPrototypeChain = false;

//	JSObject *infoObject;
//	infoObject = JS_NewObjectWithGivenProto(cx, NULL, NULL, NULL);
//	*JL_FRVAL = OBJECT_TO_JSVAL( infoObject );

	JSObject *arrayObject;
	arrayObject = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK( arrayObject );
	*JL_FRVAL = OBJECT_TO_JSVAL( arrayObject );

	JSPropertyDesc desc;

	jsval tmp;
	int index;
	index = 0;
	int prototypeLevel;
	prototypeLevel = 0;

	JSScopeProperty *jssp;

	while ( srcObj ) {

		jssp = NULL;
		JS_PropertyIterator(srcObj, &jssp);

		while ( jssp ) {

			JL_CHK( JS_GetPropertyDesc(cx, srcObj, jssp, &desc) );

			JSObject *descObj = JS_NewObject(cx, NULL, NULL, NULL);
			tmp = OBJECT_TO_JSVAL(descObj);
			JL_CHK( JS_SetElement(cx, arrayObject, index, &tmp) );

			JL_CHK( JS_IdToValue(cx, jssp->id, &tmp) );
			JL_CHK( JS_SetProperty(cx, descObj, "name", &tmp) );

			tmp = desc.value;
			if ( desc.flags & JSPD_EXCEPTION ) // doc. exception occurred fetching the property, value is exception.
				JL_CHK( JS_SetProperty(cx, descObj, "exception", &tmp) );
			else
				JL_CHK( JS_SetProperty(cx, descObj, "value", &tmp) );

			tmp = desc.flags & JSPD_VARIABLE ? JSVAL_TRUE : JSVAL_FALSE; // doc. local variable in function
			JL_CHK( JS_SetProperty(cx, descObj, "variable", &tmp) );

			tmp = desc.flags & JSPD_ARGUMENT ? JSVAL_TRUE : JSVAL_FALSE; // doc. argument to function
			JL_CHK( JS_SetProperty(cx, descObj, "argument", &tmp) );

			tmp = desc.flags & JSPD_ENUMERATE ? JSVAL_TRUE : JSVAL_FALSE; // visible to for/in loop
			JL_CHK( JS_SetProperty(cx, descObj, "enumerate", &tmp) );
//			JL_CHK( SetPropertyBool(cx, descObj, "enumerate", desc.flags & JSPD_ENUMERATE) );

			tmp = desc.flags & JSPD_READONLY ? JSVAL_TRUE : JSVAL_FALSE;
			JL_CHK( JS_SetProperty(cx, descObj, "readonly", &tmp) );

			tmp = desc.flags & JSPD_PERMANENT ? JSVAL_TRUE : JSVAL_FALSE;
			JL_CHK( JS_SetProperty(cx, descObj, "permanent", &tmp) );

			tmp = jssp->setter != NULL || jssp->getter != NULL ? JSVAL_TRUE : JSVAL_FALSE;
			JL_CHK( JS_SetProperty(cx, descObj, "native", &tmp) );

			tmp = INT_TO_JSVAL(prototypeLevel);
			JL_CHK( JS_SetProperty(cx, descObj, "prototypeLevel", &tmp) );

			tmp = OBJECT_TO_JSVAL(srcObj);
			JL_CHK( JS_SetProperty(cx, descObj, "object", &tmp) );

			index++;
			JS_PropertyIterator(srcObj, &jssp);
		}

		srcObj = JS_GetPrototype(cx, srcObj);
		prototypeLevel++;
	}

	return JS_TRUE;
	JL_BAD;
}



JS_STATIC_ASSERT(JSTRY_CATCH == 0);
JS_STATIC_ASSERT(JSTRY_FINALLY == 1);
JS_STATIC_ASSERT(JSTRY_ITER == 2);

static const char* const TryNoteNames[] = { "catch", "finally", "iter" };

static JSBool
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


/* *doc
$TOC_MEMBER $INAME
 $TYPE Script $INAME( filename, lineno )
**/
/*
DEFINE_FUNCTION_FAST( ScriptByLocation ) {

	JL_S_ASSERT_ARG(2);

	const char *filename;
	unsigned int lineno;

	JL_CHK( JsvalToString(cx, &JL_FARG(1), &filename) );
	JL_CHK( JsvalToUInt(cx, JL_FARG(2), &lineno) );
	JSScript *script = ScriptByLocation(cx, scriptFileList, filename, lineno);
	JL_CHK( script );
	JSObject *scrobj = JS_GetScriptObject(script);
//	if ( scrobj == NULL )
//		scrobj = JS_NewScriptObject(cx, script); // Doc: https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_NewScriptObject

	if ( scrobj == NULL ) {
		
		*JL_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}
	
	*JL_FRVAL = OBJECT_TO_JSVAL(scrobj);
	return JS_TRUE;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $STRING $INAME( filename, lineno )
  Returns the assembly code of the given block location.
 $H beware
  This function is only available in DEBUG mode.
**/
DEFINE_FUNCTION_FAST( DisassembleScript ) {

	JL_S_ASSERT_ARG(2);

#ifdef DEBUG 
	
	const char *filename;
	unsigned int lineno;

	JL_CHK( JsvalToString(cx, &JL_FARG(1), &filename) );
	JL_CHK( JsvalToUInt(cx, JL_FARG(2), &lineno) );
	JSScript *script = ScriptByLocation(cx, scriptFileList, filename, lineno);
	JL_CHK( script );

	int length;
	FILE *rf, *wf;
	fpipe(&rf, &wf);

	jsbytecode *pc, *end;
	uintN len;
	pc = script->main;
	end = script->code + script->length;
	while (pc < end) {

		len = js_Disassemble1(cx, script, pc, pc - script->code, JS_TRUE, wf);
		if (!len)
			return JS_FALSE;
		pc += len;
	}

	length = ftell(wf);
	fflush(wf);
	fclose(wf);

	char *data = (char*)malloc(length+1);
	JL_S_ASSERT_ALLOC( data );
	fread(data, 1, length, rf);
	data[length] = '\0';
	fclose(rf);

	JSString *jsstr = JS_NewStringCopyN(cx, data, length);
	free(data);
	JL_S_ASSERT_ALLOC( jsstr );

	*JL_FRVAL = STRING_TO_JSVAL(jsstr);

#else // DEBUG
	
	*JL_FRVAL = JSVAL_VOID;

#endif // DEBUG

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the amount of CPU time (milliseconds) that the process has executed.
**/
DEFINE_PROPERTY( processTime ) {

#ifdef XP_WIN

	__int64 creationTime, exitTime, kernelTime, userTime;
	BOOL status = GetThreadTimes(GetCurrentThread(), (FILETIME *)&creationTime, (FILETIME *)&exitTime, (FILETIME *)&kernelTime, (FILETIME *)&userTime);
//	BOOL status = GetProcessTimes(GetCurrentProcess(), (FILETIME *)&creationTime, (FILETIME *)&exitTime, (FILETIME *)&kernelTime, (FILETIME *)&userTime);
	if ( !status ) {

		char message[1024];
		JLLastSysetmErrorMessage(message, sizeof(message));
		JL_REPORT_ERROR("%s", message);
	}
	return DoubleToJsval(cx, (kernelTime + userTime) / (double)10000 , vp);
#else // XP_WIN

	*vp = JSVAL_VOID;

#endif // XP_WIN

	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the current CPU usage in percent.
**/
DEFINE_PROPERTY( cpuLoad ) {

#ifdef XP_WIN

  static PDH_STATUS status;
  static PDH_FMT_COUNTERVALUE value;
  static HQUERY query;
  static HCOUNTER counter;
  static DWORD ret;
  static bool runonce = true;

  char errorMessage[1024];

	if ( runonce ) {

		status = PdhOpenQuery(NULL, 0, &query);
		if( status != ERROR_SUCCESS ) {

			SetLastError(status);
			JLLastSysetmErrorMessage(errorMessage, sizeof(errorMessage));
			JL_REPORT_ERROR("%s", errorMessage);
		}

		PdhAddCounter(query, TEXT("\\Processor(_Total)\\% Processor Time"), 0, &counter); // A total of ALL CPU's in the system
		PdhCollectQueryData(query); // No error checking here
		runonce = false;
	}

	status = PdhCollectQueryData(query);
	if ( status != ERROR_SUCCESS ) {

		SetLastError(status);
		JLLastSysetmErrorMessage(errorMessage, sizeof(errorMessage));
		JL_REPORT_ERROR("%s", errorMessage);
	}

	status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, &ret, &value);
	if ( status != ERROR_SUCCESS ) {

		SetLastError(status);
		JLLastSysetmErrorMessage(errorMessage, sizeof(errorMessage));
		JL_REPORT_ERROR("%s", errorMessage);
	}

	return DoubleToJsval(cx, value.doubleValue, vp);
#else // XP_WIN


	// cat /proc/stat

	*vp = JSVAL_VOID;

#endif // XP_WIN

	JL_BAD;
}



#ifdef DEBUG
DEFINE_FUNCTION( TestDebug ) {

	// see https://bugzilla.mozilla.org/show_bug.cgi?id=488924
	JSObject *o = JS_NewArrayObject(cx, 0, NULL);
	JSScopeProperty *jssp;
	jssp = NULL;
	JS_PropertyIterator(o, &jssp);
	return JS_TRUE;
	JL_BAD;
}
#endif // DEBUG



CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( GetObjectPrivate )
//		FUNCTION( DumpStats )
		FUNCTION_FAST( TraceGC )

//		FUNCTION_FAST( ScriptByLocation )
		FUNCTION_FAST( DisassembleScript )

//		FUNCTION( Trap )
//		FUNCTION( Untrap )
//		FUNCTION( LineToPC )
//		FUNCTION( PCToLine )

		FUNCTION_FAST( GetActualLineno )
		FUNCTION_FAST( Locate )
		FUNCTION_FAST( DefinitionLocation )
		FUNCTION_FAST( StackFrameInfo )
		FUNCTION_FAST( EvalInStackFrame )
		FUNCTION_FAST_ARGC( PropertiesList, 1 )
		FUNCTION_FAST_ARGC( PropertiesInfo, 1 )
	#ifdef DEBUG
		FUNCTION_FAST( DumpHeap )
		FUNCTION( TestDebug )
	#endif // DEBUG
		FUNCTION_FAST( DisableJIT )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ( scriptFilenameList )
		PROPERTY_READ( currentFilename )
		PROPERTY_READ( stackSize )
	#ifdef JS_GC_ZEAL
		PROPERTY_WRITE( gcZeal )
	#endif

		PROPERTY_READ( gcNumber )
		PROPERTY_READ( gcMallocBytes )
		PROPERTY_READ( gcBytes )
		PROPERTY_READ( currentMemoryUsage )
		PROPERTY_READ( peakMemoryUsage )
		PROPERTY_READ( privateMemoryUsage )
		PROPERTY_READ( processTime )
		PROPERTY_READ( cpuLoad )
	END_STATIC_PROPERTY_SPEC

END_STATIC
