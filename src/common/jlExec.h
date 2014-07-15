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

JL_BEGIN_NAMESPACE


INLINE NEVER_INLINE JSScript* FASTCALL
compileScript( JSContext *cx, JS::HandleObject obj, const void *scriptBuffer, size_t scriptBufferSize, jl::EncodingType encoding, JS::CompileOptions &compileOptions ) {

	//compileOptions.setSourcePolicy(JS::CompileOptions::NO_SOURCE);
	
	// doc.
	//   JSOPTION_COMPILE_N_GO: caller of JS_Compile*Script promises to execute compiled script once only; enables compile-time scope chain resolution of consts.
	// see https://bugzilla.mozilla.org/show_bug.cgi?id=494363
	// see also bug 920322 : Add support for compileAndGo optimizations to XDRScript
	//if ( saveCompFile ) // saving the compiled file mean that we cannot promise to execute compiled script once only.
	//	compileOptions.setCompileAndGo(false);  // JS_SetOptions(cx, prevOpts & ~JSOPTION_COMPILE_N_GO); // previous options a restored below.
	compileOptions.setCompileAndGo( true );

	if ( compileOptions.filename() == nullptr )
		compileOptions.setFileAndLine( "<inline>", 1 );

	size_t hdrSize = 0;
	if ( encoding == jl::ENC_UNKNOWN ) {

		encoding = jl::DetectEncoding( (uint8_t*)scriptBuffer, scriptBufferSize, &hdrSize );
		if ( encoding == jl::ENC_UNKNOWN )
			encoding = jl::ENC_LATIN1;
	}

	switch ( encoding ) {
		case jl::ENC_UTF8:
			compileOptions.setUTF8( true );
		case jl::ENC_LATIN1: {

			char *scriptChars = reinterpret_cast<char *>((char*)scriptBuffer + hdrSize);
			size_t scriptCharsLength = scriptBufferSize - hdrSize;

			if ( scriptCharsLength >= 2 && scriptChars[0] == '#' && scriptChars[1] == '!' ) { // shebang support

				scriptChars[0] = '/';
				scriptChars[1] = '/';
			}
			JS::RootedScript script( cx, JS::Compile( cx, obj, compileOptions, scriptChars, scriptCharsLength ) );
			return script;
		}
		// (TBD) support big-endian
		case jl::ENC_UTF16le: {

			jschar *scriptChars = reinterpret_cast<jschar *>((char*)scriptBuffer + hdrSize);
			size_t scriptCharsLength = (scriptBufferSize - hdrSize) / 2;

			if ( scriptCharsLength >= 2 && scriptChars[0] == L( '#' ) && scriptChars[1] == L( '!' ) ) { // shebang support

				scriptChars[0] = L( '/' );
				scriptChars[1] = L( '/' );
			}
			JS::RootedScript script( cx, JS::Compile( cx, obj, compileOptions, scriptChars, scriptCharsLength ) );
			return script;
		}
	}

	return nullptr;
}




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
loadScript(JSContext *cx, IN JS::HandleObject obj, const TCHAR *fileName, jl::EncodingType encoding, bool useCompFile, bool saveCompFile) {

	JS::RootedScript script( cx );
	JS::CompileOptions compileOptions( cx );

	void *scriptBuffer = NULL;
	size_t scriptBufferSize;

	void *data = NULL;
	TCHAR compiledFileName[PATH_MAX];
	jl::strcpy_s<PATH_MAX>( compiledFileName, fileName );
	jl::strcpy_s<PATH_MAX>( compiledFileName, TEXT( "xdr" ) );

	struct _stat srcFileStat, compFileStat;
	bool hasSrcFile = jl::stat( fileName, &srcFileStat ) != -1; // errno == ENOENT
	bool hasCompFile = useCompFile && jl::stat( compiledFileName, &compFileStat ) != -1; // if not using compiled file, this is useless to compile it
	bool compFileUpToDate = ( hasCompFile && !hasSrcFile ) || ( hasCompFile && hasSrcFile && (compFileStat.st_mtime > srcFileStat.st_mtime) ); // true if comp file is up to date or alone

	JL_CHKM( hasSrcFile || hasCompFile, E_SCRIPT, E_NAME( fileName ), E_OR, E_NAME( compiledFileName ), E_NOTFOUND );

	if ( useCompFile && compFileUpToDate ) {

		int file = jl::open(compiledFileName, O_RDONLY | O_BINARY | O_SEQUENTIAL);
		JL_CHKM( file != -1, E_FILE, E_NAME(compiledFileName), E_ACCESS ); // "Unable to open file \"%s\" for reading.", compiledFileName
		size_t compFileSize;
		compFileSize = compFileStat.st_size; // filelength(file); ?
		data = jl_malloca(compFileSize);
		JL_ASSERT_ALLOC( data );
		// jl::isInBounds<unsigned int>(
		int readCount = read(file, data, compFileSize); // here we can use "Memory-Mapped I/O Functions" ( http://developer.mozilla.org/en/docs/NSPR_API_Reference:I/O_Functions#Memory-Mapped_I.2FO_Functions )
		JL_CHKM( readCount >= 0 && (size_t)readCount == compFileSize, E_FILE, E_NAME(compiledFileName), E_READ ); // "Unable to read the file \"%s\" ", compiledFileName
		close( file );

		script.set( JS_DecodeScript( cx, data, readCount, NULL ) );

		jl_freea( data );
		data = NULL;

		if ( script )
			goto good;

		JS_ClearPendingException(cx);
	}

	if ( !hasSrcFile )
		goto bad; // no source, no compiled version of the source, die.

	int scriptFile;
	scriptFile = jl::open( fileName, O_RDONLY | O_BINARY | O_SEQUENTIAL );

	JL_CHKM( scriptFile >= 0, E_FILE, E_NAME(fileName), E_ACCESS ); // "Unable to open file \"%s\" for reading.", fileName

	scriptBufferSize = lseek(scriptFile, 0, SEEK_END);
	ASSERT( scriptBufferSize <= UINT_MAX ); // Compiled file too big.

	lseek(scriptFile, 0, SEEK_SET); // see tell(scriptFile);
	scriptBuffer = jl_malloca(scriptBufferSize);
	
	int res;
	res = read(scriptFile, scriptBuffer, (unsigned int)scriptBufferSize);
	close(scriptFile);

	//JL_CHKM( res >= 0, "Unable to read file \"%s\".", fileName );
	JL_CHKM( res >= 0, E_FILE, E_NAME(fileName), E_READ );

	ASSERT( (size_t)res == scriptBufferSize );
	scriptBufferSize = (size_t)res;

	{
		jl::BufString fnUTF8 = UTF16LEToUTF8(jl::BufString(fileName));
		compileOptions.setFileAndLine(fnUTF8, 1);
		script.set( compileScript( cx, obj, scriptBuffer, scriptBufferSize, encoding, compileOptions ) );
		JL_CHK( script );
	}

	if ( !saveCompFile )
		goto good;

	int file;
	file = jl::open(compiledFileName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_SEQUENTIAL, srcFileStat.st_mode);
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
	return script;

bad:
	if ( scriptBuffer )
		jl_freea( scriptBuffer );
	if ( data )
		jl_freea( data );
	return NULL; // report a warning ?
}


