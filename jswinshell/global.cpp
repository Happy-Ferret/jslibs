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


BEGIN_STATIC


DEFINE_FUNCTION( _MessageBox ) {
	
	RT_ASSERT_ARGC(1);

	char *text;
	RT_JSVAL_TO_STRING( argv[0], text );

	char *caption = NULL;
	if ( argc >= 2 )
		RT_JSVAL_TO_STRING( argv[1], caption );
	
	UINT type = 0;
	if ( argc >= 3 )
		RT_JSVAL_TO_INT32( argv[2], type );

	MessageBox(NULL, text, caption, type );

	return JS_TRUE;
}


DEFINE_PROPERTY( clipboardGetter ) {

	BOOL res = OpenClipboard(NULL);
	RT_ASSERT( res != 0, "Unable to open the clipboard." );
	if ( IsClipboardFormatAvailable(CF_TEXT) == 0 ) {

		*vp = JSVAL_NULL;
	} else {

		HANDLE hglb = GetClipboardData(CF_TEXT);
		RT_ASSERT_RESOURCE( hglb );
		LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
		RT_ASSERT( lptstr != NULL, "Unable to lock memory." );
		JSString *str = JS_NewStringCopyZ(cx, lptstr);
		RT_ASSERT( str != NULL, "Unable to create the string.");
		*vp = STRING_TO_JSVAL(str);
		GlobalUnlock(hglb);
		CloseClipboard();
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( clipboardSetter ) {

	BOOL res = OpenClipboard(NULL);
	RT_ASSERT( res != 0, "Unable to open the clipboard." );
	EmptyClipboard(); // doc: If the application specifies a NULL window handle when opening the clipboard, EmptyClipboard succeeds but sets the clipboard owner to NULL. Note that this causes SetClipboardData to fail.
	CloseClipboard();

	if ( *vp != JSVAL_VOID ) {

		res = OpenClipboard(NULL);
		RT_ASSERT( res != 0, "Unable to open the clipboard." );
		char *str;
		int len;
		RT_JSVAL_TO_STRING_AND_LENGTH( *vp, str, len );
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, len + 1);
		RT_ASSERT_ALLOC( hglbCopy );
		LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 
		RT_ASSERT( lptstrCopy != NULL, "Unable to lock memory." );
		memcpy(lptstrCopy, str, len + 1);
		lptstrCopy[len] = 0;
		GlobalUnlock(hglbCopy);
		HANDLE h = SetClipboardData(CF_TEXT,hglbCopy);
		RT_ASSERT( h != NULL, "Unable to SetClipboardData." );
		CloseClipboard();
	}
	return JS_TRUE;
}





CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION2( MessageBox, _MessageBox )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY( clipboard )
	END_STATIC_PROPERTY_SPEC

END_STATIC
