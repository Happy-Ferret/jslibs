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
//	JL_ASSERT_INT(v);
	//JL_NativeToJsval(cx, L("ABCDE"), 5, &v);
//	JL_CHK( JL_JsvalToNative(cx, v, &str) );
	// JL_CHK( JL_NativeToJsval(cx, ival, &v) );
//	JL_NativeVectorToJsval(cx, arr, 6, &v);
//	JL_JsvalToPrimitive(cx, v, &v);
//	JL_Push(cx, obj, &v);

	//b = JL_IsStringObject(cx, obj);
	//b = JL_HasPrivate(cx, obj);


	bad: ///////////////////////////////////////////////////////////////////////////////////
	a = JLGetEIP() - a - (a-err);

	JL_JsvalToNative(cx, val, &st);



	printf("code length: %d\n", a);

	i32 = st;
	printf("tmp-%d-%f-%i\n", i32, f32, b);
}

//#define JLERR_UNEXPECTED( cond, msg ) if (unlikely( _unsafeMode )) { if ( cond ) JS_ReportErrorFlagsAndNumber( }

// ERR( E_RANGE(2, 3), E_ARGC ) => valid range is 2 to 3 (for) argument count

// ERR( E_ARGC, E_RANGE(2, 3) ) => invalid argument count, valid range is 2 to 3.



#define JL_LOCATION \
	E_TXTID_STR, "@" JL_CODE_LOCATION



const JSErrorFormatString *
_ErrorCallback(void *userRef, const char *locale, const uintN errorNumber) {

	return (JSErrorFormatString*)userRef;
}

JSBool
JL_ReportError( JSContext *cx, bool isWarning, const char *message, JSExnType exn ) {

	JSErrorFormatString format = { message, 0, exn };
	return JS_ReportErrorFlagsAndNumber(cx, isWarning ? JSREPORT_WARNING : JSREPORT_ERROR, _ErrorCallback, (void*)&format, 0);
}

JSBool
JL_Report( JSContext *cx, bool isWarning, ... ) {

   va_list vl;
	va_start(vl, isWarning);

	int id;
	JSExnType exn = JSEXN_NONE;

	char message[1024];
	char *buf = message;
	const char *str, *strEnd, *pos;

	while ( (id = va_arg(vl, int)) != 0 ) {

		if ( E_msg[id].exn != JSEXN_NONE )
			exn = E_msg[id].exn;

		str = E_msg[id].msg;

		if ( buf != message ) {

			memcpy(buf, " ", 1);
			buf += 1;
		}

		strEnd = str + strlen(str);
		pos = str;

		for (;;) {
			
			const char *newPos = strchr(pos, '%');
			if ( !newPos ) {

				memcpy(buf, pos, strEnd-pos);
				buf += strEnd-pos;
				break;
			} else {

				memcpy(buf, pos, newPos-pos);
				buf += newPos-pos;
			}
			pos = newPos;

			switch ( *++pos ) {
				case 'd':
					++pos;
					ltoa(va_arg(vl, long), buf, 10);
					buf += strlen(buf);
					break;
				case 'x':
					++pos;
					memcpy(buf, "0x", 2);
					buf += 2;
					ltoa(va_arg(vl, long), buf, 16);
					buf += strlen(buf);
					break;
				case 's': {
					++pos;
					const char * tmp = va_arg(vl, char *);
					int len = strlen(tmp);
					if ( len > 128 ) {
						
						memcpy(buf, tmp, 128);
						buf += 128;
						memcpy(buf, "...", 3);
						buf += 3;
					} else {

						memcpy(buf, tmp, len);
						buf += len;
					}
					break;
				}
				default:
					*(buf++) = '%';
					break;
			}
		}
	}
	*buf = '\0';

	va_end(vl);
	return JL_ReportError(cx, isWarning, message, exn);

bad:
	va_end(vl);
	return JS_FALSE;
}



#define JL_ERR(...) \
	JL_Report(cx, false, ##__VA_ARGS__, JL_LOCATION, 0);

#define JL_WARN(...) \
	JL_Report(cx, true, ##__VA_ARGS__, JL_LOCATION, 0);


#define JLSASSERT( COND, ACTION ) \
	JL_MACRO_BEGIN \
		if ( JL_IS_SAFE ) { \
			if (unlikely( !(COND) )) { \
				ACTION; \
			} \
		} \
	JL_MACRO_END



#define JLSASSERT_ARGRANGE(minCount, maxCount) \
	JLSASSERT( JL_INRANGE(JL_ARGC, minCount, maxCount), JL_ERR( ARGC, RANGE(minCount, maxCount) ) );

#define JLSASSERT_ARGMIN(count) \
	JLSASSERT( (argc) >= (count), JL_ERR( ARGC, MIN(count) ) );

#define JLSASSERT_ARGMAX(count) \
	JLSASSERT( (argc) >= (count), JL_ERR( ARGC, MAX(count) ) );



int main(int argc, char* argv[]) {

	_unsafeMode = false;

	JSRuntime *rt = JS_NewRuntime(0);
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, (uint32)-1);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, (uint32)-1);
	JSContext *cx = JS_NewContext(rt, 8192L);
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_JIT);
	JSObject *globalObject = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
	JS_InitStandardClasses(cx, globalObject);


	//	WARN( E_ARGC, E_RANGE(2,3) );
	//	ERR( E_TYPE( "3D-Array" ), E_ARG( "vector1" ) )


	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

bad:
	return EXIT_SUCCESS;

}
