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

#include <jsdbgapi.h>


/**doc
$CLASS_HEADER
 
**/
BEGIN_CLASS( Sandbox )
// source: http://mxr.mozilla.org/mozilla/source/js/src/js.c

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
 * $VAL $INAME( [code], [obj] )
  Evaluates the JavaScript code. If object is specified, the code is executed in that object, treating it as a sandbox.
  If code an obj are omitted, an object with new standard classes.
  $H example 1
  {{{
  var res = Sandbox.Eval('1+2+3');
  Print( res ); // prints: 6
  }}}
  $H example 2
   The following script will failed.
  {{{
  LoadModule('jsio');
  Sandbox.Eval('var f = new File('test.txt');');
  }}}
  $H example 3
  {{{
  var g = Sandbox.Eval();
  Print( g.Math == Math ); // prints: false
  }}}
**/
DEFINE_FUNCTION_FAST( Eval ) {

	J_S_ASSERT_CLASS( J_FOBJ, _class );

	JSContext *scx = JS_NewContext(JS_GetRuntime(cx), 8192L); // see host/host.cpp
	if (!scx) {

		JS_ReportOutOfMemory(cx);
		return JS_FALSE;
	}

	JSObject *sobj;
	if ( !J_FARG_ISDEF(2) ) {

		sobj = JS_NewObject(scx, classSandbox, NULL, NULL);
		if ( !sobj ) {

			JS_DestroyContextNoGC(scx);
			return JS_FALSE;
		}
	} else {

		sobj = JSVAL_TO_OBJECT(J_FARG(2));
	}

	if ( !J_FARG_ISDEF(1) ) {

		*J_FRVAL = OBJECT_TO_JSVAL(sobj);
		JS_DestroyContextNoGC(scx);
		return JS_TRUE;
	}

	JSString *jsstr = JS_ValueToString(cx, J_FARG(1));
	uintN srclen = JS_GetStringLength(jsstr); 	
	jschar *src = JS_GetStringChars(jsstr);

	JSStackFrame *fp = JS_GetScriptedCaller(cx, NULL);
	JS_SetGlobalObject(scx, sobj);
	JS_ToggleOptions(scx, JSOPTION_DONT_REPORT_UNCAUGHT);
	JSBool ok = JS_EvaluateUCScript(scx, sobj, src, srclen, fp->script->filename, JS_PCToLineNumber(cx, fp->script, fp->regs->pc), J_FRVAL);

	if (!ok) {

		jsval v;
		if (JS_GetPendingException(scx, &v))
			JS_SetPendingException(cx, v);
		else
			JS_ReportOutOfMemory(cx);
	}
	JS_DestroyContextNoGC(scx);
	return ok;
}


DEFINE_NEW_RESOLVE() {

	JSBool resolved;
   if ( (flags & JSRESOLVE_ASSIGNING) == 0) {
		
		if (!JS_ResolveStandardClass(cx, obj, id, &resolved))
			return JS_FALSE;
		if (resolved) {
			
			*objp = obj;
			return JS_TRUE;
		}
    }
    *objp = NULL;
    return JS_TRUE;
}


//DEFINE_HAS_INSTANCE() {
//
//	*bp = !JSVAL_IS_PRIMITIVE(v) && OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(v)) == _class;
//	return JS_TRUE;
//}


CONFIGURE_CLASS

	HAS_NEW_RESOLVE
//	HAS_HAS_INSTANCE

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST(Eval)
	END_STATIC_FUNCTION_SPEC

END_CLASS
