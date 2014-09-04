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

#include <../jslang/imagePub.h>

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3528 $
**/
BEGIN_CLASS( Icon )

/**doc
$TOC_MEMBER $INAME
 $INAME( image | integer )
  Icon constructor accepts an [Image] object or a integer. 
  $LF
  The integer value can be one of:
   * `0`: IDI_APPLICATION
   * `1`: IDI_QUESTION
   * `2`: IDI_INFORMATION
   * `3`: IDI_WARNING
   * `4`: IDI_ERROR
**/
DEFINE_CONSTRUCTOR() {

	HICON *phIcon = NULL;

	JL_DEFINE_ARGS;
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC_MIN(1);

	{

	JS::RootedValue iconVal(cx, JL_ARG(1));

	HICON hIcon = NULL;

	if ( iconVal.isInt32() ) {

		switch ( iconVal.toInt32() ) {
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
	if ( iconVal.isObject() ) {

		HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
		JL_IGNORE(hInst);

		int width, height, channels;
		ImageDataType dataType;
		JS::AutoCheckCannotGC nogc;
		jl::BufString data(JL_GetImageObject(cx, iconVal, &width, &height, &channels, &dataType, nogc)); // source
		JL_ASSERT( !data.isEmpty(), E_ARG, E_NUM(1), E_INVALID );
		JL_ASSERT( dataType == TYPE_UINT8, E_ARG, E_NUM(1), E_DATATYPE, E_INVALID );

		//unsigned char *imageData = (unsigned char*)data.GetConstStr();
		const uint8_t *imageData = data.toData<const uint8_t*>();

		// http://groups.google.com/group/microsoft.public.win32.programmer.gdi/browse_frm/thread/adaf38d715cef81/3825af9edde28cdc?lnk=st&q=RGB+CreateIcon&rnum=9&hl=en#3825af9edde28cdc
		HDC screenDC = GetDC(NULL); // doc: If this value is NULL, GetDC retrieves the DC for the entire screen.
		HDC colorDC = CreateCompatibleDC(screenDC);
		HDC maskDC = CreateCompatibleDC(screenDC);
		HBITMAP colorBMP = CreateCompatibleBitmap(screenDC, width, height);
		HBITMAP maskBMP = CreateCompatibleBitmap(screenDC, width, height);
		HBITMAP oldColorBMP = (HBITMAP)SelectObject(colorDC, colorBMP);
		HBITMAP oldMaskBMP = (HBITMAP)SelectObject(maskDC, maskBMP);

		int x, y;
		for ( x = 0; x < width; x++ )
			for ( y = 0; y < width; y++ ) {

				const uint8_t *offset = imageData + channels*(x + y * width);
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
		JL_ASSERT( hIcon != NULL, E_STR("icon"), E_CREATE );
	}
	
	// FYI. How do I get the dimensions of a cursor or icon? (http://blogs.msdn.com/b/oldnewthing/archive/2010/10/20/10078140.aspx)

	phIcon = (HICON*)JS_malloc(cx, sizeof(HICON)); // this is needed because JL_SetPrivate stores ONLY alligned values
	JL_CHK( phIcon );
	*phIcon = hIcon;

	JL_SetPrivate(JL_OBJ, phIcon);

	}

	return true;

bad:
	if ( phIcon ) {

		if ( *phIcon != NULL )
			DestroyIcon(*phIcon);
		JS_free(cx, phIcon);
	}
	return false;
}


DEFINE_FINALIZE() {

	HICON *phIcon = (HICON*)JL_GetPrivateFromFinalize(obj);
	if ( !phIcon )
		return;

	if ( *phIcon != NULL )
		DestroyIcon(*phIcon);

	JS_freeop(fop, phIcon);
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3528 $"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

END_CLASS
