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
#include "systray.h"
#include <stdlib.h>

/*
typedef struct
{
    BYTE        bWidth;          // Width, in pixels, of the image
    BYTE        bHeight;         // Height, in pixels, of the image
    BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
    BYTE        bReserved;       // Reserved ( must be 0)
    WORD        wPlanes;         // Color Planes
    WORD        wBitCount;       // Bits per pixel
    DWORD       dwBytesInRes;    // How many bytes in this resource?
    DWORD       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
    DWORD	dwBytes;
    DWORD	dwOffset;
} RESOURCEPOSINFO, *LPRESOURCEPOSINFO;

typedef struct
{
	UINT			Width, Height, Colors; // Width, Height and bpp
	LPBYTE			lpBits;                // ptr to DIB bits
	DWORD			dwNumBytes;            // how many bytes?
	LPBITMAPINFO	lpbi;                  // ptr to header
	LPBYTE			lpXOR;                 // ptr to XOR image bits
	LPBYTE			lpAND;                 // ptr to AND image bits
} ICONIMAGE, *LPICONIMAGE;

typedef struct
{
    BOOL        bHasChanged;                     // Has image changed?
    TCHAR       szOriginalICOFileName[MAX_PATH]; // Original name
    TCHAR       szOriginalDLLFileName[MAX_PATH]; // Original name
    UINT        nNumImages;                      // How many images?
    ICONIMAGE   IconImages[1];                   // Image entries
} ICONRESOURCE, *LPICONRESOURCE;

#define WIDTHBYTES(bits)      ((((bits) + 31)>>5)<<2)

DWORD BytesPerLine( LPBITMAPINFOHEADER lpBMIH )
{
    return WIDTHBYTES(lpBMIH->biWidth * lpBMIH->biPlanes * lpBMIH->biBitCount);
}

WORD DIBNumColors( LPSTR lpbi )
{
    WORD wBitCount;
    DWORD dwClrUsed;

    dwClrUsed = ((LPBITMAPINFOHEADER) lpbi)->biClrUsed;

    if (dwClrUsed)
        return (WORD) dwClrUsed;

    wBitCount = ((LPBITMAPINFOHEADER) lpbi)->biBitCount;

    switch (wBitCount)
    {
        case 1: return 2;
        case 4: return 16;
        case 8:	return 256;
        default:return 0;
    }
    return 0;
}

WORD PaletteSize( LPSTR lpbi )
{
    return ( DIBNumColors( lpbi ) * sizeof( RGBQUAD ) );
}

LPSTR FindDIBBits( LPSTR lpbi )
{
   return ( lpbi + *(LPDWORD)lpbi + PaletteSize( lpbi ) );
}


BOOL AdjustIconImagePointers( LPICONIMAGE lpImage )
{
    // Sanity check
    if( lpImage==NULL )
        return FALSE;
    // BITMAPINFO is at beginning of bits
    lpImage->lpbi = (LPBITMAPINFO)lpImage->lpBits;
    // Width - simple enough
    lpImage->Width = lpImage->lpbi->bmiHeader.biWidth;
    // Icons are stored in funky format where height is doubled - account for it
    lpImage->Height = (lpImage->lpbi->bmiHeader.biHeight)/2;
    // How many colors?
    lpImage->Colors = lpImage->lpbi->bmiHeader.biPlanes * lpImage->lpbi->bmiHeader.biBitCount;
    // XOR bits follow the header and color table
    lpImage->lpXOR = (LPBYTE)FindDIBBits((LPSTR)(lpImage->lpbi));
    // AND bits follow the XOR bits
    lpImage->lpAND = lpImage->lpXOR + (lpImage->Height*BytesPerLine((LPBITMAPINFOHEADER)(lpImage->lpbi)));
    return TRUE;
}


UINT ReadICOHeader( HANDLE hFile )
{
    WORD    Input;
    DWORD	dwBytesRead;

    // Read the 'reserved' WORD
    if( ! ReadFile( hFile, &Input, sizeof( WORD ), &dwBytesRead, NULL ) )
        return (UINT)-1;
    // Did we get a WORD?
    if( dwBytesRead != sizeof( WORD ) )
        return (UINT)-1;
    // Was it 'reserved' ?   (ie 0)
    if( Input != 0 )
        return (UINT)-1;
    // Read the type WORD
    if( ! ReadFile( hFile, &Input, sizeof( WORD ), &dwBytesRead, NULL ) )
        return (UINT)-1;
    // Did we get a WORD?
    if( dwBytesRead != sizeof( WORD ) )
        return (UINT)-1;
    // Was it type 1?
    if( Input != 1 )
        return (UINT)-1;
    // Get the count of images
    if( ! ReadFile( hFile, &Input, sizeof( WORD ), &dwBytesRead, NULL ) )
        return (UINT)-1;
    // Did we get a WORD?
    if( dwBytesRead != sizeof( WORD ) )
        return (UINT)-1;
    // Return the count
    return Input;
}

LPICONRESOURCE ReadIconFromICOFile( LPCTSTR szFileName )
{
    LPICONRESOURCE    	lpIR = NULL, lpNew = NULL;
    HANDLE            	hFile = NULL;
    LPRESOURCEPOSINFO	lpRPI = NULL;
    UINT                i;
    DWORD            	dwBytesRead;
    LPICONDIRENTRY    	lpIDE = NULL;


    // Open the file
    if( (hFile = CreateFile( szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL )) == INVALID_HANDLE_VALUE )
    {
        //MessageBox( hWndMain, "Error Opening File for Reading", szFileName, MB_OK );
        return NULL;
    }
    // Allocate memory for the resource structure
    if( (lpIR = (LPICONRESOURCE)malloc( sizeof(ICONRESOURCE) )) == NULL )
    {
        //MessageBox( hWndMain, "Error Allocating Memory", szFileName, MB_OK );
        CloseHandle( hFile );
        return NULL;
    }
    // Read in the header
    if( (lpIR->nNumImages = ReadICOHeader( hFile )) == (UINT)-1 )
    {
        //MessageBox( hWndMain, "Error Reading File Header", szFileName, MB_OK );
        CloseHandle( hFile );
        free( lpIR );
        return NULL;
    }
    // Adjust the size of the struct to account for the images
    if( (lpNew = (LPICONRESOURCE)realloc( lpIR, sizeof(ICONRESOURCE) + ((lpIR->nNumImages-1) * sizeof(ICONIMAGE)) )) == NULL )
    {
        //MessageBox( hWndMain, "Error Allocating Memory", szFileName, MB_OK );
        CloseHandle( hFile );
        free( lpIR );
        return NULL;
    }
    lpIR = lpNew;
    // Store the original name
    lstrcpy( lpIR->szOriginalICOFileName, szFileName );
    lstrcpy( lpIR->szOriginalDLLFileName, L"" );
    // Allocate enough memory for the icon directory entries
    if( (lpIDE = (LPICONDIRENTRY)malloc( lpIR->nNumImages * sizeof( ICONDIRENTRY ) ) ) == NULL )
    {
        //MessageBox( hWndMain, "Error Allocating Memory", szFileName, MB_OK );
        CloseHandle( hFile );
        free( lpIR );
        return NULL;
    }
    // Read in the icon directory entries
    if( ! ReadFile( hFile, lpIDE, lpIR->nNumImages * sizeof( ICONDIRENTRY ), &dwBytesRead, NULL ) )
    {
        //MessageBox( hWndMain, "Error Reading File", szFileName, MB_OK );
        CloseHandle( hFile );
        free( lpIR );
        return NULL;
    }
    if( dwBytesRead != lpIR->nNumImages * sizeof( ICONDIRENTRY ) )
    {
        //MessageBox( hWndMain, "Error Reading File", szFileName, MB_OK );
        CloseHandle( hFile );
        free( lpIR );
        return NULL;
    }
    // Loop through and read in each image
    for( i = 0; i < lpIR->nNumImages; i++ )
    {
        // Allocate memory for the resource
        if( (lpIR->IconImages[i].lpBits = (LPBYTE)malloc(lpIDE[i].dwBytesInRes)) == NULL )
        {
            //MessageBox( hWndMain, "Error Allocating Memory", szFileName, MB_OK );
            CloseHandle( hFile );
            free( lpIR );
            free( lpIDE );
            return NULL;
        }
        lpIR->IconImages[i].dwNumBytes = lpIDE[i].dwBytesInRes;
        // Seek to beginning of this image
        if( SetFilePointer( hFile, lpIDE[i].dwImageOffset, NULL, FILE_BEGIN ) == 0xFFFFFFFF )
        {
            //MessageBox( hWndMain, "Error Seeking in File", szFileName, MB_OK );
            CloseHandle( hFile );
            free( lpIR );
            free( lpIDE );
            return NULL;
        }
        // Read it in
        if( ! ReadFile( hFile, lpIR->IconImages[i].lpBits, lpIDE[i].dwBytesInRes, &dwBytesRead, NULL ) )
        {
            //MessageBox( hWndMain, "Error Reading File", szFileName, MB_OK );
            CloseHandle( hFile );
            free( lpIR );
            free( lpIDE );
            return NULL;
        }
        if( dwBytesRead != lpIDE[i].dwBytesInRes )
        {
            //MessageBox( hWndMain, "Error Reading File", szFileName, MB_OK );
            CloseHandle( hFile );
            free( lpIDE );
            free( lpIR );
            return NULL;
        }
        // Set the internal pointers appropriately
        if( ! AdjustIconImagePointers( &(lpIR->IconImages[i]) ) )
        {
            //MessageBox( hWndMain, "Error Converting to Internal Format", szFileName, MB_OK );
            CloseHandle( hFile );
            free( lpIDE );
            free( lpIR );
            return NULL;
        }
    }
    // Clean up
    free( lpIDE );
    free( lpRPI );
    CloseHandle( hFile );
    return lpIR;
}
*/


