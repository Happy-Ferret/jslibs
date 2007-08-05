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


#include "stdafx.h"

#define MODULE_NAME "jsfastcgi"
static const char *_revision = "$Rev$";

#include "fastcgi.h"
#include "fcgi.h"
#include "global.h"

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	JS_DefineProperty(cx, obj, MODULE_NAME "_build", INT_TO_JSVAL(atoi(_revision+6)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ); // 6 is the size of "$Rev: "

	INIT_CLASS( FastCGI );
	INIT_STATIC();

	return JS_TRUE;
}


BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved ) {

  switch (ul_reason_for_call) {

	  case DLL_PROCESS_ATTACH:
	  case DLL_THREAD_ATTACH:
	  case DLL_THREAD_DETACH:
	  case DLL_PROCESS_DETACH:
		  break;
  }
  return TRUE;
}

/* MS doc:

	To illustrate this, consider the following example:

	   - .EXE is linked with MSVCRT.LIB
	   - DLL A is linked with LIBCMT.LIB
	   - DLL B is linked with CRTDLL.LIB


	If the .EXE creates a CRT file handle using _create() or _open(), this file handle may only be passed to _lseek(), _read(), _write(), _close(), etc. in the .EXE file. Do not pass this CRT file handle to either DLL. Do not pass a CRT file handle obtained from either DLL to the other DLL or to the .EXE.

	If DLL A allocates a block of memory with malloc(), only DLL A may call free(), _expand(), or realloc() to operate on that block. You cannot call malloc() from DLL A and try to free that block from the .EXE or from DLL B.

	NOTE: If all three modules were linked with CRTDLL.LIB or all three were linked with MSVCRT.LIb, these restrictions would not apply.

	(source: http://support.microsoft.com/kb/94248)
*/