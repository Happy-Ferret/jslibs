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
#include "sdl.h"

#include "error.h"

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3508 $
**/
BEGIN_CLASS( Cursor )

DEFINE_FINALIZE() {

	JL_IGNORE(fop);

	SDL_Cursor *cursor = (SDL_Cursor*)JL_GetPrivate(obj);
	if ( cursor != NULL ) {

		SDL_FreeCursor(cursor); // default cursor is restored
	}
}


/**doc
$TOC_MEMBER $INAME
 $INAME( image )
  Constructs a new B/W Cursor object. using a RGB or RGBA image.
  Only the red component is used for B/W (<128: black, >= 128: white)
  The Alpha component is used to set transparents pixels (alpha == 0) an inverted pixels (alpha == 255).
  $H beware
   cursor width must be a multiple of 8.
  $H arguments
   $ARG ImageObject image
**/
DEFINE_CONSTRUCTOR() {

	SDL_Cursor *cursor = NULL;
	JLData data;

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_OBJECT(1);
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;
	
	int sWidth, sHeight, sChannels;
	data = JL_GetByteImageObject(cx, JL_ARG(1), &sWidth, &sHeight, &sChannels);
	const uint8_t *sBuffer = (const unsigned char*)data.GetConstStr();

	JL_ASSERT( sWidth % 8 == 0, E_ARG, E_NUM(1), E_FORMAT ); // "The cursor width must be a multiple of 8."
	JL_ASSERT( sChannels == 3 || sChannels == 4, E_PARAM, E_STR("channels"), E_RANGE, E_INTERVAL_NUM(3, 4) );

	int length = sWidth * sHeight;

	int cursorDataLength = 2*sHeight*sWidth/8;
	unsigned char *cursorImage = (unsigned char *)jl_calloc(cursorDataLength, 1); // data + mask
	unsigned char *cursorMask = cursorImage + sHeight*sWidth/8;
//	memset(cursorImage, 0, cursorDataLength); see calloc

	JL_updateMallocCounter(cx, cursorDataLength);
	
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

	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &hotX) );
	else
		hotX = 0;
	
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &hotY) );
	else
		hotY = 0;
		

	cursor = SDL_CreateCursor(cursorImage, cursorMask, sWidth, sHeight, hotX, hotY);
	if ( cursor == NULL ) {

		jl_free(cursorImage);
		return ThrowSdlError(cx);
	}

	jl_free(cursorImage);

	JL_SetPrivate(obj, cursor);
	return JS_TRUE;

bad:
	if ( cursor )
		SDL_FreeCursor(cursor);
	return JS_FALSE;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3508 $"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

END_CLASS
