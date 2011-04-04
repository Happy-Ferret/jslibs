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
#include "error.h"


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Console )


/**doc
=== Static Functions ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Creates a new Console object.
  $H beware
   Only one console per process is allowed. The construction fails if the calling process already has a console.
**/
DEFINE_FUNCTION( Open ) {

	BOOL status = AllocConsole();
	if ( status == FALSE )
		return WinThrowError(cx, GetLastError());
	SetConsoleTitle("");
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Detach the current process from its console.
**/
DEFINE_FUNCTION( Close ) {

//	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
//	CloseHandle(hStdout);
//	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
//	CloseHandle(hStdin);
	BOOL status = FreeConsole();
	if ( status == FALSE )
		return WinThrowError(cx, GetLastError());
//	JL_ASSERT( res != 0, "Unable to free the console." );
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
//	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( text )
  Write text to the console.
  $H arguments
   $ARG $STR text
**/
DEFINE_FUNCTION( Write ) {

	JLStr str;
	JL_ASSERT_ARG_COUNT(1);
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if ( hStdout == NULL )
		return WinThrowError(cx, GetLastError());
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	DWORD written;
	BOOL status = WriteConsole(hStdout, str.GetConstStr(), str.Length(), &written, NULL);
	if ( status == FALSE )
		return WinThrowError(cx, GetLastError());

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( amount )
  Read _amount_ bytes of text from the console.
  $H arguments
   $ARG $INT amount
**/
DEFINE_FUNCTION( Read ) {

	JL_ASSERT_ARG_COUNT(1);
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if ( hStdin == NULL )
		return WinThrowError(cx, GetLastError());
	char buffer[8192];
	DWORD read;
	BOOL res = ReadConsole(hStdin, buffer, sizeof(buffer), &read, NULL);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	*JL_RVAL = STRING_TO_JSVAL(JS_NewStringCopyN(cx, buffer, read));
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Static Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Get or set the title of the console window.
**/
DEFINE_PROPERTY_SETTER( title ) {

	JLStr str;
	JL_CHK( JL_JsvalToNative(cx, *vp, &str) );
	SetConsoleTitle(str);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( title ) {

	char buffer[2048];
	DWORD res = GetConsoleTitle(buffer, sizeof(buffer));
	if ( res == 0 )
		return JL_ThrowOSError(cx);
	*vp = STRING_TO_JSVAL(JS_NewStringCopyN(cx, buffer, res));
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Open )
		FUNCTION( Close )
		FUNCTION( Write )
		FUNCTION( Read )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY( title )
	END_STATIC_PROPERTY_SPEC

//	HAS_PRIVATE

END_CLASS

/**doc
=== Examples ===
{{{
var cons = new Console();
cons.title = 'My console';
cons.Write('Hello world');
}}}
**/
