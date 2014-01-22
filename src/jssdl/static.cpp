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
#include "../jslang/handlePub.h"

#include "error.h"

DECLARE_STATIC()


extern int volatile _maxFPS;

extern SDL_Surface volatile *_surface;
extern int volatile surfaceWidth;
extern int volatile surfaceHeight;

extern int desktopWidth;
extern int desktopHeight;
extern Uint8 desktopBitsPerPixel;

#ifndef JL_NOTHREAD

//extern JLSemaphoreHandler sdlEventsSem;

extern JLCondHandler sdlEventsCond;
extern JLMutexHandler sdlEventsLock;

extern bool surfaceReady;
extern JLCondHandler surfaceReadyCond;
extern JLMutexHandler surfaceReadyLock;

#endif // JL_NOTHREAD

bool JLSetVideoMode(int width, int height, int bpp, Uint32 flags);
void JLSwapBuffers(bool async);


DECLARE_CLASS( Cursor )

/**doc fileIndex:topmost **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_STATIC


// see PollEvent
bool FireListener( JSContext *cx, JSObject *thisObj, JSObject *listenerObj, SDL_Event *ev, jsval *rval, bool *fired ) {

	jsval fVal;

	switch (ev->type) {
		case SDL_ACTIVEEVENT:
			JL_CHK( JS_GetProperty(cx, listenerObj, "onActive", &fVal) );
			if ( JL_ValueIsCallable(cx, fVal) ) {

				jsval argv[] = {
					BOOLEAN_TO_JSVAL( ev->active.gain == 1 )
				};
				JL_CHK( JS_CallFunctionValue(cx, thisObj, fVal, COUNTOF(argv), argv, rval) );
				*fired = true;
			}
			break;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
			JL_CHK( JS_GetProperty(cx, listenerObj, ev->type == SDL_KEYDOWN ? "onKeyDown" : "onKeyUp", &fVal) );
			if ( JL_ValueIsCallable(cx, fVal) ) {

				jsval argv[] = {
					INT_TO_JSVAL(ev->key.keysym.sym),
					INT_TO_JSVAL(ev->key.keysym.mod),
					JSVAL_NULL, // unicode char
//					INT_TO_JSVAL(ev->key.keysym.scancode), // The scancode is hardware dependent, and should not be used by general applications.
				};

				if ( (ev->key.keysym.unicode & 0xFF80) == 0 ) {

					const char ch = ev->key.keysym.unicode & 0x7F;
					argv[2] = STRING_TO_JSVAL(JS_NewStringCopyN(cx, &ch, 1));
				} else {

					argv[2] = STRING_TO_JSVAL(JS_NewUCStringCopyN(cx, (jschar*)&ev->key.keysym.unicode, 1));
				}

				bool status = JS_CallFunctionValue(cx, thisObj, fVal, COUNTOF(argv), argv, rval);
				JL_CHK( status );
				*fired = true;
			}
			break;

		case SDL_MOUSEMOTION:
			JL_CHK( JS_GetProperty(cx, listenerObj, "onMouseMotion", &fVal) );
			if ( JL_ValueIsCallable(cx, fVal) ) {

				if ( ev->motion.x == (Uint16)-1 ) { // || ev->motion.y == (Uint16)-1
					
					*fired = false;
					break;
				}

				SDLMod modState = SDL_GetModState();
				jsval argv[] = {
					INT_TO_JSVAL(ev->motion.x),
					INT_TO_JSVAL(ev->motion.y),
					INT_TO_JSVAL(ev->motion.xrel),
					INT_TO_JSVAL(ev->motion.yrel),
					INT_TO_JSVAL(ev->motion.state),
					INT_TO_JSVAL(modState),
				};
				// no argv GC protection needed.
				JL_CHK( JS_CallFunctionValue(cx, thisObj, fVal, COUNTOF(argv), argv, rval) );
				*fired = true;
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			JL_CHK( JS_GetProperty(cx, listenerObj, ev->type == SDL_MOUSEBUTTONDOWN ? "onMouseButtonDown" : "onMouseButtonUp", &fVal) );
			if ( JL_ValueIsCallable(cx, fVal) ) {

				SDLMod modState = SDL_GetModState();
				Uint8 buttonState = SDL_GetMouseState(NULL, NULL); // query only button state
				jsval argv[] = {
					INT_TO_JSVAL(ev->button.button),
					INT_TO_JSVAL(ev->button.x),
					INT_TO_JSVAL(ev->button.y),
					INT_TO_JSVAL(buttonState),
					INT_TO_JSVAL(modState),
				};
				// no argv GC protection needed.
				JL_CHK( JS_CallFunctionValue(cx, thisObj, fVal, COUNTOF(argv), argv, rval) );
				*fired = true;
			}
			break;

		case SDL_QUIT:
			JL_CHK( JS_GetProperty(cx, listenerObj, "onQuit", &fVal) );
			if ( JL_ValueIsCallable(cx, fVal) ) {

				// no argv GC protection needed.
				JL_CHK( JS_CallFunctionValue(cx, thisObj, fVal, 0, NULL, rval) );
				*fired = true;
			}
			break;

		case SDL_VIDEORESIZE:
			// ... and you must respond to the event by re-calling SDL_SetVideoMode() with the requested size (or another size that suits the application).
			JL_CHK( JS_GetProperty(cx, listenerObj, "onVideoResize", &fVal) );
			if ( JL_ValueIsCallable(cx, fVal) ) {

//				SDL_Surface *surface = SDL_GetVideoSurface();
//				const SDL_VideoInfo *video = SDL_GetVideoInfo();
				jsval argv[] = {
					INT_TO_JSVAL(ev->resize.w),
					INT_TO_JSVAL(ev->resize.h)
				};
				// no argv GC protection needed.
				JL_CHK( JS_CallFunctionValue(cx, thisObj, fVal, COUNTOF(argv), argv, rval) );
				*fired = true;
			}
			break;

		case SDL_VIDEOEXPOSE:
			JL_CHK( JS_GetProperty(cx, listenerObj, "onVideoExpose", &fVal) );
			if ( JL_ValueIsCallable(cx, fVal) ) {

				// no argv GC protection needed.
				JL_CHK( JS_CallFunctionValue(cx, thisObj, fVal, 0, NULL, rval) );
				*fired = true;
			}
			break;
		default:
			*fired = false;
	}
//	return JL_IsExceptionPending(cx) ? false : true; // (TBD) why this line is needed ?
	return true;
	JL_BAD;
}


/**doc
=== Static functions ===
**/


