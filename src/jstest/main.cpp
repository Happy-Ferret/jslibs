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

#define MODULE_NAME "jstemplate"
static const char *_revision = "$Rev$";

#define XP_WIN
#include <jsapi.h>


#include "jsobj.h"
#include "jsatom.h"

#include "../common/jsclass.h"
#include "../common/jsconfiguration.h"

#define USE_UNSAFE_MODE

#include "../common/jshelper.h"
#include "jstest.h"
#include "jstest1.h"

DEFINE_UNSAFE_MODE;

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	JS_DefineProperty(cx, obj, MODULE_NAME "_build", INT_TO_JSVAL(atoi(_revision+6)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ); // 6 is the size of "$Rev: "

	SET_UNSAFE_MODE( GetConfigurationValue(cx, "unsafeMode" ) == JSVAL_TRUE );

	JSFunctionSpec _tmp[] = {{0}};
	JSBool st;

	st = INIT_CLASS( Body );
	st = INIT_CLASS( World );



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
