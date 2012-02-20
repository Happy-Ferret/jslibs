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
$SVN_REVISION $Revision: 3533 $
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
DEFINE_FUNCTION( open ) {

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
DEFINE_FUNCTION( close ) {

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
DEFINE_FUNCTION( write ) {

	JLStr str;
	JL_ASSERT_ARGC(1);
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
DEFINE_FUNCTION( read ) {

	JL_ASSERT_ARGC(1);
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
$TOC_MEMBER $INAME
 $STR $INAME( mode )
**/
DEFINE_FUNCTION( setConsoleMode ) {

	JL_ASSERT_ARGC(1);
	DWORD mode;
	JL_JsvalToNative(cx, JL_ARG(0), &mode);

	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	BOOL res = SetConsoleMode(hStdin, mode);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( x, y, chr [, color] [, backgroundColor] )
**/
DEFINE_FUNCTION( writeConsoleOutput ) {

	JLStr str;
	BOOL res;

	JL_ASSERT_ARGC_RANGE(3, 5);

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());


	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &str) );
	JL_ASSERT( str.LengthOrZero() == 1, E_ARGVALUE, E_NUM(1), E_LENGTH, E_NUM(1) );
	CHAR_INFO charInfo;
	charInfo.Char.UnicodeChar = str.GetJsStrConstOrNull()[0];

	int color, backgroundColor;
	
	if ( JL_ARG_ISDEF(4) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &color) );
	else
		color = 0;

	if ( JL_ARG_ISDEF(5) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(5), &backgroundColor) );
	else
		backgroundColor = 0;

	charInfo.Attributes = ((backgroundColor & 0xF ) << 4) | (color & 0xF);

	COORD size;
	size.X = 1;
	size.Y = 1;

	COORD coord;
	coord.X = 0;
	coord.Y = 0;

	SMALL_RECT writeRegion;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &writeRegion.Left) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &writeRegion.Top) );

	writeRegion.Left += consoleScreenBufferInfo.srWindow.Left;
	writeRegion.Top += consoleScreenBufferInfo.srWindow.Top;

	writeRegion.Right = writeRegion.Left;
	writeRegion.Bottom = writeRegion.Top;

	res = WriteConsoleOutput(hStdout, &charInfo, size, coord, &writeRegion);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STR $INAME( x, y, width, height, char, textAttribute )
**/
DEFINE_FUNCTION( fillConsoleOutput ) {

	JLStr str;
	BOOL res;

	JL_ASSERT_ARGC(6);

	COORD size;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &size.X) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &size.Y) );

	if ( size.X > 0 && size.Y > 0 ) {

		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(5), &str) );
		JL_ASSERT( str.LengthOrZero() == 1, E_ARGVALUE, E_NUM(1), E_LENGTH, E_NUM(1) );
		WCHAR unicodeChar = str.GetJsStrConstOrNull()[0];

		WORD attributes;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(6), &attributes) );

		CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
		res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
		if ( res == 0 )
			return WinThrowError(cx, GetLastError());

		COORD coord;
		coord.X = 0;
		coord.Y = 0;


		SMALL_RECT writeRegion;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &writeRegion.Left) );
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &writeRegion.Top) );

		writeRegion.Left += consoleScreenBufferInfo.srWindow.Left;
		writeRegion.Top += consoleScreenBufferInfo.srWindow.Top;

		writeRegion.Right = writeRegion.Left + size.X;
		writeRegion.Bottom = writeRegion.Top + size.Y;

		CHAR_INFO *charInfo;
		charInfo = (CHAR_INFO*)jl_malloca(sizeof(CHAR_INFO) * size.X * size.Y);
		for ( int i = 0; i < size.X * size.Y; ++i ) {
			
			charInfo[i].Attributes = attributes;
			charInfo[i].Char.UnicodeChar = unicodeChar;
		}

		res = WriteConsoleOutput(hStdout, charInfo, size, coord, &writeRegion);
		jl_freea(charInfo);
		if ( res == 0 )
			return WinThrowError(cx, GetLastError());
	}

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( scrollY )
**/
DEFINE_FUNCTION( scrollY ) {

	//see. http://msdn.microsoft.com/en-us/library/ms685113(v=vs.85).aspx

	JL_ASSERT_ARGC(1);
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

	SHORT iRows;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &iRows) );

	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	BOOL res;
	res = GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());

	SMALL_RECT srctScrollRect;
	srctScrollRect.Left = 0;
	srctScrollRect.Top = 0;
	srctScrollRect.Right = csbiInfo.dwSize.X - 1;
	srctScrollRect.Bottom = csbiInfo.dwSize.Y - iRows;

	COORD coordDest;
	coordDest.X = 0;
	coordDest.Y = -iRows;

	SMALL_RECT srctClipRect;
	srctClipRect = srctScrollRect; 

	CHAR_INFO chiFill;
	chiFill.Attributes = 0;
	chiFill.Char.AsciiChar = (char)' ';

	res = ScrollConsoleScreenBuffer(hStdout, &srctScrollRect, &srctClipRect, coordDest, &chiFill);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STR $INAME()
