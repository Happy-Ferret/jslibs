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

int maxFPS = JLINFINITE;

// video thread
static SDL_Thread *videoThreadHandler;

// swap buffer management
static SDL_Thread *swapBuffersThreadHandler;
JLSemaphoreHandler swapBuffersSem;
volatile bool swapBufferEndThread = false;

// SDL events avability
JLSemaphoreHandler sdlEventsSem;

// surface ready
volatile bool surfaceReady;
JLCondHandler surfaceReadyCond;
JLMutexHandler surfaceReadyLock;

ALWAYS_INLINE void WaitSurfaceReady() {
	
	JLMutexAcquire(surfaceReadyLock);
	while ( !surfaceReady )
		JLCondWait(surfaceReadyCond, surfaceReadyLock);
	JLMutexRelease(surfaceReadyLock);
}

ALWAYS_INLINE void SetSurfaceReady( bool readyState ) {
	
	JLMutexAcquire(surfaceReadyLock);
	surfaceReady = readyState;
	JLCondBroadcast(surfaceReadyCond);
	JLMutexRelease(surfaceReadyLock);
}

ALWAYS_INLINE bool HasSurface() {

	return SDL_GetVideoSurface() != NULL;
}


// OpenGL context owner management
static JLSemaphoreHandler contextAvailable;
static JLMutexHandler contextMutex;
HGLRC _openglContext = NULL;
HDC _deviceContext = NULL;
bool _mustReleaseGlContext = false;


bool MustReleaseGlContext() {

	JLMutexAcquire(contextMutex);
	bool polarity = _mustReleaseGlContext;
	JLMutexRelease(contextMutex);
	return polarity;
}

ALWAYS_INLINE bool HasGlContext() {

	JLMutexAcquire(contextMutex);
	bool isOwner = ( _openglContext == NULL && _deviceContext == NULL && wglGetCurrentContext() != NULL && wglGetCurrentDC() != NULL );
	JLMutexRelease(contextMutex);
	return isOwner;
}

ALWAYS_INLINE void AcquireGlContext() {
	
	JLMutexAcquire(contextMutex);
	_mustReleaseGlContext = true;
	JLMutexRelease(contextMutex);

	JLSemaphoreAcquire(contextAvailable, JLINFINITE);

	JLMutexAcquire(contextMutex);
	JL_ASSERT( _deviceContext != NULL && _openglContext != NULL );
	JL_ASSERT( wglGetCurrentDC() == NULL && wglGetCurrentContext() == NULL );
	BOOL st = wglMakeCurrent(_deviceContext, _openglContext); // doc. The OpenGL context is thread-specific. You have to make it current in the thread using glXMakeCurrent, wglMakeCurrent or aglSetCurrentContext, depending on your OS.

	while ( st != TRUE ) { // error 2004 - ERROR_TRANSFORM_NOT_SUPPORTED - "The requested transformation operation is not supported" // see http://www.gamedev.net/community/forums/topic.asp?topic_id=555299

		// (TBD) find a better way to manage this error
		//SleepMilliseconds(1);
		st = wglMakeCurrent(_deviceContext, _openglContext);
#ifdef DEBUG
		if ( st != TRUE ) {

			char err[1024];
			JLLastSysetmErrorMessage(err, 1024);
			printf("wglMakeCurrent error: %s\n", err);
		}
#endif // DEBUG
	}

	JL_ASSERT( st );
	_deviceContext = NULL;
	_openglContext = NULL;
	JLMutexRelease(contextMutex);
}