/*
static LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	return DefWindowProc(hWnd, message, wParam, lParam); // We do not want to handle this message so pass back to Windows to handle it in a default way
}
*/

BEGIN_CLASS( Systray )

DEFINE_FINALIZE() {

	NOTIFYICONDATA *nid = (NOTIFYICONDATA*)JS_GetPrivate(cx, obj);
	if ( nid != NULL ) {

		BOOL status = Shell_NotifyIcon(NIM_DELETE, nid);
//		RT_ASSERT( status == TRUE, "Unable to setup systray icon." );

	}
}

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(_class);

	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	RT_ASSERT( hInst != NULL, "Unable to GetModuleHandle." );
	WNDCLASS wc = { 0, (WNDPROC)DefWindowProc, 0, 0, hInst, NULL, NULL, NULL, NULL, L"systray" };
	ATOM rc = RegisterClass(&wc);
	RT_ASSERT( rc != 0, "Unable to RegisterClass." );
	HWND hWnd = CreateWindow( (LPCWSTR)rc, NULL, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (HWND)NULL, (HMENU)NULL, hInst, (LPVOID)NULL );
	RT_ASSERT( hWnd != NULL, "Unable to CreateWindow." );
//	JS_SetPrivate(cx, obj, hWnd);

	NOTIFYICONDATA *nid = (NOTIFYICONDATA*)malloc( sizeof(NOTIFYICONDATA) );
	memset(nid, 0, sizeof(NOTIFYICONDATA));
	nid->cbSize = sizeof(NOTIFYICONDATA);
	nid->hWnd = hWnd;
	nid->uID = (12) + 0; // doc: Values from 0 to 12 are reserved and should not be used.
	nid->uFlags = NIF_MESSAGE | NIF_TIP; // | NIF_ICON
	nid->uCallbackMessage = (WM_USER) + 0; // doc: All Message Numbers below 0x0400 are RESERVED.
	nid->hIcon = LoadIcon(NULL,MAKEINTRESOURCE(IDI_APPLICATION));
	wcscpy( nid->szTip, L"text" );

