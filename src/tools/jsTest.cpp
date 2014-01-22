#define XP_WIN

#include <wchar.h>

//#define USE_JL

#ifdef USE_JL

#include <../common/jlplatform.h>
#include <../common/jlhelper.h>
#include <../common/jlhelper.cpp>
#include <../common/jslibsModule.cpp>

#else
#endif // USE_JL

#include <js/RequiredDefines.h>

#include <jsapi.h>
#include <string.h>
#include <jsprf.h>


#pragma warning(disable : 4100) // unreferenced formal parameter


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




bool
PrintError(JSContext *cx, FILE *file, const char *message, JSErrorReport *report,
               bool reportWarnings)
{
    if (!report) {
        fprintf(file, "%s\n", message);
        fflush(file);
        return false;
    }

    /* Conditionally ignore reported warnings. */
    if (JSREPORT_IS_WARNING(report->flags) && !reportWarnings)
        return false;

    char *prefix = nullptr;
    if (report->filename)
        prefix = JS_smprintf("%s:", report->filename);
    if (report->lineno) {
        char *tmp = prefix;
        prefix = JS_smprintf("%s%u:%u ", tmp ? tmp : "", report->lineno, report->column);
        JS_free(cx, tmp);
    }
    if (JSREPORT_IS_WARNING(report->flags)) {
        char *tmp = prefix;
        prefix = JS_smprintf("%s%swarning: ",
                             tmp ? tmp : "",
                             JSREPORT_IS_STRICT(report->flags) ? "strict " : "");
        JS_free(cx, tmp);
    }

    /* embedded newlines -- argh! */
    const char *ctmp;
    while ((ctmp = strchr(message, '\n')) != 0) {
        ctmp++;
        if (prefix)
            fputs(prefix, file);
        fwrite(message, 1, ctmp - message, file);
        message = ctmp;
    }

    /* If there were no filename or lineno, the prefix might be empty */
    if (prefix)
        fputs(prefix, file);
    fputs(message, file);

    if (report->linebuf) {
        /* report->linebuf usually ends with a newline. */
        int n = strlen(report->linebuf);
        fprintf(file, ":\n%s%s%s%s",
                prefix,
                report->linebuf,
                (n > 0 && report->linebuf[n-1] == '\n') ? "" : "\n",
                prefix);
        n = report->tokenptr - report->linebuf;
        for (int i = 0, j = 0; i < n; i++) {
            if (report->linebuf[i] == '\t') {
                for (int k = (j + 8) & ~7; j < k; j++) {
                    fputc('.', file);
                }
                continue;
            }
            fputc('.', file);
            j++;
        }
        fputc('^', file);
    }
    fputc('\n', file);
    fflush(file);
    JS_free(cx, prefix);
    return true;
}

static bool gGotError = false;
static int gExitCode = 0;

enum JSShellErrNum {
#define MSG_DEF(name, number, count, exception, format) \
    name = number,
#include "js.msg"
#undef MSG_DEF
    JSShellErr_Limit
};

static void
my_ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report)
{
    gGotError = PrintError(cx, gErrFile, message, report, reportWarnings);
    if (!JSREPORT_IS_WARNING(report->flags)) {
        if (report->errorNumber == JSMSG_OUT_OF_MEMORY) {
            gExitCode = 5;
        } else {
            gExitCode = 3;
        }
    }
}



JSClass global_class = {
	"global", JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub
};

/*
bool Print(JSContext *cx, unsigned argc, jsval *vp) {

	const JS::CallArgs args(JS::CallArgsFromVp(argc, vp));
		
	JS::RootedString str(cx, JS::ToString(cx, args[2]));

	const jschar *s = JS_GetStringCharsZ(cx, str);
	_putws((const wchar_t *)s);

	return true;
}
*/