** /

// see ConsoleEndWait()

DEFINE_FUNCTION( readConsoleInput ) {

	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);

//	DWORD numberOfEvents;
//	GetNumberOfConsoleInputEvents(hStdin, &numberOfEvents);

	INPUT_RECORD inputRecord;
	DWORD numberOfEventsRead;
	BOOL res = ReadConsoleInput(hStdin, &inputRecord, 1, &numberOfEventsRead);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());

	if ( numberOfEventsRead == 0 ) {
		
		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	jsval tmp;
	JSObject *eventObject;
	eventObject = JL_NewObj(cx);
	JL_ASSERT_ALLOC( eventObject );
	*JL_RVAL = OBJECT_TO_JSVAL(eventObject);

	tmp = INT_TO_JSVAL(inputRecord.EventType);
	JL_CHK( JS_SetProperty(cx, eventObject, "type", &tmp) );

	switch ( inputRecord.EventType ) {
		case KEY_EVENT: {

			tmp = BOOLEAN_TO_JSVAL(inputRecord.Event.KeyEvent.bKeyDown);
			JL_CHK( JS_SetProperty(cx, eventObject, "keyDown", &tmp) );

			tmp = INT_TO_JSVAL(inputRecord.Event.KeyEvent.dwControlKeyState);
			JL_CHK( JS_SetProperty(cx, eventObject, "controlKeyState", &tmp) );

			JSString *str = JS_NewUCStringCopyN(cx, &inputRecord.Event.KeyEvent.uChar.UnicodeChar, 1);
			tmp = STRING_TO_JSVAL(str);
			JL_CHK( JS_SetProperty(cx, eventObject, "char", &tmp) );

			tmp = INT_TO_JSVAL(inputRecord.Event.KeyEvent.wRepeatCount);
			JL_CHK( JS_SetProperty(cx, eventObject, "repeatCount", &tmp) );

			tmp = INT_TO_JSVAL(inputRecord.Event.KeyEvent.wVirtualKeyCode);
			JL_CHK( JS_SetProperty(cx, eventObject, "virtualKeyCode", &tmp) );

			tmp = INT_TO_JSVAL(inputRecord.Event.KeyEvent.wVirtualScanCode);
			JL_CHK( JS_SetProperty(cx, eventObject, "virtualScanCode", &tmp) );
			break;
		}
		case WINDOW_BUFFER_SIZE_EVENT: {

			tmp = INT_TO_JSVAL(inputRecord.Event.WindowBufferSizeEvent.dwSize.X);
			JL_CHK( JS_SetProperty(cx, eventObject, "x", &tmp) );

			tmp = INT_TO_JSVAL(inputRecord.Event.WindowBufferSizeEvent.dwSize.Y);
			JL_CHK( JS_SetProperty(cx, eventObject, "y", &tmp) );
			break;
		}
		case FOCUS_EVENT: {

			tmp = BOOLEAN_TO_JSVAL(inputRecord.Event.FocusEvent.bSetFocus);
			JL_CHK( JS_SetProperty(cx, eventObject, "setFocus", &tmp) );
			break;
		}
	}

	return JS_TRUE;
	JL_BAD;
}
*/


/**doc
$TOC_MEMBER $INAME
 $TYPE HANDLE $INAME()
  Passively waits for a new Console event through the ProcessEvents function.
**/

struct UserProcessEvent {
	
	ProcessEvent pe;
	HANDLE consoleEvent;
	HANDLE cancelEvent;
	JSObject *obj;
};


S_ASSERT( offsetof(UserProcessEvent, pe) == 0 );

void ConsoleStartWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	HANDLE events[] = { upe->consoleEvent, upe->cancelEvent };
	DWORD status = WaitForMultipleObjects(COUNTOF(events), events, FALSE, INFINITE);
	ASSERT( status != WAIT_FAILED );
}

bool ConsoleCancelWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	SetEvent(upe->cancelEvent);
	return true;
}

