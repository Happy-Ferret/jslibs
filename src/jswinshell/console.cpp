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

	JL_IGNORE(vp, argc);

	BOOL status = ::AllocConsole();
	if ( status == FALSE )
		return WinThrowError(cx, GetLastError());
	::SetConsoleTitle(TEXT(""));
	//::SetConsoleOutputCP( CP_UTF8 );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Detach the current process from its console.
**/
DEFINE_FUNCTION( close ) {

	JL_IGNORE(argc);
	JL_DEFINE_ARGS;

	//	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
//	CloseHandle(hStdout);
//	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
//	CloseHandle(hStdin);
	BOOL status = FreeConsole();
	if ( status == FALSE )
		return WinThrowError(cx, GetLastError());
//	JL_ASSERT( res != 0, "Unable to free the console." );
	
	JL_RVAL.setUndefined();
	return true;
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

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(1);

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if ( hStdout == NULL )
		return WinThrowError(cx, GetLastError());

	{

		JS::AutoCheckCannotGC nogc;
		jl::BufString str;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &str) );
		DWORD written;
		BOOL status = ::WriteConsole(hStdout, str.toData<LPCTSTR>(), str.length(), &written, NULL);
		if ( status == FALSE )
			return WinThrowError(cx, GetLastError());

	}

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( [amount] )
  Read _amount_ bytes of text from the console.
  $H arguments
   $ARG $INT amount
**/
DEFINE_FUNCTION( read ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_RANGE(0,1);

	TCHAR buffer[8192];

	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if ( hStdin == NULL )
		return WinThrowError(cx, GetLastError());

	BOOL st;

	DWORD length;
	if ( JL_ARG_ISDEF(1) ) {

		JL_CHK( jl::getValue(cx, JL_ARG(1), &length) );
	} else {

		length = sizeof(buffer);
	}

	DWORD read;
	st = ::ReadConsole(hStdin, buffer, length, &read, NULL);
	if ( st == 0 )
		return WinThrowError(cx, GetLastError());

	JL_CHK( jl::setValue( cx, JL_RVAL, jl::strSpec( buffer, read ) ) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME( mode )
  ENABLE_ECHO_INPUT, ENABLE_EXTENDED_FLAGS, ENABLE_INSERT_MODE, ENABLE_LINE_INPUT, ENABLE_MOUSE_INPUT, ENABLE_PROCESSED_INPUT, ENABLE_QUICK_EDIT_MODE, ENABLE_WINDOW_INPUT, ENABLE_PROCESSED_OUTPUT, ENABLE_WRAP_AT_EOL_OUTPUT
**/
DEFINE_FUNCTION( setConsoleMode ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(1);

	DWORD mode;
	jl::getValue(cx, JL_ARG(1), &mode);

	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	BOOL res = SetConsoleMode(hStdin, mode);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( x, y, chr [, color] [, backgroundColor] )
**/
DEFINE_FUNCTION( writeConsoleOutput ) {

	BOOL res;

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_RANGE(3, 5);

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());

	CHAR_INFO charInfo;

	{

		JS::AutoCheckCannotGC nogc;
		jl::BufString str;
		JL_CHK( jl::getValue(cx, JL_ARG(3), &str) );
		JL_ASSERT( str.lengthOrZero() == 1, E_ARGVALUE, E_NUM(1), E_LENGTH, E_NUM(1) );
		charInfo.Char.UnicodeChar = str.charAt<WCHAR>(0);

	}

	int color, backgroundColor;
	
	if ( JL_ARG_ISDEF(4) )
		JL_CHK( jl::getValue(cx, JL_ARG(4), &color) );
	else
		color = 0;

	if ( JL_ARG_ISDEF(5) )
		JL_CHK( jl::getValue(cx, JL_ARG(5), &backgroundColor) );
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
	JL_CHK( jl::getValue(cx, JL_ARG(1), &writeRegion.Left) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &writeRegion.Top) );

	writeRegion.Left += consoleScreenBufferInfo.srWindow.Left;
	writeRegion.Top += consoleScreenBufferInfo.srWindow.Top;

	writeRegion.Right = writeRegion.Left;
	writeRegion.Bottom = writeRegion.Top;

	res = WriteConsoleOutput(hStdout, &charInfo, size, coord, &writeRegion);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STR $INAME( x, y, width, height, char, textAttribute )
**/
DEFINE_FUNCTION( fillConsoleOutput ) {

	BOOL res;

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(6);

	COORD size;
	JL_CHK( jl::getValue(cx, JL_ARG(3), &size.X) );
	JL_CHK( jl::getValue(cx, JL_ARG(4), &size.Y) );

	if ( size.X > 0 && size.Y > 0 ) {

		JS::AutoCheckCannotGC nogc;
		jl::BufString str;

		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

		JL_CHK( jl::getValue(cx, JL_ARG(5), &str) );
		JL_ASSERT( str.lengthOrZero() == 1, E_ARGVALUE, E_NUM(1), E_LENGTH, E_NUM(1) );
		WCHAR unicodeChar = str.charAt<WCHAR>(0); //.GetConstWStrOrNull()[0];

		WORD attributes;
		JL_CHK( jl::getValue(cx, JL_ARG(6), &attributes) );

		CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
		res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
		if ( res == 0 )
			return WinThrowError(cx, GetLastError());

		COORD coord;
		coord.X = 0;
		coord.Y = 0;


		SMALL_RECT writeRegion;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &writeRegion.Left) );
		JL_CHK( jl::getValue(cx, JL_ARG(2), &writeRegion.Top) );

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

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( scrollY )
**/
DEFINE_FUNCTION( scrollY ) {

	//see. http://msdn.microsoft.com/en-us/library/ms685113(v=vs.85).aspx

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(1);

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SHORT iRows;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &iRows) );

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
	chiFill.Char.AsciiChar = ' ';

	res = ScrollConsoleScreenBuffer(hStdout, &srctScrollRect, &srctClipRect, coordDest, &chiFill);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());

	JL_RVAL.setUndefined();
	return true;
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
		
		JL_RVAL.setUndefined();
		return true;
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

	return true;
	JL_BAD;
}
*/