/*

int main_bz726429(int argc, char* argv[]) {

JSRuntime *rt = JS_NewRuntime(0, JS_NO_HELPER_THREADS);
JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
JSContext *cx = JS_NewContext(rt, 8192L);
JSObject *globalObject = JS_NewGlobalObject(cx, &global_class, NULL);
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
static bool
sandbox_resolve(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, unsigned flags, JS::MutableHandleObject objp) {

	bool resolved;
	if ( (flags & JSRESOLVE_ASSIGNING) == 0 ) {

		if ( !JS_ResolveStandardClass(cx, obj, id, &resolved) )
			return false;

		if ( !resolved && JSID_IS_STRING(id) ) {
			jsval v;
			JS_IdToValue(cx, id, &v);
			if ( !wcscmp(JS_GetStringCharsZ(cx, JSVAL_TO_STRING(v)), L("Debugger") ) ) {

				if ( !JS_DefineDebuggerObject(cx, obj) ) // doc: https://developer.mozilla.org/en/SpiderMonkey/JS_Debugger_API_Guide
					return false;
				resolved = true;
			}
		}

		if ( resolved ) {

			objp.set(obj);
			return true;
		}
	}
	objp.set(NULL);
	return true;
}


static JSClass sandbox_class = {
    "Sandbox",
    JSCLASS_NEW_RESOLVE | JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub,   JS_DeletePropertyStub,
    JS_PropertyStub,   JS_StrictPropertyStub,
    JS_EnumerateStub, (JSResolveOp)sandbox_resolve,
    JS_ConvertStub,    NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};


bool Sandbox(JSContext *cx, unsigned argc, jsval *vp) {

	JSObject *obj = JS_NewGlobalObject(cx, &sandbox_class, NULL);
    JL_CHK( JS_WrapObject(cx, &obj) );
	JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
	return true;
bad:
	return false;
}




int main_test_Debugger(int argc, char* argv[]) {

	_unsafeMode = false;

    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L, JS_NO_HELPER_THREADS);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class, NULL);
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

	JS_SetDebugMode(cx, true);

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

	JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L, JS_NO_HELPER_THREADS);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);

	jsval rval;
	char *scriptText = "(new SyntaxError())";

	JSScript *script = JS_CompileScript(cx, globalObject, scriptText, strlen(scriptText), "<inline>", 1);
	JL_CHK( JS_ExecuteScript(cx, globalObject, script, &rval) );

	//jsval constructor;
	//JL_CHK( JS_GetProperty(cx, JL_GetGlobal(cx), "SyntaxError", &constructor) );

	//JSObject *err = JS_NewObjectForConstructor(cx, &constructor);
	//
	//
	////bool r = JL_ObjectIsError(cx, JSVAL_TO_OBJECT(rval));

	//JL_ObjectIsError(cx, err);


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

	JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L, JS_NO_HELPER_THREADS);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);


	const jschar *str = L("Hello World");
	int strlen = wcslen(str);

	JSObject *ab = JS_NewArrayBuffer(cx, (strlen+1)*2);

	JS_AddObjectRoot(cx, &ab);

	char *data = (char*)JS_GetArrayBufferData(ab);

	wcscpy((jschar*)data, str);

	JSString *jsstr = JS_NewExternalString(cx, (jschar*)data, strlen-1, &finalizer1);
	

	((jschar*)data)[0] = L('X');

	const jschar *tmp = JS_GetStringCharsZ(cx, jsstr); //JS_GetStringCharsZAndLength(cx, jsstr, &l);
	
	((jschar*)data)[0] = L('Y');

	JL_IGNORE(tmp);




	return EXIT_SUCCESS;
bad:
	printf("BAD\n");
	return EXIT_FAILURE;
}




int main_arraylike(int argc, char* argv[]) {

	_unsafeMode = false;

	JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L, JS_NO_HELPER_THREADS);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);


	JSObject *o = JS_NewObject(cx, NULL, NULL, NULL);

	unsigned len;
	bool err = JS_GetArrayLength(cx, o, &len);

	JSString *s = JS_NewStringCopyZ(cx, (const char *)L("hello"));

	bool tmp = JSVAL_IS_PRIMITIVE(STRING_TO_JSVAL(s));

	JL_IGNORE(err, tmp);


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

    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L, JS_NO_HELPER_THREADS);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class, NULL);
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

	JL_IGNORE(tmp);

	//printf("%d", x);
    return 0;
}

int main_NewObjectWithGivenProto_NewObject(int argc, char* argv[]) {


    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L, JS_NO_HELPER_THREADS);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);

	

//parent = GetCurrentGlobal(cx);
//
//    JSObject *scopeChain = (cx->hasfp()) ? &cx->fp()->scopeChain() : cx->globalObject;
    return scopeChain ? &scopeChain->global() : NULL;
//

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


//	return JSVAL_IS_DOUBLE(val);
//004014E9  mov         eax,dword ptr [val] 
//004014EC  mov         ecx,dword ptr [eax] 
//004014EE  cmp         dword ptr [eax+4],0FFFFFF80h 
//004014F2  mov         dword ptr [esp],ecx 
//004014F5  setbe       al   
//
//
//	return val.isDouble();
//004014E3  mov         eax,dword ptr [esp+0Ch] 
//004014E7  mov         ecx,dword ptr [eax] 
//004014E9  cmp         dword ptr [eax+4],0FFFFFF80h 
//004014ED  mov         dword ptr [esp],ecx 
//004014F0  setbe       al  

	return !!i;
}


int main_PerfTest(int argc, char* argv[]) {

    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L, JS_NO_HELPER_THREADS);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32_t)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32_t)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetErrorReporter(cx, ErrorReporter);
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_TYPE_INFERENCE );

	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);


	jsval v;

	bool b = NOIL(test_perf)(v);
	JL_IGNORE(b);
	
	//class JSString * JS_ValueToString(struct JSContext *,class JS::Value) => ?JS_ValueToString@@YAPAVJSString@@PAUJSContext@@VValue@JS@@@Z
	//JS_ValueToString

	return 0;
}


int main_bad(int argc, char* argv[]) {


	JLAutoBuffer<char> ptr(10);

	if ( argc > 0 )
		return 1;

	JLData str;

	printf("%d%p", str.Length(), ptr);



	return 0;
bad:
	return 1;
}

////////////////////////////////////////////////////////////////////

JLData JLData_test_create( int step ) {

	switch ( step ) {

		case 0:
			return JLData("test", true);
		case 1:
			return JLData("test", false, 4);
		case 2: {
			char *buf = (char*)malloc(10);
			strcpy(buf, "test");
			return JLData(buf, false, 4);
		}
		case 3: {
			char *buf = (char*)malloc(10);
			strcpy(buf, "test");
			return JLData(buf, false, 4);
		}
		case 4: {
			char *buf = (char*)malloc(10);
			strcpy(buf, "test");
			return JLData(buf, true, 4);
		}
		case 5:
			return JLData(L("test"), true);
		case 6:
			return JLData(L("test"), false, 4);
		case 7: {
			jschar *buf = (jschar*)malloc(10);
			wcscpy(buf, L("test"));
			return JLData(buf, true);
		}
		case 8: {
			jschar *buf = (jschar*)malloc(10);
			wcscpy(buf, L("test"));
			return JLData(buf, false, 4);
		}
		case 9: {
			void *buf = malloc(10);
			return JLData(buf, 10);
		}
		case 10: {
			return JLData((const void *)"test", 4);
		}
	}
	return JLData::Empty();
}



void JLData_test_test( int step, JLData &data ) {

	switch ( step ) {
		case 0:
			data.GetConstStr();
			return;
		case 1:
			data.GetConstStrZ();
			return;
		case 2:
			data.GetConstWStr();
			return;
		case 3:
			data.GetConstWStrOrNull();
			return;
		case 4:
			data.GetConstWStrZ();
			return;
		case 5:
			data.GetStrOwnership();
			return;
		case 6:
			data.GetWStrZOwnership();
			return;
		case 7:
			data.LengthOrZero();
			return;
		case 8:
			data.GetStrZOwnership();
			return;
		case 9:
			data.GetWStrOwnership();
			return;
	}
}

int main_JLData_test(int argc, char* argv[]) {

	JLData b("xxx", true);
	JLData c;
	c = b;
	JLData d(b);

	JLData e(JLData("", true));


	JLData("test", true).GetConstWStr();

	for ( int i = 0; i <= 20; ++i ) {
		
		for ( int j = 0; j <= 20; ++j ) {

			printf("%3d:%3d ", i, j);
			JLData xxx = JLData_test_create(i);
			JLData_test_test(j, xxx);
			{
				JLData yyy = xxx;
				yyy = yyy;
			}
			ASSERT( !xxx.IsSet() );

			JLData aaa = JLData_test_create(i);
			JLData_test_test(j, aaa);

		}
	}

	return 0;
}



// http://pastebin.mozilla.org/1551010

int main_test_call(int argc, char* argv[]) {

	uint32_t xdrLength;
	void *xdrData;

	{
    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L, JS_NO_HELPER_THREADS);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);

	char *scriptText = "(function() { return 123 })";
	JSScript *script = JS_CompileScript(cx, globalObject, scriptText, strlen(scriptText), "test", 1);

	jsval rval;
	bool ok = JS_ExecuteScript(cx, globalObject, script, &rval);
	ASSERT( ok );

	void *tmp = JS_EncodeInterpretedFunction(cx, JSVAL_TO_OBJECT(rval), &xdrLength);

	xdrData = malloc(xdrLength);
	memcpy(xdrData, tmp, xdrLength);

	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	}

	{
    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L, JS_NO_HELPER_THREADS);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);

	JSObject *fctObj = JS_DecodeInterpretedFunction(cx, xdrData, xdrLength, NULL, NULL);
	
	//JS::AutoObjectRooter tvr(cx, fctObj);
	JS::Rooted<JSObject*> tmpRt(cx, fctObj);

	jsval rval;
	JS_CallFunctionValue(cx, globalObject, OBJECT_TO_JSVAL(fctObj), 0, NULL, &rval); // <- crash here

	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);

	}
	
	JS_ShutDown();

	return EXIT_SUCCESS;
}






class NPropertySet {

	DLLLOCAL static NPropertySet *last;
	NPropertySet *prev_;

public:
	JSNative native_;
	const char *name_;
	unsigned argcMin_;
	unsigned argcMax_;
	NPropertySet(JSNative native, const char *name, unsigned argcMin = 0, unsigned argcMax = -1)
	: native_(native), name_(name), argcMin_(argcMin), argcMax_(argcMax != -1 ? argcMax : 7) {

		prev_ = NPropertySet::last;
		NPropertySet::last = this;
	}

	static bool Register( JSContext *cx, JS::MutableHandleObject obj ) {

		for ( NPropertySet *it = NPropertySet::last; it; it = it->prev_ ) {

			JS_DefineFunction(cx, obj, it->name_, it->native_, it->argcMax_, 0);
		}
		return true;
	}
};

NPropertySet *NPropertySet::last = NULL;


///////////


#include "jlclass2.h"

JL_CLASS( test, parentClass )

	JL_HAS_PRIVATE

	JL_SLOT(foo)
	JL_SLOT(bar)

	JL_CONST( foo, 1234 )


	JL_CONSTRUCTOR() {
	
		return true;
	}

	JL_FUNCTION( fct1, 2 ) {

		//JL_GetReservedSlot(
		_slot_foo.index;

		_const_foo.value;

		return false;
	}

	JL_PROPERTY( status )

		JL_GETTER() {

			return true;
		}


		JL_SETTER() {

			return true;
		}

	JL_PROPERTY_END

	JL_INIT() {

		return true;
	}

	// 	test::_classSpec.Register(cx, &globalObject);
}




#include "jlclass3.h"

JL_CLASS( test ) {

	JL_FUNCTION( fct1, 2 ) {

		return false;
	}


	static bool _fct1(JSContext *cx, unsigned argc, JS::Value *vp);

	jl3::jl_defClass::FunctionItem fct1_(_fct1, "fct1");

	static bool _fct1(JSContext *cx, unsigned argc, JS::Value *vp) {


		return true;
	}
};



int main_test_class2(int argc, char* argv[]) {

	JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L, JS_NO_HELPER_THREADS);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS::RootedObject globalObject(cx, JS_NewGlobalObject(cx, &global_class, NULL));
	JS_InitStandardClasses(cx, globalObject);

	
	test::_classSpec.Register(cx, &globalObject);
	

	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	
	JS_ShutDown();

	return EXIT_SUCCESS;
}

*/	




