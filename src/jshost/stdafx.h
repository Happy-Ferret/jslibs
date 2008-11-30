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
