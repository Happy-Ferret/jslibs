#define NATIVE_READ_TRANSFORMATION_MATRIX "_NATIVE_READ_TRANSFORMATION_MATRIX"
#define NATIVE_TRANSFORMATION_MATRIX_PRIVATE "_NATIVE_TRANSFORMATION_MATRIX_PRIVATE"

// ReadMatrix44 MUST be 2Bytes aligned to be stored in PRIVATE_TO_JSVAL

// OPTIMIZATION IDEA:
// pm is a float** to allow the ReadMatrix function to :
//   fill the provided buffer OR to return its own raw matrix pointer
// this avoid loosing time is data copy

typedef int __declspec(align(2)) (*ReadMatrix44)(void *pv, float *pm);
