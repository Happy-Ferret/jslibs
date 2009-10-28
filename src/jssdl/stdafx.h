// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "jlhelper.h"
#include "jlclass.h"
#include "jlconfiguration.h"

#define DECLSPEC

#define malloc jl_malloc_fct
#define calloc jl_calloc_fct
#define realloc jl_realloc_fct
#define free jl_free_fct

//namespace sdl {
#include <SDL.h>
//}

#undef malloc
#undef calloc
#undef realloc
#undef free

//using namespace sdl;
