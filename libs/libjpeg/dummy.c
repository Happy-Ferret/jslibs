#include <sys/types.h>
void* jl_malloc_fct( size_t size ) { return (void*)0; }
void* jl_calloc_fct( size_t num, size_t size ) { return (void*)0; }
void* jl_realloc_fct( void *ptr, size_t size ) { return (void*)0; }
void jl_free_fct( void *ptr ) { }

int main (int argc, char **argv) { }

