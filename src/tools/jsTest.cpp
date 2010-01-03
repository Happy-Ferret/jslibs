#include <cstring>
#include <stdlib.h>
#include <stdio.h>

#define XP_WIN
#include <jsapi.h>

#include "../common/jlhelper.h"
#include "../common/vector3.h"

bool _unsafeMode = false;




struct JLConstIntegerSpec {
    int ival;
    const char *name;
};

struct JLClassSpec {
	JSExtendedClass xclasp;
	JLClassName className;
	JSNative constructor;
	uintN nargs;
	JLClassName parentProtoName;
	JSPropertySpec *ps;
	JSPropertySpec *static_ps;
	JSFunctionSpec *fs;
	JSFunctionSpec *static_fs;
	JSConstDoubleSpec *ds;
	JLConstIntegerSpec *is;
	unsigned int revision;
};


namespace TEST {
	extern JLClassSpec *classSpec;
}

ALWAYS_INLINE JSBool JLInitClass(JSContext *cx, JSObject *obj, JLClassSpec *cs);


ALWAYS_INLINE int ClassNameTooLong() {
	JL_ASSERT(false);
	return 0;
}


ALWAYS_INLINE unsigned int xxx( const JLClassName *n ) {

	return 
		(!*n[ 0] ?  0 : (*n[ 0]) ^ 
		(!*n[ 1] ?  1 : (*n[ 1]<<1) ^ 
		(!*n[ 2] ?  2 : (*n[ 2]) ^ 
		(!*n[ 3] ?  3 : (*n[ 3]<<2) ^ 
		(!*n[ 4] ?  4 : (*n[ 4]) ^ 
		(!*n[ 5] ?  5 : (*n[ 5]<<1) ^ 
		(!*n[ 6] ?  6 : (*n[ 6]) ^ 
		(!*n[ 7] ?  7 : (*n[ 7]<<2) ^ 
		(!*n[ 8] ?  8 : (*n[ 8]) ^ 
		(!*n[ 9] ?  9 : (*n[ 9]<<1) ^ 
		(!*n[10] ? 10 : (*n[10]) ^ 
		(!*n[11] ? 11 : (*n[11]<<2) ^ 
		(!*n[12] ? 12 : (*n[12]) ^ 
		(!*n[13] ? 13 : (*n[13]<<1) ^ 
		(!*n[14] ? 14 : (*n[14]) ^ 
		(!*n[15] ? 15 : (*n[15]<<2) ^
		(!*n[16] ? 16 : (*n[16]) ^ 
		(!*n[17] ? 17 : (*n[17]<<1) ^ 
		(!*n[18] ? 18 : (*n[18]) ^ 
		(!*n[19] ? 19 : (*n[19]<<2) ^
		0)))))))))))))))))))) &0x1FF;
}


int main(int argc, char* argv[]) {

//	JLInitClass(NULL, NULL, TEST::classSpec);

	Vector3 pt;
	Vector3 vel, pos;
	
	Vector3Set(&vel, 0,2,0);
	Vector3Set(&pos, 3,2,0);

	Vector3Set(&pt, 3, 2, 0);

	Vector3SubVector3(&pt, &pt, &pos);
	Vector3Normalize(&pt, &pt);
	float dot = Vector3Dot(&pt, &vel);
	Vector3Mult(&pt, &pt, dot);




	return EXIT_SUCCESS;
}