/**doc
$TOC_MEMBER $INAME
 $ARRAY | $UNDEF $INAME( bitsPerPixel, flags )
  Returns the available screen dimensions for the given format, sorted largest to smallest.
  $H arguments
   $ARG $INT bitsPerPixel: bit depth, the number of bits per pixel (8, 16, 32)
   $ARG bitmsak flags: a bitwise-ored combination of flags. see SetVideoMode() function.
  $H return value
   * an Array that contains the list of available screen dimensions for the given format and video flags, sorted largest to smallest.
   * an empty Array if there are no dimensions available for a particular format.
   * $UNDEF if any dimension is okay for the given format.
**/
DEFINE_FUNCTION( getVideoModeList ) {

	JL_ASSERT_ARGC_MIN(2);

	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo(); // If called before SDL_SetVideoMode(), 'vfmt' is the pixel format of the "best" video mode.
	SDL_PixelFormat format;
	format = *videoInfo->vfmt;

	double flags;

	if ( JL_ARG_ISDEF(1) ) {

		Uint8 bpp;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &bpp) );
		format.BitsPerPixel = bpp;
		format.BytesPerPixel = bpp / 8; // (TBD) need to set both ?
	}

	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &flags) );
	else
		flags = 0;

	SDL_Rect **modes = SDL_ListModes(&format, (Uint32)flags);

	if ( modes == (SDL_Rect **)-1 ) { // any dimension is okay for this format.
		
		JL_RVAL.setUndefined();
		return true;
	}

	JSObject *modesArray = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK(modesArray);
	*JL_RVAL = OBJECT_TO_JSVAL( modesArray );

	jsval tmp;
	for ( int i = 0; modes[i] != NULL; i++ ) {

		JSObject *rectArray = JS_NewArrayObject(cx, 2, NULL);
		JL_CHK(rectArray);

		tmp = OBJECT_TO_JSVAL(rectArray);
		JL_CHK( JL_SetElement(cx, modesArray, i, &tmp) );

		tmp = INT_TO_JSVAL(modes[i]->w);
		JL_CHK( JL_SetElement(cx, rectArray, 0, &tmp) );

		tmp = INT_TO_JSVAL(modes[i]->h);
		JL_CHK( JL_SetElement(cx, rectArray, 1, &tmp) );
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( width, height, bitsPerPixel, flags )
  Check to see if a particular video mode is supported.
  $H arguments
   See SetVideoMode() function.
  $H return value
   If the requested mode is not supported under any bit depth or returns the bits-per-pixel of the closest available mode with the iven width and height.
   If this bits-per-pixel is different from the one used when setting the video mode, SDL_SetVideoMode() will succeed, but will emulate the requested bits-per-pixel with a shadow surface.
**/
DEFINE_FUNCTION( videoModeOK ) {

	int width, height, bpp;
	Uint32 flags;

	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &width) );
	else
		width = 0; // by default, use current width

	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &height) );
	else
		height = 0; // by default, use current height

	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &bpp) );
	else
		bpp = 0; // by default, use current bpp

	if ( JL_ARG_ISDEF(4) ) {

		double tmp;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &tmp) ); // we need to use doubles because some values are greater than 2^31
		flags = (Uint32)tmp;
	} else
		flags = 0;

	int status = SDL_VideoModeOK(width, height, bpp, (Uint32)flags);
//	JL_CHK(JL_NativeToJsval(cx, status != 0, JL_RVAL) );
	JL_CHK( JL_NativeToJsval(cx, status, JL_RVAL) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [width], [height], [bitsPerPixel], [flags] )
  Set the requested video mode (allocating a shadow buffer if necessary).
  $H arguments
   $ARG $INT width: with
   $ARG $INT height: height
   $ARG $INT bitsPerPixel: bit depth, the number of bits per pixel (8, 16, 32). If omited, use the current bpp value.
   $ARG bitmsak flags: a bitwise-ored combination of the following flags. If omited, use the previous flags.
    SWSURFACE, HWSURFACE, ASYNCBLIT, ANYFORMAT, HWPALETTE, DOUBLEBUF, OPENGL, OPENGLBLIT, RESIZABLE, NOFRAME HWACCEL, SRCCOLORKEY, RLEACCELOK, RLEACCEL, SRCALPHA, PREALLOC
**/
DEFINE_FUNCTION( setVideoMode ) {

	JL_ASSERT_ARGC_RANGE(0, 4);

	int width, height, bpp;
	Uint32 flags;
//	bool fullscreen;

//	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo(); // If called before SDL_SetVideoMode(), 'vfmt' is the pixel format of the "best" video mode.
//	SDL_PixelFormat format = *videoInfo->vfmt;

	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &width) );
	else
		width = _surface != NULL ? _surface->w : 0; // by default, use current width

	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &height) );
	else
		height = _surface != NULL ? _surface->h : 0; // by default, use current height

	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &bpp) );
	else
		bpp = _surface != NULL ? _surface->format->BitsPerPixel : 0; // by default, use current or the default bpp

	if ( JL_ARG_ISDEF(4) ) {

		double tmp;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &tmp) ); // we need to use doubles because some values are greater than 2^31
		flags = (Uint32)tmp;
	} else {

		flags = _surface != NULL ? _surface->flags : 0; // if not given, use the previous setting or a default value.
	}

	//if ( JL_ARG_ISDEF(5) )
	//	JL_CHK( JL_JsvalToNative(cx, JL_ARG(5), &fullscreen) ); // we need to use doubles because some values are greater than 2^31
	//else
	//	fullscreen = surface != NULL ? (surface->flags & SDL_FULLSCREEN) != 0 : 0;
	//flags = flags & ~SDL_FULLSCREEN;
	//if ( fullscreen )
	//	flags = flags | SDL_FULLSCREEN;

	bool status = JLSetVideoMode(width, height, bpp, flags);
 	if ( !status )
		return ThrowSdlError(cx);
	//_surface = SDL_SetVideoMode(width, height, bpp, flags);
 //	if ( !_surface )
	//	return ThrowSdlError(cx);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the current video surface width.
