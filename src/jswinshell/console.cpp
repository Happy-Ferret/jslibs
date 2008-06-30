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

#include "stdlib.h"

/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Console )

/**doc
 * $INAME()
  TBD
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

	BOOL res = AllocConsole();
	SetConsoleTitle("");
	RT_ASSERT( res != 0, "Unable to create the console." );
	return JS_TRUE;
}

DEFINE_FINALIZE() {

	BOOL res = FreeConsole();
}


/**doc
=== Methods ===
**/

/**doc
 * $VOID $INAME()
  TBD
**/
DEFINE_FUNCTION( Close ) {

	BOOL res = FreeConsole();
	RT_ASSERT( res != 0, "Unable to free the console." );
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( string )
  TBD
**/DEFINE_FUNCTION( Write ) {
	
	RT_ASSERT_ARGC( 1 );
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	RT_ASSERT( hStdout != NULL, "Unable to create the stdout." );
	const char *str;
	size_t len;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], str, len );
	DWORD written;
	WriteConsole(hStdout, str, len, &written, NULL);
	CloseHandle(hStdout);
	return JS_TRUE;
}


/**doc
 * $STR $INAME( amount )
  TBD
**/DEFINE_FUNCTION( Read ) {
	
	RT_ASSERT_ARGC( 1 );
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	RT_ASSERT( hStdin != NULL, "Unable to create the stdin." );
	char buffer[8192];
	DWORD read;
	BOOL res = ReadConsole(hStdin, buffer, sizeof(buffer), &read, NULL);
	RT_ASSERT( res > 0, "Unable to ReadConsole." );
	*rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, buffer, read));
	CloseHandle(hStdin);
	return JS_TRUE;
}

/**doc
=== Properties ===
**/

/**doc
 * $STR $INAME
  Get or set the title of the console window.
**/
DEFINE_PROPERTY( titleSetter ) {

	const char *str;
	RT_JSVAL_TO_STRING( *vp, str );
	SetConsoleTitle(str);
	return JS_TRUE;
}

DEFINE_PROPERTY( titleGetter ) {

	char buffer[2048];
	DWORD res = GetConsoleTitle(buffer, sizeof(buffer));
	RT_ASSERT( res >= 0, "Unable to GetConsoleTitle." );
	if ( res == 0 )
		*vp = JS_GetEmptyStringValue(cx);
	else
		*vp = STRING_TO_JSVAL(JS_NewStringCopyN(cx, buffer, res));
	return JS_TRUE;
}


CONFIGURE_CLASS

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
