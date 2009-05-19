#ifdef WIN32
	#define _CRT_SECURE_NO_WARNINGS
	#define _CRT_NONSTDC_NO_WARNINGS
	#define XP_WIN
#else
	#define XP_UNIX
#endif

#include <jsapi.h>
#include <jsxdrapi.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef WIN32
	#include <io.h>
#else
	#include <unistd.h>
#endif

#ifndef O_BINARY
	#define O_BINARY 0
#endif

#ifndef O_SEQUENTIAL
	#define O_SEQUENTIAL 0
#endif


int main(int argc, char* argv[]) {

	if ( argc < 2 )
		return EXIT_FAILURE;

	JSRuntime *rt = JS_NewRuntime(8 * 1024 * 1024);
	JSContext *cx = JS_NewContext(rt, 8192);

	printf("Compile file %s\n", argv[1]);
	//JSScript *script = JS_CompileFile(cx, JS_GetGlobalObject(cx), argv[1]);
	FILE *srcFile = fopen(argv[1], "r");

	fseek(srcFile, 0, SEEK_END);
	int fileSize = ftell(srcFile);
	fseek(srcFile, 0, SEEK_SET);

	bool allowEmptyDest = argc >= 3 && strcmp(argv[3], "true") == 0;

	uint32 prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_COMPILE_N_GO);

	JSScript *script = JS_CompileFileHandle(cx, JS_GetGlobalObject(cx), argc >= 4 ? argv[4] : NULL, srcFile);
	fclose(srcFile);
	if ( !script ) {

		printf( "unable to compile %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	printf("Writing file %s\n", argv[2]);
	int file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_SEQUENTIAL, 00644);
	if ( file <= 0 ) {

		printf( "unable to create %s\n", argv[2]);
		return EXIT_FAILURE;
	}

	if ( fileSize > 0 || !allowEmptyDest ) {

		JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
		JS_XDRScript(xdr, &script);
		uint32 length;
		void *buf = JS_XDRMemGetData(xdr, &length);
		write(file, buf, length);
		JS_XDRDestroy(xdr);
	}
	close(file);

	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

	printf("Done.\n");
	return EXIT_SUCCESS;
}