/**doc
$TOC_MEMBER $INAME
 $TYPE HANDLE $INAME()
  Passively waits for a new Console event through the processEvents function.
**/

struct ConsoleProcessEvent : public ProcessEvent2 {
	
	HANDLE consoleEvent;
	HANDLE cancelEvent;

	bool prepareWait(JSContext *cx, JS::HandleObject obj) {
	
		return true;
	}

	void startWait() {

		HANDLE events[] = { consoleEvent, cancelEvent };
		DWORD status = WaitForMultipleObjects(COUNTOF(events), events, FALSE, INFINITE);
		ASSERT( status != WAIT_FAILED );
	}

	bool cancelWait() {

		SetEvent(cancelEvent);
		return true;
	}

	bool endWait(bool *hasEvent, JSContext *cx, JS::HandleObject obj) {

		*hasEvent = WaitForSingleObject(consoleEvent, 0) == WAIT_OBJECT_0;

		JS::RootedObject thisObj(cx, &getSlot(0).toObject());

		if ( *hasEvent ) {
	
			HANDLE hStdin = consoleEvent; //GetStdHandle(STD_INPUT_HANDLE);
			INPUT_RECORD inputRecord;
			DWORD numberOfEventsRead;
			BOOL res = ReadConsoleInput(hStdin, &inputRecord, 1, &numberOfEventsRead);
			if ( res == 0 )
				return WinThrowError(cx, GetLastError());

			if ( numberOfEventsRead > 0 ) {

				JS::RootedValue fct(cx);

				switch ( inputRecord.EventType ) {
					case KEY_EVENT: {

						JL_CHK( jl::getProperty(cx, thisObj, inputRecord.Event.KeyEvent.bKeyDown ? "onKeyDown" : "onKeyUp", &fct) );
						if ( jl::isCallable(cx, fct) ) {

							for ( int i = 0; i < inputRecord.Event.KeyEvent.wRepeatCount; ++i ) {
							
								JL_CHK( jl::callNoRval(cx, thisObj, fct,
									jl::strSpec(&inputRecord.Event.KeyEvent.uChar.UnicodeChar, 1),
									inputRecord.Event.KeyEvent.wVirtualKeyCode,
									!!(inputRecord.Event.KeyEvent.dwControlKeyState & (RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED)),
									!!(inputRecord.Event.KeyEvent.dwControlKeyState & (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED)),
									!!(inputRecord.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)
								) );
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
						#if(_WIN32_WINNT >= 0x0600)
							case MOUSE_HWHEELED:
								eventName = "onMouseHorizontalWheel";
								break;
						#endif /* _WIN32_WINNT >= 0x0600 */
							default:
								eventName = NULL;
							}
						}

						JL_CHK( jl::getProperty(cx, thisObj, eventName, &fct) );
						if ( jl::isCallable(cx, fct) ) {
/*
							argv[1] = INT_TO_JSVAL(inputRecord.Event.MouseEvent.dwMousePosition.X);
							argv[2] = INT_TO_JSVAL(inputRecord.Event.MouseEvent.dwMousePosition.Y);
							argv[3] = INT_TO_JSVAL(inputRecord.Event.MouseEvent.dwButtonState & 0x0000FFFF);
							argv[4] = INT_TO_JSVAL((signed int)inputRecord.Event.MouseEvent.dwButtonState >> 16);
							argv[5] = BOOLEAN_TO_JSVAL(inputRecord.Event.MouseEvent.dwControlKeyState & (RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED));
							argv[6] = BOOLEAN_TO_JSVAL(inputRecord.Event.MouseEvent.dwControlKeyState & (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED));
							argv[7] = BOOLEAN_TO_JSVAL(inputRecord.Event.MouseEvent.dwControlKeyState & SHIFT_PRESSED);
							JL_CHK( JS_CallFunctionValue(cx, thisObj, fct, 7, argv+1, argv) );
*/

							JL_CHK( jl::callNoRval(cx, thisObj, fct,
								inputRecord.Event.MouseEvent.dwMousePosition.X,
								inputRecord.Event.MouseEvent.dwMousePosition.Y,
								inputRecord.Event.MouseEvent.dwButtonState & 0x0000FFFF,
								(signed int)inputRecord.Event.MouseEvent.dwButtonState >> 16,
								inputRecord.Event.MouseEvent.dwControlKeyState & (RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED),
								inputRecord.Event.MouseEvent.dwControlKeyState & (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED),
								inputRecord.Event.MouseEvent.dwControlKeyState & SHIFT_PRESSED
							) );
						}
						break;
					}

					case WINDOW_BUFFER_SIZE_EVENT: {

						JL_CHK( jl::getProperty(cx, thisObj, "onSize", &fct) );
						if ( jl::isCallable(cx, fct) ) {

							JL_CHK( jl::callNoRval(cx, thisObj, fct,
								inputRecord.Event.WindowBufferSizeEvent.dwSize.X,
								inputRecord.Event.WindowBufferSizeEvent.dwSize.Y
							) );
						}
						break;
					}

					case FOCUS_EVENT: {

						JL_CHK( jl::getProperty(cx, thisObj, "onFocus", &fct) );
						if ( jl::isCallable(cx, fct) ) {

							JL_CHK( jl::callNoRval(cx, thisObj, fct, inputRecord.Event.FocusEvent.bSetFocus) );
						}
						break;
					}
				}
			}
		}
		return true;
		JL_BAD;
	}

