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

#include "host2.h"

JL_BEGIN_NAMESPACE


const JSClass HostRuntime::_globalClass = {
	NAME_GLOBAL_CLASS, JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_DeletePropertyStub,
	JS_PropertyStub, JS_StrictPropertyStub,
	JS_EnumerateStub, JS_ResolveStub,
	JS_ConvertStub
};


BEGIN_CLASS( host2 )

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( unsafeMode ) {

	JL_IGNORE( id, obj );

	JL_CHK( JL_NativeToJsval(cx, Host::getHostPrivate(cx).unsafeMode(), vp) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( jsVersion ) {

	JL_IGNORE( id, obj );

	JL_CHK( JL_NativeToJsval(cx, JS_GetVersion(cx), vp) ); // btw, see JS_GetImplementationVersion()
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( incrementalGarbageCollector ) {

	JL_IGNORE( id, obj );

	uint32_t gcMode = JS_GetGCParameter(JL_GetRuntime(cx), JSGC_MODE);
	JL_CHK( JL_NativeToJsval(cx, gcMode == JSGC_MODE_INCREMENTAL, vp) ); // JSGC_MODE_GLOBAL
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( incrementalGarbageCollector ) {

	JL_IGNORE( strict, id, obj );

	bool incGc;
	JL_CHK( JL_JsvalToNative(cx, vp, &incGc) );
	JS_SetGCParameter(JL_GetRuntime(cx), JSGC_MODE, incGc ? JSGC_MODE_INCREMENTAL : JSGC_MODE_GLOBAL);
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
**/
DEFINE_FUNCTION( stdout ) {

	JL_DEFINE_ARGS;
	JL_RVAL.setUndefined();
	JLData str;
	Host &host(Host::getHostPrivate(cx));
	for ( unsigned i = 0; i < argc; ++i ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(i+1), &str) );
		int status = host.stdOutput(str.GetConstStr(), str.Length());
		JL_ASSERT_WARN( status != -1, E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stdout"), E_WRITE );
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
**/
DEFINE_FUNCTION( stderr ) {

	JL_DEFINE_ARGS;
	JL_RVAL.setUndefined();
	JLData str;
	Host &host(Host::getHostPrivate(cx));
	for ( unsigned i = 0; i < argc; ++i ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(i+1), &str) );
		int status = host.stdError(str.GetConstStr(), str.Length());
		JL_ASSERT_WARN( status != -1, E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stderr"), E_WRITE );
	}
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME()
**/
DEFINE_FUNCTION( stdin ) {

	JL_DEFINE_ARGS;

	Host &host(Host::getHostPrivate(cx));

	char buffer[8192];
	int status = host.stdInput(buffer, COUNTOF(buffer));
	if ( status > 0 ) {
		
		JS::RootedString jsstr(cx, JS_NewStringCopyN(cx, buffer, status));
		JL_CHK( jsstr );
		JL_RVAL.setString(jsstr);
	} else {

		JL_WARN( E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stdin"), E_READ );
		JL_RVAL.setString(JL_GetEmptyString(cx));
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $OBJ | null $INAME()
**/
DEFINE_FUNCTION( loadModule ) {

	JLLibraryHandler module = JLDynamicLibraryNullHandler;
	JLData str;

	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );

	char libFileName[PATH_MAX];
	strncpy( libFileName, str.GetConstStr(), str.Length() );
	libFileName[str.Length()] = '\0';
	strcat( libFileName, DLL_EXT );
// MAC OSX: 	'@executable_path' ??

/*
	while (0) { // namespace management. Avoid using val ns = {}, loadModule.call(ns, '...');

		if ( JL_ARG_ISDEF(2) ) {

			if ( JSVAL_IS_OBJECT(JL_ARG(2)) ) {
				obj = JSVAL_TO_OBJECT(JL_ARG(2));
			} else {
				const char *ns;
				JL_CHK( JL_JsvalToNative(cx, &JL_ARG(2), &ns) );

				jsval existingNsVal;
				JL_CHK( JS_GetProperty(cx, obj, ns, &existingNsVal) );
				JSObject *nsObj;
				if ( existingNsVal == JSVAL_VOID ) {

					nsObj = JS_NewObject(cx, NULL, NULL, NULL);
					JL_CHK( JS_DefineProperty(cx, obj, ns, OBJECT_TO_JSVAL(nsObj), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) ); // doc. On success, JS_DefineProperty returns true. If the property already exists or cannot be created, JS_DefineProperty returns false.
				} else {

					JL_ASSERT_OBJECT( existingNsVal );
					nsObj = JSVAL_TO_OBJECT( existingNsVal );
				}
				obj = nsObj;
			}
		}
	}
*/

	HostPrivate *hpv;
	hpv = JL_GetHostPrivate(cx);
	JL_ASSERT( hpv, E_HOST, E_STATE, E_COMMENT("context private") );
	JL_ASSERT( libFileName != NULL && *libFileName != '\0', E_ARG, E_NUM(1), E_DEFINED );

	module = JLDynamicLibraryOpen(libFileName);
	if ( !JLDynamicLibraryOk(module) ) {

		JL_SAFE_BEGIN
		char errorBuffer[256];
		JLDynamicLibraryLastErrorMessage( errorBuffer, sizeof(errorBuffer) );
		JL_WARN( E_OS, E_OPERATION, E_DETAILS, E_STR(errorBuffer), E_COMMENT(libFileName) );
		JL_SAFE_END

		JL_RVAL.setBoolean(false);
		return true;
	}

	for ( jl::QueueCell *it = jl::QueueBegin(&hpv->moduleList); it; it = jl::QueueNext(it) ) {

		if ( (JLLibraryHandler)jl::QueueGetData(it) == module ) {

			JLDynamicLibraryClose(&module);
			JL_RVAL.setNull(); // already loaded
			return true;
		}
	}

	uint32_t uid;
	uid = JLDynamicLibraryId(module); // module unique ID
	ModuleInitFunction moduleInit;
	moduleInit = (ModuleInitFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_INIT);
	JL_ASSERT( moduleInit, E_MODULE, E_NAME(libFileName), E_INIT ); // "Invalid module."
	
//	CHKHEAP();

	if ( !moduleInit(cx, JL_OBJ, uid) ) {

		JL_CHK( !JL_IsExceptionPending(cx) );
		char filename[PATH_MAX];
		JLDynamicLibraryName((void*)moduleInit, filename, sizeof(filename));
		JL_ERR( E_MODULE, E_NAME(filename), E_INIT );
	}

//	CHKHEAP();

	jl::QueueUnshift( &hpv->moduleList, module ); // store the module (LIFO)
	
	//JL_CHK( JL_NewNumberValue(cx, uid, JL_RVAL) ); // really needed ? yes, UnloadModule will need this ID, ... but UnloadModule is too complicated to implement and will never exist.
	JL_RVAL.setObject(*JL_OBJ);

	return true;

bad:
	if ( JLDynamicLibraryOk(module) )
		JLDynamicLibraryClose(&module);
	return false;
}


DEFINE_INIT() {

	JL_IGNORE( proto, sc );

	JL_GetHostPrivate(cx)->hostObject = obj;
	return true;
}


/**qa
	QA.ASSERTOP(global, 'has', 'host');
	QA.ASSERTOP(host, 'has', 'stdout');
	QA.ASSERTOP(host, 'has', 'unsafeMode');
**/
CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))

	HAS_INIT

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_ARGC(loadModule, 1)

		// note: we have to support: var prevStderr = host.stderr; host.stderr = function(txt) { file.Write(txt); prevStderr(txt) };
		// route: print() => host->stdout() => JSDefaultStdoutFunction() => hpv->hostStdOut()
		FUNCTION_ARGC(stdin, 0)
		FUNCTION_ARGC(stdout, 1)
		FUNCTION_ARGC(stderr, 1)
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( unsafeMode )
		PROPERTY_GETTER( jsVersion )
		PROPERTY( incrementalGarbageCollector )
	END_STATIC_PROPERTY_SPEC

END_CLASS


//////////////////////////////////////////////////////////////////////////////
// HostRuntime definition
/*
bool
HostRuntime::OperationCallback(JSContext *cx) {

	JSOperationCallback tmp = JS_SetOperationCallback(JL_GetRuntime(cx), NULL);
	JS_MaybeGC(cx);
	JS_SetOperationCallback(JL_GetRuntime(cx), tmp);
	return true;
}

void
HostRuntime::ErrorReporterBasic(JSContext *cx, const char *message, JSErrorReport *report) {

	JL_IGNORE( cx );
	if ( !report )
		fprintf(stderr, "%s\n", message);
	else
		fprintf(stderr, "%s (%s:%d)\n", message, report->filename ? report->filename : "<no filename>", (unsigned int)report->lineno);
}


HostRuntime::HostRuntime() : rt(NULL), cx(NULL), _isEnding(false), _skipCleanup(false) {
}

JSRuntime *
HostRuntime::runtime() const {

	return rt;
}
	
JSContext *
HostRuntime::context() const {

	return cx;
}

bool
HostRuntime::create( uint32_t maxMem, uint32_t maxAlloc, uint32_t maybeGCInterval ) {

	rt = JS_NewRuntime(maxAlloc, JS_NO_HELPER_THREADS); // JSGC_MAX_MALLOC_BYTES
	JL_CHK( rt );

	// Number of JS_malloc bytes before last ditch GC.
	ASSERT( JS_GetGCParameter(rt, JSGC_MAX_MALLOC_BYTES) == maxAlloc ); // JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, maxAlloc);

	// doc: maxMem specifies the number of allocated bytes after which garbage collection is run. Maximum nominal heap before last ditch GC.
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, maxMem); 

	//JS_SetGCParametersBasedOnAvailableMemory

	cx = JS_NewContext(rt, 8192); // set the chunk size of the stack pool to 8192. see http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3
	JL_CHK( cx ); //, "unable to create the context." );

	// Info: Increasing JSContext stack size slows down my scripts:
	//   http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3

	JS_SetErrorReporter(cx, ErrorReporterBasic);

	JS::ContextOptionsRef(cx)
		.setVarObjFix(true)
		.setTypeInference(true)
		.setIon(true);

	//JS_SetNativeStackQuota(cx, DEFAULT_MAX_STACK_SIZE); // see https://developer.mozilla.org/En/SpiderMonkey/JSAPI_User_Guide
	JS_SetGCParameter(rt, JSGC_MODE, JSGC_MODE_INCREMENTAL); // JSGC_MODE_GLOBAL


	// JSOPTION_ANONFUNFIX: https://bugzilla.mozilla.org/show_bug.cgi?id=376052 
	// JS_SetOptions doc: https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_SetOptions

	// beware: avoid using JSOPTION_COMPILE_N_GO here.

	{

	JS::CompartmentOptions options;
	options
		.setVersion(JSVERSION_LATEST)
		.setInvisibleToDebugger(false)
		.setMergeable(false);

	JS::RootedObject globalObject(cx, JS_NewGlobalObject(cx, &globalClass, NULL, JS::DontFireOnNewGlobalHook, options));
	JL_CHK( globalObject ); // "unable to create the global object." );

	// set globalObject as current global object.
	JL_CHK( JS_EnterCompartment(cx, globalObject) );

	JL_CHK( JS_InitStandardClasses(cx, globalObject) );
	JL_CHK( JS_DefineDebuggerObject(cx, globalObject) ); // doc: https://developer.mozilla.org/en/SpiderMonkey/JS_Debugger_API_Guide
	JL_CHK( JS_InitReflect(cx, globalObject) );
	#ifdef JS_HAS_CTYPES
	JL_CHK( JS_InitCTypesClass(cx, globalObject) );
	#endif

	JS_FireOnNewGlobalObject(cx, globalObject);

	}

	return true;
	JL_BAD;
}


bool
HostRuntime::destroy(bool skipCleanup) {

	ASSERT( _isEnding == false );
	_isEnding = true;
	_skipCleanup = skipCleanup;


	//	don't try to break linked objects with JS_GC(cx) !

//	jsval tmp;
//	JL_CHK( GetConfigurationValue(cx, JLID_NAME(_getErrorMessage), &tmp) );
//	if ( tmp != JSVAL_VOID && JSVAL_TO_PRIVATE(tmp) )
//		jl_free( JSVAL_TO_PRIVATE(tmp) );


// cleanup

	// doc:
	//  - Is the only side effect of JS_DestroyContextNoGC that any finalizers I may have specified in custom objects will not get called ?
	//  - Not if you destroy all contexts (whether by NoGC or not), destroy all runtimes, and call JS_ShutDown before exiting or hibernating.
	//    The last JS_DestroyContext* API call will run a GC, no matter which API of that form you call on the last context in the runtime. /be
		
	// see create()
	JS_LeaveCompartment(cx, NULL);
	JS_DestroyContext(cx);
	cx = NULL;

	#ifdef DEBUG
	JS_DumpHeap(rt, fopen("dump.txt", "w"), NULL, JSTRACE_OBJECT, NULL, 2, NULL);
	#endif

	JS_DestroyRuntime(rt);
	rt = NULL;

	return true;

bad:
	// on error, do the minimum.
	if ( cx ) {

		JS_LeaveCompartment(cx, NULL);
		JS_DestroyContext(cx);
		JS_DestroyRuntime(rt);
	}
	return false;
}
*/

JL_END_NAMESPACE
