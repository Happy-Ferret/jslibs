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
#include "console.h"

#include "error.h"

#include "stdlib.h"

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Console )

/**doc
$TOC_MEMBER $INAME
 $INAME()
  Creates a new Console object.
  $H beware
   Only one console per process is allowed. The construction fails if the calling process already has a console.
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();

	BOOL status = AllocConsole();
	if ( status == FALSE )
		return WinThrowError(cx, GetLastError());
	SetConsoleTitle("");
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FINALIZE() {

//	BOOL res = FreeConsole();
}


/**doc
=== Methods ===
**/

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
//	JL_S_ASSERT( res != 0, "Unable to free the console." );
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

	JL_S_ASSERT_ARG_MIN( 1 );
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if ( hStdout == NULL )
		return WinThrowError(cx, GetLastError());
	const char *str;
	size_t len;
	JL_CHK( JsvalToStringAndLength(cx, &argv[0], &str, &len) );
	DWORD written;
	BOOL status = WriteConsole(hStdout, str, len, &written, NULL);
	if ( status == FALSE )
		return WinThrowError(cx, GetLastError());
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

	JL_S_ASSERT_ARG_MIN( 1 );
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if ( hStdin == NULL )
		return WinThrowError(cx, GetLastError());
	char buffer[8192];
	DWORD read;
	BOOL res = ReadConsole(hStdin, buffer, sizeof(buffer), &read, NULL);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	*rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, buffer, read));
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Get or set the title of the console window.
**/
DEFINE_PROPERTY( titleSetter ) {

	const char *str;
	JL_CHK( JsvalToString(cx, vp, &str) );
	SetConsoleTitle(str);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( titleGetter ) {

	char buffer[2048];
	DWORD res = GetConsoleTitle(buffer, sizeof(buffer));
	JL_S_ASSERT( res >= 0, "Unable to GetConsoleTitle." );
	if ( res == 0 )
		*vp = JS_GetEmptyStringValue(cx);
	else
		*vp = STRING_TO_JSVAL(JS_NewStringCopyN(cx, buffer, res));
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Write )
		FUNCTION( Read )
		FUNCTION( Close )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( title )
	END_PROPERTY_SPEC

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
