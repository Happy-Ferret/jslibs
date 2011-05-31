#define XP_WIN

#include <../common/jlhelper.h>
#include <../common/jlhelper.cpp>
#include <../common/jslibsModule.cpp>

#include <jsapi.h>
#include <jsvalue.h>
#include <string.h>
#include <jsprf.h>


void StderrWrite(JSContext *cx, const char *message, size_t length) {

	fwrite(message, 1, length, stderr);
}


void ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report) {

	// trap JSMSG_OUT_OF_MEMORY error to avoid calling ErrorReporter_stdErrRouter() that may allocate memory that will lead to nested call.
	if (unlikely( report && report->errorNumber == JSMSG_OUT_OF_MEMORY )) { // (TBD) do something better
		
		fprintf(stderr, "%s (%s:%d)\n", message, report->filename, report->lineno);
		return;
	}

	bool reportWarnings = JL_IS_SAFE; // no warnings in unsafe mode.

	char buffer[1024];
	char *buf = buffer;

	#if defined(fprintf) || defined(fputs) || defined(fwrite) || defined(fputc)
		#error CANNOT DEFINE MACROS fprintf, fputs, fwrite, fputc
	#endif

	#define fprintf(FILE, FORMAT, ...) \
	JL_MACRO_BEGIN \
		size_t remaining = sizeof(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		int count = snprintf(buf, remaining, FORMAT, ##__VA_ARGS__); \
		buf += count < 0 ? remaining : count; \
	JL_MACRO_END

	#define fputs(STR, FILE) \
	JL_MACRO_BEGIN \
		size_t remaining = sizeof(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		size_t len = JL_MIN(strlen(STR), remaining); \
		memcpy(buf, STR, len); \
		buf += len; \
	JL_MACRO_END

	#define fwrite(STR, SIZE, COUNT, FILE) \
	JL_MACRO_BEGIN \
		size_t remaining = sizeof(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		size_t len = JL_MIN(size_t((SIZE)*(COUNT)), remaining); \
		memcpy(buf, (STR), len); \
		buf += len; \
	JL_MACRO_END

	#define fputc(CHR, FILE) \
	JL_MACRO_BEGIN \
		size_t remaining = sizeof(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		buf[0] = (CHR); \
		buf += 1; \
	JL_MACRO_END
	

// copy-paste from /js/src/js.cpp (my_ErrorReporter)
//	 ---8<---

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
    n = strlen(report->linebuf);
    fprintf(gErrFile, ":\n%s%s%s%s",
            prefix,
            report->linebuf,
            (n > 0 && report->linebuf[n-1] == '\n') ? "" : "\n",
            prefix);
    n = report->tokenptr - report->linebuf;
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
    //if (!JSREPORT_IS_WARNING(report->flags)) {
    //    if (report->errorNumber == JSMSG_OUT_OF_MEMORY) {
    //        gExitCode = EXITCODE_OUT_OF_MEMORY;
    //    } else {
    //        gExitCode = EXITCODE_RUNTIME_ERROR;
    //    }
    //}
    JS_free(cx, prefix);

//	 ---8<---

	#undef fprintf
	#undef fputs
	#undef fwrite
	#undef fputc
	 
	StderrWrite(cx, buffer, buf-buffer);
}

JSClass global_class = {
	 "global", JSCLASS_GLOBAL_FLAGS, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
};

JSBool Exec(JSContext *cx, uintN argc, jsval *vp) {

	char *scriptText = "function();";
	JSObject *scriptObject = JS_CompileScript(cx, JS_GetGlobalObject(cx), scriptText, strlen(scriptText), "<exec>", 1);
	return scriptObject ? JS_TRUE : JS_FALSE;
}

int main(int argc, char* argv[]) {

	_unsafeMode = false;

	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_JIT);
	JSObject *globalObject = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);

	JL_CHK( JS_DefineFunction(cx, globalObject, "Exec", Exec, 0, 0) );

	JS_SetErrorReporter(cx, ErrorReporter);

	char *scriptText = "\n\n\nExec();";
	JSObject *scriptObject = JS_CompileScript(cx, globalObject, scriptText, strlen(scriptText), "<inline>", 1);

	jsval rval;
	JL_CHK( JS_ExecuteScript(cx, globalObject, scriptObject, &rval) );


	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

	return EXIT_SUCCESS;
bad:
	printf("BAD\n");
	return EXIT_FAILURE;
}
