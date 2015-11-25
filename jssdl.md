<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jssdl/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jssdl/qa.js) -
# jssdl module #

> jssdl is a wrapper to the Simple DirectMedia Layer (SDL) library.
> Simple DirectMedia Layer is a cross-platform multimedia library designed to provide low level access to
> audio, keyboard, mouse, joystick, 3D hardware via OpenGL, and 2D video framebuffer.
> It is used by MPEG playback software, emulators, and many popular games,
> including the award winning Linux port of "Civilization: Call To Power."
> 



---

## jssdl static members ##
- [top](#jssdl_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jssdl/static.cpp?r=2555) -

### Static functions ###

#### <font color='white' size='1'><b>GetVideoModeList</b></font> ####
> <sub>Array</sub> | _undefined_ <b>GetVideoModeList</b>( bitsPerPixel, flags )
> > Returns the available screen dimensions for the given format, sorted largest to smallest.
> > ##### arguments: #####
      1. <sub>integer</sub> _bitsPerPixel_: bit depth, the number of bits per pixel (8, 16, 32)
      1. <sub>bitmsak</sub> _flags_: a bitwise-ored combination of flags. see SetVideoMode() function.
> > ##### return value: #####
      * an Array that contains the list of available screen dimensions for the given format and video flags, sorted largest to smallest.
      * an empty Array if there are no dimensions available for a particular format.
      * _undefined_ if any dimension is okay for the given format.

#### <font color='white' size='1'><b>VideoModeOK</b></font> ####

> <sub>integer</sub> <b>VideoModeOK</b>( width, height, bitsPerPixel, flags )
> > Check to see if a particular video mode is supported.
> > ##### arguments: #####
> > > See SetVideoMode() function.

> > ##### return value: #####
> > > If the requested mode is not supported under any bit depth or returns the bits-per-pixel of the closest available mode with the iven width and height.
> > > If this bits-per-pixel is different from the one used when setting the video mode, SDL\_SetVideoMode() will succeed, but will emulate the requested bits-per-pixel with a shadow surface.

#### <font color='white' size='1'><b>SetVideoMode</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>SetVideoMode</b>( width, height [[.md](.md) , bitsPerPixel [[.md](.md) , flags ] ] )
> > Set the requested video mode (allocating a shadow buffer if necessary).
> > ##### arguments: #####
      1. <sub>integer</sub> _width_: with
      1. <sub>integer</sub> _height_: height
      1. <sub>integer</sub> _bitsPerPixel_: bit depth, the number of bits per pixel (8, 16, 32). If omited, use the current bpp value.
      1. <sub>bitmsak</sub> _flags_: a bitwise-ored combination of the following flags. If omited, use the previous flags.
> > > > SWSURFACE, HWSURFACE, ASYNCBLIT, ANYFORMAT, HWPALETTE, DOUBLEBUF, FULLSCREEN, OPENGL, OPENGLBLIT, RESIZABLE, NOFRAME HWACCEL, SRCCOLORKEY, RLEACCELOK, RLEACCEL, SRCALPHA, PREALLOC

#### <font color='white' size='1'><b>videoWidth</b></font> ####

> <b>videoWidth</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the current video surface width.

#### <font color='white' size='1'><b>videoHeight</b></font> ####

> <b>videoHeight</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the current video surface height.

#### <font color='white' size='1'><b>icon</b></font> ####

> <sub>ImageObject</sub> <b>icon</b> <font color='red'><sub>write-only</sub></font>
> > Sets the window manager icon for the display window.

#### <font color='white' size='1'><b>ToggleFullScreen</b></font> ####

> <sub>boolean</sub> <b>ToggleFullScreen</b>()
> > Toggle fullscreen mode without changing the contents of the screen.
> > ##### return value: #####
> > > true if this function was able to toggle fullscreen mode (change from running in a window to fullscreen, or vice-versa). false if it is not implemented, or fails.

#### <font color='white' size='1'><b>fullScreen</b></font> ####

> <sub>boolean</sub> <b>fullScreen</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is true if the current video surface is a full screen display

#### <font color='white' size='1'><b>Iconify</b></font> ####

> <sub>boolean</sub> <b>Iconify</b>()
> > Iconify the window in window managed environments. A successful iconification will result in an SDL\_APPACTIVE loss event.

#### <font color='white' size='1'><b>SetGamma</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>SetGamma</b>( red, green, blue )
> > Set the gamma correction for each of the color channels. The gamma values range (approximately) between 0.1 and 10.0 .
> > ##### arguments: #####
> > > ARG integer red
> > > ARG integer green
> > > ARG integer blue

#### <font color='white' size='1'><b>GlSwapBuffers</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>GlSwapBuffers</b>()
> > Perform a GL buffer swap on the current GL context.

#### <font color='white' size='1'><b>GlSetAttribute</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>GlSetAttribute</b>( attribute, value )
> > Set an attribute of the OpenGL subsystem before intialization.
> > ##### arguments: #####
      1. <sub>enum</sub> _attribute_:
      1. <sub>integer</sub> _value_:

#### <font color='white' size='1'><b>GlGetAttribute</b></font> ####

> <sub>integer</sub> <b>GlGetAttribute</b>( attribute )
> > Get an attribute of the OpenGL subsystem from the windowing interface. This is of course different from getting the values from SDL's internal OpenGL subsystem, which only stores the values you request before initialization.
> > ##### arguments: #####
      1. <sub>enum</sub> _attribute_:
      1. <sub>integer</sub> _value_:
> > ##### return value: #####
> > > the value of the requested attribute.

#### <font color='white' size='1'><b>caption</b></font> ####

> <sub>string</sub> <b>caption</b>
> > Sets/Gets the title of the display window, if any.

#### <font color='white' size='1'><b>grabInput</b></font> ####

> <sub>boolean</sub> <b>grabInput</b>
> > Sets/Gets the input grab state.
> > Grabbing means that the mouse is confined to the application window, and nearly all keyboard input is passed directly to the application, and not interpreted by a window manager, if any.

#### <font color='white' size='1'><b>showCursor</b></font> ####

> <sub>boolean</sub> <b>showCursor</b>
> > Sets/Gets whether or not the cursor is shown on the screen. The cursor start off displayed, but can be turned off.

#### <font color='white' size='1'><b>SetCursor</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>SetCursor</b>( image )
> Set the currently active cursor to the specified one. If the cursor is currently visible, the change will be immediately represented on the display.
> > ##### arguments: #####
      1. <sub>ImageObject</sub> _image_:

#### <font color='white' size='1'><b>videoDriverName</b></font> ####

> <sub>string</sub> <b>videoDriverName</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the name of the video driver or undefined  if it has not been initialized.

#### <font color='white' size='1'><b>PollEvent</b></font> ####

> <sub>value</sub> <b>PollEvent</b>( listeners )
> > ##### arguments: #####
      1. <sub>Object</sub> _listeners_: is an object that contains callback functions.
> > ##### example: #####
```
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
```

#### <font color='white' size='1'><b>WarpMouse</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>WarpMouse</b>( x, y )
> > Set the position of the mouse cursor (generates a mouse motion event).
> > ##### arguments: #####
      1. <sub>integer</sub> _x_
      1. <sub>integer</sub> _y_

#### <font color='white' size='1'><b>mouseX</b></font> ####

> <sub>integer</sub> <b>mouseX</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the current mouse cursor X position.

#### <font color='white' size='1'><b>mouseY</b></font> ####

> <sub>integer</sub> <b>mouseY</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the current mouse cursor Y position.

#### <font color='white' size='1'><b>buttonState</b></font> ####

> <sub>integer</sub> <b>buttonState</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the current state of the mouse. The current button state is returned as a button bitmask.
> > > <b><code>BUTTON_LMASK</code></b>
> > > <b><code>BUTTON_MMASK</code></b>
> > > <b><code>BUTTON_RMASK</code></b>
> > > <b><code>BUTTON_UPMASK</code></b>
> > > <b><code>BUTTON_DOWNMASK</code></b>
> > > <b><code>BUTTON_X1MASK</code></b>
> > > <b><code>BUTTON_X2MASK</code></b>

> > ##### example 1: #####
```
  var leftButtonState = ( ( <b>buttonState</b> & BUTTON_LMASK ) != 0 );
```
> > ##### example 2: #####
```
  var hasButtonDown = ( ( <b>buttonState</b> & ( BUTTON_LMASK | BUTTON_MMASK | BUTTON_RMASK ) ) != 0 );
```

#### <font color='white' size='1'><b>modifierState</b></font> ####

> <sub>integer</sub> <b>modifierState</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the current key modifier state bitmask.
> > > <b><code>KMOD_NONE</code></b>
> > > <b><code>KMOD_LSHIFT</code></b>
> > > <b><code>KMOD_RSHIFT</code></b>
> > > <b><code>KMOD_LCTRL</code></b>
> > > <b><code>KMOD_RCTRL</code></b>
> > > <b><code>KMOD_LALT</code></b>
> > > <b><code>KMOD_RALT</code></b>
> > > <b><code>KMOD_LMETA</code></b>
> > > <b><code>KMOD_RMETA</code></b>
> > > <b><code>KMOD_NUM</code></b>
> > > <b><code>KMOD_CAPS</code></b>
> > > <b><code>KMOD_MODE</code></b>

#### <font color='white' size='1'><b>GetKeyState</b></font> ####

> <sub>boolean</sub> <b>GetKeyState</b>( keysym )
> > Get a snapshot of the current state of the keyboard.
> > ##### arguments: #####
      1. <sub>enum</sub> _keysym_: the key to be tested. see key constants below.

#### <font color='white' size='1'><b>GetKeyName</b></font> ####

> <sub>string</sub> <b>GetKeyName</b>( keysym )
> > Get the name of an keysym.
> > ##### arguments: #####
      1. <sub>enum</sub> _keysym_

#### <font color='white' size='1'><b>keyRepeatDelay</b></font> ####

> <sub>integer</sub> <b>keyRepeatDelay</b>
> > Sets/Gets keyboard repeat delay. This is the initial delay in ms between the time when a key is pressed, and keyboard repeat begins. If set to 0, keyboard repeat is disabled.

#### <font color='white' size='1'><b>keyRepeatInterval</b></font> ####

> <sub>integer</sub> <b>keyRepeatInterval</b>
> > Sets/Gets keyboard repeat interval. This is the time in ms between keyboard repeat events.

#### <font color='white' size='1'><b>appStateActive</b></font> ####

> <sub>integer</sub> <b>appStateActive</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the current state of the application. If true, the user is able to see your application, otherwise it has been iconified or disabled.

#### <font color='white' size='1'><b>hasRDTSC</b></font> ####

> <sub>boolean</sub> <b>hasRDTSC</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)