	~ConsoleProcessEvent() {

		CloseHandle(cancelEvent);
	}
};


DEFINE_FUNCTION( events ) {
	
	JL_DEFINE_ARGS;
		JL_ASSERT_ARGC(0);

	ConsoleProcessEvent *upe = new ConsoleProcessEvent();
	JL_ASSERT_ALLOC(upe);
	JL_CHK( HandleCreate(cx, upe, JL_RVAL) );

	upe->cancelEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // auto-reset
	JL_ASSERT(upe->cancelEvent, E_OS, E_OBJ, E_CREATE, E_COMMENT("cancel event"));
	upe->consoleEvent = GetStdHandle(STD_INPUT_HANDLE);

	return true;
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
	
	JL_IGNORE(strict, id, obj);

	JS::AutoCheckCannotGC nogc;
	jl::BufString str;
	JL_CHK( jl::getValue(cx, vp, &str) );
	SetConsoleTitle(str);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( title ) {

	JL_IGNORE(id, obj);

	TCHAR buffer[2048];
	DWORD res = ::GetConsoleTitle(buffer, COUNTOF(buffer));
	if ( res == 0 )
		return jl::throwOSError(cx);

	JL_CHK( jl::setValue( cx, vp, jl::strSpec( buffer, res ) ) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Get or set the title of the console window.
**/
DEFINE_PROPERTY_SETTER( width ) {

	JL_IGNORE(strict, id, obj);

	BOOL res;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	res = GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	JL_CHK( jl::getValue(cx, vp, &csbiInfo.srWindow.Right) );
	csbiInfo.srWindow.Right += csbiInfo.srWindow.Left - 1;

	// GetLargestConsoleWindowSize

	res = SetConsoleWindowInfo(hStdout, TRUE, &csbiInfo.srWindow);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( width ) {

	JL_IGNORE(id, obj);

	BOOL res;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	res = GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	vp.setInt32(csbiInfo.srWindow.Right - csbiInfo.srWindow.Left + 1);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Get or set the title of the console window.
**/
DEFINE_PROPERTY_SETTER( height ) {

	JL_IGNORE(strict, id, obj);

	BOOL res;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	res = GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	JL_CHK( jl::getValue(cx, vp, &csbiInfo.srWindow.Bottom) );
	csbiInfo.srWindow.Bottom += csbiInfo.srWindow.Top - 1;
	res = SetConsoleWindowInfo(hStdout, TRUE, &csbiInfo.srWindow);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( height ) {

	JL_IGNORE(id, obj);

	BOOL res;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	res = GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	vp.setInt32(csbiInfo.srWindow.Bottom - csbiInfo.srWindow.Top + 1 );
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STR $INAME
**/
DEFINE_PROPERTY_SETTER( textAttribute ) {

	JL_IGNORE(strict, id, obj);

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD attributes;
	JL_CHK( jl::getValue(cx, vp, &attributes) );
	BOOL res = SetConsoleTextAttribute(hStdout, attributes);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( textAttribute ) {

	JL_IGNORE(id, obj);

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	BOOL res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	vp.setInt32(consoleScreenBufferInfo.wAttributes);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
**/
DEFINE_PROPERTY_GETTER( cursorPositionX ) {

	JL_IGNORE(id, obj);

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	BOOL res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	vp.setInt32(consoleScreenBufferInfo.dwCursorPosition.X - consoleScreenBufferInfo.srWindow.Left);
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( cursorPositionX ) {

	JL_IGNORE(strict, id, obj);

	BOOL res;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	SHORT x;
	JL_CHK( jl::getValue(cx, vp, &x) );
	consoleScreenBufferInfo.dwCursorPosition.X = consoleScreenBufferInfo.srWindow.Left + jl::minmax(x, 0, consoleScreenBufferInfo.srWindow.Right - consoleScreenBufferInfo.srWindow.Left);
	res = SetConsoleCursorPosition(hStdout, consoleScreenBufferInfo.dwCursorPosition);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
**/
DEFINE_PROPERTY_GETTER( cursorPositionY ) {

	JL_IGNORE(id, obj);

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	BOOL res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	vp.setInt32(consoleScreenBufferInfo.dwCursorPosition.Y - consoleScreenBufferInfo.srWindow.Top);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( cursorPositionY ) {

	JL_IGNORE(id, obj, strict);

	BOOL res;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	res = GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	SHORT y;
	JL_CHK( jl::getValue(cx, vp, &y) );
	consoleScreenBufferInfo.dwCursorPosition.Y = consoleScreenBufferInfo.srWindow.Top + jl::minmax(y, 0, consoleScreenBufferInfo.srWindow.Bottom - consoleScreenBufferInfo.srWindow.Top);
	res = SetConsoleCursorPosition(hStdout, consoleScreenBufferInfo.dwCursorPosition);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( x, y )
**/
DEFINE_FUNCTION( setCursorPosition ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(2);

	COORD position;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &position.X) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &position.Y) );

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
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $STR $INAME
**/
DEFINE_PROPERTY_GETTER( cursorSize ) {

	JL_IGNORE(id, obj);

	BOOL res;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	res = GetConsoleCursorInfo(hStdout, &cursorInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	vp.setInt32( cursorInfo.bVisible == TRUE ? cursorInfo.dwSize : 0 );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( cursorSize ) {

	JL_IGNORE(strict, id, obj);

	BOOL res;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	res = GetConsoleCursorInfo(hStdout, &cursorInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	DWORD size;
	JL_CHK( jl::getValue(cx, vp, &size) );
	if ( size == 0 ) {

		cursorInfo.bVisible = FALSE;
	} else {

		cursorInfo.bVisible = TRUE;
		cursorInfo.dwSize = size;
	}
	res = SetConsoleCursorInfo(hStdout, &cursorInfo);
	if ( res == 0 )
		return WinThrowError(cx, GetLastError());
	return true;
	JL_BAD;
}



CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))

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

	BEGIN_CONST

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

	END_CONST

END_CLASS

/**doc
=== Examples ===
{{{
var cons = new Console();
cons.title = 'My console';
cons.Write('Hello world');
}}}
**/
