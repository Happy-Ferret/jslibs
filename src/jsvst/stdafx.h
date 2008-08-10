// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "../common/platform.h"

#include <fcntl.h>
#ifdef XP_WIN
	#include <io.h>
#endif

#ifdef XP_UNIX
	#include <unistd.h>
	#include <dlfcn.h>
#endif //XP_UNIX


//#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <errno.h>
// #include <limits.h> // included by ../common/platform.h
#include <signal.h>

#include <jsapi.h>

#include "../common/jsHelper.h"

#include "../host/host.h"
#include "../common/jsNames.h"

/*
#include "jsstddef.h"
#include "jsprf.h"
#include "jsscript.h"
#include "jscntxt.h"
*/
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "../common/platform.h"

#include <fcntl.h>
#ifdef XP_WIN
	#include <io.h>
#endif

#ifdef XP_UNIX
	#include <unistd.h>
	#include <dlfcn.h>
#endif //XP_UNIX


//#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <errno.h>
// #include <limits.h> // included by ../common/platform.h
#include <signal.h>

#include <jsapi.h>

#include "../common/jsHelper.h"

#include "../host/host.h"
#include "../common/jsNames.h"

/*
#include "jsstddef.h"
#include "jsprf.h"
#include "jsscript.h"
#include "jscntxt.h"
*/
