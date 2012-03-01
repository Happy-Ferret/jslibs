#define XP_WIN

#include <wchar.h>

#include <../common/jlhelper.h>
#include <../common/jlhelper.cpp>
#include <../common/jslibsModule.cpp>

#include <jsapi.h>
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

JSBool Print(JSContext *cx, uintN argc, jsval *vp) {
		
	JSString *str = JS_ValueToString(cx, vp[2]);

	_putws(JS_GetStringCharsZ(cx, str));

	return JS_TRUE;
}


int main_bz726429(int argc, char* argv[]) {

JSRuntime *rt = JS_NewRuntime(0);
JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
JSContext *cx = JS_NewContext(rt, 8192L);
JSObject *globalObject = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
JS_InitStandardClasses(cx, globalObject);

JSString *jsstr = JS_ValueToString(cx, INT_TO_JSVAL(10));
jsval tmp = STRING_TO_JSVAL(jsstr);
JS_SetProperty(cx, globalObject, "rootme", &tmp);
_putws(JS_GetStringCharsZ(cx, jsstr));

JS_DestroyContext(cx);
JS_DestroyRuntime(rt);
JS_ShutDown();

	return EXIT_SUCCESS;
bad:
	printf("BAD\n");
	return EXIT_FAILURE;
}




// source: http://mxr.mozilla.org/mozilla/source/js/src/js.c
static JSBool
sandbox_resolve(JSContext *cx, JSObject *obj, jsid id, uintN flags, JSObject **objp) {

	JSBool resolved;
	if ( (flags & JSRESOLVE_ASSIGNING) == 0 ) {

		if ( !JS_ResolveStandardClass(cx, obj, id, &resolved) )
			return JS_FALSE;

		if ( !resolved && JSID_IS_STRING(id) ) {
			jsval v;
			JS_IdToValue(cx, id, &v);
			if ( !wcscmp(JS_GetStringCharsZ(cx, JSVAL_TO_STRING(v)), L"Debugger" ) ) {

				if ( !JS_DefineDebuggerObject(cx, obj) ) // doc: https://developer.mozilla.org/en/SpiderMonkey/JS_Debugger_API_Guide
					return JS_FALSE;
				resolved = JS_TRUE;
			}
		}

		if ( resolved ) {

			*objp = obj;
			return JS_TRUE;
		}
	}
	*objp = NULL;
	return JS_TRUE;
}


static JSClass sandbox_class = {
    "Sandbox",
    JSCLASS_NEW_RESOLVE | JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub,   JS_PropertyStub,
    JS_PropertyStub,   JS_StrictPropertyStub,
    JS_EnumerateStub, (JSResolveOp)sandbox_resolve,
    JS_ConvertStub,    NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};


JSBool Sandbox(JSContext *cx, uintN argc, jsval *vp) {

	JSObject *obj = JS_NewCompartmentAndGlobalObject(cx, &sandbox_class, NULL);
    JL_CHK( JS_WrapObject(cx, &obj) );
	JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
bad:
	return JS_FALSE;
}




int main_test_Debugger(int argc, char* argv[]) {

	_unsafeMode = false;

    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_RELIMIT | JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);

	jsval rval;

	JL_CHK( JS_DefineFunction(cx, globalObject, "sandbox", Sandbox, 0, 0) );
	JL_CHK( JS_DefineFunction(cx, globalObject, "print", Print, 0, 0) );

	char *scriptText = "\
		var Dbg = sandbox().Debugger; \
		var d = new Dbg(this); \
		d.onNewScript = function(script) { print('url:'+script.url+' / '+script.lineCount)  }; \
		var a = new Function('var a=1;\\nreturn 123+a') \
	";

	JSScript *script = JS_CompileScript(cx, globalObject, scriptText, strlen(scriptText), "<inline>", 1);

	JS_SetDebugMode(cx, JS_TRUE);

	JL_CHK( JS_ExecuteScript(cx, globalObject, script, &rval) );

	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

	return EXIT_SUCCESS;
bad:
	printf("BAD\n");
	return EXIT_FAILURE;
}


#include <jsvalserializer.h>

int main_testconstructor(int argc, char* argv[]) {

	_unsafeMode = false;

    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_RELIMIT | JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);

	jsval rval;
	char *scriptText = "(new SyntaxError())";

	JSScript *script = JS_CompileScript(cx, globalObject, scriptText, strlen(scriptText), "<inline>", 1);
	JL_CHK( JS_ExecuteScript(cx, globalObject, script, &rval) );
/*
	jsval constructor;
	JL_CHK( JS_GetProperty(cx, JL_GetGlobal(cx), "SyntaxError", &constructor) );

	JSObject *err = JS_NewObjectForConstructor(cx, &constructor);
	
	
	//bool r = JL_ObjectIsError(cx, JSVAL_TO_OBJECT(rval));

	JL_ObjectIsError(cx, err);
*/


//	jl::Serializer *ser;
//	ser = jl::JsvalToSerializer(cx, rval);

//	jl::Unserializer *unser;
//	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	//uint32_t gKey;
	//unser->Read(cx, gKey);




	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

	return EXIT_SUCCESS;
bad:
	printf("BAD\n");
	return EXIT_FAILURE;
}


void strFinalize(const JSStringFinalizer *fin, jschar *chars) {
}

