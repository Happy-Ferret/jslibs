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

#include <js/OldDebugAPI.h> // JS_DefineDebuggerObject


/*
int _puts(JSContext *cx, const char *str) {

	JS::RootedValue stdoutFunction(cx);

	JL_CHK( JS_GetPropertyById(cx, jl::Host::getHost(cx).hostObject(), JLID(cx, stdout), &stdoutFunction) );

	if ( jl::isCallable(cx, stdoutFunction) ) {

		int len = (int)strlen(str);
		JSString *jsstr = JS_NewStringCopyN(cx, str, len);
		if ( jsstr == NULL )
			return EOF;
	
		//	jsstr = JS_ConcatStrings(cx, jsstr, JS_NewStringCopyZ(cx, "\n"));

		JS::RootedObject global(cx, JL_GetGlobal(cx));
		bool status = jl::callNoRval(cx, global, stdoutFunction, ???);
		if ( status == true )
			return len;
	}

bad:
	return EOF;
}
*/

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


DEFINE_FUNCTION( registerDumpHeap ) {

	struct DumpHeap: jl::Callback {

		jl::HostRuntime &_hostRuntime;
		DumpHeap( jl::HostRuntime &hostRuntime )
		: _hostRuntime(hostRuntime) {
		}
		void operator()() {

			FILE *file = fopen("dump.txt", "w");
			IFDEBUG( JS_DumpHeap(_hostRuntime.runtime(), file, nullptr, JSTRACE_OBJECT, nullptr, 1, nullptr) );
			fclose(file);
		}
	};

	jl::HostRuntime &hostRuntime = jl::Host::getHost(cx).hostRuntime();

	hostRuntime.addListener(jl::EventId::BEFORE_DESTROY_RUNTIME, new DumpHeap(hostRuntime) );

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**doc
=== Static properties ===
**/



/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  see: https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JS_Debugger_API_Reference/Debugger
**/
DEFINE_PROPERTY_GETTER( Debugger ) {

	JL_DEFINE_PROP_ARGS;
	JS::RootedObject global(cx, JL_GetGlobal(cx));
	JS::CompartmentOptions compartmentOptions( CompartmentOptionsRef(global) );
	compartmentOptions.setInvisibleToDebugger(true);
	JS::RootedObject dbgGlobal(cx, JS_NewGlobalObject(cx, JL_GetClass(global), nullptr, JS::DontFireOnNewGlobalHook, compartmentOptions));

	{
		JSAutoCompartment ac(cx, dbgGlobal);
		//JL_CHK( JS_InitStandardClasses(cx, dbgGlobal) );
		JL_CHK( JS_DefineDebuggerObject(cx, dbgGlobal) ); // doc: https://developer.mozilla.org/en/SpiderMonkey/JS_Debugger_API_Guide
		JS_FireOnNewGlobalObject(cx, dbgGlobal);
	}

	JL_CHK( JS_WrapObject(cx, &dbgGlobal) );
	JL_CHK( jl::getProperty(cx, dbgGlobal, id, &vp) ); // id is "Debugger"
		
	JL_CHK( args.store(true) );

	return true;
	JL_BAD;
}



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

	JL_DEFINE_PROP_ARGS;
	JL_CHK( jl::setValue(cx, JL_RVAL, JS_GetGCParameter(JL_GetRuntime(cx), JSGC_BYTES)) );
	return true;
	JL_BAD;
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

	JL_DEFINE_PROP_ARGS;

#ifdef JS_GC_ZEAL

	uint8_t zeal;
	JL_CHKM( jl::getValue(cx, JL_RVAL, &zeal), E_VALUE, E_INVALID );
	JS_SetGCZeal(cx, zeal, 1); // JS_DEFAULT_ZEAL_FREQ
	return jl::StoreProperty(cx, obj, id, vp, false); // make the value available for default getter

#else // JS_GC_ZEAL

	JL_WARN( E_THISOPERATION, E_NOTSUPPORTED );
	vp.setUndefined();
	return true;

#endif // JS_GC_ZEAL

	JL_BAD;
}




DEFINE_FUNCTION( debugOutput ) {

	JL_DEFINE_ARGS;

	JL_ASSERT_ARGC(1);

#if defined(_MSC_VER) && defined(DEBUG)
	{
	jl::BufString str;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &str) );
	OutputDebugString(str); // (TBD) not thread-safe, use a critical section
	}
	JL_RVAL.setBoolean(true);
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

	JL_DEFINE_ARGS;

	// *((int *)0) = 0;

#if defined(WIN32)
	__debugbreak();
#elif defined(__APPLE__)
	__debugbreak();
#else
	raise(SIGABRT);
#endif

	JL_RVAL.setUndefined();
	return true;
}

/*
static bool crashGuardInner(JSContext *cx, JS::HandleObject obj, JS::HandleValue fctVal) {

#if defined(WIN)

	EXCEPTION_POINTERS * eps = 0;
	__try {

		JL_CHK( jl::callNoRval(cx, obj, fctVal) );
		return true;

//	} __except (eps = GetExceptionInformation(), ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)) {
	} __except (eps = GetExceptionInformation(), EXCEPTION_EXECUTE_HANDLER) {
		
		JL_ERR(E_FUNC, E_INTERNAL);
	}

#elif defined UNIX

	JL_CHK( jl::callNoRval(cx, obj, fctVal) );

#else

	#error NOT IMPLEMENTED YET	// (TBD)

#endif

	return true;
	JL_BAD;
}


DEFINE_FUNCTION( crashGuard ) {

	JL_DEFINE_ARGS;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_CALLABLE(1);

	JL_RVAL.setBoolean(crashGuardInner(cx, JL_OBJ, JL_ARG(1)));

	return true;
	JL_BAD;
}
*/


DEFINE_FUNCTION( setPerfTestMode ) {

	JL_DEFINE_ARGS;
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

	JL_DEFINE_ARGS;
	JL_RVAL.setUndefined();
	return true;
}

DEFINE_FUNCTION( test2Debug ) {

	JL_DEFINE_ARGS;
	JL_RVAL.setUndefined();
	return true;
}

#endif // DEBUG



CONFIGURE_STATIC

	REVISION(jl::SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC

		FUNCTION_ARGC( debugOutput, 1 )

		FUNCTION( registerDumpHeap )
		FUNCTION( debugBreak )
//		FUNCTION( crashGuard )
		FUNCTION( setPerfTestMode )

	#ifdef VALGRIND
		FUNCTION( createLeak )
		FUNCTION( VALGRIND_COUNT_ERRORS )
		FUNCTION( VALGRIND_DO_QUICK_LEAK_CHECK )
		FUNCTION( VALGRIND_DO_LEAK_CHECK )
		FUNCTION( VALGRIND_COUNT_LEAKS )
	#endif // VALGRIND

		// for internal tests
	#ifdef DEBUG
		FUNCTION( testDebug )
		FUNCTION( test2Debug )
	#endif // DEBUG

	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC

		PROPERTY_GETTER( Debugger )

		PROPERTY_SETTER( gcZeal )

		PROPERTY_GETTER( gcNumber )
//		PROPERTY_GETTER( gcMallocBytes )
		PROPERTY_GETTER( gcBytes )

	END_STATIC_PROPERTY_SPEC

END_STATIC
