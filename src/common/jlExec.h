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

#pragma once

#include <sys/stat.h> // stat() used by JL_LoadScript()

JL_BEGIN_NAMESPACE


// XDR and bytecode compatibility:
//   Backward compatibility is when you run old bytecode on a new engine, and that should work.
//   What you seem to want is forward compatibility, which is new bytecode on an old engine, which is nothing we've ever promised.
// year 2038 bug :
//   Later than midnight, January 1, 1970, and before 19:14:07 January 18, 2038, UTC ( see _stat64 )
// note:
//	You really want to use Script.prototype.thaw and Script.prototype.freeze.  At
//	least imitate their implementations in jsscript.c (script_thaw and
//	script_freeze).  But you might do better to call these via JS_CallFunctionName
//	on your script object.
//
//	/be
INLINE NEVER_INLINE JSScript* FASTCALL
loadScript(JSContext *cx, IN JS::HandleObject obj, const char *fileName, jl::EncodingType encoding, bool useCompFile, bool saveCompFile) {

	char *scriptBuffer = NULL;
	size_t scriptFileSize;
	jschar *scriptText = NULL;
	size_t scriptTextLength;

	JS::RootedScript script( cx );
	JS::CompileOptions compileOptions( cx );

	//JS::CompartmentOptionsRef(cx).cloneSingletonsOverride().set(true);

	void *data = NULL;
	char compiledFileName[PATH_MAX];
	strcpy_s<PATH_MAX>( compiledFileName, fileName );
	strcat_s<PATH_MAX>( compiledFileName, "xdr" );

	struct stat srcFileStat, compFileStat;
	//OutputDebugString( fileName );
	bool hasSrcFile = stat( fileName, &srcFileStat ) != -1; // errno == ENOENT
	bool hasCompFile = useCompFile && stat( compiledFileName, &compFileStat ) != -1; // if not using compiled file, this is useless to compile it
	bool compFileUpToDate = ( hasCompFile && !hasSrcFile ) || ( hasCompFile && hasSrcFile && (compFileStat.st_mtime > srcFileStat.st_mtime) ); // true if comp file is up to date or alone

	JL_CHKM( hasSrcFile || hasCompFile, E_SCRIPT, E_NAME( fileName ), E_OR, E_NAME( compiledFileName ), E_NOTFOUND );

	if ( useCompFile && compFileUpToDate ) {

		int file = open(compiledFileName, O_RDONLY | O_BINARY | O_SEQUENTIAL);
		JL_CHKM( file != -1, E_FILE, E_NAME(compiledFileName), E_ACCESS ); // "Unable to open file \"%s\" for reading.", compiledFileName
		size_t compFileSize;
		compFileSize = compFileStat.st_size; // filelength(file); ?
		data = jl_malloca(compFileSize);
		JL_ASSERT_ALLOC( data );
		// jl::isInBounds<unsigned int>(
		int readCount = read(file, data, compFileSize); // here we can use "Memory-Mapped I/O Functions" ( http://developer.mozilla.org/en/docs/NSPR_API_Reference:I/O_Functions#Memory-Mapped_I.2FO_Functions )
		JL_CHKM( readCount >= 0 && (size_t)readCount == compFileSize, E_FILE, E_NAME(compiledFileName), E_READ ); // "Unable to read the file \"%s\" ", compiledFileName
		close( file );

		// we want silent failures.
//		JSErrorReporter prevErrorReporter = JS_SetErrorReporter(cx, NULL);
//		JSDebugErrorHook debugErrorHook = cx->debugHooks->debugErrorHook;
//		void *debugErrorHookData = cx->debugHooks->debugErrorHookData;
//		JS_SetDebugErrorHook(JL_GetRuntime(cx), NULL, NULL);

		script = JS_DecodeScript(cx, data, readCount, NULL);

//		JS_SetDebugErrorHook(JL_GetRuntime(cx), debugErrorHook, debugErrorHookData);
//		if (cx->lastMessage)
//			JS_free(cx, cx->lastMessage);
//		cx->lastMessage = NULL;
//		JS_SetErrorReporter(cx, prevErrorReporter);

		if ( script ) {

			// (TBD) manage BIG_ENDIAN here ?
			jl_freea(data);
			data = NULL;

//			JL_ASSERT_WARN( JS_GetScriptVersion(cx, script) >= JS_GetVersion(cx), E_NAME(compiledFileName), E_STR("XDR"), E_VERSION );
			goto good;
		} else {

			JS_ClearPendingException(cx);

//			JL_REPORT_WARNING_NUM( JLSMSG_RUNTIME_ERROR, "bad script XDR magic number");

//			if ( JL_IsExceptionPending(cx) )
//				JS_ReportPendingException(cx);

			jl_freea(data);
			data = NULL;
//			if ( JL_IsExceptionPending(cx) )
//				JS_ClearPendingException(cx);
		}
	}

	if ( !hasSrcFile )
		goto bad; // no source, no compiled version of the source, die.

	// doc.
	//   JSOPTION_COMPILE_N_GO: caller of JS_Compile*Script promises to execute compiled script once only; enables compile-time scope chain resolution of consts.
	// see https://bugzilla.mozilla.org/show_bug.cgi?id=494363
	// see also bug 920322 : Add support for compileAndGo optimizations to XDRScript
	//if ( saveCompFile ) // saving the compiled file mean that we cannot promise to execute compiled script once only.
	//	compileOptions.setCompileAndGo(false);  // JS_SetOptions(cx, prevOpts & ~JSOPTION_COMPILE_N_GO); // previous options a restored below.

#define JL_UC
#ifndef JL_UC

	FILE *scriptFile;
	scriptFile = fopen(fileName, "r");
	JL_ASSERT( scriptFile != NULL, E_FILE, E_NAME(scriptFile), E_ACCESS ); // "Script file \"%s\" cannot be opened."

	// shebang support
	char s, b;
	s = getc(scriptFile);
	if ( s == '#' ) {

		b = getc(scriptFile);
		if ( b == '!' ) {

			ungetc('/', scriptFile);
			ungetc('/', scriptFile);
		} else {

			ungetc(b, scriptFile);
			ungetc(s, scriptFile);
		}
	} else {

		ungetc(s, scriptFile);
	}

	script = JS_CompileFileHandle(cx, obj, fileName, scriptFile);
	fclose(scriptFile);

#else //JL_UC


	int scriptFile;
	scriptFile = open( fileName, O_RDONLY | O_BINARY | O_SEQUENTIAL );

	JL_CHKM( scriptFile >= 0, E_FILE, E_NAME(fileName), E_ACCESS ); // "Unable to open file \"%s\" for reading.", fileName

	scriptFileSize = lseek(scriptFile, 0, SEEK_END);
	ASSERT( scriptFileSize <= UINT_MAX ); // Compiled file too big.

	lseek(scriptFile, 0, SEEK_SET); // see tell(scriptFile);
	scriptBuffer = (char*)jl_malloca(scriptFileSize);
	int res;
	res = read(scriptFile, (void*)scriptBuffer, (unsigned int)scriptFileSize);
	close(scriptFile);

	//JL_CHKM( res >= 0, "Unable to read file \"%s\".", fileName );
	JL_CHKM( res >= 0, E_FILE, E_NAME(fileName), E_READ );


	ASSERT( (size_t)res == scriptFileSize );
	scriptFileSize = (size_t)res;

	if ( encoding == jl::ENC_UNKNOWN )
		encoding = jl::DetectEncoding((uint8_t**)&scriptBuffer, &scriptFileSize);

	switch ( encoding ) {
		default:
			JL_WARN( E_SCRIPT, E_ENCODING, E_INVALID, E_NAME(fileName) );
			// then use ASCII as default.
		case jl::ENC_ASCII: {

			char *scriptText = scriptBuffer;
			size_t scriptTextLength = scriptFileSize;
			if ( scriptTextLength >= 2 && scriptText[0] == '#' && scriptText[1] == '!' ) { // shebang support

				scriptText[0] = '/';
				scriptText[1] = '/';
			}
			compileOptions.setFileAndLine(fileName, 1);
			compileOptions.setCompileAndGo(true);
			//compileOptions.setSourcePolicy(JS::CompileOptions::NO_SOURCE);

			//printf("DEBUG %p", obj.address());

			script = JS_CompileScript(cx, obj, scriptText, scriptTextLength, compileOptions);
			break;
		}
		case jl::ENC_UTF16le: { // (TBD) support big-endian

			jschar *scriptText = (jschar*)scriptBuffer;
			size_t scriptTextLength = scriptFileSize / 2;
			if ( scriptTextLength >= 2 && scriptText[0] == L('#') && scriptText[1] == L('!') ) { // shebang support

				scriptText[0] = L('/');
				scriptText[1] = L('/');
			}
			compileOptions.setFileAndLine(fileName, 1);
			compileOptions.setCompileAndGo(true);
			//compileOptions.setSourcePolicy(JS::CompileOptions::NO_SOURCE);

			//script = JS_CompileUCScript(cx, obj, scriptText, scriptTextLength, compileOptions);
			script = JS::Compile(cx, obj, compileOptions, scriptText, scriptTextLength);
			break;
		}
		case jl::ENC_UTF8: {

			scriptText = (jschar*)jl_malloca(scriptFileSize * 2);
			scriptTextLength = scriptFileSize * 2;
			JL_CHKM( jl::UTF8ToUTF16LE((unsigned char*)scriptText, &scriptTextLength, (unsigned char*)scriptBuffer, &scriptFileSize) >= 0, E_SCRIPT, E_ENCODING, E_INVALID, E_COMMENT("UTF8") ); // "Unable do decode UTF8 data."

			if ( scriptTextLength >= 2 && scriptText[0] == L('#') && scriptText[1] == L('!') ) { // shebang support

				scriptText[0] = L('/');
				scriptText[1] = L('/');
			}
			compileOptions.setFileAndLine(fileName, 1);
			compileOptions.setCompileAndGo(true);
			//compileOptions.setSourcePolicy(JS::CompileOptions::NO_SOURCE);

			script = JS_CompileUCScript(cx, obj, scriptText, scriptTextLength, compileOptions);
			break;
		}
	}

	//JL_CHKM( script, E_SCRIPT, E_NAME(fileName), E_COMPILE ); // do not overwrite the default exception.
	JL_CHK( script );

#endif //JL_UC

	if ( !saveCompFile )
		goto good;

	int file;
	file = open(compiledFileName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_SEQUENTIAL, srcFileStat.st_mode);
	if ( file == -1 ) // if the file cannot be write, this is not an error ( eg. read-only drive )
		goto good;

	uint32_t length;
	void *buf;
	buf = JS_EncodeScript(cx, script, &length);
	JL_CHK( buf );

	// manage BIG_ENDIAN here ?
	JL_CHK( write(file, buf, length) != -1 ); // On error, -1 is returned, and errno is set appropriately.
	JL_CHK( close(file) == 0 );
	js_free(buf);
	goto good;

good:

	if ( scriptBuffer )
		jl_freea(scriptBuffer);

	if ( scriptText )
		jl_freea(scriptText);

//	JS_SetOptions(cx, prevOpts);
	return script;

bad:

	jl_freea(scriptBuffer); // jl_freea(NULL) is legal
	jl_freea(scriptText);
//	JS_SetOptions(cx, prevOpts);
	jl_freea(data); // jl_free(NULL) is legal
	return NULL; // report a warning ?
}


