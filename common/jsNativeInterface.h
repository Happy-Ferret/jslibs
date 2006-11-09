#define NI_READ_RESOURCE "_NI_READ_RESOURCE"
typedef bool (*NIResourceRead)( void *pv, unsigned char *buf, unsigned int *amount );

#define NI_READ_MATRIX44 "_NI_READ_MATRIX"
// **pm
//   in: a valid float[16]
//  out: pointer provided as input OR another pointer to float
typedef int (*NIMatrix44Read)(void *pv, float **pm); // **pm allows NIMatrix44Read to return its own data pointer ( should be const )