//SHGetFileInfo

//	ReadIconFromICOFile(
//	LookUpIconIdFromDirectory;

	BOOL status = Shell_NotifyIcon(NIM_ADD, nid);
	RT_ASSERT( status == TRUE, "Unable to setup systray icon." );

	JS_SetPrivate(cx, obj, (void*)nid);

//	Shell_NotifyIcon(NIM_MODIFY, &nid);
//	Shell_NotifyIcon(NIM_DELETE, &nid);

// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/structures/notifyicondata.asp
//	_tcscpy(nid.szTip, "tooltip");

	return JS_TRUE;
}

DEFINE_PROPERTY( icon ) {

	JSObject *imgObj = JSVAL_TO_OBJECT(*vp);

	RT_ASSERT_CLASS_NAME(imgObj, "Image");
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);

	jsval tmp;
	JS_GetProperty(cx, imgObj, "width", &tmp);
	RT_ASSERT(JSVAL_IS_INT(tmp), RT_ERROR_UNEXPECTED_TYPE);
	int width = JSVAL_TO_INT(tmp);

	JS_GetProperty(cx, imgObj, "height", &tmp);
	RT_ASSERT(JSVAL_IS_INT(tmp), RT_ERROR_UNEXPECTED_TYPE);
	int height = JSVAL_TO_INT(tmp);

	JS_GetProperty(cx, imgObj, "channels", &tmp);
	RT_ASSERT(JSVAL_IS_INT(tmp), RT_ERROR_UNEXPECTED_TYPE);
	int channels = JSVAL_TO_INT(tmp);

	char *imageData = (char*)JS_GetPrivate(cx, imgObj);

	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JS_GetPrivate(cx, obj);
	nid->uFlags |= NIF_ICON;

//	void* mask = malloc( width * height * channels );
//	RT_ASSERT( mask != NULL, "Unable to create the icon mask." );
//	memset(mask, 127, width * height * channels );

	// http://www.we11er.co.uk/
	// To determine the width and height supported by the display driver, use the GetSystemMetrics function, specifying the SM_CXICON or SM_CYICON value.