ALWAYS_INLINE bool FASTCALL
executeScriptText( JSContext *cx, IN JS::HandleObject obj, const char *scriptText, bool compileOnly, OUT JS::MutableHandleValue rval ) {

//	uint32_t prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) /*| JSOPTION_COMPILE_N_GO*/); //  | JSOPTION_DONT_REPORT_UNCAUGHT
	// JSOPTION_COMPILE_N_GO:
	//  caller of JS_Compile*Script promises to execute compiled script once only; enables compile-time scope chain resolution of consts.
	// JSOPTION_DONT_REPORT_UNCAUGHT:
	//  When returning from the outermost API call, prevent uncaught exceptions from being converted to error reports
	//  we can use JS_ReportPendingException to report it manually

	JS::AutoSaveContextOptions autoCxOpts(cx);

// compile & executes the script

	//JSPrincipals *principals = (JSPrincipals*)jl_malloc(sizeof(JSPrincipals));
	//JSPrincipals tmp = {0};
	//*principals = tmp;
	//principals->codebase = (char*)jl_malloc(PATH_MAX);
	//strncpy(principals->codebase, scriptFileName, PATH_MAX-1);
	//principals->refcount = 1;
	//principals->destroy = HostPrincipalsDestroy;

	JS::RootedScript script(cx);
	JS::CompileOptions compileOptions(cx);
	compileOptions.setFileAndLine("inline", 1);
	compileOptions.setCompileAndGo(true);
	//compileOptions.setSourcePolicy(JS::CompileOptions::NO_SOURCE);

	script = JS_CompileScript(cx, obj, scriptText, strlen(scriptText), compileOptions);
	JL_CHK( script );

	// mendatory else the exception is converted into an error before JL_IsExceptionPending can be used. Exceptions can be reported with JS_ReportPendingException().
	JS::ContextOptionsRef(cx).setDontReportUncaught(true);

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.
	if ( !compileOnly )
		JL_CHK( JS_ExecuteScript(cx, obj, script, rval) ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	else
		rval.setUndefined();

	return true;
	JL_BAD;
}


ALWAYS_INLINE bool FASTCALL
executeScriptFileName( JSContext *cx, IN JS::HandleObject obj, const char *scriptFileName, bool compileOnly, OUT JS::MutableHandleValue rval ) {

	JS::AutoSaveContextOptions autoCxOpts(cx);

	JS::RootedScript script(cx, loadScript(cx, obj, scriptFileName, jl::ENC_UNKNOWN, true, false)); // use xdr if available, but don't save it.
	JL_CHK( script );


	// mendatory else the exception is converted into an error before JL_IsExceptionPending can be used. Exceptions can be reported with JS_ReportPendingException().
	JS::ContextOptionsRef(cx).setDontReportUncaught(true);

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.
	if ( !compileOnly )
		JL_CHK( JS_ExecuteScript(cx, obj, script, rval) ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	else
		rval.setUndefined();

	return true;
	JL_BAD;
}

JL_END_NAMESPACE