static const JSStringFinalizer finalizer1 = { strFinalize };


int main_depstring(int argc, char* argv[]) {

	_unsafeMode = false;

    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_RELIMIT | JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);


	const jschar *str = L"Hello World";
	int strlen = wcslen(str);

	JSObject *ab = JS_NewArrayBuffer(cx, (strlen+1)*2);

	JS_AddObjectRoot(cx, &ab);

	char *data = (char*)JS_GetArrayBufferData(ab);

	wcscpy((jschar*)data, str);

	JSString *jsstr = JS_NewExternalString(cx, (jschar*)data, strlen-1, &finalizer1);
	

	((jschar*)data)[0] = L'X';

	size_t l;
	const jschar *tmp = JS_GetStringCharsZ(cx, jsstr); //JS_GetStringCharsZAndLength(cx, jsstr, &l);
	
	((jschar*)data)[0] = L'Y';





	return EXIT_SUCCESS;
bad:
	printf("BAD\n");
	return EXIT_FAILURE;
}




int main_arraylike(int argc, char* argv[]) {

	_unsafeMode = false;

    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_RELIMIT | JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);


	JSObject *o = JS_NewObject(cx, NULL, NULL, NULL);

	jsuint len;
	JSBool err = JS_GetArrayLength(cx, o, &len);

	JSString *s = JS_NewStringCopyZ(cx, (const char *)L"hello");

	bool tmp = JSVAL_IS_PRIMITIVE(STRING_TO_JSVAL(s));


	return EXIT_SUCCESS;
bad:
	printf("BAD\n");
	return EXIT_FAILURE;
}

/////////////////////////////////////////////////////////

#include <time.h>


int _fastcall func(JSContext *cx, JSObject *obj, int i)
{   
	
    return (int)cx + (int)obj + i;
}

int _stdcall func2(JSContext *cx, JSObject *obj, int i)
{   
    return (int)cx + (int)obj + i;
}

__declspec(noinline) int main_fastcall(int argc, char* argv[]) {

    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_RELIMIT | JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);


	JSObject *o = JS_NewObject(cx, NULL, NULL, NULL);


    int iter = 300;
    int x = 0;
    clock_t t = clock();

	int tmp = 213;

	_asm { int 3 }

    for (int j = 0; j <= iter;j++)
        for (int i = 0; i <= 1000000;i++)
            x = NOIL(func)(cx, o, i);
	
	printf("__fastcall: %d\n", clock() - t);

	t = clock();
    for (int j = 0; j <= iter;j++)
        for (int i = 0; i <= 1000000;i++)
            x = NOIL(func2)(cx, o, i);

	printf("_stdcall: %d\n", clock() - t);

	//printf("%d", x);
    return 0;
}

int main_NewObjectWithGivenProto_NewObject(int argc, char* argv[]) {


    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_RELIMIT | JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);

	
/*
parent = GetCurrentGlobal(cx);

    JSObject *scopeChain = (cx->hasfp()) ? &cx->fp()->scopeChain() : cx->globalObject;
    return scopeChain ? &scopeChain->global() : NULL;
*/
// -> GetGlobalForScopeChain

//	JSObject *o2 = JS_GetParent(JS_NewObject(cx, NULL, NULL, NULL));
//	JSObject *o1 = JS_GetParent(JL_NewObjectWithGivenProto(cx, NULL, NULL, NULL));

	//JSObject *o1 = JS_GetGlobalForScopeChain(cx);
//	JSObject *o2 = JS_GetGlobalObject(cx);
//	JSObject *o3 = JS_GetGlobalForObject(cx, o2);


	return 0;
}

#include <jsfriendapi.h>

bool test_perf(jsval &val) {

	int i = sizeof(js::shadow::Object);

/*
	return JSVAL_IS_DOUBLE(val);
004014E9  mov         eax,dword ptr [val] 
004014EC  mov         ecx,dword ptr [eax] 
004014EE  cmp         dword ptr [eax+4],0FFFFFF80h 
004014F2  mov         dword ptr [esp],ecx 
004014F5  setbe       al   
*/
/*
	return val.isDouble();
004014E3  mov         eax,dword ptr [esp+0Ch] 
004014E7  mov         ecx,dword ptr [eax] 
004014E9  cmp         dword ptr [eax+4],0FFFFFF80h 
004014ED  mov         dword ptr [esp],ecx 
004014F0  setbe       al  
*/
	return i;
}



int main_PerfTest(int argc, char* argv[]) {

    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_RELIMIT | JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);


	jsval v;

	bool b = NOIL(test_perf)(v);
	


	return 0;
}

template <class T>
struct Auto {

	void *_ptr;
	AutoFree(T void*ptr) : _ptr(ptr) {
	}

	operator void *() const {
	
		return _ptr;
	}

	void Forget() {
		_ptr = NULL;
	}

	~AutoFree() {
		
		jl_free(_ptr);
	}
};


int main_bad(int argc, char* argv[]) {


	AutoFree ptr(jl_malloc(123));

	if ( argc > 0 )
		return 1;

	JLStr str;

	printf("%d%p", str.Length(), ptr);



	return 0;
bad:
	return 1;
}


int main(int argc, char* argv[]) {

	//return main_PerfTest(argc, argv);
	return main_bad(argc, argv);
}
