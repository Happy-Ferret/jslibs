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
**/
BEGIN_CLASS( Sdl ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.
}

DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

//	J_S_ASSERT_ARG_MIN(3);

	int width, height, bpp;
	double flags;

	if ( J_ARG_ISDEF(1) )
		J_CHK( JsvalToInt(cx, J_ARG(1), &width) );
	else
		width = 0; // by default, use current width

	if ( J_ARG_ISDEF(2) )
		J_CHK( JsvalToInt(cx, J_ARG(2), &height) );
	else
		height = 0; // by default, use current height

	if ( J_ARG_ISDEF(3) )
		J_CHK( JsvalToInt(cx, J_ARG(3), &bpp) );
	else
		bpp = 0; // by default, use current bpp

	if ( J_ARG_ISDEF(4) )
		J_CHK( JsvalToDouble(cx, J_ARG(4), &flags) );
	else
		flags = SDL_HWSURFACE | SDL_FULLSCREEN;

	SDL_Surface *surface = SDL_SetVideoMode(width, height, bpp, (Uint32)flags);

	if ( surface == NULL )
		return ThrowSdlError(cx);

	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( SetIcon ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_OBJECT( J_FARG(1) );
	
	jsval image = J_FARG(1);

	JSObject *imageObj = JSVAL_TO_OBJECT( image );
	int sWidth, sHeight, sChannels;
	J_CHK( GetPropertyInt(cx, imageObj, "width", &sWidth) );
	J_CHK( GetPropertyInt(cx, imageObj, "height", &sHeight) );
	J_CHK( GetPropertyInt(cx, imageObj, "channels", &sChannels) );
	
	J_S_ASSERT( sChannels == 3 || sChannels == 4, "Unsupported image format." );

	const char *sBuffer;
	size_t bufferLength;
	J_CHK( JsvalToStringAndLength(cx, &image, &sBuffer, &bufferLength ) ); // warning: GC on the returned buffer !

	J_S_ASSERT( bufferLength == sWidth * sHeight * sChannels * 1, "Invalid image format." );

	Uint32 rmask, gmask, bmask, amask;

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		 rmask = 0xff000000;
		 gmask = 0x00ff0000;
		 bmask = 0x0000ff00;
		 amask = 0x000000ff;
	#else
		 rmask = 0x000000ff;
		 gmask = 0x0000ff00;
		 bmask = 0x00ff0000;
		 amask = 0xff000000;
	#endif

	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void*)sBuffer, sWidth, sHeight, 8 * sChannels, sWidth * sChannels, rmask, gmask, bmask, amask);

	if ( surface == NULL )
		return ThrowSdlError(cx);

	SDL_WM_SetIcon(surface, NULL);
	SDL_FreeSurface(surface);
	return JS_TRUE;
}



DEFINE_FUNCTION_FAST( SwapGlBuffers ) {

	SDL_GL_SwapBuffers();
	return JS_TRUE;
}


DEFINE_PROPERTY_SETTER( glDoubleBuffer ) {

	bool db;
	J_CHK( JsvalToBool(cx, *vp, &db) );
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, db ? 1 : 0);
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( glDoubleBuffer ) {

	int db;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &db);
	J_CHK( BoolToJsval(cx, db != 0, vp) );
	return JS_TRUE;
}


DEFINE_INIT() {

	SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
	return JS_TRUE;
}

CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

	HAS_INIT
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST(SwapGlBuffers)
		FUNCTION_FAST(SetIcon)
	END_STATIC_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( glDoubleBuffer )
	END_PROPERTY_SPEC

	BEGIN_CONST_DOUBLE_SPEC
		CONST_DOUBLE( SWSURFACE	 ,SDL_SWSURFACE  )
		CONST_DOUBLE( HWSURFACE	 ,SDL_HWSURFACE  )
		CONST_DOUBLE( ASYNCBLIT	 ,SDL_ASYNCBLIT  )
		CONST_DOUBLE( ANYFORMAT	 ,SDL_ANYFORMAT  )
		CONST_DOUBLE( HWPALETTE	 ,SDL_HWPALETTE  )
		CONST_DOUBLE( DOUBLEBUF	 ,SDL_DOUBLEBUF  )
		CONST_DOUBLE( FULLSCREEN ,SDL_FULLSCREEN )
		CONST_DOUBLE( OPENGL		 ,SDL_OPENGL     )
		CONST_DOUBLE( OPENGLBLIT ,SDL_OPENGLBLIT )
		CONST_DOUBLE( RESIZABLE	 ,SDL_RESIZABLE  )
		CONST_DOUBLE( NOFRAME	 ,SDL_NOFRAME    )
		CONST_DOUBLE( HWACCEL	 ,SDL_HWACCEL    )
		CONST_DOUBLE( SRCCOLORKEY,SDL_SRCCOLORKEY)
		CONST_DOUBLE( RLEACCELOK ,SDL_RLEACCELOK )
		CONST_DOUBLE( RLEACCEL	 ,SDL_RLEACCEL   )
		CONST_DOUBLE( SRCALPHA	 ,SDL_SRCALPHA   )
		CONST_DOUBLE( PREALLOC	 ,SDL_PREALLOC   )
	END_CONST_DOUBLE_SPEC

END_CLASS
