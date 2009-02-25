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
#include "sdl.h"

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Cursor )

DEFINE_FINALIZE() {

	SDL_Cursor *cursor = (SDL_Cursor*)JS_GetPrivate(cx, obj);
	if ( cursor != NULL ) {

		SDL_FreeCursor(cursor); // default cursor is restored
	}
}


/**doc
 * $INAME( image )
  Constructs a new B/W Cursor object. using a RGB or RGBA image.
  Only the red component is used for B/W (<128: black, >= 128: white)
  The Alpha component is used to set transparents pixels (alpha == 0) an inverted pixels (alpha == 255).
  $H arguments
   $ARG ImageObject image
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_OBJECT( J_ARG(1) );
	
	JSObject *imageObj = JSVAL_TO_OBJECT( J_ARG(1) );
	int sWidth, sHeight, sChannels;
	J_CHK( GetPropertyInt(cx, imageObj, "width", &sWidth) );
	J_CHK( GetPropertyInt(cx, imageObj, "height", &sHeight) );
	J_CHK( GetPropertyInt(cx, imageObj, "channels", &sChannels) );

	const unsigned char *sBuffer;
	size_t bufferLength;
	J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), (const char**)&sBuffer, &bufferLength ) ); // warning: GC on the returned buffer !

	J_S_ASSERT( bufferLength == sWidth * sHeight * sChannels * 1, "Invalid image format." );

	J_S_ASSERT( sWidth % 8 == 0, "The cursor width must be a multiple of 8.");
	J_S_ASSERT( sChannels == 3 || sChannels == 4, "Invalid image format (need RGB or RGBA).");

	int length = sWidth * sHeight;

	int cursorDataLength = 2*sHeight*sWidth/8;
	unsigned char *cursorImage = (unsigned char *)malloc(cursorDataLength); // data + mask
	unsigned char *cursorMask = cursorImage + sHeight*sWidth/8;
	memset(cursorImage, 0, cursorDataLength);
	
	// data  mask    resulting pixel on screen
	//  0     1       White
	//  1     1       Black
	//  0     0       Transparent
	//  1     0       Inverted color if possible, black if not.
	
	for ( int i = 0; i < length; i++ ) {
	
		unsigned char bit = 0x80 >> (i % 8);
		if ( sBuffer[i*4 + 0] < 128 )
			cursorImage[i/8] |= bit;

		// Transparent/Inverted if no alpha
		if ( sChannels == 4 && sBuffer[i*4 + 3] == 0 )
			cursorMask[i/8] |= bit;
	}

	int hotX, hotY;

	if ( J_ARG_ISDEF(2) )
		J_CHK( JsvalToInt(cx, J_ARG(2), &hotX) );
	else
		hotX = 0;
	
	if ( J_ARG_ISDEF(3) )
		J_CHK( JsvalToInt(cx, J_ARG(3), &hotY) );
	else
		hotY = 0;
		

	SDL_Cursor *cursor = SDL_CreateCursor(cursorImage, cursorMask, sWidth, sHeight, hotX, hotY);
	if ( cursor == NULL ) {

		free(cursorImage);
		return ThrowSdlError(cx);
	}

	J_CHK( JS_SetPrivate(cx, obj, cursor) );

	free(cursorImage);
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

END_CLASS