/*
	char *bitmap = (char*)malloc( width * height * 4 );
	RT_ASSERT( bitmap != NULL, "Unable to create the icon bitmap." );
	for ( int i = 0; i < width * height; i++ ) {

		//b, g, r, ?
		bitmap[i*4 + 0] = imageData[i*3 + 2];
		bitmap[i*4 + 1] = imageData[i*3 + 1];
		bitmap[i*4 + 2] = imageData[i*3 + 0];
		bitmap[i*4 + 3] = 0;
	}
	HICON icon = CreateIcon( hInst, width, height, 4, 8, (const BYTE *)NULL, (const BYTE *)bitmap );
*/
CreateIcon

/*
	BITMAP bm;
	bm.bmType = 0; // This member must be zero.
	bm.bmWidth = width;
	bm.bmHeight = height;
	bm.bmWidthBytes = width * channels;
	bm.bmPlanes = channels;
	bm.bmBitsPixel = 3*8; // 24
	bm.bmBits = imageData;
	CreateBitmapIndirect( &bm );
*/

	BITMAPV4HEADER bh;
	bh.bV4Size = sizeof(BITMAPV4HEADER);
	bh.bV4Width = width;
	bh.bV4Height = height;
	bh.bV4Planes = 1; // doc: This value must be set to 1.
	bh.bV4BitCount = channels * 8;
	bh.bV4V4Compression = BI_BITFIELDS; // BI_RGB; // An uncompressed format.
	bh.bV4SizeImage = 0; // This may be set to zero for BI_RGB bitmaps.
	bh.bV4XPelsPerMeter = 0;
	bh.bV4YPelsPerMeter = 0;
	bh.bV4ClrUsed = 0; // doc: If this value is zero, the bitmap uses the maximum number of colors corresponding to the value of the bV5BitCount member for the compression mode specified by bV5Compression.
	bh.bV4ClrImportant = 0; // If this value is zero, all colors are required.
	bh.bV4RedMask = 0xff000000;
	bh.bV4GreenMask = 0x00ff0000;
	bh.bV4BlueMask = 0x0000ff00;
	bh.bV4AlphaMask = 0x000000ff;
	bh.bV4CSType = LCS_WINDOWS_COLOR_SPACE;
//	bh.bV5Endpoints = 0; // doc: This member is ignored unless the bV5CSType member specifies LCS_CALIBRATED_RGB.
	bh.bV4GammaRed = 0;
	bh.bV4GammaGreen = 0;
	bh.bV4GammaBlue = 0;

	HBITMAP bitmap = CreateDIBitmap( GetDC(NULL), (PBITMAPINFOHEADER)&bh, CBM_INIT, imageData, &bi, DIB_RGB_COLORS );

/*
	BITMAPINFO bi;
	bi.bmiHeader.
	HBITMAP bitmap = CreateCompatibleBitmap( GetDC(NULL), width, height ); // doc: When you no longer need the bitmap, call the DeleteObject function to delete it.
	SetDIBits( GetDC(NULL), bitmap, 0, height, imageData, &bi, DIB_RGB_COLORS );
*/


	ICONINFO ii;
	ii.fIcon = TRUE;
	ii.xHotspot = 0;
	ii.yHotspot = 0;
	ii.hbmMask = NULL;
	ii.hbmColor = bitmap;// HBITMAP
	HICON icon = CreateIconIndirect( &ii );

// Device-Independent Bitmaps
//   http://msdn.microsoft.com/library/default.asp?url=/library/en-us/gdi/bitmaps_9c6r.asp

//	CreateBitmap( width, height, channels, 8, (const BYTE *)imageData );


//	free(mask);

	RT_ASSERT( icon != NULL, "Unable to create the icon." );
	nid->hIcon = icon;
	BOOL status = Shell_NotifyIcon(NIM_MODIFY, nid);
	RT_ASSERT( status == TRUE, "Unable to setup systray icon." );

	//DestroyIcon // doc: Before closing, your application must use DestroyIcon to destroy any icon it created by using CreateIconIndirect. It is not necessary to destroy icons created by other functions.

	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

//DEFINE_FUNCTION( Call ) {
//	return JS_TRUE;
//}



//DEFINE_FUNCTION( Func ) {
//	return JS_TRUE;
//}

	BEGIN_FUNCTION_SPEC
//		FUNCTION(Func)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE(icon) // (TBD) _STORE ? is needed to keep the reference to the image ( aboid GC ) ???
	END_PROPERTY_SPEC

	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

END_CLASS

/*


http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/functions/shell_notifyicon.asp


Icons in Win32:

http://msdn2.microsoft.com/en-us/library/ms997538.aspx


*/
