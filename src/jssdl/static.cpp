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

#include "static.h"
#include "error.h"

DECLARE_CLASS( Cursor )

/**doc fileIndex:topmost **/

BEGIN_STATIC


DEFINE_FUNCTION_FAST( GetVideoModeList ) {

	J_S_ASSERT_ARG_MIN(2);

	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo(); // If called before SDL_SetVideoMode(), 'vfmt' is the pixel format of the "best" video mode.
	SDL_PixelFormat format = *videoInfo->vfmt;

	double flags;

	if ( J_FARG_ISDEF(1) ) {
		
		unsigned int bpp;
		J_CHK( JsvalToUInt(cx, J_FARG(1), &bpp) );
		format.BitsPerPixel = bpp;
		format.BytesPerPixel = bpp / 8;
	}

	if ( J_FARG_ISDEF(2) )
		J_CHK( JsvalToDouble(cx, J_FARG(2), &flags) );
	else
		flags = 0;

	SDL_Rect **modes = SDL_ListModes(&format, (Uint32)flags);

	if ( modes == (SDL_Rect **)-1 ) {
		
		// any dimension is okay for this format.
		*J_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	JSObject *modesArray = JS_NewArrayObject(cx, 0, NULL);
	J_S_ASSERT_ALLOC(modesArray);
	*J_FRVAL = OBJECT_TO_JSVAL( modesArray );

	jsval tmp;
	for ( int i = 0; modes[i] != NULL; i++ ) {
	
		JSObject *rectArray = JS_NewArrayObject(cx, 2, NULL);
		J_S_ASSERT_ALLOC(rectArray);

		tmp = OBJECT_TO_JSVAL(rectArray);
		J_CHK( JS_SetElement(cx, modesArray, i, &tmp) );

		tmp = INT_TO_JSVAL(modes[i]->w);
		J_CHK( JS_SetElement(cx, rectArray, 0, &tmp) );

		tmp = INT_TO_JSVAL(modes[i]->h);
		J_CHK( JS_SetElement(cx, rectArray, 1, &tmp) );
	}
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( HasVideoMode ) {

	int width, height, bpp;
	double flags;

	if ( J_FARG_ISDEF(1) )
		J_CHK( JsvalToInt(cx, J_FARG(1), &width) );
	else
		width = 0; // by default, use current width

	if ( J_FARG_ISDEF(2) )
		J_CHK( JsvalToInt(cx, J_FARG(2), &height) );
	else
		height = 0; // by default, use current height

	if ( J_FARG_ISDEF(3) )
		J_CHK( JsvalToInt(cx, J_FARG(3), &bpp) );
	else
		bpp = 0; // by default, use current bpp

	if ( J_FARG_ISDEF(4) )
		J_CHK( JsvalToDouble(cx, J_FARG(4), &flags) );
	else
		flags = 0;

	int status = SDL_VideoModeOK(width, height, bpp, (Uint32)flags);
	J_CHK( BoolToJsval(cx, status != 0, J_FRVAL) );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( SetVideoMode ) {

	int width, height, bpp;
	double flags;

	if ( J_FARG_ISDEF(1) )
		J_CHK( JsvalToInt(cx, J_FARG(1), &width) );
	else
		width = 0; // by default, use current width

	if ( J_FARG_ISDEF(2) )
		J_CHK( JsvalToInt(cx, J_FARG(2), &height) );
	else
		height = 0; // by default, use current height

	if ( J_FARG_ISDEF(3) )
		J_CHK( JsvalToInt(cx, J_FARG(3), &bpp) );
	else
		bpp = 0; // by default, use current bpp

	if ( J_FARG_ISDEF(4) )
		J_CHK( JsvalToDouble(cx, J_FARG(4), &flags) );
	else
		flags = 0;

	SDL_Surface *surface = SDL_SetVideoMode(width, height, bpp, (Uint32)flags);
	if ( surface == NULL )
		return ThrowSdlError(cx);
	return JS_TRUE;
}


DEFINE_PROPERTY( videoWidth ) {

	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
	*vp = videoInfo != NULL ? INT_TO_JSVAL( videoInfo->current_w ) : 0;
	return JS_TRUE;
}

DEFINE_PROPERTY( videoHeight ) {

	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
	*vp = videoInfo != NULL ? INT_TO_JSVAL( videoInfo->current_h ) : 0;
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



DEFINE_FUNCTION_FAST( ToggleFullScreen ) {

	SDL_Surface *surface = SDL_GetVideoSurface();
	int status = SDL_WM_ToggleFullScreen( surface );
	*J_FRVAL = status == 1 ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;	
}

DEFINE_PROPERTY( fullScreen ) {

	bool fullScreen = ( (SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) != 0 );
	J_CHK( BoolToJsval(cx, fullScreen, vp) );
	return JS_TRUE;	
}


DEFINE_FUNCTION_FAST( Iconify ) {

	SDL_WM_IconifyWindow();
	return JS_TRUE;	
}


DEFINE_FUNCTION_FAST( SetGamma ) {
	
	J_S_ASSERT_ARG_MIN(3);
	float r,g,b;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &r) );
	J_CHK( JsvalToFloat(cx, J_FARG(2), &g) );
	J_CHK( JsvalToFloat(cx, J_FARG(3), &b) );
	if ( SDL_SetGamma(r,g,b) != 0 )
		ThrowSdlError(cx);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( GlSwapBuffers ) {

	SDL_GL_SwapBuffers();
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( GlSetAttribute ) {

	int attr;
	int value;
	J_S_ASSERT_ARG_MIN(2);
	J_CHK( JsvalToInt(cx, J_FARG(1), &attr) );
	J_CHK( JsvalToInt(cx, J_FARG(2), &value) );
	if ( SDL_GL_SetAttribute((SDL_GLattr)attr, value) == -1 )
		return ThrowSdlError(cx);
	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( GlGetAttribute ) {

	int attr;
	int value;
	J_S_ASSERT_ARG_MIN(2);
	J_CHK( JsvalToInt(cx, J_FARG(1), &attr) );
	if ( SDL_GL_GetAttribute((SDL_GLattr)attr, &value) == -1 )
		return ThrowSdlError(cx);
	J_CHK( IntToJsval(cx, value, J_FRVAL) );
	return JS_TRUE;
}






DEFINE_PROPERTY_SETTER( caption ) {

	const char *title;
	J_CHK( JsvalToString(cx, vp, &title) );
	SDL_WM_SetCaption(title, NULL);
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( caption ) {

	char *title;
	SDL_WM_GetCaption(&title, NULL);
	J_CHK( StringToJsval(cx, title, vp) );
	return JS_TRUE;
}


DEFINE_PROPERTY_SETTER( grabInput ) {

	bool grab;
	J_CHK( JsvalToBool(cx, *vp, &grab) );
	SDL_WM_GrabInput( grab ? SDL_GRAB_ON : SDL_GRAB_OFF );
	return JS_TRUE;	
}

DEFINE_PROPERTY_GETTER( grabInput ) {

	SDL_GrabMode mode = SDL_WM_GrabInput( SDL_GRAB_QUERY );
	J_CHK( BoolToJsval(cx, mode == SDL_GRAB_ON ? true : false, vp) );
	return JS_TRUE;	
}

DEFINE_PROPERTY_SETTER( showCursor ) {

	bool show;
	J_CHK( JsvalToBool(cx, *vp, &show) );
	SDL_ShowCursor( show ? 1 : 0 );
	return JS_TRUE;	
}

DEFINE_PROPERTY_GETTER( showCursor ) {

	int show = SDL_ShowCursor( -1 ); // query
	J_CHK( BoolToJsval(cx, show == 0 ? false : true, vp) );
	return JS_TRUE;	
}


DEFINE_FUNCTION_FAST( SetCursor ) {

	J_S_ASSERT_ARG_MIN(1);

	J_S_ASSERT_OBJECT( J_FARG(1) );
	JSObject *cursorObj = JSVAL_TO_OBJECT( J_FARG(1) );
	J_S_ASSERT_CLASS( cursorObj, classCursor );
	SDL_Cursor *cursor = (SDL_Cursor *)JS_GetPrivate(cx, cursorObj);
	J_S_ASSERT_RESOURCE( cursor );


	SDL_SetCursor(cursor);

	return JS_TRUE;	
}


DEFINE_PROPERTY( videoDriverName ) {
	
	char name[1024];
	SDL_VideoDriverName(name, sizeof(name));
	J_CHK( StringToJsval(cx, name, vp) );
	return JS_TRUE;
}




JSBool FireListener( JSContext *cx, JSObject *listenerObj, SDL_Event *ev, jsval *rval ) {

	jsval fVal;
	switch (ev->type) {
		case SDL_ACTIVEEVENT:
			J_CHK( JS_GetProperty(cx, listenerObj, "onActive", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

				jsval argv[] = { 
					ev->active.gain == 1 ? JSVAL_TRUE : JSVAL_FALSE 
				};
				J_CHK( JS_CallFunctionValue(cx, listenerObj, fVal, COUNTOF(argv), argv, rval) );
			}
			break;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
			J_CHK( JS_GetProperty(cx, listenerObj, ev->type == SDL_KEYDOWN ? "onKeyDown" : "onKeyUp", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

				JSString *ucChar = JS_NewUCStringCopyN(cx, &ev->key.keysym.unicode, 1);
				jsval argv[] = { 
					INT_TO_JSVAL(ev->key.keysym.sym),
					INT_TO_JSVAL(ev->key.keysym.mod),
					STRING_TO_JSVAL(ucChar),
					INT_TO_JSVAL(ev->key.keysym.scancode),
				};
				J_CHK( JS_CallFunctionValue(cx, listenerObj, fVal, COUNTOF(argv), argv, rval) );
			}
			break;

		case SDL_MOUSEMOTION:
			J_CHK( JS_GetProperty(cx, listenerObj, "onMouseMotion", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

				jsval argv[] = { 
					INT_TO_JSVAL(ev->motion.x), 
					INT_TO_JSVAL(ev->motion.y), 
					INT_TO_JSVAL(ev->motion.xrel),
					INT_TO_JSVAL(ev->motion.yrel),
					INT_TO_JSVAL(ev->motion.state),
				};
				J_CHK( JS_CallFunctionValue(cx, listenerObj, fVal, COUNTOF(argv), argv, rval) );
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			J_CHK( JS_GetProperty(cx, listenerObj, ev->type == SDL_KEYDOWN ? "onMouseButtonDown" : "onMouseButtonUp", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

				Uint8 buttonState = SDL_GetMouseState(NULL, NULL); // query only button state
				jsval argv[] = {
					INT_TO_JSVAL(ev->button.button),
					INT_TO_JSVAL(ev->button.x),
					INT_TO_JSVAL(ev->button.y),
					INT_TO_JSVAL(buttonState),
				};
				J_CHK( JS_CallFunctionValue(cx, listenerObj, fVal, COUNTOF(argv), argv, rval) );
			}
			break;

		case SDL_QUIT:
			J_CHK( JS_GetProperty(cx, listenerObj, "onQuit", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

				J_CHK( JS_CallFunctionValue(cx, listenerObj, fVal, 0, NULL, rval) );
			}
			break;
		
		case SDL_VIDEORESIZE:
			J_CHK( JS_GetProperty(cx, listenerObj, "onVideoResize", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

				jsval argv[] = { 
					INT_TO_JSVAL(ev->resize.w), 
					INT_TO_JSVAL(ev->resize.h)
				};
				J_CHK( JS_CallFunctionValue(cx, listenerObj, fVal, COUNTOF(argv), argv, rval) );
			}
			break;

		case SDL_VIDEOEXPOSE:
			J_CHK( JS_GetProperty(cx, listenerObj, "onVideoExpose", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

				J_CHK( JS_CallFunctionValue(cx, listenerObj, fVal, 0, NULL, rval) );
			}
			break;
	}
	return JS_IsExceptionPending(cx) ? JS_FALSE : JS_TRUE;
}


DEFINE_FUNCTION_FAST( PollEvent ) {

	J_S_ASSERT_OBJECT( J_FARG(1) );

	SDL_Event ev;
	SDL_PumpEvents();
	int status = SDL_PeepEvents(&ev, 1, SDL_GETEVENT, SDL_ACTIVEEVENTMASK | SDL_KEYEVENTMASK | SDL_MOUSEEVENTMASK | SDL_VIDEORESIZEMASK | SDL_VIDEOEXPOSEMASK | SDL_QUITMASK );
	if ( status == -1 )
		return ThrowSdlError(cx);

	if ( status == 1 ) {

		J_CHK( FireListener(cx, JSVAL_TO_OBJECT(J_FARG(1)), &ev, J_FRVAL) );
	} else {

		*J_FRVAL = JSVAL_VOID;
	}
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( WarpMouse ) {

	J_S_ASSERT_ARG_MIN(2);
	unsigned int x, y;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &x) );
	J_CHK( JsvalToUInt(cx, J_FARG(1), &y) );
	SDL_WarpMouse(x, y);
	return JS_TRUE;
}


DEFINE_PROPERTY( mouseX ) {

	int x;
	Uint8 buttonState = SDL_GetMouseState(&x, NULL); // query only button state
	*vp = INT_TO_JSID( x ); // query only button state
	return JS_TRUE;
}

DEFINE_PROPERTY( mouseY ) {

	int y;
	Uint8 buttonState = SDL_GetMouseState(NULL, &y); // query only button state
	*vp = INT_TO_JSID( y ); // query only button state
	return JS_TRUE;
}

DEFINE_PROPERTY( buttonState ) {

	*vp = INT_TO_JSID( SDL_GetMouseState(NULL, NULL) ); // query only button state
	return JS_TRUE;
}




CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST( SetIcon )
		FUNCTION_FAST( GetVideoModeList )
		FUNCTION_FAST( HasVideoMode )
		FUNCTION_FAST( SetVideoMode )
		FUNCTION_FAST( ToggleFullScreen )
		FUNCTION_FAST( Iconify )
		FUNCTION_FAST_ARGC( SetGamma, 3 )
		FUNCTION_FAST( GlSwapBuffers )
		FUNCTION_FAST_ARGC( GlSetAttribute, 2 )
		FUNCTION_FAST_ARGC( GlGetAttribute, 1 )
		FUNCTION_FAST_ARGC( PollEvent, 1 )
		FUNCTION_FAST_ARGC( WarpMouse, 2 )
		FUNCTION_FAST_ARGC( SetCursor, 1 )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY( caption )
		PROPERTY( grabInput )
		PROPERTY( showCursor )
		PROPERTY_READ( videoWidth )
		PROPERTY_READ( videoHeight )
		PROPERTY_READ( videoDriverName )
		PROPERTY_READ( fullScreen )
		PROPERTY_READ( mouseX )
		PROPERTY_READ( mouseY )
		PROPERTY_READ( buttonState )
	END_STATIC_PROPERTY_SPEC

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

		CONST_DOUBLE( GL_RED_SIZE				, SDL_GL_RED_SIZE				 )
		CONST_DOUBLE( GL_GREEN_SIZE			, SDL_GL_GREEN_SIZE			 )
		CONST_DOUBLE( GL_BLUE_SIZE				, SDL_GL_BLUE_SIZE			 )
		CONST_DOUBLE( GL_ALPHA_SIZE			, SDL_GL_ALPHA_SIZE			 )
		CONST_DOUBLE( GL_BUFFER_SIZE			, SDL_GL_BUFFER_SIZE			 )
		CONST_DOUBLE( GL_DOUBLEBUFFER			, SDL_GL_DOUBLEBUFFER		 )
		CONST_DOUBLE( GL_DEPTH_SIZE			, SDL_GL_DEPTH_SIZE			 )
		CONST_DOUBLE( GL_STENCIL_SIZE			, SDL_GL_STENCIL_SIZE		 )
		CONST_DOUBLE( GL_ACCUM_RED_SIZE		, SDL_GL_ACCUM_RED_SIZE		 )
		CONST_DOUBLE( GL_ACCUM_GREEN_SIZE	, SDL_GL_ACCUM_GREEN_SIZE	 )
		CONST_DOUBLE( GL_ACCUM_BLUE_SIZE		, SDL_GL_ACCUM_BLUE_SIZE	 )
		CONST_DOUBLE( GL_ACCUM_ALPHA_SIZE	, SDL_GL_ACCUM_ALPHA_SIZE	 )
		CONST_DOUBLE( GL_STEREO					, SDL_GL_STEREO				 )
		CONST_DOUBLE( GL_MULTISAMPLEBUFFERS	, SDL_GL_MULTISAMPLEBUFFERS )
		CONST_DOUBLE( GL_MULTISAMPLESAMPLES	, SDL_GL_MULTISAMPLESAMPLES )
		CONST_DOUBLE( GL_ACCELERATED_VISUAL	, SDL_GL_ACCELERATED_VISUAL )
		CONST_DOUBLE( GL_SWAP_CONTROL			, SDL_GL_SWAP_CONTROL		 )
	END_CONST_DOUBLE_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER( K_UNKNOWN, SDLK_UNKNOWN)
		CONST_INTEGER( K_FIRST, SDLK_FIRST)
		CONST_INTEGER( K_BACKSPACE, SDLK_BACKSPACE)
		CONST_INTEGER( K_TAB, SDLK_TAB)
		CONST_INTEGER( K_CLEAR, SDLK_CLEAR)
		CONST_INTEGER( K_RETURN, SDLK_RETURN)
		CONST_INTEGER( K_PAUSE, SDLK_PAUSE)
		CONST_INTEGER( K_ESCAPE, SDLK_ESCAPE)
		CONST_INTEGER( K_SPACE, SDLK_SPACE)
		CONST_INTEGER( K_EXCLAIM, SDLK_EXCLAIM)
		CONST_INTEGER( K_QUOTEDBL, SDLK_QUOTEDBL)
		CONST_INTEGER( K_HASH, SDLK_HASH)
		CONST_INTEGER( K_DOLLAR, SDLK_DOLLAR)
		CONST_INTEGER( K_AMPERSAND, SDLK_AMPERSAND)
		CONST_INTEGER( K_QUOTE, SDLK_QUOTE)
		CONST_INTEGER( K_LEFTPAREN, SDLK_LEFTPAREN)
		CONST_INTEGER( K_RIGHTPAREN, SDLK_RIGHTPAREN)
		CONST_INTEGER( K_ASTERISK, SDLK_ASTERISK)
		CONST_INTEGER( K_PLUS, SDLK_PLUS)
		CONST_INTEGER( K_COMMA, SDLK_COMMA)
		CONST_INTEGER( K_MINUS, SDLK_MINUS)
		CONST_INTEGER( K_PERIOD, SDLK_PERIOD)
		CONST_INTEGER( K_SLASH, SDLK_SLASH)
		CONST_INTEGER( K_0, SDLK_0)
		CONST_INTEGER( K_1, SDLK_1)
		CONST_INTEGER( K_2, SDLK_2)
		CONST_INTEGER( K_3, SDLK_3)
		CONST_INTEGER( K_4, SDLK_4)
		CONST_INTEGER( K_5, SDLK_5)
		CONST_INTEGER( K_6, SDLK_6)
		CONST_INTEGER( K_7, SDLK_7)
		CONST_INTEGER( K_8, SDLK_8)
		CONST_INTEGER( K_9, SDLK_9)
		CONST_INTEGER( K_COLON, SDLK_COLON)
		CONST_INTEGER( K_SEMICOLON, SDLK_SEMICOLON)
		CONST_INTEGER( K_LESS, SDLK_LESS)
		CONST_INTEGER( K_EQUALS, SDLK_EQUALS)
		CONST_INTEGER( K_GREATER, SDLK_GREATER)
		CONST_INTEGER( K_QUESTION, SDLK_QUESTION)
		CONST_INTEGER( K_AT, SDLK_AT)
		CONST_INTEGER( K_LEFTBRACKET, SDLK_LEFTBRACKET)
		CONST_INTEGER( K_BACKSLASH, SDLK_BACKSLASH)
		CONST_INTEGER( K_RIGHTBRACKET, SDLK_RIGHTBRACKET)
		CONST_INTEGER( K_CARET, SDLK_CARET)
		CONST_INTEGER( K_UNDERSCORE, SDLK_UNDERSCORE)
		CONST_INTEGER( K_BACKQUOTE, SDLK_BACKQUOTE)
		CONST_INTEGER( K_a, SDLK_a)
		CONST_INTEGER( K_b, SDLK_b)
		CONST_INTEGER( K_c, SDLK_c)
		CONST_INTEGER( K_d, SDLK_d)
		CONST_INTEGER( K_e, SDLK_e)
		CONST_INTEGER( K_f, SDLK_f)
		CONST_INTEGER( K_g, SDLK_g)
		CONST_INTEGER( K_h, SDLK_h)
		CONST_INTEGER( K_i, SDLK_i)
		CONST_INTEGER( K_j, SDLK_j)
		CONST_INTEGER( K_k, SDLK_k)
		CONST_INTEGER( K_l, SDLK_l)
		CONST_INTEGER( K_m, SDLK_m)
		CONST_INTEGER( K_n, SDLK_n)
		CONST_INTEGER( K_o, SDLK_o)
		CONST_INTEGER( K_p, SDLK_p)
		CONST_INTEGER( K_q, SDLK_q)
		CONST_INTEGER( K_r, SDLK_r)
		CONST_INTEGER( K_s, SDLK_s)
		CONST_INTEGER( K_t, SDLK_t)
		CONST_INTEGER( K_u, SDLK_u)
		CONST_INTEGER( K_v, SDLK_v)
		CONST_INTEGER( K_w, SDLK_w)
		CONST_INTEGER( K_x, SDLK_x)
		CONST_INTEGER( K_y, SDLK_y)
		CONST_INTEGER( K_z, SDLK_z)
		CONST_INTEGER( K_DELETE, SDLK_DELETE)
		CONST_INTEGER( K_KP0, SDLK_KP0)
		CONST_INTEGER( K_KP1, SDLK_KP1)
		CONST_INTEGER( K_KP2, SDLK_KP2)
		CONST_INTEGER( K_KP3, SDLK_KP3)
		CONST_INTEGER( K_KP4, SDLK_KP4)
		CONST_INTEGER( K_KP5, SDLK_KP5)
		CONST_INTEGER( K_KP6, SDLK_KP6)
		CONST_INTEGER( K_KP7, SDLK_KP7)
		CONST_INTEGER( K_KP8, SDLK_KP8)
		CONST_INTEGER( K_KP9, SDLK_KP9)
		CONST_INTEGER( K_KP_PERIOD, SDLK_KP_PERIOD)
		CONST_INTEGER( K_KP_DIVIDE, SDLK_KP_DIVIDE)
		CONST_INTEGER( K_KP_MULTIPLY, SDLK_KP_MULTIPLY)
		CONST_INTEGER( K_KP_MINUS, SDLK_KP_MINUS)
		CONST_INTEGER( K_KP_PLUS, SDLK_KP_PLUS)
		CONST_INTEGER( K_KP_ENTER, SDLK_KP_ENTER)
		CONST_INTEGER( K_KP_EQUALS, SDLK_KP_EQUALS)
		CONST_INTEGER( K_UP, SDLK_UP)
		CONST_INTEGER( K_DOWN, SDLK_DOWN)
		CONST_INTEGER( K_RIGHT, SDLK_RIGHT)
		CONST_INTEGER( K_LEFT, SDLK_LEFT)
		CONST_INTEGER( K_INSERT, SDLK_INSERT)
		CONST_INTEGER( K_HOME, SDLK_HOME)
		CONST_INTEGER( K_END, SDLK_END)
		CONST_INTEGER( K_PAGEUP, SDLK_PAGEUP)
		CONST_INTEGER( K_PAGEDOWN, SDLK_PAGEDOWN)
		CONST_INTEGER( K_F1, SDLK_F1)
		CONST_INTEGER( K_F2, SDLK_F2)
		CONST_INTEGER( K_F3, SDLK_F3)
		CONST_INTEGER( K_F4, SDLK_F4)
		CONST_INTEGER( K_F5, SDLK_F5)
		CONST_INTEGER( K_F6, SDLK_F6)
		CONST_INTEGER( K_F7, SDLK_F7)
		CONST_INTEGER( K_F8, SDLK_F8)
		CONST_INTEGER( K_F9, SDLK_F9)
		CONST_INTEGER( K_F10, SDLK_F10)
		CONST_INTEGER( K_F11, SDLK_F11)
		CONST_INTEGER( K_F12, SDLK_F12)
		CONST_INTEGER( K_F13, SDLK_F13)
		CONST_INTEGER( K_F14, SDLK_F14)
		CONST_INTEGER( K_F15, SDLK_F15)
		CONST_INTEGER( K_NUMLOCK, SDLK_NUMLOCK)
		CONST_INTEGER( K_CAPSLOCK, SDLK_CAPSLOCK)
		CONST_INTEGER( K_SCROLLOCK, SDLK_SCROLLOCK)
		CONST_INTEGER( K_RSHIFT, SDLK_RSHIFT)
		CONST_INTEGER( K_LSHIFT, SDLK_LSHIFT)
		CONST_INTEGER( K_RCTRL, SDLK_RCTRL)
		CONST_INTEGER( K_LCTRL, SDLK_LCTRL)
		CONST_INTEGER( K_RALT, SDLK_RALT)
		CONST_INTEGER( K_LALT, SDLK_LALT)
		CONST_INTEGER( K_RMETA, SDLK_RMETA)
		CONST_INTEGER( K_LMETA, SDLK_LMETA)
		CONST_INTEGER( K_LSUPER, SDLK_LSUPER)
		CONST_INTEGER( K_RSUPER, SDLK_RSUPER)
		CONST_INTEGER( K_MODE, SDLK_MODE)
		CONST_INTEGER( K_COMPOSE, SDLK_COMPOSE)
		CONST_INTEGER( K_HELP, SDLK_HELP)
		CONST_INTEGER( K_PRINT, SDLK_PRINT)
		CONST_INTEGER( K_SYSREQ, SDLK_SYSREQ)
		CONST_INTEGER( K_BREAK, SDLK_BREAK)
		CONST_INTEGER( K_MENU, SDLK_MENU)
		CONST_INTEGER( K_POWER, SDLK_POWER)
		CONST_INTEGER( K_EURO, SDLK_EURO)
		CONST_INTEGER( K_UNDO, SDLK_UNDO)
	END_CONST_INTEGER_SPEC

END_STATIC
