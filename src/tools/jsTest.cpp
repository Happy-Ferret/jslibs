#define XP_WIN

#include "../common/jlhelper.h"
#include "../common/jlhelper.cpp"
#include "../common/jslibsModule.cpp"

#include <jsapi.h>
#include <jsvalue.h>
#include <string.h>


JSClass global_class = {
	 "global", JSCLASS_GLOBAL_FLAGS, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
};


#include <stdlib.h>


static __declspec(noinline) void Test( JSContext *cx, JSObject *obj, uintN argc, jsval &v ) {

	float f = rand();
	uint64_t ival = f;

	JLStr str;

	size_t err = JLGetEIP(); size_t a = JLGetEIP(); ////////////////////////////////////////

	JL_S_ASSERT_INT(v);


	//JL_NativeToJsval(cx, L"ABCDE", 5, &v);

//	JL_CHK( JL_JsvalToNative(cx, v, &str) );

	// JL_CHK( JL_NativeToJsval(cx, ival, &v) );


	bad: ///////////////////////////////////////////////////////////////////////////////////
	a = JLGetEIP() - a - (a-err);


	printf("code length: %d\n", a);

	printf("tmp-%d-%f\n", ival, f);
}


int main(int argc, char* argv[]) {

	_unsafeMode = true;

	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);

	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_JIT);
	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class);
	JS_InitStandardClasses(cx, globalObject);

	jschar str[] = L"ABCD";
	JSString *jsstr = JS_NewUCStringCopyN(cx, str, 4);


	jsval val;
	val = STRING_TO_JSVAL(jsstr);
	val = DOUBLE_TO_JSVAL(1.234);

	Test(cx, globalObject, 0, val);

	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

bad:
	return EXIT_SUCCESS;
}
