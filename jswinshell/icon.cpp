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
#include "icon.h"

#include "stdlib.h"

BEGIN_CLASS( Icon )

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING();
	RT_ASSERT_THIS_CLASS();
	RT_ASSERT_ARGC(1);

	jsval iconVal = argv[0];

	HICON hIcon = NULL;

	if ( JSVAL_IS_INT(iconVal) ) {

		switch ( JSVAL_TO_INT(iconVal) ) {
			case 0:
				hIcon = LoadIcon(NULL,MAKEINTRESOURCE(IDI_APPLICATION));
				break;
			case 1:
				hIcon = LoadIcon(NULL,MAKEINTRESOURCE(IDI_QUESTION));
				break;
			case 2:
				hIcon = LoadIcon(NULL,MAKEINTRESOURCE(IDI_INFORMATION));
				break;
			case 3:
				hIcon = LoadIcon(NULL,MAKEINTRESOURCE(IDI_WARNING));
				break;
			case 4:
				hIcon = LoadIcon(NULL,MAKEINTRESOURCE(IDI_ERROR));
				break;
		}
	} else
	if ( JSVAL_IS_OBJECT(iconVal) && iconVal != JSVAL_NULL ) {

		HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
		JSObject *imgObj = JSVAL_TO_OBJECT(iconVal);

		RT_ASSERT_CLASS_NAME(imgObj, "Image"); // (TBD) need something better/safer ?
		jsval tmp;
		JS_GetProperty(cx, imgObj, "width", &tmp);
		RT_ASSERT_INT(tmp);
		int width = JSVAL_TO_INT(tmp);
		JS_GetProperty(cx, imgObj, "height", &tmp);
		RT_ASSERT_INT(tmp);
		int height = JSVAL_TO_INT(tmp);
		JS_GetProperty(cx, imgObj, "channels", &tmp);
		RT_ASSERT_INT(tmp);
		int channels = JSVAL_TO_INT(tmp);
		unsigned char *imageData = (unsigned char*)JS_GetPrivate(cx, imgObj);

		// http://groups.google.com/group/microsoft.public.win32.programmer.gdi/browse_frm/thread/adaf38d715cef81/3825af9edde28cdc?lnk=st&q=RGB+CreateIcon&rnum=9&hl=en#3825af9edde28cdc
		HDC screenDC = GetDC(NULL); // doc: If this value is NULL, GetDC retrieves the DC for the entire screen.
		HDC colorDC = CreateCompatibleDC(screenDC);
		HDC maskDC = CreateCompatibleDC(screenDC);
		HBITMAP colorBMP = CreateCompatibleBitmap(screenDC, width, height);
		HBITMAP maskBMP = CreateCompatibleBitmap(screenDC, width, height);
		HBITMAP oldColorBMP = (HBITMAP)SelectObject(colorDC, colorBMP);
		HBITMAP oldMaskBMP = (HBITMAP)SelectObject(maskDC, maskBMP);

		for ( int x = 0; x < width; x++ )
			for ( int y = 0; y < width; y++ ) {

				unsigned char *offset = imageData + channels*(x + y * width);
				SetPixel(colorDC, x,y, RGB(offset[0],offset[1],offset[2]) );
				if ( channels == 4 ) // image has alpha channel ?
					SetPixel(maskDC, x,y, RGB( 255-offset[3], 255-offset[3], 255-offset[3] ) );
				else
					SetPixel(maskDC, x,y, RGB(0,0,0) );
			}

		SelectObject(colorDC, oldColorBMP); 
		SelectObject(maskDC, oldMaskBMP);
		DeleteDC(colorDC);
		DeleteDC(maskDC);
		ReleaseDC(NULL, screenDC);

		ICONINFO ii = { TRUE, 0, 0, maskBMP, colorBMP };
		hIcon = CreateIconIndirect( &ii );
		DeleteObject(colorBMP);
		DeleteObject(maskBMP); 
		RT_ASSERT( hIcon != NULL, "Unable to create the icon." );
	}

	HICON *phIcon = (HICON*)malloc(sizeof(HICON)); // this is needed because JS_SetPrivate stores ONLY alligned values
	RT_ASSERT_ALLOC( phIcon );
	*phIcon = hIcon;
	JS_SetPrivate(cx, obj, phIcon);
	return JS_TRUE;
}

DEFINE_FINALIZE() {

	HICON *phIcon = (HICON*)JS_GetPrivate(cx, obj);
	if ( phIcon != NULL ) {

		if ( *phIcon != NULL )
			DestroyIcon(*phIcon);

		free(phIcon);
	}
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
