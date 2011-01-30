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

NEVER_INLINE int test() {




	printf("xxx\n");
	return 6;
}


//#include <intrin.h>

ALWAYS_INLINE uint32_t
my_JLCPUID() {

	// see. http://msdn.microsoft.com/en-us/library/hskdteyh(v=vs.90).aspx
	// and. http://faydoc.tripod.com/cpu/cpuid.htm

	int id = 0;
	int CPUInfo[4] = {-1};

	__cpuid(CPUInfo, 0);
	id ^= CPUInfo[0] ^ CPUInfo[1] ^ CPUInfo[2] ^ CPUInfo[3];

	__cpuid(CPUInfo, 1);
	id ^= CPUInfo[0];
	id ^= CPUInfo[1] & 0x00ffffff; // remove "Initial APIC ID"
	id ^= CPUInfo[2];
	id ^= CPUInfo[3];

	__cpuid(CPUInfo, 0x80000000);
	 if ( 0x80000001 <= CPUInfo[0] ) {

		 __cpuid(CPUInfo, 0x80000001);
		 id ^= CPUInfo[0] ^ CPUInfo[1] ^ CPUInfo[2] ^ CPUInfo[3];
	 }

	 return (uint32_t)id;
}


static __declspec(noinline) void Test( JSContext *cx, JSObject *obj, uintN argc, jsval &v ) {

	float f = rand();
	uint64_t ival = f;

	JLStr str;

	float arr[] = { 9,8,7,6,5,4 };

	v = OBJECT_TO_JSVAL(JS_NewArrayObject(cx, 0, NULL));


	size_t err = JLGetEIP(); size_t a = JLGetEIP(); ////////////////////////////////////////




	float nvec[10];
	jsuint realLen;
	JL_CHK( JL_JsvalToNativeVector(cx, v, nvec, COUNTOF(nvec), &realLen ) );


//	JL_S_ASSERT_INT(v);
	//JL_NativeToJsval(cx, L"ABCDE", 5, &v);
//	JL_CHK( JL_JsvalToNative(cx, v, &str) );
	// JL_CHK( JL_NativeToJsval(cx, ival, &v) );

//	JL_NativeVectorToJsval(cx, arr, 6, &v);

//	JL_JsvalToPrimitive(cx, v, &v);

//	JL_Push(cx, obj, &v);


	bad: ///////////////////////////////////////////////////////////////////////////////////
	a = JLGetEIP() - a - (a-err);

	
	uint32_t id = my_JLCPUID();
	printf("cpuid = %u\n", id);



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
