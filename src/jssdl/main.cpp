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

#include "queue.h"

#include "static.h"
#include "error.h"
#include "sdl.h"

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

int volatile _maxFPS;

SDL_Surface volatile *_surface;
int volatile surfaceWidth;
int volatile surfaceHeight;

int desktopWidth;
int desktopHeight;
Uint8 desktopBitsPerPixel;


#ifndef JL_NOTHREAD

const char volatile *_error; // SDL error is managed per thread

JLSemaphoreHandler threadReadySem;

// video thread
static SDL_Thread *videoThreadHandler;

// SDL events avability

JLCondHandler sdlEventsCond;
JLMutexHandler sdlEventsLock;

// swap buffer management
static SDL_Thread *swapBuffersThreadHandler;
JLSemaphoreHandler swapBuffersSem;
bool volatile swapBufferEndThread;

// surface management
JLMutexHandler surfaceLock; // avoid SDL_SetVideoMode while SDL_GL_SwapBuffers

HDC volatile _hdc;

bool surfaceReady;
JLCondHandler surfaceReadyCond;
JLMutexHandler surfaceReadyLock;

ALWAYS_INLINE void WaitSurfaceReady() {
	
	JLMutexAcquire(surfaceReadyLock);
	while ( !surfaceReady )
		JLCondWait(surfaceReadyCond, surfaceReadyLock);
	JLMutexRelease(surfaceReadyLock);
}

ALWAYS_INLINE void SurfaceReady() {

	JLMutexAcquire(surfaceReadyLock);
	surfaceReady = true;
	JLCondBroadcast(surfaceReadyCond);
	JLMutexRelease(surfaceReadyLock);
}

ALWAYS_INLINE void InvalidateSurface() {
	
	JLMutexAcquire(surfaceReadyLock);
	surfaceReady = false;
	JLCondBroadcast(surfaceReadyCond);
	JLMutexRelease(surfaceReadyLock);
}


// internal events management
jl::Queue internalEventQueue;
JLMutexHandler internalEventQueueMutex;
static JLSemaphoreHandler internalEventSem;

#define INTERNALEVENT_END 0
#define INTERNALEVENT_SET_VIDEO_MODE 1

struct InternalEvent {
	
	int type;
	union {
		struct {
			int width; int height; int bpp; Uint32 flags;
		} vm;
	};
};


// API called from static.cpp
bool JLSetVideoMode(int width, int height, int bpp, Uint32 flags) {

	if ( _surface != NULL ) {

		WaitSurfaceReady();
		InvalidateSurface();

		JL_ASSERT( wglGetCurrentContext() );
		JL_ASSERT( wglGetCurrentDC() );
	}

	InternalEvent *iev = (InternalEvent*)jl_malloc(sizeof(InternalEvent));
	iev->type = INTERNALEVENT_SET_VIDEO_MODE;
	iev->vm.width = width;
	iev->vm.height = height;
	iev->vm.bpp = bpp;
	iev->vm.flags = flags;
	JLMutexAcquire(internalEventQueueMutex);
	jl::QueuePush(&internalEventQueue, iev);
	JLMutexRelease(internalEventQueueMutex);
	JLSemaphoreRelease(internalEventSem);

	WaitSurfaceReady();

	if ( !_surface && _error ) {

		SDL_SetError((const char*)_error); // transmit the error to the current thread
		_error = NULL;
		return false;
	}

	// create the OpenGL context for this thread if needed.
	HGLRC hglrc = wglGetCurrentContext();
	if ( hglrc == NULL ) {
		
		// Creating an OpenGL Context (http://www.opengl.org/wiki/Creating_an_OpenGL_Context)
		JL_ASSERT( _hdc != NULL );
		hglrc = wglCreateContext(_hdc);
		JL_ASSERT( hglrc != NULL );
		BOOL st = wglMakeCurrent(_hdc, hglrc);
		JL_ASSERT( st == TRUE );
	}

	return true;
}


void JLSwapBuffers(bool async) {

	if ( _surface == NULL )
		return;

	WaitSurfaceReady();
	InvalidateSurface();

	JL_ASSERT( wglGetCurrentContext() );
	JL_ASSERT( wglGetCurrentDC() );

	if ( !async ) {

		if ( _maxFPS == JLINFINITE ) {
					
			JLMutexAcquire(surfaceLock);
			SDL_GL_SwapBuffers();
			JLMutexRelease(surfaceLock);
		} else {

			double t0 = AccurateTimeCounter();
			
			JLMutexAcquire(surfaceLock);
			SDL_GL_SwapBuffers();
			JLMutexRelease(surfaceLock);

			double t1 = AccurateTimeCounter();
			double wait = 1000/_maxFPS - (t1 - t0);
			if ( wait > 0 )
				SleepMilliseconds(uint32_t(wait));
		}
		SurfaceReady();
	} else {

		JLSemaphoreRelease(swapBuffersSem);
	}
}