ALWAYS_INLINE void ReleaseGlContext() {
	
	JLMutexAcquire(contextMutex);
	HGLRC hglrc = wglGetCurrentContext();
	JL_ASSERT( _deviceContext == NULL && _openglContext == NULL );
	HDC hdc = wglGetCurrentDC();
	JL_ASSERT( hdc != NULL && hglrc != NULL );
	_deviceContext = hdc;
	_openglContext = hglrc;
	BOOL st = wglMakeCurrent(NULL, NULL); // doc. makes the calling thread's current rendering context no longer current, and releases the device context that is used by the rendering context. In this case, hdc  is ignored.
	JL_ASSERT( st );
	_mustReleaseGlContext = false;
	JLMutexRelease(contextMutex);
	JLSemaphoreRelease(contextAvailable);
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


// asynchronous API called from static.cpp
void JLSetVideoMode(int width, int height, int bpp, Uint32 flags, bool async) {

	if ( HasSurface() )
		WaitSurfaceReady();
	SetSurfaceReady(false);

	if ( HasGlContext() )
		ReleaseGlContext();

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

	if ( async )
		return;
	WaitSurfaceReady();
	AcquireGlContext();
}


void JLAsyncSwapBuffers(bool async) {

//	SDL_GL_SwapBuffers(); // IT WORKS !
//	return;


	WaitSurfaceReady();
	SetSurfaceReady(false);

	if ( HasGlContext() )
		ReleaseGlContext();
	JLSemaphoreRelease(swapBuffersSem);

	if ( async )
		return;
	WaitSurfaceReady();
	AcquireGlContext();
}


int SwapBuffersThread( void *unused ) {

	for (;;) {

		JLSemaphoreAcquire(swapBuffersSem, JLINFINITE);
		if ( swapBufferEndThread )
			break;
		if ( !HasSurface() )
			continue;

		if ( maxFPS == JLINFINITE ) {
		
			AcquireGlContext();
			SDL_GL_SwapBuffers();
			ReleaseGlContext();
		} else {

			int t0 = AccurateTimeCounter();
			AcquireGlContext();
			SDL_GL_SwapBuffers();
			ReleaseGlContext();
			int wait = 1000/maxFPS - (AccurateTimeCounter() - t0);
			if ( wait > 0 )
				SleepMilliseconds(wait); // (TBD) to avoid blocking on exit, use a timed semaphore
		}
		SetSurfaceReady(true);
	}
	return 0;
}



int EventFilter( const SDL_Event *e ) {

	if ( e->type == SDL_VIDEORESIZE ) {

		AcquireGlContext();
		const SDL_Surface* currentSurface = SDL_GetVideoSurface();
		SDL_Surface *surface = SDL_SetVideoMode(e->resize.w, e->resize.h, currentSurface->format->BitsPerPixel, currentSurface->flags);
		JL_ASSERT( surface != NULL );
		ReleaseGlContext();
		SetSurfaceReady(true);
	}
	return 1; // 1, then the event will be added to the internal queue.
}


int VideoThread( void *unused ) {

	int status;
	status = SDL_InitSubSystem(SDL_INIT_VIDEO); // (TBD) SDL_INIT_EVENTTHREAD on Linux ?
	JL_ASSERT( status != -1 );

	SDL_SetEventFilter(EventFilter);
	JLSemaphoreRelease(sdlEventsSem); // first event = thread ready

	bool end = false;
	while ( !end ) {
	
		// (TBD) hdc must be owned by this thread while SDL_PumpEvents and SDL_PeepEvents are running else wglMakeCurrent will rise an error when BeginPaint(SDL_Window, &ps); is running (on WM_PAINT)
		SDL_PumpEvents();
		status = SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_ALLEVENTS);

		JL_ASSERT( status != -1 );
		if ( status == 1 )
			JLSemaphoreRelease(sdlEventsSem);

		int st = JLSemaphoreAcquire(internalEventSem, 5);
		JL_ASSERT( st != JLERROR );
		if ( st == JLTIMEOUT )
			continue;

		JLMutexAcquire(internalEventQueueMutex);
		JL_ASSERT( !jl::QueueIsEmpty(&internalEventQueue) );
		InternalEvent *iev = (InternalEvent*)jl::QueueShift(&internalEventQueue);
		JLMutexRelease(internalEventQueueMutex);

		switch ( iev->type ) {
			case INTERNALEVENT_SET_VIDEO_MODE: {

				if ( HasSurface() )
					AcquireGlContext();
				SDL_Surface *surface = SDL_SetVideoMode(iev->vm.width, iev->vm.height, iev->vm.bpp, iev->vm.flags); // char *sdlError = SDL_GetError();
				JL_ASSERT( surface != NULL );
				ReleaseGlContext();
				SetSurfaceReady(true);
				break;
			}
			case INTERNALEVENT_END:
				end = true;
				break;
			default:
				JL_ASSERT( false );
		}
		jl_free(iev);
	}

	if ( HasSurface() )
		AcquireGlContext();
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	return 0;
}