#### <font color='white' size='1'><b>hasMMX</b></font> ####
> <sub>boolean</sub> <b>hasMMX</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)

#### <font color='white' size='1'><b>hasMMXExt</b></font> ####
> <sub>boolean</sub> <b>hasMMXExt</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)

#### <font color='white' size='1'><b>has3DNow</b></font> ####
> <sub>boolean</sub> <b>has3DNow</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)

#### <font color='white' size='1'><b>has3DNowExt</b></font> ####
> <sub>boolean</sub> <b>has3DNowExt</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)

#### <font color='white' size='1'><b>hasSSE</b></font> ####
> <sub>boolean</sub> <b>hasSSE</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)

#### <font color='white' size='1'><b>hasSSE2</b></font> ####
> <sub>boolean</sub> <b>hasSSE2</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)

#### <font color='white' size='1'><b>hasAltiVec</b></font> ####
> <sub>boolean</sub> <b>hasAltiVec</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)

> ##### keysym enum: #####
> > `K_UNKNOWN, K_FIRST, K_BACKSPACE, K_TAB, K_CLEAR, K_RETURN, K_PAUSE, K_ESCAPE, K_SPACE, K_EXCLAIM, K_QUOTEDBL, K_HASH, K_DOLLAR, K_AMPERSAND, K_QUOTE, K_LEFTPAREN, K_RIGHTPAREN, K_ASTERISK, K_PLUS, K_COMMA, K_MINUS, K_PERIOD, K_SLASH, `
> > `K_COLON, K_SEMICOLON, K_LESS, K_EQUALS, K_GREATER, K_QUESTION, K_AT, K_LEFTBRACKET, K_BACKSLASH, K_RIGHTBRACKET, K_CARET, K_UNDERSCORE, K_BACKQUOTE, K_DELETE, `
> > `K_UP, K_DOWN, K_RIGHT, K_LEFT, K_INSERT, K_HOME, K_END, K_PAGEUP, K_PAGEDOWN, `
> > `K_NUMLOCK, K_CAPSLOCK, K_SCROLLOCK, K_RSHIFT, K_LSHIFT, K_RCTRL, K_LCTRL, K_RALT, K_LALT, K_RMETA, K_LMETA, K_LSUPER, K_RSUPER, K_MODE, K_COMPOSE, K_HELP, K_PRINT, K_SYSREQ, K_BREAK, K_MENU, K_POWER, K_EURO, K_UNDO, `
> > `K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10, K_F11, K_F12, K_F13, K_F14, K_F15, `
> > `K_0, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9, `
> > `K_a, K_b, K_c, K_d, K_e, K_f, K_g, K_h, K_i, K_j, K_k, K_l, K_m, K_n, K_o, K_p, K_q, K_r, K_s, K_t, K_u, K_v, K_w, K_x, K_y, K_z, `
> > `K_KP0, K_KP1, K_KP2, K_KP3, K_KP4, K_KP5, K_KP6, K_KP7, K_KP8, K_KP9, K_KP_PERIOD, K_KP_DIVIDE, K_KP_MULTIPLY, K_KP_MINUS, K_KP_PLUS, K_KP_ENTER, K_KP_EQUALS, `


---

## class jssdl::Cursor ##
- [top](#jssdl_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jssdl/cursor.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( image )
> > Constructs a new B/W Cursor object. using a RGB or RGBA image.
> > Only the red component is used for B/W (<128: black, >= 128: white)
> > The Alpha component is used to set transparents pixels (alpha == 0) an inverted pixels (alpha == 255).
> > ##### arguments: #####
      1. <sub>ImageObject</sub> _image_



---

## jssdl::SdlError class ##

> You cannot construct this class.<br />
> Its aim is to catch jssdl runtime error exception.

#### <font color='white' size='1'><b>text</b></font> ####
> <b>text</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the text of the error.


---

- [top](#jssdl_module.md) - [main](JSLibs.md) -