ALWAYS_INLINE bool FASTCALL
executeScriptText( JSContext *cx, IN JS::HandleObject obj, const void *scriptText, size_t scriptSize, jl::EncodingType encoding, bool compileOnly, OUT JS::MutableHandleValue rval ) {

	JS::AutoSaveContextOptions autoCxOpts(cx);
	JS::CompileOptions compileOptions(cx);

	JS::RootedScript script( cx, compileScript( cx, obj, scriptText, scriptSize, encoding, compileOptions ) );
	JL_CHK( script );

	// mendatory else the exception is converted into an error before JL_IsExceptionPending can be used. Exceptions can be reported with JS_ReportPendingException().
	JS::ContextOptionsRef(cx).setDontReportUncaught(true);

	if ( !compileOnly )
		JL_CHK( JS_ExecuteScript(cx, obj, script, rval) ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	else
		rval.setUndefined();

	return true;
	JL_BAD;
}


ALWAYS_INLINE bool FASTCALL
executeScriptFileName( JSContext *cx, IN JS::HandleObject obj, const TCHAR *scriptFileName, jl::EncodingType encoding, bool compileOnly, OUT JS::MutableHandleValue rval ) {

	JS::AutoSaveContextOptions autoCxOpts(cx);

	JS::RootedScript script( cx, loadScript( cx, obj, scriptFileName, encoding, true, false ) ); // use xdr if available, but don't save it.
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