JSBool ConsoleEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;
	CloseHandle(upe->cancelEvent);
	*hasEvent = WaitForSingleObject(upe->consoleEvent, 0) == WAIT_OBJECT_0;

	if ( *hasEvent ) {
	
		HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
		INPUT_RECORD inputRecord;
		DWORD numberOfEventsRead;
		BOOL res = ReadConsoleInput(hStdin, &inputRecord, 1, &numberOfEventsRead);
		if ( res == 0 )
			return WinThrowError(cx, GetLastError());

		if ( numberOfEventsRead > 0 ) {

			jsval fct, argv[8];
			switch ( inputRecord.EventType ) {
				case KEY_EVENT: {

					JL_CHK( JS_GetProperty(cx, upe->obj, inputRecord.Event.KeyEvent.bKeyDown ? "onKeyDown" : "onKeyUp", &fct) );
					if ( JL_ValueIsCallable(cx, fct) ) {

						JSString *str = JS_NewUCStringCopyN(cx, &inputRecord.Event.KeyEvent.uChar.UnicodeChar, 1);
						argv[1] = STRING_TO_JSVAL(str);
						argv[2] = INT_TO_JSVAL(inputRecord.Event.KeyEvent.wVirtualKeyCode);
						argv[3] = BOOLEAN_TO_JSVAL(inputRecord.Event.KeyEvent.dwControlKeyState & (RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED));
						argv[4] = BOOLEAN_TO_JSVAL(inputRecord.Event.KeyEvent.dwControlKeyState & (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED));
						argv[5] = BOOLEAN_TO_JSVAL(inputRecord.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED);
						for ( int i = 0; i < inputRecord.Event.KeyEvent.wRepeatCount; ++i ) {
							
							JL_CHK( JS_CallFunctionValue(cx, upe->obj, fct, 5, argv+1, argv) );
						}
					}
					break;
				}

				case MOUSE_EVENT: {

					const char *eventName;
					if ( inputRecord.Event.MouseEvent.dwEventFlags == 0 ) {
	
						eventName = inputRecord.Event.MouseEvent.dwButtonState == 0 ? "onMouseUp" : "onMouseDown";
					} else {
						switch ( inputRecord.Event.MouseEvent.dwEventFlags ) {
						case MOUSE_MOVED:
							eventName = "onMouseMove";
							break;
						case DOUBLE_CLICK:
							eventName = "onDblClick";
							break;
						case MOUSE_WHEELED:
							eventName = "onMouseWheel";
							break;
						}
					}
					JL_CHK( JS_GetProperty(cx, upe->obj, eventName, &fct) );
					if ( JL_ValueIsCallable(cx, fct) ) {

						argv[1] = INT_TO_JSVAL(inputRecord.Event.MouseEvent.dwMousePosition.X);
						argv[2] = INT_TO_JSVAL(inputRecord.Event.MouseEvent.dwMousePosition.Y);
						argv[3] = INT_TO_JSVAL(inputRecord.Event.MouseEvent.dwButtonState & 0x0000FFFF);
						argv[4] = INT_TO_JSVAL((signed int)inputRecord.Event.MouseEvent.dwButtonState >> 16);
						argv[5] = BOOLEAN_TO_JSVAL(inputRecord.Event.MouseEvent.dwControlKeyState & (RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED));
						argv[6] = BOOLEAN_TO_JSVAL(inputRecord.Event.MouseEvent.dwControlKeyState & (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED));
						argv[7] = BOOLEAN_TO_JSVAL(inputRecord.Event.MouseEvent.dwControlKeyState & SHIFT_PRESSED);
						JL_CHK( JS_CallFunctionValue(cx, upe->obj, fct, 7, argv+1, argv) );
					}
					break;
				}

				case WINDOW_BUFFER_SIZE_EVENT: {

					JL_CHK( JS_GetProperty(cx, upe->obj, "onSize", &fct) );
					if ( JL_ValueIsCallable(cx, fct) ) {

						argv[1] = INT_TO_JSVAL(inputRecord.Event.WindowBufferSizeEvent.dwSize.X);
						argv[2] = INT_TO_JSVAL(inputRecord.Event.WindowBufferSizeEvent.dwSize.Y);
						JL_CHK( JS_CallFunctionValue(cx, upe->obj, fct, 2, argv+1, argv) );
					}
					break;
				}

				case FOCUS_EVENT: {

					JL_CHK( JS_GetProperty(cx, upe->obj, "onFocus", &fct) );
					if ( JL_ValueIsCallable(cx, fct) ) {

						argv[1] = BOOLEAN_TO_JSVAL(inputRecord.Event.FocusEvent.bSetFocus);
						JL_CHK( JS_CallFunctionValue(cx, upe->obj, fct, 1, argv+1, argv) );
					}
					break;
				}
			}
		}
	}
	return JS_TRUE;
