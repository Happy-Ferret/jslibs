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

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_STATIC

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
DEFINE_FUNCTION_FAST( GetVideoModeList ) {

	JL_S_ASSERT_ARG_MIN(2);

	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo(); // If called before SDL_SetVideoMode(), 'vfmt' is the pixel format of the "best" video mode.
	SDL_PixelFormat format;
	format = *videoInfo->vfmt;

	double flags;

	if ( JL_FARG_ISDEF(1) ) {

		unsigned int bpp;
		JL_CHK( JsvalToUInt(cx, JL_FARG(1), &bpp) );
		format.BitsPerPixel = bpp;
		format.BytesPerPixel = bpp / 8;
	}

	if ( JL_FARG_ISDEF(2) )
		JL_CHK( JsvalToDouble(cx, JL_FARG(2), &flags) );
	else
		flags = 0;

	SDL_Rect **modes = SDL_ListModes(&format, (Uint32)flags);

	if ( modes == (SDL_Rect **)-1 ) {

		// any dimension is okay for this format.
		*JL_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	JSObject *modesArray = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK(modesArray);
	*JL_FRVAL = OBJECT_TO_JSVAL( modesArray );

	jsval tmp;
	for ( int i = 0; modes[i] != NULL; i++ ) {

		JSObject *rectArray = JS_NewArrayObject(cx, 2, NULL);
		JL_CHK(rectArray);

		tmp = OBJECT_TO_JSVAL(rectArray);
		JL_CHK( JS_SetElement(cx, modesArray, i, &tmp) );

		tmp = INT_TO_JSVAL(modes[i]->w);
		JL_CHK( JS_SetElement(cx, rectArray, 0, &tmp) );

		tmp = INT_TO_JSVAL(modes[i]->h);
		JL_CHK( JS_SetElement(cx, rectArray, 1, &tmp) );
	}
	return JS_TRUE;
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
DEFINE_FUNCTION_FAST( VideoModeOK ) {

	int width, height, bpp;
	Uint32 flags;

	if ( JL_FARG_ISDEF(1) )
		JL_CHK( JsvalToInt(cx, JL_FARG(1), &width) );
	else
		width = 0; // by default, use current width

	if ( JL_FARG_ISDEF(2) )
		JL_CHK( JsvalToInt(cx, JL_FARG(2), &height) );
	else
		height = 0; // by default, use current height

	if ( JL_FARG_ISDEF(3) )
		JL_CHK( JsvalToInt(cx, JL_FARG(3), &bpp) );
	else
		bpp = 0; // by default, use current bpp

	if ( JL_FARG_ISDEF(4) ) {

		double tmp;
		JL_CHK( JsvalToDouble(cx, JL_FARG(4), &tmp) ); // we need to use doubles because some values are greater than 2^31
		flags = (Uint32)tmp;
	} else
		flags = 0;

	int status = SDL_VideoModeOK(width, height, bpp, (Uint32)flags);
//	JL_CHK( BoolToJsval(cx, status != 0, JL_FRVAL) );
	JL_CHK( IntToJsval(cx, status, JL_FRVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( width, height [ , bitsPerPixel [ , flags ] ] )
  Set the requested video mode (allocating a shadow buffer if necessary).
  $H arguments
   $ARG $INT width: with
   $ARG $INT height: height
   $ARG $INT bitsPerPixel: bit depth, the number of bits per pixel (8, 16, 32). If omited, use the current bpp value.
   $ARG bitmsak flags: a bitwise-ored combination of the following flags. If omited, use the previous flags.
    SWSURFACE, HWSURFACE, ASYNCBLIT, ANYFORMAT, HWPALETTE, DOUBLEBUF, FULLSCREEN, OPENGL, OPENGLBLIT, RESIZABLE, NOFRAME HWACCEL, SRCCOLORKEY, RLEACCELOK, RLEACCEL, SRCALPHA, PREALLOC
**/
DEFINE_FUNCTION_FAST( SetVideoMode ) {

	int width, height, bpp;
	Uint32 flags;

	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo(); // If called before SDL_SetVideoMode(), 'vfmt' is the pixel format of the "best" video mode.
//	SDL_PixelFormat format = *videoInfo->vfmt;

	const SDL_Surface* currentSurface = SDL_GetVideoSurface();

	if ( JL_FARG_ISDEF(1) )
		JL_CHK( JsvalToInt(cx, JL_FARG(1), &width) );
	else
		width = 0; // by default, use current width

	if ( JL_FARG_ISDEF(2) )
		JL_CHK( JsvalToInt(cx, JL_FARG(2), &height) );
	else
		height = 0; // by default, use current height

	if ( JL_FARG_ISDEF(3) )
		JL_CHK( JsvalToInt(cx, JL_FARG(3), &bpp) );
	else
		bpp = currentSurface != NULL ? currentSurface->format->BitsPerPixel : 0; // by default, use current or the default bpp

	if ( JL_FARG_ISDEF(4) ) {

		double tmp;
		JL_CHK( JsvalToDouble(cx, JL_FARG(4), &tmp) ); // we need to use doubles because some values are greater than 2^31
		flags = (Uint32)tmp;
	} else
		flags = currentSurface != NULL ? currentSurface->flags : 0; // if not given, use the previous setting or a default value.

	SDL_Surface *surface = SDL_SetVideoMode(width, height, bpp, flags);
	if ( surface == NULL )
		return ThrowSdlError(cx);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the current video surface width.
**/
DEFINE_PROPERTY( videoWidth ) {

	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
	*vp = videoInfo != NULL ? INT_TO_JSVAL( videoInfo->current_w ) : 0;
	return JS_TRUE;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the current video surface height.
**/
DEFINE_PROPERTY( videoHeight ) {

	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
	*vp = videoInfo != NULL ? INT_TO_JSVAL( videoInfo->current_h ) : 0;
	return JS_TRUE;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE ImageObject $INAME $WRITEONLY
  Sets the window manager icon for the display window.
**/
DEFINE_PROPERTY( icon ) {

	jsval image = *vp;

	if ( JSVAL_IS_VOID(image) ) {

		SDL_WM_SetIcon(NULL, NULL);
		return JS_TRUE;
	}

	JL_S_ASSERT_OBJECT(image);

	JSObject *imageObj = JSVAL_TO_OBJECT( image );
	int sWidth, sHeight, sChannels;
	JL_CHK( GetPropertyInt(cx, imageObj, "width", &sWidth) );
	JL_CHK( GetPropertyInt(cx, imageObj, "height", &sHeight) );
	JL_CHK( GetPropertyInt(cx, imageObj, "channels", &sChannels) );

	JL_S_ASSERT( sChannels == 3 || sChannels == 4, "Unsupported image format." );

	const char *sBuffer;
	size_t bufferLength;
	JL_CHK( JsvalToStringAndLength(cx, &image, &sBuffer, &bufferLength ) ); // warning: GC on the returned buffer !

	JL_S_ASSERT( bufferLength == sWidth * sHeight * sChannels * 1, "Invalid image format." );

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

	SDL_WM_SetIcon(surface, NULL); // (TBD) manage mask with image alpha ?
	SDL_FreeSurface(surface);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Toggle fullscreen mode without changing the contents of the screen.
  $H return value
   true if this function was able to toggle fullscreen mode (change from running in a window to fullscreen, or vice-versa). false if it is not implemented, or fails.
**/
DEFINE_FUNCTION_FAST( ToggleFullScreen ) {

	SDL_Surface *surface = SDL_GetVideoSurface();
	int status = SDL_WM_ToggleFullScreen( surface );
	*JL_FRVAL = status == 1 ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
  Is true if the current video surface is a full screen display
**/
DEFINE_PROPERTY( fullScreen ) {

	bool fullScreen = ( (SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) != 0 );
	JL_CHK( BoolToJsval(cx, fullScreen, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Iconify the window in window managed environments. A successful iconification will result in an SDL_APPACTIVE loss event.
**/
DEFINE_FUNCTION_FAST( Iconify ) {

	SDL_WM_IconifyWindow();
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
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
DEFINE_FUNCTION_FAST( SetGamma ) {

	JL_S_ASSERT_ARG_MIN(3);
	float r,g,b;
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &r) );
	JL_CHK( JsvalToFloat(cx, JL_FARG(2), &g) );
	JL_CHK( JsvalToFloat(cx, JL_FARG(3), &b) );
	if ( SDL_SetGamma(r, g, b) != 0 )
		return ThrowSdlError(cx);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Perform a GL buffer swap on the current GL context.
**/
DEFINE_FUNCTION_FAST( GlSwapBuffers ) {

	SDL_GL_SwapBuffers();
	// (TBD) check error	*SDL_GetError() != '\0' ???
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( attribute, value )
  Set an attribute of the OpenGL subsystem before intialization.
  $H arguments
   $ARG $ENUM attribute:
   $ARG $INT value:
**/
DEFINE_FUNCTION_FAST( GlSetAttribute ) {

	int attr;
	int value;
	JL_S_ASSERT_ARG_MIN(2);
	JL_CHK( JsvalToInt(cx, JL_FARG(1), &attr) );
	JL_CHK( JsvalToInt(cx, JL_FARG(2), &value) );
	if ( SDL_GL_SetAttribute((SDL_GLattr)attr, value) == -1 )
		return ThrowSdlError(cx);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
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
DEFINE_FUNCTION_FAST( GlGetAttribute ) {

	int attr;
	int value;
	JL_S_ASSERT_ARG_MIN(2);
	JL_CHK( JsvalToInt(cx, JL_FARG(1), &attr) );
	if ( SDL_GL_GetAttribute((SDL_GLattr)attr, &value) == -1 )
		return ThrowSdlError(cx);
	JL_CHK( IntToJsval(cx, value, JL_FRVAL) );
	return JS_TRUE;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Sets/Gets the title of the display window, if any.
**/
DEFINE_PROPERTY_SETTER( caption ) {

	const char *title;
	JL_CHK( JsvalToString(cx, vp, &title) );
	SDL_WM_SetCaption(title, NULL);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( caption ) {

	char *title;
	SDL_WM_GetCaption(&title, NULL);
	if ( title == NULL ) // (TBD) check if possible
		*vp = JSVAL_VOID;
	else
		JL_CHK( StringToJsval(cx, title, vp) );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Sets/Gets the input grab state.
  Grabbing means that the mouse is confined to the application window, and nearly all keyboard input is passed directly to the application, and not interpreted by a window manager, if any.
**/
DEFINE_PROPERTY_SETTER( grabInput ) {

	bool grab;
	JL_CHK( JsvalToBool(cx, *vp, &grab) );
	SDL_WM_GrabInput( grab ? SDL_GRAB_ON : SDL_GRAB_OFF );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( grabInput ) {

	SDL_GrabMode mode = SDL_WM_GrabInput( SDL_GRAB_QUERY );
	JL_CHK( BoolToJsval(cx, mode == SDL_GRAB_ON ? true : false, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Sets/Gets whether or not the cursor is shown on the screen. The cursor start off displayed, but can be turned off.
**/
DEFINE_PROPERTY_SETTER( showCursor ) {

	bool show;
	JL_CHK( JsvalToBool(cx, *vp, &show) );
	SDL_ShowCursor( show ? 1 : 0 );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( showCursor ) {

	int show = SDL_ShowCursor( -1 ); // query
	JL_CHK( BoolToJsval(cx, show == 0 ? false : true, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( image )
 Set the currently active cursor to the specified one. If the cursor is currently visible, the change will be immediately represented on the display.
  $H arguments
   $ARG ImageObject image:
**/
DEFINE_FUNCTION_FAST( SetCursor ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_OBJECT( JL_FARG(1) );
	JSObject *cursorObj = JSVAL_TO_OBJECT( JL_FARG(1) );
	JL_S_ASSERT_CLASS( cursorObj, classCursor );
	SDL_Cursor *cursor = (SDL_Cursor *)JL_GetPrivate(cx, cursorObj);
	JL_S_ASSERT_RESOURCE( cursor );
	SDL_SetCursor(cursor);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Is the name of the video driver or undefined  if it has not been initialized.
**/
DEFINE_PROPERTY( videoDriverName ) {

	char name[1024];
	char *status = SDL_VideoDriverName(name, sizeof(name));
	if ( status != NULL )
		JL_CHK( StringToJsval(cx, name, vp) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


// see PollEvent
JSBool FireListener( JSContext *cx, JSObject *listenerObj, SDL_Event *ev, jsval *rval ) {

	jsval fVal;

//	printf("DBG: event %d\n", ev->type);
	switch (ev->type) {
		case SDL_ACTIVEEVENT:
			JL_CHK( JS_GetProperty(cx, listenerObj, "onActive", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

				jsval argv[] = {
					ev->active.gain == 1 ? JSVAL_TRUE : JSVAL_FALSE
				};
				JL_CHK( JS_CallFunctionValue(cx, listenerObj, fVal, COUNTOF(argv), argv, rval) );
			}
			break;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
			JL_CHK( JS_GetProperty(cx, listenerObj, ev->type == SDL_KEYDOWN ? "onKeyDown" : "onKeyUp", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

				JSString *ucChar = JS_NewUCStringCopyN(cx, &ev->key.keysym.unicode, 1);
				jsval argv[] = {
					INT_TO_JSVAL(ev->key.keysym.sym),
					INT_TO_JSVAL(ev->key.keysym.mod),
					STRING_TO_JSVAL(ucChar),
					INT_TO_JSVAL(ev->key.keysym.scancode),
				};

				JSTempValueRooter tvr;
				JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr); // protects the new string against the GC
				JSBool status = JS_CallFunctionValue(cx, listenerObj, fVal, COUNTOF(argv), argv, rval);
				JS_POP_TEMP_ROOT(cx, &tvr);
				JL_CHK( status );
			}
			break;

		case SDL_MOUSEMOTION:
			JL_CHK( JS_GetProperty(cx, listenerObj, "onMouseMotion", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

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
				JL_CHK( JS_CallFunctionValue(cx, listenerObj, fVal, COUNTOF(argv), argv, rval) );
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			JL_CHK( JS_GetProperty(cx, listenerObj, ev->type == SDL_MOUSEBUTTONDOWN ? "onMouseButtonDown" : "onMouseButtonUp", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

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
				JL_CHK( JS_CallFunctionValue(cx, listenerObj, fVal, COUNTOF(argv), argv, rval) );
			}
			break;

		case SDL_QUIT:
			JL_CHK( JS_GetProperty(cx, listenerObj, "onQuit", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

				// no argv GC protection needed.
				JL_CHK( JS_CallFunctionValue(cx, listenerObj, fVal, 0, NULL, rval) );
			}
			break;

		case SDL_VIDEORESIZE:
			// ... and you must respond to the event by re-calling SDL_SetVideoMode() with the requested size (or another size that suits the application).
			JL_CHK( JS_GetProperty(cx, listenerObj, "onVideoResize", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

				jsval argv[] = {
					INT_TO_JSVAL(ev->resize.w),
					INT_TO_JSVAL(ev->resize.h)
				};
				// no argv GC protection needed.
				JL_CHK( JS_CallFunctionValue(cx, listenerObj, fVal, COUNTOF(argv), argv, rval) );
			}
			break;

		case SDL_VIDEOEXPOSE:
			JL_CHK( JS_GetProperty(cx, listenerObj, "onVideoExpose", &fVal) );
			if ( JsvalIsFunction(cx, fVal) ) {

				// no argv GC protection needed.
				JL_CHK( JS_CallFunctionValue(cx, listenerObj, fVal, 0, NULL, rval) );
			}
			break;
	}
	return JS_IsExceptionPending(cx) ? JS_FALSE : JS_TRUE; // (TBD) why this line is needed ?
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
    PollEvent(listeners);
   }
   }}}
**/
DEFINE_FUNCTION_FAST( PollEvent ) {

	SDL_Event ev;
	SDL_PumpEvents();

	//	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

	int status = SDL_PeepEvents(&ev, 1, SDL_GETEVENT, SDL_ALLEVENTS); // see SDL_EventState
	if ( status == -1 )
		return ThrowSdlError(cx);

	if ( status == 0 ) {
		
		*JL_FRVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	if ( JL_FARG_ISDEF(1) ) {
	
		JL_S_ASSERT_OBJECT( JL_FARG(1) );
		JL_CHK( FireListener(cx, JSVAL_TO_OBJECT(JL_FARG(1)), &ev, JL_FRVAL) );
	}

	*JL_FRVAL = JSVAL_TRUE;
	return JS_TRUE;
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
DEFINE_FUNCTION_FAST( WarpMouse ) {

	JL_S_ASSERT_ARG_MIN(2);
	unsigned int x, y;
	JL_CHK( JsvalToUInt(cx, JL_FARG(1), &x) );
	JL_CHK( JsvalToUInt(cx, JL_FARG(1), &y) );
	SDL_WarpMouse(x, y);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the current mouse cursor X position.
**/
DEFINE_PROPERTY( mouseX ) {

	int x;
	Uint8 buttonState = SDL_GetMouseState(&x, NULL); // query only button state
	*vp = INT_TO_JSVAL( x ); // query only button state
	return JS_TRUE;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the current mouse cursor Y position.
**/
DEFINE_PROPERTY( mouseY ) {

	int y;
	Uint8 buttonState = SDL_GetMouseState(NULL, &y); // query only button state
	*vp = INT_TO_JSVAL( y ); // query only button state
	return JS_TRUE;
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
DEFINE_PROPERTY( buttonState ) {

	*vp = INT_TO_JSVAL( SDL_GetMouseState(NULL, NULL) ); // query only button state
	return JS_TRUE;
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
DEFINE_PROPERTY( modifierState ) {

	*vp = INT_TO_JSVAL( SDL_GetModState() );
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( keysym )
  Get a snapshot of the current state of the keyboard.
  $H arguments
   $ARG $ENUM keysym: the key to be tested. see key constants below.
**/
DEFINE_FUNCTION_FAST( GetKeyState ) {

	JL_S_ASSERT_ARG_MIN(1);
	unsigned int key;
	JL_CHK( JsvalToUInt(cx, JL_FARG(1), &key) );
	Uint8 *keystate = SDL_GetKeyState(NULL);
	JL_CHK( BoolToJsval(cx, keystate[key] != 0, JL_FRVAL) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME( keysym )
  Get the name of an keysym.
  $H arguments
   $ARG $ENUM keysym
**/
DEFINE_FUNCTION_FAST( GetKeyName ) {

	JL_S_ASSERT_ARG_MIN(1);
	unsigned int key;
	JL_CHK( JsvalToUInt(cx, JL_FARG(1), &key) );
	char *keyName = SDL_GetKeyName((SDLKey)key);
	JSString *jsStr = JS_NewStringCopyZ(cx, keyName);
	*JL_FRVAL = STRING_TO_JSVAL(jsStr);
	return JS_TRUE;
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
	JL_CHK( JsvalToInt(cx, *vp, &delay) );
	int status = SDL_EnableKeyRepeat(delay, interval);
	if ( status == -1 )
		return ThrowSdlError(cx);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( keyRepeatDelay ) {

	int delay, interval;
	SDL_GetKeyRepeat(&delay, &interval);
	JL_CHK( IntToJsval(cx, delay, vp) );
	return JS_TRUE;
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
	JL_CHK( JsvalToInt(cx, *vp, &interval) );
	int status = SDL_EnableKeyRepeat(delay, interval);
	if ( status == -1 )
		return ThrowSdlError(cx);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( keyRepeatInterval ) {

	int delay, interval;
	SDL_GetKeyRepeat(&delay, &interval);
	JL_CHK( IntToJsval(cx, interval, vp) );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the current state of the application. If true, the user is able to see your application, otherwise it has been iconified or disabled.
**/
DEFINE_PROPERTY( appStateActive ) {

	JL_CHK( BoolToJsval(cx, (SDL_GetAppState() & SDL_APPACTIVE) != 0, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY( hasRDTSC ) {

	JL_CHK( BoolToJsval(cx, SDL_HasRDTSC() == SDL_TRUE, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY( hasMMX ) {

	JL_CHK( BoolToJsval(cx, SDL_HasMMX() == SDL_TRUE, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY( hasMMXExt ) {

	JL_CHK( BoolToJsval(cx, SDL_HasMMXExt() == SDL_TRUE, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY( has3DNow ) {

	JL_CHK( BoolToJsval(cx, SDL_Has3DNow() == SDL_TRUE, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY( has3DNowExt ) {

	JL_CHK( BoolToJsval(cx, SDL_Has3DNowExt() == SDL_TRUE, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY( hasSSE ) {

	JL_CHK( BoolToJsval(cx, SDL_HasSSE() == SDL_TRUE, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY( hasSSE2 ) {

	JL_CHK( BoolToJsval(cx, SDL_HasSSE2() == SDL_TRUE, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY( hasAltiVec ) {

	JL_CHK( BoolToJsval(cx, SDL_HasAltiVec() == SDL_TRUE, vp) );
	return JS_TRUE;
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


CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST( GetVideoModeList )
		FUNCTION_FAST( VideoModeOK )
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
		FUNCTION_FAST_ARGC( GetKeyState, 1 )
		FUNCTION_FAST_ARGC( GetKeyName, 1 )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_WRITE( icon )
		PROPERTY( caption )
		PROPERTY( grabInput )
		PROPERTY( showCursor )
		PROPERTY( keyRepeatInterval )
		PROPERTY( keyRepeatDelay )
		PROPERTY_READ( videoWidth )
		PROPERTY_READ( videoHeight )
		PROPERTY_READ( videoDriverName )
		PROPERTY_READ( fullScreen )
		PROPERTY_READ( mouseX )
		PROPERTY_READ( mouseY )
		PROPERTY_READ( buttonState )
		PROPERTY_READ( modifierState )

		PROPERTY_READ(hasRDTSC)
		PROPERTY_READ(hasMMX)
		PROPERTY_READ(hasMMXExt)
		PROPERTY_READ(has3DNow)
		PROPERTY_READ(has3DNowExt)
		PROPERTY_READ(hasSSE)
		PROPERTY_READ(hasSSE2)
		PROPERTY_READ(hasAltiVec)

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

	END_CONST_INTEGER_SPEC

END_STATIC

/*
manage mouse acceleration for win32:
	http://www.google.fr/codesearch?hl=fr&q=disable+%22mouse+acceleration%22+-x11+show:yYwX0Cc1jnM:XGt_kaGeQc8:yYwX0Cc1jnM&sa=N&cd=1&ct=rc&cs_p=http://hg.openjdk.java.net/jdk7/jaxp/jdk&cs_f=src/windows/native/sun/windows/awt_Robot.cpp#l62

*/