**/
DEFINE_PROPERTY_GETTER( videoWidth ) {

//	const SDL_VideoInfo *info = SDL_GetVideoInfo();
//	const SDL_Surface *surface = SDL_GetVideoSurface();
	*vp = _surface != NULL ? INT_TO_JSVAL( surfaceWidth ) : JSVAL_VOID;
	return true;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the current video surface height.
**/
DEFINE_PROPERTY_GETTER( videoHeight ) {

//	const SDL_Surface *surface = SDL_GetVideoSurface();
	*vp = _surface != NULL ? INT_TO_JSVAL( surfaceHeight ) : JSVAL_VOID;
	return true;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the number of bits per pixel of the current video surface.
**/
DEFINE_PROPERTY_GETTER( videoBitsPerPixel ) {

//	const SDL_Surface *surface = SDL_GetVideoSurface();
	*vp = _surface != NULL ? INT_TO_JSVAL( _surface->format->BitsPerPixel ) : JSVAL_VOID;
	return true;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the current desktop surface width.
**/
DEFINE_PROPERTY_GETTER( desktopWidth ) {

	*vp = INT_TO_JSVAL( desktopWidth );
	return true;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the current desktop surface height.
**/
DEFINE_PROPERTY_GETTER( desktopHeight ) {

	*vp = INT_TO_JSVAL( desktopHeight );
	return true;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the number of bits per pixel of the desktop surface.
**/
DEFINE_PROPERTY_GETTER( desktopBitsPerPixel ) {

	*vp = *vp = INT_TO_JSVAL( desktopBitsPerPixel );
	return true;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the current video flags.
**/
DEFINE_PROPERTY_GETTER( videoFlags ) {

//	const SDL_Surface *surface = SDL_GetVideoSurface();
	if ( _surface != NULL ) {

		JL_CHK( JL_NativeToJsval(cx, uint32_t(_surface->flags), vp) ); // we need to use doubles because some values are greater than 2^31
	} else {

		*vp = JSVAL_VOID;
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE ImageObject $INAME $WRITEONLY
  Sets the window manager icon for the display window.
**/
DEFINE_PROPERTY_SETTER( icon ) {

	JLData data;
	jsval image = *vp;

	if ( image.isUndefined() ) {

		SDL_WM_SetIcon(NULL, NULL);
		return true;
	}

	JL_ASSERT_IS_OBJECT(image, "");
	int sWidth, sHeight, sChannels;
	ImageDataType dataType;
	data = JL_GetImageObject(cx, image, &sWidth, &sHeight, &sChannels, &dataType);
	JL_ASSERT( data.IsSet(), E_ARG, E_INVALID );
	JL_ASSERT( dataType == TYPE_UINT8, E_ARG, E_DATATYPE, E_INVALID );
	JL_ASSERT( sChannels == 3 || sChannels == 4, E_PARAM, E_STR("channels"), E_RANGE, E_INTERVAL_NUM(3, 4) );

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

	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void*)data.GetConstStr(), sWidth, sHeight, 8 * sChannels, sWidth * sChannels, rmask, gmask, bmask, amask);

	if ( surface == NULL )
		return ThrowSdlError(cx);

	SDL_WM_SetIcon(surface, NULL); // (TBD) manage mask with image alpha ?
	SDL_FreeSurface(surface);
	return true;
	JL_BAD;
}

/*
/ **doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Toggle fullscreen mode without changing the contents of the screen.
  $H return value
   true if this function was able to toggle fullscreen mode (change from running in a window to fullscreen, or vice-versa). false if it is not implemented, or fails.
** /
DEFINE_FUNCTION( toggleFullScreen ) {

	SDL_Surface *surface = SDL_GetVideoSurface();
	if ( surface == NULL ) {

		JL_RVAL.setUndefined();
		return true;
	}

//	int status = SDL_WM_ToggleFullScreen( surface ); // This is currently only implemented in the X11 video driver ??
//	*JL_RVAL = status == 1 ? JSVAL_TRUE : JSVAL_FALSE;

	bool hasFullscreen = (surface->flags & SDL_FULLSCREEN) != 0;
	JLSetVideoMode(surface->w, surface->h, surface->format->BitsPerPixel, hasFullscreen ? surface->flags & ~SDL_FULLSCREEN : surface->flags | SDL_FULLSCREEN);
  	JL_RVAL.setUndefined();
	return true;
}
*/

/* use (videoFlags & FULLSCREEN) instead
/ **doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
  Is true if the current video surface is a full screen display
** /
DEFINE_PROPERTY( isFullScreen ) {

	bool fullScreen = ( (SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) != 0 );
	JL_CHK(JL_NativeToJsval(cx, fullScreen, vp) );
	return true;
	JL_BAD;
}
*/


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Iconify the window in window managed environments. A successful iconification will result in an SDL_APPACTIVE loss event.
**/
DEFINE_FUNCTION( iconify ) {

	SDL_WM_IconifyWindow();
	JL_RVAL.setUndefined();
	return true;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( red, green, blue )
  Set the gamma correction for each of the color channels. The gamma values range (approximately) between 0.1 and 10.0 .
  $H arguments
   ARG integer red
   ARG integer green
   ARG integer blue
**/
DEFINE_FUNCTION( setGamma ) {

	JL_ASSERT_ARGC_MIN(3);

	float r,g,b;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &r) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &g) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &b) );
	if ( SDL_SetGamma(r, g, b) != 0 )
		return ThrowSdlError(cx);
	
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [async = false] )
  Perform a GL buffer swap on the current GL context.
  If _async_ is true, you must wait for the SurfaceReadyEvents through the processEvents() function before drawing again.
**/
DEFINE_FUNCTION( glSwapBuffers ) {

	JL_ASSERT_ARGC_RANGE(0,1);

	bool async;
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &async) );
	else
		async = false;

	SDL_ClearError(); // JLSwapBuffers or SDL_GL_SwapBuffers does not return an error status.
	JLSwapBuffers(async); // SDL_GL_SwapBuffers();
	if ( HasSDLError() )
		return ThrowSdlError(cx);
	
	// (TBD) check error	*SDL_GetError() != '\0' ???
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( attribute, value )
  Set an attribute of the OpenGL subsystem before intialization.
  $H arguments
   $ARG $ENUM attribute:
   $ARG $INT value:
**/
DEFINE_FUNCTION( glSetAttribute ) {

	int attr;
	int value;
	JL_ASSERT_ARGC_MIN(2);

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &attr) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &value) );
	if ( SDL_GL_SetAttribute((SDL_GLattr)attr, value) == -1 )
		return ThrowSdlError(cx);
	
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( attribute )
  Get an attribute of the OpenGL subsystem from the windowing interface. This is of course different from getting the values from SDL's internal OpenGL subsystem, which only stores the values you request before initialization.
  $H arguments
   $ARG $ENUM attribute:
   $ARG $INT value:
  $H return value
   the value of the requested attribute.
**/
DEFINE_FUNCTION( glGetAttribute ) {

	int attr;
	int value;
	JL_ASSERT_ARGC_MIN(2);

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &attr) );
	if ( SDL_GL_GetAttribute((SDL_GLattr)attr, &value) == -1 )
		return ThrowSdlError(cx);
	JL_CHK( JL_NativeToJsval(cx, value, JL_RVAL) );

	return true;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Sets/Gets the title of the display window, if any.
