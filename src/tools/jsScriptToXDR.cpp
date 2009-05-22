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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>


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


int main(int argc, char* argv[]) {
	
	if ( argc < 2 )
		return EXIT_FAILURE;

	printf("Deleting the destination file %s\n", argv[2]);
	remove(argv[2]);

	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetVersion(cx, (JSVersion)JSVERSION_LATEST);
	JS_SetErrorReporter(cx, my_ErrorReporter);

	JSObject *globalObject = JS_NewObject(cx, NULL, NULL, NULL);
	JS_SetGlobalObject(cx, globalObject);
	JS_InitStandardClasses(cx, globalObject);

	//JSScript *script = JS_CompileFile(cx, JS_GetGlobalObject(cx), argv[1]);
	printf("Opening source file %s\n", argv[1]);
	FILE *srcFile = fopen(argv[1], "r");
	if ( !srcFile ) {
	
		printf("Unable to open %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	fseek(srcFile, 0, SEEK_END);
	int fileSize = ftell(srcFile);
	rewind(srcFile);

	printf("Source script size: %d\n", fileSize );

	bool allowEmptyDest = argc >= 3 && strcmp(argv[3], "true") == 0;
	printf("Empty source give empty destination: %s\n", allowEmptyDest ? "true" : "false");

	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_COMPILE_N_GO | JSOPTION_VAROBJFIX | JSOPTION_XML | /*JSOPTION_JIT |*/ JSOPTION_STRICT );

	printf("Script name: %s\n", argc >= 4 ? argv[4] : "(NULL)" );
	printf("Compiling file %s\n", argv[1]);
	JSScript *script = JS_CompileFileHandle(cx, globalObject, argc >= 4 ? argv[4] : NULL, srcFile);
	fclose(srcFile);
	if ( !script ) {

		printf( "Unable to compile %s\n", argv[1]);
		
		return EXIT_FAILURE;
	}

	printf("Opening destination file %s\n", argv[2]);
	int file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_SEQUENTIAL, 00644);
	if ( file <= 0 ) {

		printf("Unable to create %s\n", argv[2]);
		return EXIT_FAILURE;
	}

	if ( fileSize > 0 || !allowEmptyDest ) {

		JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
		JS_XDRScript(xdr, &script);
		uint32 length;
		void *buf = JS_XDRMemGetData(xdr, &length);
		write(file, buf, length);
		printf("Writing %d bytes.\n", length);
		JS_XDRDestroy(xdr);
	} else {

		printf("Empty destination file.\n");
	}
	close(file);

	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

	printf("Done.\n");
	return EXIT_SUCCESS;
}