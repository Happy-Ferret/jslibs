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


#define FBLOCK_DEF(...) { struct { void operator()( ##__VA_ARGS__ ) {
#define FBLOCK_CALL(...) } } inner; inner( ##__VA_ARGS__ ); }


static __declspec(noinline) void Test( JSContext *cx, JSObject *obj, jsval v ) {

	size_t a = JLGetEIP();

	JL_S_ASSERT_STRING(v);


bad:

	a = JLGetEIP() - a;
	printf("code length: %d\n", a);
}




int main(int argc, char* argv[]) {

int i = 1;

FBLOCK_DEF( int i, int j )
	
	printf("test%d\n", i);

FBLOCK_CALL( i, 5 )



	_unsafeMode = false;

	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);

	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_JIT);
	JSObject *globalObject = JS_NewGlobalObject(cx, &global_class);
	JS_InitStandardClasses(cx, globalObject);

	jschar str[] = L"ABCD";
	JSString *jsstr = JS_NewUCStringCopyN(cx, str, 4);


	jsval val = STRING_TO_JSVAL(jsstr);

	Test(cx, globalObject, val);

	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

bad:
	return EXIT_SUCCESS;
}
