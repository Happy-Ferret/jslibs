// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "../common/jsHelper.h"
#include "../common/jsClass.h"
#include "../common/jsConfiguration.h"

#define XMALLOC jl_malloc_fct
#define XCALLOC jl_calloc_fct
#define XREALLOC jl_realloc_fct
#define XFREE jl_free_fct

#include <tomcrypt.h>

//#undef XMALLOC
//#undef XCALLOC
//#undef XREALLOC
//#undef XFREE

#include "error.h"