int SwapBuffersThread( void *unused ) {

	HGLRC hglrc = NULL; // SwapBuffersThread must have its own opengl context else SwapBuffers(GL_hdc); (in WIN_GL_SwapBuffers) will crash !

	double t0 = AccurateTimeCounter();

	JLSemaphoreRelease(threadReadySem);

	for (;;) {

		JLSemaphoreAcquire(swapBuffersSem, JLINFINITE);
		if ( swapBufferEndThread )
			break;

		if ( hglrc == NULL ) {

			JL_ASSERT( _hdc );
			hglrc = wglCreateContext(_hdc);
			JL_ASSERT( hglrc != NULL );
			BOOL st = wglMakeCurrent(_hdc, hglrc);
			JL_ASSERT( st != NULL );
		}

		if ( _maxFPS == JLINFINITE ) {
		
			JLMutexAcquire(surfaceLock);
			SDL_GL_SwapBuffers();
			JLMutexRelease(surfaceLock);
		} else {
			
			JLMutexAcquire(surfaceLock);
			SDL_GL_SwapBuffers();
			JLMutexRelease(surfaceLock);

			double t1 = AccurateTimeCounter();
			double wait = 1000/_maxFPS - (t1 - t0);
			if ( wait > 0 ) {

				int st = JLSemaphoreAcquire(swapBuffersSem, int(wait));
				if ( st == JLOK )
					JLSemaphoreRelease(swapBuffersSem);
			}
			t0 = t1;
		}
		SurfaceReady();
	}

	// delete the gl context of this thread.
	if ( hglrc != NULL ) {

		BOOL st = wglMakeCurrent(NULL, NULL);
		if ( st == TRUE ) // fail in this case: Ogl.Begin( Ogl.LINE_STRIP ); xxxx;
			wglDeleteContext(hglrc);
	}
	return 0;
}


int EventFilter( const SDL_Event *e ) {

	if ( e->type == SDL_VIDEORESIZE ) {
		
		surfaceWidth = e->resize.w;
		surfaceHeight = e->resize.h;
	}
	return 1; // the event will be added to the internal queue.
}


int VideoThread( void *unused ) {

	int status;
	status = SDL_InitSubSystem(SDL_INIT_VIDEO); // (TBD) SDL_INIT_EVENTTHREAD on Linux ?
	JL_ASSERT( status != -1 );
	SDL_SetEventFilter(EventFilter);

	bool end = false;
	
	JLSemaphoreRelease(threadReadySem);
	while ( !end ) {
	
		SDL_PumpEvents();

		JLMutexAcquire(sdlEventsLock);
		status = SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_ALLEVENTS);
		if ( status == 1 ) // we have at least one SDL event, see SDLEndWait()
			JLCondBroadcast(sdlEventsCond);
		JLMutexRelease(sdlEventsLock);
		JL_ASSERT( status != -1 );

/*
		if ( videoResize ) {

			videoResize = false;

			JL_ASSERT( _surface != NULL );
			JLMutexAcquire(surfaceLock);
			_surface = SDL_SetVideoMode(surfaceWidth, surfaceHeight, _surface->format->BitsPerPixel, _surface->flags);
			JLMutexRelease(surfaceLock);
			JL_ASSERT( _surface != NULL );
			surfaceWidth = _surface->w;
			surfaceHeight = _surface->h;
		}
*/

		int st = JLSemaphoreAcquire(internalEventSem, 5); // see SDL_WaitEvent()
		JL_ASSERT( st != JLERROR );
		if ( st == JLTIMEOUT ) // no internal event
			continue;

		JLMutexAcquire(internalEventQueueMutex);
		JL_ASSERT( !jl::QueueIsEmpty(&internalEventQueue) );
		InternalEvent *iev = (InternalEvent*)jl::QueueShift(&internalEventQueue);
		JLMutexRelease(internalEventQueueMutex);

		switch ( iev->type ) {
			case INTERNALEVENT_SET_VIDEO_MODE: {

				JLMutexAcquire(surfaceLock);
				_surface = SDL_SetVideoMode(iev->vm.width, iev->vm.height, iev->vm.bpp, iev->vm.flags); // char *sdlError = SDL_GetError();
				JLMutexRelease(surfaceLock);

				if ( _surface == NULL ) {

					JL_ASSERT( _error == NULL );
					_error = SDL_GetError(); // store the error
					SDL_ClearError();
					SurfaceReady();
					break;
				}
	
				surfaceWidth = _surface->w;
				surfaceHeight = _surface->h;

				JL_ASSERT( _hdc == NULL || _hdc == wglGetCurrentDC() ); // assert hdc has not changed
				if ( _hdc == NULL ) {

					_hdc = wglGetCurrentDC();
					JL_ASSERT( _hdc );

#ifdef XP_WIN
					RECT rect;
					GetWindowRect(WindowFromDC(_hdc), &rect);
					MoveWindow(WindowFromDC(_hdc), rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top-1, FALSE);
					MoveWindow(WindowFromDC(_hdc), rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, FALSE);
#endif // XP_WIN

				}
				JL_ASSERT( wglGetCurrentContext() );
				SurfaceReady();
				break;
			}
			case INTERNALEVENT_END:
				end = true;
				break;
			default:
				JL_ASSERT( false ); // invalid case
		}
		jl_free(iev);
	}

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	return 0;
}