bad:
	return JS_FALSE;
}


DEFINE_FUNCTION( events ) {
	
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(0);

	UserProcessEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), sizeof(UserProcessEvent), (void**)&upe, NULL, JL_RVAL) );
	upe->pe.startWait = ConsoleStartWait;
	upe->pe.cancelWait = ConsoleCancelWait;
	upe->pe.endWait = ConsoleEndWait;

	upe->cancelEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	upe->consoleEvent = GetStdHandle(STD_INPUT_HANDLE);
	upe->obj = JL_OBJ;
	JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, OBJECT_TO_JSVAL(upe->obj)) ); // GC protection
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


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Get or set the title of the console window.
**/
DEFINE_PROPERTY_SETTER( width ) {

	BOOL res;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	res = GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	JL_CHK( JL_JsvalToNative(cx, *vp, &csbiInfo.srWindow.Right) );
	csbiInfo.srWindow.Right += csbiInfo.srWindow.Left - 1;

	// GetLargestConsoleWindowSize

	res = SetConsoleWindowInfo(hStdout, TRUE, &csbiInfo.srWindow);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( width ) {

	BOOL res;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	res = GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	*vp = INT_TO_JSVAL(csbiInfo.srWindow.Right - csbiInfo.srWindow.Left + 1);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Get or set the title of the console window.
**/
DEFINE_PROPERTY_SETTER( height ) {

	BOOL res;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	res = GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	JL_CHK( JL_JsvalToNative(cx, *vp, &csbiInfo.srWindow.Bottom) );
	csbiInfo.srWindow.Bottom += csbiInfo.srWindow.Top - 1;
	res = SetConsoleWindowInfo(hStdout, TRUE, &csbiInfo.srWindow);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( height ) {

	BOOL res;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	res = GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	*vp = INT_TO_JSVAL(csbiInfo.srWindow.Bottom - csbiInfo.srWindow.Top + 1 );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STR $INAME
**/
DEFINE_PROPERTY_SETTER( textAttribute ) {

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD attributes;
	JL_CHK( JL_JsvalToNative(cx, *vp, &attributes) );
	BOOL res = SetConsoleTextAttribute(hStdout, attributes);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( textAttribute ) {

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	BOOL res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	*vp = INT_TO_JSVAL(consoleScreenBufferInfo.wAttributes);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
**/
DEFINE_PROPERTY_GETTER( cursorPositionX ) {

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	BOOL res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	*vp = INT_TO_JSVAL(consoleScreenBufferInfo.dwCursorPosition.X - consoleScreenBufferInfo.srWindow.Left);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( cursorPositionX ) {

	BOOL res;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	SHORT x;
	JL_CHK( JL_JsvalToNative(cx, *vp, &x) );
	consoleScreenBufferInfo.dwCursorPosition.X = consoleScreenBufferInfo.srWindow.Left + JL_MINMAX(x, 0, consoleScreenBufferInfo.srWindow.Right - consoleScreenBufferInfo.srWindow.Left);
	res = SetConsoleCursorPosition(hStdout, consoleScreenBufferInfo.dwCursorPosition);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
**/
DEFINE_PROPERTY_GETTER( cursorPositionY ) {

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	BOOL res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	*vp = INT_TO_JSVAL(consoleScreenBufferInfo.dwCursorPosition.Y - consoleScreenBufferInfo.srWindow.Top);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( cursorPositionY ) {

	BOOL res;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	SHORT y;
	JL_CHK( JL_JsvalToNative(cx, *vp, &y) );
	consoleScreenBufferInfo.dwCursorPosition.Y = consoleScreenBufferInfo.srWindow.Top + JL_MINMAX(y, 0, consoleScreenBufferInfo.srWindow.Bottom - consoleScreenBufferInfo.srWindow.Top);
	res = SetConsoleCursorPosition(hStdout, consoleScreenBufferInfo.dwCursorPosition);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( x, y )
**/
DEFINE_FUNCTION( setCursorPosition ) {

	JL_ASSERT_ARGC(2);
	COORD position;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &position.X) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &position.Y) );

	BOOL res;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());

	position.X += consoleScreenBufferInfo.srWindow.Left;
	position.Y += consoleScreenBufferInfo.srWindow.Top;

	res = SetConsoleCursorPosition(hStdout, position);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $STR $INAME
**/
DEFINE_PROPERTY_GETTER( cursorSize ) {

	BOOL res;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	res = GetConsoleCursorInfo(hStdout, &cursorInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	*vp = INT_TO_JSVAL( cursorInfo.bVisible == TRUE ? cursorInfo.dwSize : 0 );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( cursorSize ) {

	BOOL res;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	res = GetConsoleCursorInfo(hStdout, &cursorInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	DWORD size;
	JL_CHK( JL_JsvalToNative(cx, *vp, &size) );
	if ( size == 0 ) {

		cursorInfo.bVisible = FALSE;
	} else {

		cursorInfo.bVisible = TRUE;
		cursorInfo.dwSize = size;
	}
	res = SetConsoleCursorInfo(hStdout, &cursorInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	return JS_TRUE;
	JL_BAD;
}



CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 3533 $"))

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( open )
		FUNCTION( close )
		FUNCTION( write )
		FUNCTION( read )

		FUNCTION( setConsoleMode )
//		FUNCTION( readConsoleInput )

		FUNCTION( writeConsoleOutput )
		FUNCTION( fillConsoleOutput )
		FUNCTION( setCursorPosition )
		FUNCTION( scrollY )
		
		FUNCTION( events )

	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY( title )
		PROPERTY( width )
		PROPERTY( height )
		PROPERTY( textAttribute )
		PROPERTY( cursorPositionX )
		PROPERTY( cursorPositionY )
		PROPERTY( cursorSize )
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC

		CONST_INTEGER_SINGLE( FOREGROUND_BLUE )
		CONST_INTEGER_SINGLE( FOREGROUND_GREEN )
		CONST_INTEGER_SINGLE( FOREGROUND_RED )
		CONST_INTEGER_SINGLE( FOREGROUND_INTENSITY )
		CONST_INTEGER_SINGLE( BACKGROUND_BLUE )
		CONST_INTEGER_SINGLE( BACKGROUND_GREEN )
		CONST_INTEGER_SINGLE( BACKGROUND_RED )
		CONST_INTEGER_SINGLE( BACKGROUND_INTENSITY )
		CONST_INTEGER_SINGLE( COMMON_LVB_LEADING_BYTE )
		CONST_INTEGER_SINGLE( COMMON_LVB_TRAILING_BYTE )
		CONST_INTEGER_SINGLE( COMMON_LVB_GRID_HORIZONTAL )
		CONST_INTEGER_SINGLE( COMMON_LVB_GRID_LVERTICAL )
		CONST_INTEGER_SINGLE( COMMON_LVB_GRID_RVERTICAL )
		CONST_INTEGER_SINGLE( COMMON_LVB_REVERSE_VIDEO )
		CONST_INTEGER_SINGLE( COMMON_LVB_UNDERSCORE )
		CONST_INTEGER_SINGLE( COMMON_LVB_SBCSDBCS )

		CONST_INTEGER_SINGLE( ENABLE_ECHO_INPUT )
		CONST_INTEGER_SINGLE( ENABLE_EXTENDED_FLAGS )
		CONST_INTEGER_SINGLE( ENABLE_INSERT_MODE )
		CONST_INTEGER_SINGLE( ENABLE_LINE_INPUT )
		CONST_INTEGER_SINGLE( ENABLE_MOUSE_INPUT )
		CONST_INTEGER_SINGLE( ENABLE_PROCESSED_INPUT )
		CONST_INTEGER_SINGLE( ENABLE_QUICK_EDIT_MODE )
		CONST_INTEGER_SINGLE( ENABLE_WINDOW_INPUT )
		CONST_INTEGER_SINGLE( ENABLE_PROCESSED_OUTPUT )
		CONST_INTEGER_SINGLE( ENABLE_WRAP_AT_EOL_OUTPUT )

		CONST_INTEGER_SINGLE( FOCUS_EVENT )
		CONST_INTEGER_SINGLE( KEY_EVENT )
		CONST_INTEGER_SINGLE( MENU_EVENT )
		CONST_INTEGER_SINGLE( MOUSE_EVENT )
		CONST_INTEGER_SINGLE( WINDOW_BUFFER_SIZE_EVENT )

		CONST_INTEGER_SINGLE( FROM_LEFT_1ST_BUTTON_PRESSED )
		CONST_INTEGER_SINGLE( RIGHTMOST_BUTTON_PRESSED )
		CONST_INTEGER_SINGLE( FROM_LEFT_2ND_BUTTON_PRESSED )
		CONST_INTEGER_SINGLE( FROM_LEFT_3RD_BUTTON_PRESSED )
		CONST_INTEGER_SINGLE( FROM_LEFT_4TH_BUTTON_PRESSED )

	END_CONST_INTEGER_SPEC


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