// video initializaion
void StartVideo() {

	jl::QueueInitialize(&internalEventQueue);
	internalEventQueueMutex = JLMutexCreate();
	internalEventSem = JLSemaphoreCreate(0);

	contextAvailable = JLSemaphoreCreate(0);
	contextMutex = JLMutexCreate();

	surfaceReady = false;
	surfaceReadyCond = JLCondCreate();
	surfaceReadyLock = JLMutexCreate();

	sdlEventsSem = JLSemaphoreCreate(0);

	swapBuffersSem = JLSemaphoreCreate(0);
	swapBuffersThreadHandler = SDL_CreateThread(SwapBuffersThread, NULL); // http://www.libsdl.org/intro.en/usingthreads.html

	videoThreadHandler = SDL_CreateThread(VideoThread, NULL); // http://www.libsdl.org/intro.en/usingthreads.html
	JL_ASSERT( videoThreadHandler != NULL ); // return ThrowSdlError(cx);

	JLSemaphoreAcquire(sdlEventsSem, JLINFINITE); // first event = thread ready
}


void EndVideo() {

	swapBufferEndThread = true;
	JLSemaphoreRelease(swapBuffersSem);
	SDL_WaitThread(swapBuffersThreadHandler, NULL);
	JLSemaphoreFree(&swapBuffersSem);

	InternalEvent *iev = (InternalEvent*)jl_malloc(sizeof(InternalEvent));
	iev->type = INTERNALEVENT_END;

	if ( SDL_GetVideoSurface() != NULL && HasGlContext() )
		ReleaseGlContext();

	JLMutexAcquire(internalEventQueueMutex);
	jl::QueuePush(&internalEventQueue, iev);
	JLMutexRelease(internalEventQueueMutex);
	JLSemaphoreRelease(internalEventSem);

	SDL_WaitThread(videoThreadHandler, NULL);

	JLMutexFree(&surfaceReadyLock);
	JLCondFree(&surfaceReadyCond);

	JLSemaphoreFree(&sdlEventsSem);
	JLMutexFree(&contextMutex);
	JLSemaphoreFree(&contextAvailable);

	while ( !jl::QueueIsEmpty(&internalEventQueue) )
		jl_free(jl::QueuePop(&internalEventQueue));
	JLMutexFree(&internalEventQueueMutex);
	JLSemaphoreFree(&internalEventSem);
}



EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	if ( SDL_WasInit(0) != 0 )
		JL_REPORT_ERROR("SDL module already in use.");

	JL_CHK( InitJslibsModule(cx, id)  );

	int status = SDL_Init(SDL_INIT_NOPARACHUTE);

	INIT_CLASS( SdlError );

	if ( status != 0 )
		return ThrowSdlError(cx);
	StartVideo();

//	SDL_EnableUNICODE(1); // see unicodeKeyboardTranslation property

	INIT_STATIC();
	INIT_CLASS( Cursor );

	typedef void* (__cdecl *glGetProcAddress_t)(const char*);
	JL_CHK( SetNativePrivatePointer(cx, JS_GetGlobalObject(cx), "_glGetProcAddress", (glGetProcAddress_t)SDL_GL_GetProcAddress) );

	return JS_TRUE;
	JL_BAD;
}

EXTERN_C DLLEXPORT void ModuleFree() {

	EndVideo();

	SDL_Quit();
}
