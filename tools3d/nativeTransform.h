#define NATIVE_READ_TRANSFORMATION_MATRIX "_NATIVE_READ_TRANSFORMATION_MATRIX"
#define NATIVE_TRANSFORMATION_MATRIX_PRIVATE "_NATIVE_TRANSFORMATION_MATRIX_PRIVATE"

// &ReadMatrix44 MUST be 2Bytes aligned to be stored in PRIVATE_TO_JSVAL
typedef int __declspec(align(2)) (*ReadMatrix44)(void *pv, float *m);
