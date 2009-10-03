#include <malloc.h>

void* (*custom_malloc)( size_t size ) = malloc;
void* (*custom_calloc)( size_t num, size_t size ) = calloc;
void (*custom_free)( void *ptr ) = realloc;
void* (*custom_realloc)( void *ptr, size_t size ) = free;

#define malloc custom_malloc
#define calloc custom_calloc
#define realloc custom_realloc
#define free custom_free