// video initializaion
void StartVideo() {

	_error = NULL;

	_maxFPS = JLINFINITE;
	_hdc = NULL; // see INTERNALEVENT_SET_VIDEO_MODE case

	surfaceLock = JLMutexCreate();

	jl::QueueInitialize(&internalEventQueue);
	internalEventQueueMutex = JLMutexCreate();
	internalEventSem = JLSemaphoreCreate(0);

	sdlEventsCond = JLCondCreate();
	sdlEventsLock = JLMutexCreate();

	_surface = NULL;
	surfaceReady = false;
	surfaceReadyCond = JLCondCreate();
	surfaceReadyLock = JLMutexCreate();

	threadReadySem = JLSemaphoreCreate(0);

	swapBufferEndThread = false;
	swapBuffersSem = JLSemaphoreCreate(0);
	swapBuffersThreadHandler = SDL_CreateThread(SwapBuffersThread, NULL); // http://www.libsdl.org/intro.en/usingthreads.html
	JL_ASSERT( swapBuffersThreadHandler != NULL ); // return ThrowSdlError(cx);
	JLSemaphoreAcquire(threadReadySem, JLINFINITE);

	videoThreadHandler = SDL_CreateThread(VideoThread, NULL); // http://www.libsdl.org/intro.en/usingthreads.html
	JL_ASSERT( videoThreadHandler != NULL ); // return ThrowSdlError(cx);
	JLSemaphoreAcquire(threadReadySem, JLINFINITE);
	
	JLSemaphoreFree(&threadReadySem);
}


void EndVideo() {

	HGLRC hglrc = wglGetCurrentContext();
	if ( hglrc != NULL ) {

		BOOL st = wglMakeCurrent(NULL, NULL);
		if ( st == TRUE ) // fail in this case: Ogl.Begin( Ogl.LINE_STRIP ); xxxx;
			wglDeleteContext(hglrc);
	}

	swapBufferEndThread = true;
	JLSemaphoreRelease(swapBuffersSem);
	SDL_WaitThread(swapBuffersThreadHandler, NULL);
	JLSemaphoreFree(&swapBuffersSem);

	InternalEvent *iev = (InternalEvent*)jl_malloc(sizeof(InternalEvent));
	iev->type = INTERNALEVENT_END;
	JLMutexAcquire(internalEventQueueMutex);
	jl::QueuePush(&internalEventQueue, iev);
	JLMutexRelease(internalEventQueueMutex);
	JLSemaphoreRelease(internalEventSem);
	SDL_WaitThread(videoThreadHandler, NULL);

	while ( !jl::QueueIsEmpty(&internalEventQueue) )
		jl_free(jl::QueuePop(&internalEventQueue));
	JLMutexFree(&internalEventQueueMutex);
	JLSemaphoreFree(&internalEventSem);

	JLMutexFree(&surfaceReadyLock);
	JLCondFree(&surfaceReadyCond);

	JLMutexFree(&sdlEventsLock);
	JLCondFree(&sdlEventsCond);

	JLMutexFree(&surfaceLock);
}

#else // JL_NOTHREAD

bool JLSetVideoMode(int width, int height, int bpp, Uint32 flags) {

	_surface = SDL_SetVideoMode(width, height, bpp, flags);
	return _surface != NULL;
}

void JLSwapBuffers(bool async) {
	
	SDL_GL_SwapBuffers();
}

void StartVideo() {
	
	int status = SDL_Init(SDL_INIT_VIDEO);
	JL_ASSERT( status != -1 );
	_surface = NULL;
	_maxFPS = JLINFINITE;
}

void EndVideo() {

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

#endif // JL_NOTHREAD



EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	if ( SDL_WasInit(0) != 0 )
		JL_REPORT_ERROR("SDL module already in use.");

	JL_CHK( InitJslibsModule(cx, id) );

	int status = SDL_Init(SDL_INIT_NOPARACHUTE);

	INIT_CLASS( SdlError );

	if ( status != 0 )
		return ThrowSdlError(cx);

	StartVideo();

	const SDL_VideoInfo *vi = SDL_GetVideoInfo(); // Get the current information about the video hardware
	desktopWidth = vi->current_w;
	desktopHeight = vi->current_h;
	desktopBitsPerPixel = vi->vfmt->BitsPerPixel; // bad: If this is called before SDL_SetVideoMode(), the 'vfmt' member of the returned structure will contain the pixel format of the "best" video mode.

//	typedef void* (__cdecl *glGetProcAddress_t)(const char*);
//	JL_CHK( SetNativePrivatePointer(cx, JS_GetGlobalObject(cx), "_glGetProcAddress", (glGetProcAddress_t)SDL_GL_GetProcAddress) );

//	SDL_EnableUNICODE(1); // see unicodeKeyboardTranslation property

	INIT_STATIC();
	INIT_CLASS( Cursor );

	return JS_TRUE;
	JL_BAD;
}

EXTERN_C DLLEXPORT void ModuleFree() {

	EndVideo();

	SDL_Quit();
}
