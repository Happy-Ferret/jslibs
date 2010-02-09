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
#include "sdl.h"

#include <windows.h>

#include "jslibsModule.cpp"

DECLARE_CLASS( Cursor )

/**doc t:header
$MODULE_HEADER
 jssdl is a wrapper to the Simple DirectMedia Layer (SDL) library.
 Simple DirectMedia Layer is a cross-platform multimedia library designed to provide low level access to
 audio, keyboard, mouse, joystick, 3D hardware via OpenGL, and 2D video framebuffer.
 It is used by MPEG playback software, emulators, and many popular games,
 including the award winning Linux port of "Civilization: Call To Power."
 $FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

static JLEventHandler videoThreadReady;
static SDL_Thread *videoThreadHandler;
static JLEventHandler commandDone;
JLEventHandler sdlEvent;

volatile bool surfaceReady;
JLEventHandler surfaceReadyEvent;

#define USEREVENT_END 0
#define USEREVENT_SET_VIDEO_MODE 1
#define USEREVENT_SET_SWAP_BUFFERS 2

HGLRC _openglContext = NULL;
HDC _deviceContext = NULL;

bool AcquireGlContext() {

	if ( _deviceContext && _openglContext ) {

		BOOL st = wglMakeCurrent(_deviceContext, _openglContext); // doc. The OpenGL context is thread-specific. You have to make it current in the thread using glXMakeCurrent, wglMakeCurrent or aglSetCurrentContext, depending on your OS.
		JL_ASSERT( st );
		_deviceContext = NULL;
		_openglContext = NULL;
		return true;
	} else {
		return false;
	}
}

bool ReleaseGlContext() {

	HDC hdc = wglGetCurrentDC();
	HGLRC hglrc = wglGetCurrentContext();
	if ( hdc && hglrc ) {

		JL_ASSERT( _deviceContext == NULL );
		JL_ASSERT( _openglContext == NULL );
		_deviceContext = hdc;
		_openglContext = hglrc;
		BOOL st = wglMakeCurrent(NULL, NULL); // doc. makes the calling thread's current rendering context no longer current, and releases the device context that is used by the rendering context. In this case, hdc  is ignored.
		JL_ASSERT( st );
		return true;
	} else {

		return false;
	}
}



struct VideoMode {

	int width; int height; int bpp; Uint32 flags;
	SDL_Surface *returnValue;
};

SDL_Surface* JLSetVideoMode(int width, int height, int bpp, Uint32 flags) {

	surfaceReady = false;

	VideoMode vm = { width, height, bpp, flags };

	SDL_Event ev;
	ev.type = SDL_USEREVENT;
	ev.user.code = USEREVENT_SET_VIDEO_MODE;
	ev.user.data1 = &vm;
	ReleaseGlContext();
	SDL_PushEvent(&ev);
	JLEventWait(commandDone, -1);
	AcquireGlContext();
	return vm.returnValue;
}


bool JLAsyncSwapBuffers() {

	surfaceReady = false;

	if ( !ReleaseGlContext() )
		return false;
	SDL_Event ev;
	ev.type = SDL_USEREVENT;
	ev.user.code = USEREVENT_SET_SWAP_BUFFERS;
	SDL_PushEvent(&ev);
	return true;
}


int EventFilter( const SDL_Event *e ) {

	if ( e->type == SDL_VIDEORESIZE ) {
/*
		AcquireGlContext();
		const SDL_Surface* currentSurface = SDL_GetVideoSurface();
		SDL_Surface *surface = SDL_SetVideoMode(e->resize.w, e->resize.h, currentSurface->format->BitsPerPixel, currentSurface->flags);
//		const char *errorMessage = SDL_GetError();
		JL_ASSERT( surface != NULL );
		ReleaseGlContext();
*/
/*
		SDL_Event ev = *e;
		SDL_PushEvent(&ev);
		JLEventTrigger(sdlEvent); // signal a non-user event
		return 0;
*/
	}
	return 1; // 1, then the event will be added to the internal queue.
}


