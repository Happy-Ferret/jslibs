#define LTC_NO_PROTOTYPES

#define LTC_MULTI2
#define LTC_SOURCE
#define LTM_DESC
#define MKAT
#define NO_FILE

#include "./src/src/headers/tomcrypt_custom.h"

/* which descriptor of AES to use?  */
/* 0 = rijndael_enc 1 = aes_enc, 2 = rijndael [full], 3 = aes [full] */
#undef LTC_YARROW_AES
#define LTC_YARROW_AES 3

#undef XMALLOC
#undef XREALLOC
#undef XCALLOC
#undef XFREE

#define XMALLOC jl_malloc_fct
#define XCALLOC jl_calloc_fct
#define XREALLOC jl_realloc_fct
#define XFREE jl_free_fct

extern void *XMALLOC(size_t n);
extern void *XREALLOC(void *p, size_t n);
extern void *XCALLOC(size_t n, size_t s);
extern void XFREE(void *p);