int main_min(int argc, char* argv[]) {

	JSClass globalClass = {
		"global", JSCLASS_GLOBAL_FLAGS,
		JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
		JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub
	};

	JS_Init();

    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L, JS_NO_HELPER_THREADS);
	JSContext *cx = JS_NewContext(rt, 8192L);
	
	{
	JS::RootedObject globalObject(cx, JS_NewGlobalObject(cx, &globalClass, NULL, JS::FireOnNewGlobalHook));
	JSAutoCompartment ac(cx, globalObject);

	JS::RootedObject test(cx);
	JSObject *ptr = test;

	 //JS_InternUCString(cx, (const wchar_t*)L"sdsadfsadf");
	JS_NewUCStringCopyN(cx, (const wchar_t*)L"sdsadfsadf", 11);


	char *scriptText = "(function() { return 123 })";
	JS::CompileOptions compileOptions(cx);
	JS::RootedScript script(cx, JS_CompileScript(cx, globalObject, scriptText, strlen(scriptText), compileOptions));

	JS::RootedValue rval(cx);
	bool ok = JS_ExecuteScript(cx, globalObject, script, rval.address());
	
	}

	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);

	JS_ShutDown();

	return EXIT_SUCCESS;
}


int main(int argc, char* argv[]) {

	//return main_test_class2(argc, argv);
	//return main_PerfTest(argc, argv);
	//return main_test_call(argc, argv);
	return main_min(argc, argv);
}