int VideoThread( void *unused ) {

	int status;
	status = SDL_InitSubSystem(SDL_INIT_VIDEO); // (TBD) SDL_INIT_EVENTTHREAD on Linux ?
	JL_ASSERT( status != -1 );
	SDL_SetEventFilter(EventFilter);
	JLEventTrigger(videoThreadReady);
	SDL_PumpEvents();

	SDL_Event ev;
	for (;;) {
		
		status = SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_ALLEVENTS);
		JL_ASSERT( status != -1 );
		if ( status == 0 ) {

			SDL_Delay(5);
			SDL_PumpEvents();
			continue;
		}

		JL_ASSERT( status == 1 );

		status = SDL_PeepEvents(&ev, 1, SDL_GETEVENT, SDL_EVENTMASK(SDL_USEREVENT));
		JL_ASSERT( status != -1 );
		if ( status == 0 ) {
			
			JLEventTrigger(sdlEvent); // signal a non-user event
			continue;
		}

		JL_ASSERT( ev.type == SDL_USEREVENT );

		switch ( ev.user.code ) {
			case USEREVENT_END:
				goto end;
			case USEREVENT_SET_VIDEO_MODE: {

				VideoMode *vm = (VideoMode*)ev.user.data1;
				AcquireGlContext();
				vm->returnValue = SDL_SetVideoMode(vm->width, vm->height, vm->bpp, vm->flags);
				JL_ASSERT( vm->returnValue != NULL );
				ReleaseGlContext();
				surfaceReady = true;
				status = JLEventTrigger(surfaceReadyEvent);
				JL_ASSERT( status != JLERROR );
				status = JLEventTrigger(commandDone);
				JL_ASSERT( status != JLERROR );
				break;
			}
			case USEREVENT_SET_SWAP_BUFFERS: {

				AcquireGlContext();
				SDL_GL_SwapBuffers();
				ReleaseGlContext();
				surfaceReady = true;
				status = JLEventTrigger(surfaceReadyEvent);
				JL_ASSERT( status != JLERROR );
				break;
			}
		}
	}

end:
	AcquireGlContext();
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	return 0;
}


void StartVideo() {

	commandDone = JLEventCreate(true);
	sdlEvent = JLEventCreate(true);
	surfaceReadyEvent = JLEventCreate(true);
	surfaceReady = false;

	videoThreadReady = JLEventCreate(false);
	// http://www.libsdl.org/intro.en/usingthreads.html
	videoThreadHandler = SDL_CreateThread(VideoThread, NULL);
//	if ( videoThreadHandler == NULL )
//		return ThrowSdlError(cx);
	JLEventWait(videoThreadReady, -1);
	JLEventFree(&videoThreadReady);
}

void EndVideo() {

	SDL_Event ev;
	ev.type = SDL_USEREVENT;
	ev.user.code = USEREVENT_END;
	SDL_PushEvent(&ev);
	SDL_WaitThread(videoThreadHandler, NULL);
	JLEventFree(&commandDone);
	JLEventFree(&sdlEvent);
	JLEventFree(&surfaceReadyEvent);
}


EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	JL_CHK( InitJslibsModule(cx, id)  );

	int status = SDL_Init(SDL_INIT_NOPARACHUTE);

	INIT_CLASS( SdlError );

	if ( status != 0 )
		return ThrowSdlError(cx);

	SDL_EnableUNICODE(1);

	StartVideo();

	INIT_STATIC();
	INIT_CLASS( Cursor );

	typedef void* (__cdecl *glGetProcAddress_t)(const char*);
	JL_CHK( SetPrivateNativeFunction(cx, JS_GetGlobalObject(cx), "_glGetProcAddress", (glGetProcAddress_t)SDL_GL_GetProcAddress) );

	return JS_TRUE;
	JL_BAD;
}

EXTERN_C DLLEXPORT void ModuleFree() {

	EndVideo();

	SDL_Quit();
}
