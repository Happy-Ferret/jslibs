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

// video thread
static SDL_Thread *videoThreadHandler;

// internal events management
jl::Queue internalEventQueue;
JLMutexHandler internalEventQueueMutex;
static JLSemaphoreHandler internalEventSem;

#define INTERNALEVENT_END 0
#define INTERNALEVENT_SET_VIDEO_MODE 1
#define INTERNALEVENT_SWAP_BUFFERS 2

struct InternalEvent {
	
	int type;
	union {
		struct {
			int width; int height; int bpp; Uint32 flags;
		} vm;
	};
};

// SDL events avability
JLSemaphoreHandler sdlEventsSem;

// surface ready
volatile bool surfaceReady;
JLEventHandler surfaceReadyEvent;

// OpenGL context owner management
static JLSemaphoreHandler contextAvailable;
static JLMutexHandler contextMutex;
HGLRC _openglContext = NULL;
HDC _deviceContext = NULL;

bool OwnGlContext() {

	JLMutexAcquire(contextMutex);
	bool isOwner = (wglGetCurrentContext() != NULL && wglGetCurrentDC() != NULL);
	JLMutexRelease(contextMutex);
	return isOwner;
}

void AcquireGlContext() {

	JLSemaphoreAcquire(contextAvailable, JLINFINITE);
	JLMutexAcquire(contextMutex);
	JL_ASSERT( _deviceContext != NULL && _openglContext != NULL );
	JL_ASSERT( wglGetCurrentDC() == NULL && wglGetCurrentContext() == NULL );
	BOOL st = wglMakeCurrent(_deviceContext, _openglContext); // doc. The OpenGL context is thread-specific. You have to make it current in the thread using glXMakeCurrent, wglMakeCurrent or aglSetCurrentContext, depending on your OS.
	JL_ASSERT( st );
	_deviceContext = NULL;
	_openglContext = NULL;
	JLMutexRelease(contextMutex);
}

void ReleaseGlContext() {
	
	JLMutexAcquire(contextMutex);
	HDC hdc = wglGetCurrentDC();
	HGLRC hglrc = wglGetCurrentContext();
	JL_ASSERT( hdc != NULL && hglrc != NULL );
	JL_ASSERT( _deviceContext == NULL && _openglContext == NULL );
	_deviceContext = hdc;
	_openglContext = hglrc;
	BOOL st = wglMakeCurrent(NULL, NULL); // doc. makes the calling thread's current rendering context no longer current, and releases the device context that is used by the rendering context. In this case, hdc  is ignored.
	JL_ASSERT( st );
	JLMutexRelease(contextMutex);
	JLSemaphoreRelease(contextAvailable);
}


// asynchronous API called from static.cpp
void JLSetVideoMode(int width, int height, int bpp, Uint32 flags, bool async) {

	bool hasSurface = (SDL_GetVideoSurface() != NULL);
	if ( hasSurface )
		while ( !surfaceReady )
			JLEventWait(surfaceReadyEvent, JLINFINITE);

	surfaceReady = false;

	if ( OwnGlContext() )
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
	while ( !surfaceReady )
		JLEventWait(surfaceReadyEvent, JLINFINITE);
	AcquireGlContext();
}


void JLAsyncSwapBuffers(bool async) {

	while ( !surfaceReady )
		JLEventWait(surfaceReadyEvent, JLINFINITE);

	surfaceReady = false;

	if ( OwnGlContext() )
		ReleaseGlContext();

	InternalEvent *iev = (InternalEvent*)jl_malloc(sizeof(InternalEvent));
	iev->type = INTERNALEVENT_SWAP_BUFFERS;
	JLMutexAcquire(internalEventQueueMutex);
	jl::QueuePush(&internalEventQueue, iev);
	JLMutexRelease(internalEventQueueMutex);
	JLSemaphoreRelease(internalEventSem);

	if ( async )
		return;
	while ( !surfaceReady )
		JLEventWait(surfaceReadyEvent, JLINFINITE);
	AcquireGlContext();
}


int EventFilter( const SDL_Event *e ) {

	if ( e->type == SDL_VIDEORESIZE ) {
/*
		AcquireGlContext();
		const SDL_Surface* currentSurface = SDL_GetVideoSurface();
		SDL_Surface *surface = SDL_SetVideoMode(e->resize.w, e->resize.h, currentSurface->format->BitsPerPixel, currentSurface->flags);
		// const char *errorMessage = SDL_GetError();
		JL_ASSERT( surface != NULL );
		ReleaseGlContext();
		surfaceReady = true;
		int status = JLEventTrigger(surfaceReadyEvent);
		JL_ASSERT( status == JLOK );
*/
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

				bool hasSurface = (SDL_GetVideoSurface() != NULL);
				if ( hasSurface )
					AcquireGlContext();
				SDL_Surface *surface = SDL_SetVideoMode(iev->vm.width, iev->vm.height, iev->vm.bpp, iev->vm.flags); // char *sdlError = SDL_GetError();
				JL_ASSERT( surface != NULL );
				ReleaseGlContext();

				surfaceReady = true;
				status = JLEventTrigger(surfaceReadyEvent);
				JL_ASSERT( status == JLOK );
				break;
			}
			case INTERNALEVENT_SWAP_BUFFERS: {

				JL_ASSERT( SDL_GetVideoSurface() != NULL ); // has a surface

				AcquireGlContext();
				SDL_GL_SwapBuffers();
				ReleaseGlContext();

				surfaceReady = true;
				status = JLEventTrigger(surfaceReadyEvent);
				JL_ASSERT( status == JLOK );
				break;
			}
			case INTERNALEVENT_END:
				end = true;
				break;
		}

		jl_free(iev);
	}

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
	surfaceReadyEvent = JLEventCreate(true);

	sdlEventsSem = JLSemaphoreCreate(0);

	videoThreadHandler = SDL_CreateThread(VideoThread, NULL); // http://www.libsdl.org/intro.en/usingthreads.html
	JL_ASSERT( videoThreadHandler != NULL ); // return ThrowSdlError(cx);

	JLSemaphoreAcquire(sdlEventsSem, JLINFINITE); // first event = thread ready
}


void EndVideo() {

	InternalEvent *iev = (InternalEvent*)jl_malloc(sizeof(InternalEvent));
	iev->type = INTERNALEVENT_END;

	JLMutexAcquire(internalEventQueueMutex);
	jl::QueuePush(&internalEventQueue, iev);
	JLMutexRelease(internalEventQueueMutex);
	JLSemaphoreRelease(internalEventSem);

	SDL_WaitThread(videoThreadHandler, NULL);

	JLEventFree(&surfaceReadyEvent);
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
