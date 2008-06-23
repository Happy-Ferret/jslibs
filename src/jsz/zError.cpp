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

//#include <jsdbgapi.h>
//#include <jscntxt.h>
//#include <jsscript.h>

#include <zlib.h>

#include "zError.h"

/**doc
----
== jsz::ZError class ==
You cannot construct this class.
Its aim is to throw as an exception on any zlib runtime error.
**/

JSClass ZError_class = {
  "ZError", JSCLASS_HAS_RESERVED_SLOTS(2),
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};


JSBool ZError_getter_code(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JS_GetReservedSlot( cx, obj, 0, vp );
  return JS_TRUE;
}

JSBool ZError_getter_text(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JS_GetReservedSlot( cx, obj, 1, vp );
	return JS_TRUE;
}

JSBool ZError_getter_const(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JS_GetReservedSlot( cx, obj, 0, vp );
	int errorCode = JSVAL_TO_INT(*vp);

	char *errStr;
	switch (errorCode) {
		case 0:
			errStr = "Z_OK";
			break;
		case 1:
			errStr = "Z_STREAM_END";
			break;
		case 2:
			errStr = "Z_NEED_DICT";
			break;
		case -1:
			errStr = "Z_ERRNO";
			break;
		case -2:
			errStr = "Z_STREAM_ERROR";
			break;
		case -3:
			errStr = "Z_DATA_ERROR";
			break;
		case -4:
			errStr = "Z_MEM_ERROR";
			break;
		case -5:
			errStr = "Z_BUF_ERROR";
			break;
		case -6:
			errStr = "Z_VERSION_ERROR";
			break;
		default:
			errStr = "UNKNOWN_ERROR";
	}

	JSString *str = JS_NewStringCopyZ( cx, errStr );
	*vp = STRING_TO_JSVAL( str );
  return JS_TRUE;
}


JSPropertySpec ZError_PropertySpec[] = { // *name, tinyid, flags, getter, setter
  { "code",  0, JSPROP_READONLY|JSPROP_PERMANENT, ZError_getter_code, NULL },
  { "text",  0, JSPROP_READONLY|JSPROP_PERMANENT, ZError_getter_text, NULL },
  { "const",  0, JSPROP_READONLY|JSPROP_PERMANENT, ZError_getter_const, NULL },
  { 0 }
};


JSBool ZError_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}


JSBool ThrowZError( JSContext *cx, int errorCode, const char *errorMessage ) {

/*
	const char * filename = NULL;
	uintN lineno;
  for (JSStackFrame *fp = cx->fp; fp; fp = fp->down)
      if (fp->script && fp->pc) {

          filename = fp->script->filename;
          lineno = JS_PCToLineNumber(cx, fp->script, fp->pc);
          break;
      }
	JS_ReportWarning( cx, "ThrowZError %s:%d", filename, lineno );
*/

	JS_ReportWarning( cx, "ZError exception" );

	JSObject *error = JS_NewObject( cx, &ZError_class, NULL, NULL );

	JS_SetReservedSlot( cx, error, 0, INT_TO_JSVAL(errorCode) );
	JS_SetReservedSlot( cx, error, 1, errorMessage != NULL ? STRING_TO_JSVAL(JS_NewStringCopyZ( cx, errorMessage )) : JSVAL_VOID );

	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
  return JS_FALSE;
}

JSObject *InitErrorClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &ZError_class, ZError_construct, 0, ZError_PropertySpec, NULL, NULL, NULL );
}
