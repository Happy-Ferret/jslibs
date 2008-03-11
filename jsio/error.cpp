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


BEGIN_CLASS( IoError )

DEFINE_CONSTRUCTOR() {

	REPORT_ERROR( "This object cannot be construct." ); // but constructor must be defined
	return JS_TRUE;
}

DEFINE_PROPERTY( code ) {

	JS_GetReservedSlot( cx, obj, 0, vp );  // (TBD) use the obj.name proprety directly instead of slot 0 ?
	return JS_TRUE;
}

DEFINE_PROPERTY( os ) {

	JS_GetReservedSlot( cx, obj, 1, vp );  // (TBD) use the obj.name proprety directly instead of slot 1 ?
	return JS_TRUE;
}

DEFINE_PROPERTY( text ) {

	JS_GetReservedSlot( cx, obj, 0, vp );  // (TBD) use the obj.name proprety directly instead of slot 0 ?
	PRErrorCode errorCode = JSVAL_TO_INT(*vp);
	JSString *str = JS_NewStringCopyZ( cx, PR_ErrorToString( errorCode, PR_LANGUAGE_EN ) );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
}

DEFINE_FUNCTION( toString ) {

	RT_CHECK_CALL( text(cx, obj, 0, rval) );
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( code )
		PROPERTY_READ( os )
		PROPERTY_READ( text )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
	END_FUNCTION_SPEC

	HAS_RESERVED_SLOTS(2)

END_CLASS



JSBool ThrowIoErrorArg( JSContext *cx, PRErrorCode errorCode, PRInt32 osError ) {

/*
	const char * filename = NULL;
	uintN lineno;
  for (JSStackFrame *fp = cx->fp; fp; fp = fp->down)
      if (fp->script && fp->pc) {

          filename = fp->script->filename;
          lineno = JS_PCToLineNumber(cx, fp->script, fp->pc);
          break;
      }
	JS_ReportWarning( cx, "ThrowNSPRError %s:%d", filename, lineno );
*/

	JS_ReportWarning( cx, "IoError exception" );
	JSObject *error = JS_NewObject( cx, &classIoError, NULL, NULL );
	JS_SetReservedSlot( cx, error, 0, INT_TO_JSVAL(errorCode) );
	JS_SetReservedSlot( cx, error, 1, INT_TO_JSVAL(osError) );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	return JS_FALSE;
}


JSBool ThrowIoError( JSContext *cx ) {

	return ThrowIoErrorArg(cx, PR_GetError(), PR_GetOSError());
}
