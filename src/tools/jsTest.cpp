#ifdef WIN32
	#define _CRT_SECURE_NO_WARNINGS
	#define _CRT_NONSTDC_NO_WARNINGS
	#define XP_WIN
	#include <io.h>
#else // Linux
	#define XP_UNIX
	#include <unistd.h>
#endif

#include <jsapi.h>
#include <jsxdrapi.h>
#include <jsprf.h>

#include <jscntxt.h>

#include <jsscript.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

bool _unsafeMode = false;
#include "../common/jsHelper.h"


#ifndef O_BINARY
	#define O_BINARY 0
#endif

#ifndef O_SEQUENTIAL
	#define O_SEQUENTIAL 0
#endif

FILE *gErrFile = stdout;
bool reportWarnings = true;

// copied from src/shell/js.cpp
static void
my_ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report)
{
    int i, j, k, n;
    char *prefix, *tmp;
    const char *ctmp;

    if (!report) {
        fprintf(gErrFile, "%s\n", message);
        return;
    }

    /* Conditionally ignore reported warnings. */
    if (JSREPORT_IS_WARNING(report->flags) && !reportWarnings)
        return;

    prefix = NULL;
    if (report->filename)
        prefix = JS_smprintf("%s:", report->filename);
    if (report->lineno) {
        tmp = prefix;
        prefix = JS_smprintf("%s%u: ", tmp ? tmp : "", report->lineno);
        JS_free(cx, tmp);
    }
    if (JSREPORT_IS_WARNING(report->flags)) {
        tmp = prefix;
        prefix = JS_smprintf("%s%swarning: ",
                             tmp ? tmp : "",
                             JSREPORT_IS_STRICT(report->flags) ? "strict " : "");
        JS_free(cx, tmp);
    }

    /* embedded newlines -- argh! */
    while ((ctmp = strchr(message, '\n')) != 0) {
        ctmp++;
        if (prefix)
            fputs(prefix, gErrFile);
        fwrite(message, 1, ctmp - message, gErrFile);
        message = ctmp;
    }

    /* If there were no filename or lineno, the prefix might be empty */
    if (prefix)
        fputs(prefix, gErrFile);
    fputs(message, gErrFile);

    if (!report->linebuf) {
        fputc('\n', gErrFile);
        goto out;
    }

    /* report->linebuf usually ends with a newline. */
    n = (int)strlen(report->linebuf);
    fprintf(gErrFile, ":\n%s%s%s%s",
            prefix,
            report->linebuf,
            (n > 0 && report->linebuf[n-1] == '\n') ? "" : "\n",
            prefix);
    n = (int)(report->tokenptr - report->linebuf);
    for (i = j = 0; i < n; i++) {
        if (report->linebuf[i] == '\t') {
            for (k = (j + 8) & ~7; j < k; j++) {
                fputc('.', gErrFile);
            }
            continue;
        }
        fputc('.', gErrFile);
        j++;
    }
    fputs("^\n", gErrFile);
 out:
/*
	 if (!JSREPORT_IS_WARNING(report->flags)) {
        if (report->errorNumber == JSMSG_OUT_OF_MEMORY) {
            gExitCode = EXITCODE_OUT_OF_MEMORY;
        } else {
            gExitCode = EXITCODE_RUNTIME_ERROR;
        }
    }
*/
    JS_free(cx, prefix);
}



static void MyFinalize(JSContext *cx, JSObject *obj) {
	
	printf("Finalize\n");
}

static JSBool MyConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	
	return JS_TRUE;
}

static JSBool MyFunction(JSContext *cx, uintN argc, jsval *vp) {

	return JS_TRUE;
}