**/
DEFINE_PROPERTY_SETTER( caption ) {

	JLData title;
	JL_CHK( JL_JsvalToNative(cx, *vp, &title) );
	SDL_WM_SetCaption(title, NULL);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( caption ) {

	char *title;
	SDL_WM_GetCaption(&title, NULL);
	if ( title == NULL ) // (TBD) check if possible
		*vp = JSVAL_VOID;
	else
		JL_CHK( JL_NativeToJsval(cx, title, vp) );
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Sets/Gets the input grab state.
  Grabbing means that the mouse is confined to the application window, 
  and nearly all keyboard input is passed directly to the application, 
  and not interpreted by a window manager, if any.$LF
  To simulate an endless mouse surface, use $INAME like this:
{{{
 showCursor = false;
 grabInput = true;
}}}

**/
DEFINE_PROPERTY_SETTER( grabInput ) {

	bool grab;
	JL_CHK( JL_JsvalToNative(cx, *vp, &grab) );
	SDL_WM_GrabInput( grab ? SDL_GRAB_ON : SDL_GRAB_OFF );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( grabInput ) {

	SDL_GrabMode mode = SDL_WM_GrabInput( SDL_GRAB_QUERY );
	JL_CHK(JL_NativeToJsval(cx, mode == SDL_GRAB_ON ? true : false, vp) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Sets/Gets whether or not the cursor is shown on the screen. The cursor start off displayed, but can be turned off.
**/
DEFINE_PROPERTY_SETTER( showCursor ) {

	bool show;
	JL_CHK( JL_JsvalToNative(cx, *vp, &show) );
	SDL_ShowCursor( show ? 1 : 0 );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( showCursor ) {

	int show = SDL_ShowCursor( -1 ); // query
	JL_CHK(JL_NativeToJsval(cx, show == 0 ? false : true, vp) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( image )
 Set the currently active cursor to the specified one. If the cursor is currently visible, the change will be immediately represented on the display.
  $H arguments
   $ARG ImageObject image:
**/
DEFINE_FUNCTION( setCursor ) {

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	JSObject *cursorObj = JSVAL_TO_OBJECT( JL_ARG(1) );
	JL_ASSERT_INSTANCE( cursorObj, JL_CLASS(Cursor) );
	SDL_Cursor *cursor = (SDL_Cursor *)JL_GetPrivate(cursorObj);
	JL_ASSERT_OBJECT_STATE( cursor, JL_CLASS_NAME(Cursor) );
	SDL_SetCursor(cursor);
	
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Is the name of the video driver or undefined  if it has not been initialized.
**/
DEFINE_PROPERTY_GETTER( videoDriverName ) {

	char name[1024];
	char *status = SDL_VideoDriverName(name, sizeof(name));
	if ( status != NULL )
		JL_CHK( JL_NativeToJsval(cx, name, vp) );
	else
		*vp = JSVAL_VOID;
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( listeners )
  $H arguments
   $ARG $OBJ listeners: is an object that contains callback functions.
  $H return value
   $TRUE if an event has been processed.
  $H example
   {{{
   var done = false;
   var listeners = {
    onQuit: function() {
     done = true;
    },
    onKeyDown: function( key, modifiers ) {
     ...
    }
   }
   while ( !done ) {

    pollEvent(listeners);
   }
   }}}
**/
DEFINE_FUNCTION( pollEvent ) {

	JL_DEFINE_FUNCTION_OBJ;
	SDL_Event ev;
	SDL_PumpEvents();

	//	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

	int status = SDL_PeepEvents(&ev, 1, SDL_GETEVENT, SDL_ALLEVENTS); // see SDL_EventState
	if ( status == -1 )
		return ThrowSdlError(cx);

	if ( status == 0 ) {
		
		*JL_RVAL = JSVAL_FALSE;
		return true;
	}

	if ( JL_ARG_ISDEF(1) ) {
		
		bool fired;
		JL_ASSERT_ARG_IS_OBJECT(1);
		JL_CHK( FireListener(cx, obj, JSVAL_TO_OBJECT(JL_ARG(1)), &ev, JL_RVAL, &fired) );
	}

	*JL_RVAL = JSVAL_TRUE;
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( x, y )
  Set the position of the mouse cursor (generates a mouse motion event).
  $H arguments
   $ARG $INT x
   $ARG $INT y
**/
DEFINE_FUNCTION( warpMouse ) {

	JL_ASSERT_ARGC_MIN(2);

	Uint16 x, y;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &x) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &y) );
	SDL_WarpMouse(x, y);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the current mouse cursor X position.
**/
DEFINE_PROPERTY_GETTER( mouseX ) {

	int x;
	SDL_GetMouseState(&x, NULL); // query only button state
	*vp = INT_TO_JSVAL( x ); // query only button state
	return true;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the current mouse cursor Y position.
**/
DEFINE_PROPERTY_GETTER( mouseY ) {

	int y;
	SDL_GetMouseState(NULL, &y); // query only button state
	*vp = INT_TO_JSVAL( y ); // query only button state
	return true;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the current state of the mouse. The current button state is returned as a button bitmask.
   $CONST BUTTON_LMASK
   $CONST BUTTON_MMASK
   $CONST BUTTON_RMASK
   $CONST BUTTON_UPMASK
   $CONST BUTTON_DOWNMASK
   $CONST BUTTON_X1MASK
   $CONST BUTTON_X2MASK
  $H example 1
  {{{
  var leftButtonState = ( ( $INAME & BUTTON_LMASK ) != 0 );
  }}}
  $H example 2
  {{{
  var hasButtonDown = ( ( $INAME & ( BUTTON_LMASK | BUTTON_MMASK | BUTTON_RMASK ) ) != 0 );
  }}}
**/
DEFINE_PROPERTY_GETTER( buttonState ) {

	*vp = INT_TO_JSVAL( SDL_GetMouseState(NULL, NULL) ); // query only button state
	return true;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the current key modifier state bitmask.
   $CONST KMOD_NONE
   $CONST KMOD_LSHIFT
   $CONST KMOD_RSHIFT
   $CONST KMOD_LCTRL
   $CONST KMOD_RCTRL
   $CONST KMOD_LALT
   $CONST KMOD_RALT
   $CONST KMOD_LMETA
   $CONST KMOD_RMETA
   $CONST KMOD_NUM
   $CONST KMOD_CAPS
   $CONST KMOD_MODE
**/
DEFINE_PROPERTY_GETTER( modifierState ) {

	*vp = INT_TO_JSVAL( SDL_GetModState() );
	return true;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( keysym )
  Get a snapshot of the current state of the keyboard.
  $H arguments
   $ARG $ENUM keysym: the key to be tested. see key constants below.
**/
DEFINE_FUNCTION( getKeyState ) {

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	int key;
	key = JL_ARG(1).toInt32();
	JL_ASSERT( key > SDLK_FIRST && key < SDLK_LAST, E_ARG, E_NUM(1), E_INVALID );
	Uint8 *keystate = SDL_GetKeyState(NULL);
	ASSERT( keystate != NULL );
	return JL_NativeToJsval(cx, keystate[key] != 0, JL_RVAL);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME( keysym )
  Get the name of the keysym.
  $H arguments
   $ARG $ENUM keysym
**/
DEFINE_FUNCTION( getKeyName ) {

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	int key;
	key = JL_ARG(1).toInt32();
	JL_ASSERT( key > SDLK_FIRST && key < SDLK_LAST, E_ARG, E_NUM(1), E_INVALID );
	char *keyName = SDL_GetKeyName((SDLKey)key);
	JSString *jsStr = JS_NewStringCopyZ(cx, keyName);
	*JL_RVAL = STRING_TO_JSVAL(jsStr);
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Sets/Gets keyboard repeat delay. This is the initial delay in ms between the time when a key is pressed, and keyboard repeat begins. If set to 0, keyboard repeat is disabled.
**/
DEFINE_PROPERTY_SETTER( keyRepeatDelay ) {

	int delay, interval;
	SDL_GetKeyRepeat(&delay, &interval);
	JL_CHK( JL_JsvalToNative(cx, *vp, &delay) );
	int status = SDL_EnableKeyRepeat(delay, interval);
	if ( status == -1 )
		return ThrowSdlError(cx);
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( keyRepeatDelay ) {

	int delay, interval;
	SDL_GetKeyRepeat(&delay, &interval);
	JL_CHK( JL_NativeToJsval(cx, delay, vp) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Sets/Gets keyboard repeat interval. This is the time in ms between keyboard repeat events.
**/
DEFINE_PROPERTY_SETTER( keyRepeatInterval ) {

	int delay, interval;
	SDL_GetKeyRepeat(&delay, &interval);
	JL_CHK( JL_JsvalToNative(cx, *vp, &interval) );
	int status = SDL_EnableKeyRepeat(delay, interval);
	if ( status == -1 )
		return ThrowSdlError(cx);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( keyRepeatInterval ) {

	int delay, interval;
	SDL_GetKeyRepeat(&delay, &interval);
	JL_CHK( JL_NativeToJsval(cx, interval, vp) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Enable/Disable UNICODE translation of keyboard input. This translation has some overhead, so translation defaults off.
**/
DEFINE_PROPERTY_SETTER( unicodeKeyboardTranslation ) {

	bool enable;
	JL_CHK( JL_JsvalToNative(cx, *vp, &enable) );
	SDL_EnableUNICODE(enable ? 1 : 0);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( unicodeKeyboardTranslation ) {

	int enable = SDL_EnableUNICODE(-1);
	*vp = BOOLEAN_TO_JSVAL( enable == 1 );
	return true;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the current state of the application. If true, the user is able to see your application, otherwise it has been iconified or disabled.
**/
DEFINE_PROPERTY_GETTER( appStateActive ) {

	JL_CHK(JL_NativeToJsval(cx, (SDL_GetAppState() & SDL_APPACTIVE) != 0, vp) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( hasRDTSC ) {

	JL_CHK(JL_NativeToJsval(cx, SDL_HasRDTSC() == SDL_TRUE, vp) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( hasMMX ) {

	JL_CHK(JL_NativeToJsval(cx, SDL_HasMMX() == SDL_TRUE, vp) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( hasMMXExt ) {

	JL_CHK(JL_NativeToJsval(cx, SDL_HasMMXExt() == SDL_TRUE, vp) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( has3DNow ) {

	JL_CHK(JL_NativeToJsval(cx, SDL_Has3DNow() == SDL_TRUE, vp) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( has3DNowExt ) {

	JL_CHK(JL_NativeToJsval(cx, SDL_Has3DNowExt() == SDL_TRUE, vp) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( hasSSE ) {

	JL_CHK(JL_NativeToJsval(cx, SDL_HasSSE() == SDL_TRUE, vp) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( hasSSE2 ) {

	JL_CHK(JL_NativeToJsval(cx, SDL_HasSSE2() == SDL_TRUE, vp) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( hasAltiVec ) {

	JL_CHK(JL_NativeToJsval(cx, SDL_HasAltiVec() == SDL_TRUE, vp) );
	return true;
	JL_BAD;
}


/**doc
 $H keysym enum
  `K_UNKNOWN, K_FIRST, K_BACKSPACE, K_TAB, K_CLEAR, K_RETURN, K_PAUSE, K_ESCAPE, K_SPACE, K_EXCLAIM, K_QUOTEDBL, K_HASH, K_DOLLAR, K_AMPERSAND, K_QUOTE, K_LEFTPAREN, K_RIGHTPAREN, K_ASTERISK, K_PLUS, K_COMMA, K_MINUS, K_PERIOD, K_SLASH, `
  `K_COLON, K_SEMICOLON, K_LESS, K_EQUALS, K_GREATER, K_QUESTION, K_AT, K_LEFTBRACKET, K_BACKSLASH, K_RIGHTBRACKET, K_CARET, K_UNDERSCORE, K_BACKQUOTE, K_DELETE, `
  `K_UP, K_DOWN, K_RIGHT, K_LEFT, K_INSERT, K_HOME, K_END, K_PAGEUP, K_PAGEDOWN, `
  `K_NUMLOCK, K_CAPSLOCK, K_SCROLLOCK, K_RSHIFT, K_LSHIFT, K_RCTRL, K_LCTRL, K_RALT, K_LALT, K_RMETA, K_LMETA, K_LSUPER, K_RSUPER, K_MODE, K_COMPOSE, K_HELP, K_PRINT, K_SYSREQ, K_BREAK, K_MENU, K_POWER, K_EURO, K_UNDO, `
  `K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10, K_F11, K_F12, K_F13, K_F14, K_F15, `
  `K_0, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9, `
  `K_a, K_b, K_c, K_d, K_e, K_f, K_g, K_h, K_i, K_j, K_k, K_l, K_m, K_n, K_o, K_p, K_q, K_r, K_s, K_t, K_u, K_v, K_w, K_x, K_y, K_z, `
  `K_KP0, K_KP1, K_KP2, K_KP3, K_KP4, K_KP5, K_KP6, K_KP7, K_KP8, K_KP9, K_KP_PERIOD, K_KP_DIVIDE, K_KP_MULTIPLY, K_KP_MINUS, K_KP_PLUS, K_KP_ENTER, K_KP_EQUALS, `
**/


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Hold the current version of SDL.
**/
DEFINE_PROPERTY_GETTER( version ) {

	*vp = INT_TO_JSVAL( SDL_COMPILEDVERSION );
	return jl::StoreProperty(cx, obj, id, vp, true);
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Sets/Gets the frames per second limit. Use Infinity for no limit.
**/
DEFINE_PROPERTY_SETTER( maxFPS ) {

	if ( JL_ValueIsPInfinity(cx, *vp) ) {

		_maxFPS = JLINFINITE;
	} else {

		unsigned int value;
		JL_CHK( JL_JsvalToNative(cx, *vp, &value) );
		_maxFPS = value;
	}
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( maxFPS ) {

	if ( _maxFPS == JLINFINITE ) {
		
		*vp = JS_GetPositiveInfinityValue(cx);
	} else {

		JL_CHK( JL_NativeToJsval(cx, int32_t(_maxFPS), vp) );
	}
	return true;
	JL_BAD;
}


#ifndef JL_NOTHREAD

/**doc
$TOC_MEMBER $INAME
 $TYPE HANDLE $INAME()
  Passively waits for the SDL drawing surface ready state through the processEvents function.
**/
struct SurfaceReadyProcessEvent {
	
	ProcessEvent pe;

	bool cancel;
	JSObject *callbackFctThis;
	jsval callbackFctVal;
};

S_ASSERT( offsetof(SurfaceReadyProcessEvent, pe) == 0 );

static bool SurfaceReadyPrepareWait( volatile ProcessEvent *pe, JSContext *, JSObject * ) {

	SurfaceReadyProcessEvent *upe = (SurfaceReadyProcessEvent*)pe;
	upe->cancel = false;

	return true;
}

static void SurfaceReadyStartWait( volatile ProcessEvent *pe ) {

	SurfaceReadyProcessEvent *upe = (SurfaceReadyProcessEvent*)pe;

	JLMutexAcquire(surfaceReadyLock);
//	surfaceIdle = true;
	while ( !surfaceReady && !upe->cancel )
		JLCondWait(surfaceReadyCond, surfaceReadyLock);
	JLMutexRelease(surfaceReadyLock);
}

static bool SurfaceReadyCancelWait( volatile ProcessEvent *pe ) {

	SurfaceReadyProcessEvent *upe = (SurfaceReadyProcessEvent*)pe;

	JLMutexAcquire(surfaceReadyLock);
	upe->cancel = true;
	JLCondBroadcast(surfaceReadyCond);
	JLMutexRelease(surfaceReadyLock);
	return true;
}


bool SurfaceReadyEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject *obj ) {

//	JLMutexAcquire(surfaceReadyLock);
//	surfaceIdle = false;
//	JLMutexRelease(surfaceReadyLock);

	SurfaceReadyProcessEvent *upe = (SurfaceReadyProcessEvent*)pe;

	*hasEvent = surfaceReady;
	if ( !*hasEvent )
		return true;

	if ( upe->callbackFctVal.isUndefined() )
		return true;

	jsval rval;
	JL_CHK( JS_CallFunctionValue(cx, upe->callbackFctThis, upe->callbackFctVal, 0, NULL, &rval) );
	return true;
	JL_BAD;
}


DEFINE_FUNCTION( surfaceReadyEvents ) {

	JL_ASSERT_ARGC_RANGE(0, 1);

	SurfaceReadyProcessEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), &upe, NULL, JL_RVAL) );
	upe->pe.prepareWait = SurfaceReadyPrepareWait;
	upe->pe.startWait = SurfaceReadyStartWait;
	upe->pe.cancelWait = SurfaceReadyCancelWait;
	upe->pe.endWait = SurfaceReadyEndWait;

	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_CALLABLE(1);

		upe->callbackFctVal = JL_ARG(1);
		upe->callbackFctThis = JSVAL_TO_OBJECT(JL_OBJVAL); // store "this" object.

		JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, upe->callbackFctVal) ); // GC protection
		JL_CHK( SetHandleSlot(cx, *JL_RVAL, 1, JL_OBJVAL) ); // GC protection
	} else {

		upe->callbackFctVal = JSVAL_VOID;
	}

	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE HANDLE $INAME()
  Passively waits for a SDL event through the processEvents function.
  $H note
   Receiving a SDL event do not mean that the surface is ready to be drawn.$LF
	Call processEvents( SurfaceReadyEvents() ); to ensur the surface is ready to use.
**/
struct SdlEventsProcessEvent {
	
	ProcessEvent pe;

	bool cancel;
	JSObject *thisObj;
	JSObject *listenersObj;
};

S_ASSERT( offsetof(SdlEventsProcessEvent, pe) == 0 );

static bool SDLPrepareWait( volatile ProcessEvent *pe, JSContext *cx, JSObject *obj ) {
	
	SdlEventsProcessEvent *upe = (SdlEventsProcessEvent*)pe;
	upe->cancel = false;

	return true;
}

void SDLStartWait( volatile ProcessEvent *pe ) {

	SdlEventsProcessEvent *upe = (SdlEventsProcessEvent*)pe;
	int status = 0;
	JLMutexAcquire(sdlEventsLock);
	while ( !upe->cancel && (status = SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_ALLEVENTS)) == 0 ) // no cancel and no SDL event
		JLCondWait(sdlEventsCond, sdlEventsLock);
	JLMutexRelease(sdlEventsLock);
	ASSERT( status != -1 );
	// ASSERT( upe->cancel || status == 1 ); // (TBD) understand this case
}

bool SDLCancelWait( volatile ProcessEvent *pe ) {

	SdlEventsProcessEvent *upe = (SdlEventsProcessEvent*)pe;

	JLMutexAcquire(sdlEventsLock);
	upe->cancel = true;
	JLCondBroadcast(sdlEventsCond);
	JLMutexRelease(sdlEventsLock);

	return true;
}

bool SDLEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	SdlEventsProcessEvent *upe = (SdlEventsProcessEvent*)pe;

	int status;
	bool fired; // unused
	jsval rval; // unused
	SDL_Event ev[64];

	*hasEvent = false;

	for (;;) {

		status = SDL_PeepEvents(ev, COUNTOF(ev), SDL_GETEVENT, SDL_ALLEVENTS);

		if ( status == 0 )
			break;

		if ( status < 0 )
			JL_CHK( ThrowSdlError(cx) );

		*hasEvent = true;

		for ( int i = 0; i < status; i++ )
			JL_CHK( FireListener(cx, upe->thisObj, upe->listenersObj, &ev[i], &rval, &fired) );

		if ( status < COUNTOF(ev) )
			break;
	}

	return true;
	JL_BAD;
}


DEFINE_FUNCTION( sdlEvents ) {

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	SdlEventsProcessEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), &upe, NULL, JL_RVAL) );
	upe->pe.prepareWait = SDLPrepareWait;
	upe->pe.startWait = SDLStartWait;
	upe->pe.cancelWait = SDLCancelWait;
	upe->pe.endWait = SDLEndWait;

	upe->thisObj = JSVAL_TO_OBJECT(JL_OBJVAL); // store "this" object.
	upe->listenersObj = JSVAL_TO_OBJECT( JL_ARG(1) );

	JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, JL_OBJVAL) ); // GC protection
	JL_CHK( SetHandleSlot(cx, *JL_RVAL, 1, JL_ARG(1)) ); // GC protection

	return true;
	JL_BAD;
}

#endif // JL_NOTHREAD


CONFIGURE_STATIC

	REVISION(jl::SvnRevToInt("$Revision$"))

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( getVideoModeList )
		FUNCTION( videoModeOK )
		FUNCTION_ARGC( setVideoMode, 5 )
//		FUNCTION( toggleFullScreen )
		FUNCTION( iconify )
		FUNCTION_ARGC( setGamma, 3 )
		FUNCTION( glSwapBuffers )
		FUNCTION_ARGC( glSetAttribute, 2 )
		FUNCTION_ARGC( glGetAttribute, 1 )
		FUNCTION_ARGC( pollEvent, 1 )
		FUNCTION_ARGC( warpMouse, 2 )
		FUNCTION_ARGC( setCursor, 1 )
		FUNCTION_ARGC( getKeyState, 1 )
		FUNCTION_ARGC( getKeyName, 1 )
#ifndef JL_NOTHREAD
		FUNCTION( surfaceReadyEvents )
		FUNCTION_ARGC( sdlEvents, 1 )
#endif // JL_NOTHREAD
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC

		PROPERTY( maxFPS )
		PROPERTY_SETTER( icon )
		PROPERTY( caption )
		PROPERTY( grabInput )
		PROPERTY( showCursor )
		PROPERTY( keyRepeatInterval )
		PROPERTY( keyRepeatDelay )
		PROPERTY( unicodeKeyboardTranslation )
		PROPERTY_GETTER( desktopWidth )
		PROPERTY_GETTER( desktopHeight )
		PROPERTY_GETTER( desktopBitsPerPixel )
		PROPERTY_GETTER( videoWidth )
		PROPERTY_GETTER( videoHeight )
		PROPERTY_GETTER( videoBitsPerPixel )
		PROPERTY_GETTER( videoFlags )
		PROPERTY_GETTER( videoDriverName )
//		PROPERTY_GETTER( isFullScreen )
		PROPERTY_GETTER( mouseX )
		PROPERTY_GETTER( mouseY )
		PROPERTY_GETTER( buttonState )
		PROPERTY_GETTER( modifierState )
		PROPERTY_GETTER( appStateActive )

		PROPERTY_GETTER(hasRDTSC)
		PROPERTY_GETTER(hasMMX)
		PROPERTY_GETTER(hasMMXExt)
		PROPERTY_GETTER(has3DNow)
		PROPERTY_GETTER(has3DNowExt)
		PROPERTY_GETTER(hasSSE)
		PROPERTY_GETTER(hasSSE2)
		PROPERTY_GETTER(hasAltiVec)

//		PROPERTY_GETTER( version )

	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST

		CONST_INTEGER( SWSURFACE , SDL_SWSURFACE  )
		CONST_INTEGER( HWSURFACE , SDL_HWSURFACE  )
		CONST_INTEGER( ASYNCBLIT , SDL_ASYNCBLIT  )

		CONST_INTEGER( ANYFORMAT   , SDL_ANYFORMAT  )
		CONST_INTEGER( HWPALETTE   , SDL_HWPALETTE  )
		CONST_INTEGER( DOUBLEBUF   , SDL_DOUBLEBUF  )
		CONST_INTEGER( FULLSCREEN  , SDL_FULLSCREEN )
		CONST_INTEGER( OPENGL      , SDL_OPENGL     )
		CONST_INTEGER( OPENGLBLIT  , SDL_OPENGLBLIT )
		CONST_INTEGER( RESIZABLE   , SDL_RESIZABLE  )
		CONST_INTEGER( NOFRAME     , SDL_NOFRAME    )

//		CONST_INTEGER( HWACCEL	 ,SDL_HWACCEL    )
//		CONST_INTEGER( SRCCOLORKEY,SDL_SRCCOLORKEY)
//		CONST_INTEGER( RLEACCELOK ,SDL_RLEACCELOK )
//		CONST_INTEGER( RLEACCEL	 ,SDL_RLEACCEL   )
//		CONST_INTEGER( SRCALPHA	 ,SDL_SRCALPHA   )
//		CONST_INTEGER( PREALLOC	 ,SDL_PREALLOC   )

		CONST_INTEGER( GL_RED_SIZE           , SDL_GL_RED_SIZE )
		CONST_INTEGER( GL_GREEN_SIZE         , SDL_GL_GREEN_SIZE )
		CONST_INTEGER( GL_BLUE_SIZE          , SDL_GL_BLUE_SIZE )
		CONST_INTEGER( GL_ALPHA_SIZE         , SDL_GL_ALPHA_SIZE )
		CONST_INTEGER( GL_BUFFER_SIZE        , SDL_GL_BUFFER_SIZE )
		CONST_INTEGER( GL_DOUBLEBUFFER       , SDL_GL_DOUBLEBUFFER )
		CONST_INTEGER( GL_DEPTH_SIZE         , SDL_GL_DEPTH_SIZE )
		CONST_INTEGER( GL_STENCIL_SIZE       , SDL_GL_STENCIL_SIZE )
		CONST_INTEGER( GL_ACCUM_RED_SIZE     , SDL_GL_ACCUM_RED_SIZE )
		CONST_INTEGER( GL_ACCUM_GREEN_SIZE   , SDL_GL_ACCUM_GREEN_SIZE )
		CONST_INTEGER( GL_ACCUM_BLUE_SIZE    , SDL_GL_ACCUM_BLUE_SIZE )
		CONST_INTEGER( GL_ACCUM_ALPHA_SIZE   , SDL_GL_ACCUM_ALPHA_SIZE )
		CONST_INTEGER( GL_STEREO             , SDL_GL_STEREO )
		CONST_INTEGER( GL_MULTISAMPLEBUFFERS , SDL_GL_MULTISAMPLEBUFFERS )
		CONST_INTEGER( GL_MULTISAMPLESAMPLES , SDL_GL_MULTISAMPLESAMPLES )
		CONST_INTEGER( GL_ACCELERATED_VISUAL , SDL_GL_ACCELERATED_VISUAL ) // Recent ATI drivers not cooperating with SDL_GL_ACCELERATED_VISUAL (http://icculus.org/pipermail/quake3-bugzilla/2008-January/000485.html)
		CONST_INTEGER( GL_SWAP_CONTROL       , SDL_GL_SWAP_CONTROL )

		CONST_INTEGER( K_UNKNOWN			, SDLK_UNKNOWN)
		CONST_INTEGER( K_FIRST				, SDLK_FIRST)
		CONST_INTEGER( K_BACKSPACE			, SDLK_BACKSPACE)
		CONST_INTEGER( K_TAB					, SDLK_TAB)
		CONST_INTEGER( K_CLEAR				, SDLK_CLEAR)
		CONST_INTEGER( K_RETURN				, SDLK_RETURN)
		CONST_INTEGER( K_PAUSE				, SDLK_PAUSE)
		CONST_INTEGER( K_ESCAPE				, SDLK_ESCAPE)
		CONST_INTEGER( K_SPACE				, SDLK_SPACE)
		CONST_INTEGER( K_EXCLAIM			, SDLK_EXCLAIM)
		CONST_INTEGER( K_QUOTEDBL			, SDLK_QUOTEDBL)
		CONST_INTEGER( K_HASH				, SDLK_HASH)
		CONST_INTEGER( K_DOLLAR				, SDLK_DOLLAR)
		CONST_INTEGER( K_AMPERSAND			, SDLK_AMPERSAND)
		CONST_INTEGER( K_QUOTE				, SDLK_QUOTE)
		CONST_INTEGER( K_LEFTPAREN			, SDLK_LEFTPAREN)
		CONST_INTEGER( K_RIGHTPAREN		, SDLK_RIGHTPAREN)
		CONST_INTEGER( K_ASTERISK			, SDLK_ASTERISK)
		CONST_INTEGER( K_PLUS				, SDLK_PLUS)
		CONST_INTEGER( K_COMMA				, SDLK_COMMA)
		CONST_INTEGER( K_MINUS				, SDLK_MINUS)
		CONST_INTEGER( K_PERIOD				, SDLK_PERIOD)
		CONST_INTEGER( K_SLASH				, SDLK_SLASH)
		CONST_INTEGER( K_0					, SDLK_0)
		CONST_INTEGER( K_1					, SDLK_1)
		CONST_INTEGER( K_2					, SDLK_2)
		CONST_INTEGER( K_3					, SDLK_3)
		CONST_INTEGER( K_4					, SDLK_4)
		CONST_INTEGER( K_5					, SDLK_5)
		CONST_INTEGER( K_6					, SDLK_6)
		CONST_INTEGER( K_7					, SDLK_7)
		CONST_INTEGER( K_8					, SDLK_8)
		CONST_INTEGER( K_9					, SDLK_9)
		CONST_INTEGER( K_COLON				, SDLK_COLON)
		CONST_INTEGER( K_SEMICOLON			, SDLK_SEMICOLON)
		CONST_INTEGER( K_LESS				, SDLK_LESS)
		CONST_INTEGER( K_EQUALS				, SDLK_EQUALS)
		CONST_INTEGER( K_GREATER			, SDLK_GREATER)
		CONST_INTEGER( K_QUESTION			, SDLK_QUESTION)
		CONST_INTEGER( K_AT					, SDLK_AT)
		CONST_INTEGER( K_LEFTBRACKET		, SDLK_LEFTBRACKET)
		CONST_INTEGER( K_BACKSLASH			, SDLK_BACKSLASH)
		CONST_INTEGER( K_RIGHTBRACKET		, SDLK_RIGHTBRACKET)
		CONST_INTEGER( K_CARET				, SDLK_CARET)
		CONST_INTEGER( K_UNDERSCORE		, SDLK_UNDERSCORE)
		CONST_INTEGER( K_BACKQUOTE			, SDLK_BACKQUOTE)
		CONST_INTEGER( K_a					, SDLK_a)
		CONST_INTEGER( K_b					, SDLK_b)
		CONST_INTEGER( K_c					, SDLK_c)
		CONST_INTEGER( K_d					, SDLK_d)
		CONST_INTEGER( K_e					, SDLK_e)
		CONST_INTEGER( K_f					, SDLK_f)
		CONST_INTEGER( K_g					, SDLK_g)
		CONST_INTEGER( K_h					, SDLK_h)
		CONST_INTEGER( K_i					, SDLK_i)
		CONST_INTEGER( K_j					, SDLK_j)
		CONST_INTEGER( K_k					, SDLK_k)
		CONST_INTEGER( K_l					, SDLK_l)
		CONST_INTEGER( K_m					, SDLK_m)
		CONST_INTEGER( K_n					, SDLK_n)
		CONST_INTEGER( K_o					, SDLK_o)
		CONST_INTEGER( K_p					, SDLK_p)
		CONST_INTEGER( K_q					, SDLK_q)
		CONST_INTEGER( K_r					, SDLK_r)
		CONST_INTEGER( K_s					, SDLK_s)
		CONST_INTEGER( K_t					, SDLK_t)
		CONST_INTEGER( K_u					, SDLK_u)
		CONST_INTEGER( K_v					, SDLK_v)
		CONST_INTEGER( K_w					, SDLK_w)
		CONST_INTEGER( K_x					, SDLK_x)
		CONST_INTEGER( K_y					, SDLK_y)
		CONST_INTEGER( K_z					, SDLK_z)
		CONST_INTEGER( K_DELETE				, SDLK_DELETE)
		CONST_INTEGER( K_KP0					, SDLK_KP0)
		CONST_INTEGER( K_KP1					, SDLK_KP1)
		CONST_INTEGER( K_KP2					, SDLK_KP2)
		CONST_INTEGER( K_KP3					, SDLK_KP3)
		CONST_INTEGER( K_KP4					, SDLK_KP4)
		CONST_INTEGER( K_KP5					, SDLK_KP5)
		CONST_INTEGER( K_KP6					, SDLK_KP6)
		CONST_INTEGER( K_KP7					, SDLK_KP7)
		CONST_INTEGER( K_KP8					, SDLK_KP8)
		CONST_INTEGER( K_KP9					, SDLK_KP9)
		CONST_INTEGER( K_KP_PERIOD			, SDLK_KP_PERIOD)
		CONST_INTEGER( K_KP_DIVIDE			, SDLK_KP_DIVIDE)
		CONST_INTEGER( K_KP_MULTIPLY		, SDLK_KP_MULTIPLY)
		CONST_INTEGER( K_KP_MINUS			, SDLK_KP_MINUS)
		CONST_INTEGER( K_KP_PLUS			, SDLK_KP_PLUS)
		CONST_INTEGER( K_KP_ENTER			, SDLK_KP_ENTER)
		CONST_INTEGER( K_KP_EQUALS			, SDLK_KP_EQUALS)
		CONST_INTEGER( K_UP					, SDLK_UP)
		CONST_INTEGER( K_DOWN				, SDLK_DOWN)
		CONST_INTEGER( K_RIGHT				, SDLK_RIGHT)
		CONST_INTEGER( K_LEFT				, SDLK_LEFT)
		CONST_INTEGER( K_INSERT				, SDLK_INSERT)
		CONST_INTEGER( K_HOME				, SDLK_HOME)
		CONST_INTEGER( K_END					, SDLK_END)
		CONST_INTEGER( K_PAGEUP				, SDLK_PAGEUP)
		CONST_INTEGER( K_PAGEDOWN			, SDLK_PAGEDOWN)
		CONST_INTEGER( K_F1					, SDLK_F1)
		CONST_INTEGER( K_F2					, SDLK_F2)
		CONST_INTEGER( K_F3					, SDLK_F3)
		CONST_INTEGER( K_F4					, SDLK_F4)
		CONST_INTEGER( K_F5					, SDLK_F5)
		CONST_INTEGER( K_F6					, SDLK_F6)
		CONST_INTEGER( K_F7					, SDLK_F7)
		CONST_INTEGER( K_F8					, SDLK_F8)
		CONST_INTEGER( K_F9					, SDLK_F9)
		CONST_INTEGER( K_F10					, SDLK_F10)
		CONST_INTEGER( K_F11					, SDLK_F11)
		CONST_INTEGER( K_F12					, SDLK_F12)
		CONST_INTEGER( K_F13					, SDLK_F13)
		CONST_INTEGER( K_F14					, SDLK_F14)
		CONST_INTEGER( K_F15					, SDLK_F15)
		CONST_INTEGER( K_NUMLOCK			, SDLK_NUMLOCK)
		CONST_INTEGER( K_CAPSLOCK			, SDLK_CAPSLOCK)
		CONST_INTEGER( K_SCROLLOCK			, SDLK_SCROLLOCK)
		CONST_INTEGER( K_RSHIFT				, SDLK_RSHIFT)
		CONST_INTEGER( K_LSHIFT				, SDLK_LSHIFT)
		CONST_INTEGER( K_RCTRL				, SDLK_RCTRL)
		CONST_INTEGER( K_LCTRL				, SDLK_LCTRL)
		CONST_INTEGER( K_RALT				, SDLK_RALT)
		CONST_INTEGER( K_LALT				, SDLK_LALT)
		CONST_INTEGER( K_RMETA				, SDLK_RMETA)
		CONST_INTEGER( K_LMETA				, SDLK_LMETA)
		CONST_INTEGER( K_LSUPER				, SDLK_LSUPER)
		CONST_INTEGER( K_RSUPER				, SDLK_RSUPER)
		CONST_INTEGER( K_MODE				, SDLK_MODE)
		CONST_INTEGER( K_COMPOSE			, SDLK_COMPOSE)
		CONST_INTEGER( K_HELP				, SDLK_HELP)
		CONST_INTEGER( K_PRINT				, SDLK_PRINT)
		CONST_INTEGER( K_SYSREQ				, SDLK_SYSREQ)
		CONST_INTEGER( K_BREAK				, SDLK_BREAK)
		CONST_INTEGER( K_MENU				, SDLK_MENU)
		CONST_INTEGER( K_POWER				, SDLK_POWER)
		CONST_INTEGER( K_EURO				, SDLK_EURO)
		CONST_INTEGER( K_UNDO				, SDLK_UNDO)

		CONST_INTEGER( BUTTON_LEFT      ,SDL_BUTTON_LEFT )
		CONST_INTEGER( BUTTON_MIDDLE    ,SDL_BUTTON_MIDDLE )
		CONST_INTEGER( BUTTON_RIGHT     ,SDL_BUTTON_RIGHT )
		CONST_INTEGER( BUTTON_WHEELUP   ,SDL_BUTTON_WHEELUP )
		CONST_INTEGER( BUTTON_WHEELDOWN ,SDL_BUTTON_WHEELDOWN )
		CONST_INTEGER( BUTTON_X1        ,SDL_BUTTON_X1 )
		CONST_INTEGER( BUTTON_X2        ,SDL_BUTTON_X2 )

		CONST_INTEGER( BUTTON_LMASK		,SDL_BUTTON_LMASK )
		CONST_INTEGER( BUTTON_MMASK		,SDL_BUTTON_MMASK )
		CONST_INTEGER( BUTTON_RMASK		,SDL_BUTTON_RMASK )
		CONST_INTEGER( BUTTON_UPMASK		,SDL_BUTTON(SDL_BUTTON_WHEELUP) )
		CONST_INTEGER( BUTTON_DOWNMASK	,SDL_BUTTON(SDL_BUTTON_WHEELDOWN) )
		CONST_INTEGER( BUTTON_X1MASK		,SDL_BUTTON_X1MASK )
		CONST_INTEGER( BUTTON_X2MASK		,SDL_BUTTON_X2MASK )

		CONST_INTEGER( KMOD_NONE		,KMOD_NONE		)
		CONST_INTEGER( KMOD_LSHIFT		,KMOD_LSHIFT	)
		CONST_INTEGER( KMOD_RSHIFT		,KMOD_RSHIFT	)
		CONST_INTEGER( KMOD_LCTRL		,KMOD_LCTRL		)
		CONST_INTEGER( KMOD_RCTRL		,KMOD_RCTRL		)
		CONST_INTEGER( KMOD_LALT		,KMOD_LALT		)
		CONST_INTEGER( KMOD_RALT		,KMOD_RALT		)
		CONST_INTEGER( KMOD_LMETA		,KMOD_LMETA		)
		CONST_INTEGER( KMOD_RMETA		,KMOD_RMETA		)
		CONST_INTEGER( KMOD_NUM			,KMOD_NUM		)
		CONST_INTEGER( KMOD_CAPS		,KMOD_CAPS		)
		CONST_INTEGER( KMOD_MODE		,KMOD_MODE		)

	END_CONST

END_STATIC

/*
manage mouse acceleration for win32:
	http://www.google.fr/codesearch?hl=fr&q=disable+%22mouse+acceleration%22+-x11+show:yYwX0Cc1jnM:XGt_kaGeQc8:yYwX0Cc1jnM&sa=N&cd=1&ct=rc&cs_p=http://hg.openjdk.java.net/jdk7/jaxp/jdk&cs_f=src/windows/native/sun/windows/awt_Robot.cpp#l62

*/
