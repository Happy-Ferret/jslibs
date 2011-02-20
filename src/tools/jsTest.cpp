#define XP_WIN

#include <../common/jlhelper.h>
#include <../common/jlhelper.cpp>
#include <../common/jslibsModule.cpp>

#include <jsapi.h>
#include <jsvalue.h>
#include <string.h>


JSClass global_class = {
	 "global", JSCLASS_GLOBAL_FLAGS, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
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

static __declspec(noinline) int GenInt() {
	
	return 2;
}

static __declspec(noinline) void SetBool( bool b ) {

	volatile bool c = b;
}


static __declspec(noinline) void Test( JSContext *cx, JSObject *obj, uintN argc, jsval &val ) {

	float f32 = rand();
	double f64 = f32;

	uint64_t i64 = f32;
	
	volatile int i32 = 1;


	JLStr str;

	float arr[] = { 9,8,7,6,5,4 };

//	val = OBJECT_TO_JSVAL(JS_NewArrayObject(cx, 0, NULL));

	argc = GenInt();

	bool b = f32 > 1;
	size_t st;

	size_t err = JLGetEIP(); size_t a = JLGetEIP(); ////////////////////////////////////////


	b = double(9007199254740992) == double(9007199254740993);

	
//	b = JL_IsData(cx, val);
	/*
	float nvec[10];
	jsuint realLen;
	JL_CHK( JL_JsvalToNativeVector(cx, v, nvec, COUNTOF(nvec), &realLen ) );
*/
//	JL_S_ASSERT_INT(v);
	//JL_NativeToJsval(cx, L("ABCDE"), 5, &v);
//	JL_CHK( JL_JsvalToNative(cx, v, &str) );
	// JL_CHK( JL_NativeToJsval(cx, ival, &v) );
//	JL_NativeVectorToJsval(cx, arr, 6, &v);
//	JL_JsvalToPrimitive(cx, v, &v);
//	JL_Push(cx, obj, &v);

	//b = JL_IsStringObject(cx, obj);
	//b = JL_HasPrivate(cx, obj);

	b = JL_DOUBLE_IS_INTEGER(1000.000);



	bad: ///////////////////////////////////////////////////////////////////////////////////
	a = JLGetEIP() - a - (a-err);

	JL_JsvalToNative(cx, val, &st);



	printf("code length: %d\n", a);

	i32 = st;
	printf("tmp-%d-%f-%i\n", i32, f32, b);
}




int main(int argc, char* argv[]) {


	bool b = double(-9007199254740992) == double(-9007199254740993);
	bool c = double(-9007199254740991) == double(-9007199254740992);

	printf("%d, %d\n", b, c);



	_unsafeMode = false;

	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);

	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_JIT);
	JSObject *globalObject = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);

//	jschar str[] = L("ABCD");
//	JSString *jsstr = JS_NewUCString(cx, str, 2);


	long i = 0;

	_InterlockedIncrement (&i);


	jsval val;

	jschar str[] = L("ABCD");
	jschar *str1 = str;

	JL_NativeToJsval(cx, OwnerlessJsstr(str), i, &val);




	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

bad:
	return EXIT_SUCCESS;

}