int Test1() {

	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);

	JSObject *globalObject = JS_NewObject(cx, NULL, NULL, NULL);
	JS_SetGlobalObject(cx, globalObject);
	JS_InitStandardClasses(cx, globalObject);

	JS_SetScriptStackQuota(cx, JS_DEFAULT_SCRIPT_STACK_QUOTA);
	JS_SetVersion(cx, (JSVersion)JSVERSION_LATEST);
	JS_SetErrorReporter(cx, my_ErrorReporter);
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_STRICT | JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_JIT | JSOPTION_COMPILE_N_GO);

	globalObject->defineProperty(cx, ATOM_TO_JSID(JS_GetRuntime(cx)->atomState.typeAtoms[JSTYPE_VOID]), JSVAL_VOID, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY);

	static JSExtendedClass myClass = { { "MyClass", JSCLASS_HAS_RESERVED_SLOTS(1) | JSCLASS_HAS_PRIVATE, JS_PropertyStub , JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub , JS_ConvertStub, MyFinalize, JSCLASS_NO_OPTIONAL_MEMBERS }, 0 };

	JSFunctionSpec fs[] = { JS_FN( "MyFunction", MyFunction, 0, 0 ), {0} };
	JSObject *proto = JS_InitClass(cx, globalObject, NULL, &myClass.base, MyConstructor, 0, NULL, fs, NULL, NULL);

	// https://bugzilla.mozilla.org/show_bug.cgi?id=522101
	// [22:25]	<brendan>	soubok: i gave you bugzilla canconfirm and editbugs privs so your bugs will start out NEW not UNCONFIRMED
	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
	JSScopeProperty *jssp = NULL;
	JS_PropertyIterator(arr, &jssp); // Assertion failure: OBJ_IS_NATIVE(obj)


	char scriptSrc[] = "\
var list = [];\
function Add() {\
   \
    list.push(arguments);\
}\
\
function Run() {\
   \
    for each ( var item in list )\
        item[0](); \
}\
\
for ( var i = 0; i < 10; i++ )\
    Add(function(s) {});\
\
Run(); \
	\
";

//	JSScript *script = JS_CompileScript(cx, globalObject, scriptSrc, strlen(scriptSrc), "mytest", 1);
	
	FILE *file = fopen("mytest.js", "r");
	JSScript *script = JS_CompileFileHandle(cx, globalObject, "mytest.js", file);

	jsval rval;
	JSBool st = JS_ExecuteScript(cx, globalObject, script, &rval);


	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();
	return EXIT_SUCCESS;
}



int Test2() {


/*
jsval nan;
char scriptSrc[] = "Number.NaN";
JS_EvaluateScript(cx, globalObject, scriptSrc, strlen(scriptSrc), "test", 1, &nan); // "Access violation reading location 0xfffffffc." in TraceRecorder::lazilyImportGlobalSlot() (globalObj->dslots == NULL).


double v1 = *JSVAL_TO_DOUBLE(JS_GetNaNValue(cx));
double v2 = v1;


/*
s64_t s64;

s64 = *(s64_t*)JSVAL_TO_DOUBLE(JS_GetNaNValue(cx));

__int64 d1 = *(__int64*)JSVAL_TO_DOUBLE(JS_GetNaNValue(cx));
__int64 d2 = *(__int64*)JSVAL_TO_DOUBLE(nan);
res = d1 == d1;
*/

/*
	JSScript *script = JS_CompileScript(cx, globalObject, scriptSrc, strlen(scriptSrc), "myScript", 1);

	jsval rval;
	JSBool res = JS_ExecuteScript(cx, globalObject, script, &rval);
	printf("result: %d\n", res);

	printf("gcBytes before: %d\n", rt->gcBytes );
	JS_GC(cx);
	printf("gcBytes after: %d\n", rt->gcBytes );
*/	

	return EXIT_SUCCESS;
}

/*
void   __cdecl free(__inout_opt void * _Memory) {

	
}
*/

int Test3() {

	

//	void *mem = malloc(100);
//	free(mem);



	return EXIT_SUCCESS;
}



int main(int argc, char* argv[]) {



	Test1();
//	Test3();
	//	return Test2();

	return EXIT_SUCCESS;
}
