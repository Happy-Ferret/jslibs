// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "jlhelper.h"
#include "jlclass.h"

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
// #include <limits.h> // included by jlplatform.h
#include <signal.h>


#include "../host/host.h"

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

#include "jlplatform.h"

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
// #include <limits.h> // included by jlplatform.h
#include <signal.h>

#include "jlhelper.h"

#include "../host/host.h"

/*
#include "jsstddef.h"
#include "jsprf.h"
#include "jsscript.h"
#include "jscntxt.h"
*/
