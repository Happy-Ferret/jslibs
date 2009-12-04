#include <cstring>
#include <stdlib.h>
#include <stdio.h>

#define XP_WIN
#include <jsapi.h>


typedef const char ClassName[20];

inline unsigned int ClassNameHashFct( ClassName n ) {

	return 
		(!n[ 0] ?  0 : (n[ 0]) ^ 
		(!n[ 1] ?  1 : (n[ 1]<<1) ^ 
		(!n[ 2] ?  2 : (n[ 2]) ^ 
		(!n[ 3] ?  3 : (n[ 3]<<1) ^ 
		(!n[ 4] ?  4 : (n[ 4]) ^ 
		(!n[ 5] ?  5 : (n[ 5]<<1) ^ 
		(!n[ 6] ?  6 : (n[ 6]) ^ 
		(!n[ 7] ?  7 : (n[ 7]<<1) ^ 
		(!n[ 8] ?  8 : (n[ 8]) ^ 
		(!n[ 9] ?  9 : (n[ 9]<<1) ^ 
		(!n[10] ? 10 : (n[10]) ^ 
		(!n[11] ? 11 : (n[11]<<1) ^ 
		(!n[12] ? 12 : (n[12]) ^ 
		(!n[13] ? 13 : (n[13]<<1) ^ 
		(!n[14] ? 14 : (n[14]) ^ 
		(!n[15] ? 15 : (n[15]<<1) ^
		(!n[16] ? 16 : (n[16]) ^ 
		(!n[17] ? 17 : (n[17]<<1) ^ 
		(!n[18] ? 18 : (n[18]) ^ 
		(!n[19] ? 19 : (n[19]<<1) ^
		0))))))))))))))))))));
}

struct ClassNameHash {
	void *data;
	ClassName name;
};

ClassNameHash classNameHash[1<<9];


unsigned int GetClassNameHash( const ClassName *name, void *data ) {
	
	unsigned int index = ClassNameHashFct(*name);

	while ( classNameHash[index].data != 0 ) {

		index = (index + 1) % sizeof(classNameHash)/sizeof(*classNameHash);
	}
	return 0;
}



static const char *className = "TEST";


static JSBool _test(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}

static JSBool Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}

struct JLClassSpec {
	JSExtendedClass xclasp;
	JSNative constructor;
	const char *parentProto;
	JSFunctionSpec *fs;
};

static JLClassSpec* JLClassSpecInit() {

	static JLClassSpec ci;
	ci.xclasp.base.name = className;
	ci.xclasp.base.flags = 0x12345678;
	ci.xclasp.base.flags += 1;
	
	ci.constructor = Constructor;
	ci.parentProto = NULL;

	static JSFunctionSpec fs[] = {
	
		JS_FS( "test", _test, 0, 0, 0 ),
	};

	ci.fs = fs;
//	JS_InitClass(

	return &ci;
}


static JLClassSpec *cli = JLClassSpecInit();



int main(int argc, char* argv[]) {

	static ClassName classname = "12345678";
	
	ClassName *classname1 = (ClassName*)malloc(sizeof(ClassName));

	memcpy((void*)classname1, classname, sizeof(ClassName));

	if ( classname == *classname1 ) {
		
		printf("ok");
	}
	



	return EXIT_SUCCESS;
}
