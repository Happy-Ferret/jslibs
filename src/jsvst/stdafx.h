/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */


#pragma once

#include <jlhelper.h>
#include <jlclass.h>

#include <fcntl.h>
#ifdef WIN
	#include <io.h>
#endif

#ifdef UNIX
	#include <unistd.h>
	#include <dlfcn.h>
#endif


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

#pragma once

#include <jlplatform.h>

#include <fcntl.h>
#ifdef WIN
	#include <io.h>
#endif

#ifdef UNIX
	#include <unistd.h>
	#include <dlfcn.h>
#endif


//#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <errno.h>
// #include <limits.h> // included by jlplatform.h
#include <signal.h>

#include <jlhelper.h>

#include "../host/host.h"
