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

DECLARE_CLASS( BranchLimit )

/**doc
$CLASS_HEADER
 
**/
BEGIN_CLASS( Sandbox )
// source: http://mxr.mozilla.org/mozilla/source/js/src/js.c


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
 * $VAL $INAME( [ code ], [ obj ], [ branchLimit ] )
  Evaluates the JavaScript code in a sandbox.
  $H arguments
   $ARG string code: the code to be executed.
	$ARG object obj: if defined, the code is executed in that object, treating it as a sandbox.
	$ARG integer branchLimit: if defined, a BranchLimit exception is thrown when _branchLimit_ branch is reach.
  $H note
   If code and obj are omitted, an object with new standard classes is used instead (see example 3).
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
  $H example 4
  {{{
  try {
   var res = Sandbox.Eval('while (true);', undefined, 1000);
  } catch (ex if ex instanceof BranchLimit) {
   Print( 'script execution too long !' );
  }
  }}}
**/

struct SandboxContextPrivate {

	size_t branchCount;
	size_t maxBranchCount;
};

static JSBool BranchCallback(JSContext *cx, JSScript *script) {
	
	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(cx);
	pv->branchCount++;
	if (pv->branchCount >= pv->maxBranchCount) {

		JSObject *branchLimitExceptionObj = JS_NewObject( cx, classBranchLimit, NULL, NULL );
		JS_SetPendingException( cx, OBJECT_TO_JSVAL( branchLimitExceptionObj ) );
		return JS_FALSE;
	}
	return JS_TRUE;
}

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
//		J_CHK( JS_SetParent(cx, sobj, JSVAL_NULL) );
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

	SandboxContextPrivate pv;
	if ( J_FARG_ISDEF(3) ) {
		
		pv.branchCount = 0;
		J_CHK( JsvalToUInt(cx, J_FARG(3), &pv.maxBranchCount) );
		JS_SetContextPrivate(scx, &pv);
		JS_SetBranchCallback(scx, BranchCallback); // (TBD) deprecated
		JS_ToggleOptions(scx, JSOPTION_NATIVE_BRANCH_CALLBACK);
	}

	JSBool ok = JS_EvaluateUCScript(scx, sobj, src, srclen, fp->script->filename, JS_PCToLineNumber(cx, fp->script, fp->regs->pc), J_FRVAL);

	if (!ok) {

		jsval v;
		if (JS_GetPendingException(scx, &v))
			JS_SetPendingException(cx, v);
		else
			JS_ReportOutOfMemory(cx); // (TBD) change this. calling Halt() in the script throw this error.
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
