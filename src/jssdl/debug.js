try { 

LoadModule('jsdebug');
LoadModule('jsstd');
LoadModule('jssdl');
LoadModule('jsgraphics');

GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DEPTH_SIZE, 16 );
GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );

unicodeKeyboardTranslation = true;
keyRepeatDelay = 300;
keyRepeatInterval = 50;
//SetVideoMode(1280, 1024, 32, HWACCEL | OPENGL | RESIZABLE | FULLSCREEN, false);
SetVideoMode(800, 600, 32, HWACCEL | OPENGL | RESIZABLE, false);
//maxFPS = 100;

Ogl.MatrixMode(Ogl.PROJECTION);
//Ogl.Ortho(-1, 1, -1, 1, 0.01, 100);
Ogl.Perspective(60, 0.01, 1000);
Ogl.MatrixMode(Ogl.MODELVIEW);

var frame = 0;

function SurfaceReady() {

	frame++;

	Ogl.Clear(Ogl.COLOR_BUFFER_BIT);
	Ogl.LoadIdentity();
	
	Ogl.LookAt(Math.cos(frame/100)*10, Math.sin(frame/100)*10, frame/100 + 5, 0,0,5, 0,0,1);
	
	Ogl.Begin(Ogl.LINE_LOOP);
	for ( var i = 0; i < 100; i++ ) {
		Ogl.Vertex(0,0, i/10);
		Ogl.Vertex(0,1, i/10);
		Ogl.Vertex(1,0, i/10);
	}
	Ogl.End();
	
	
	Sleep(100);
	Ogl.Flush();
	Ogl.Finish();

	GlSwapBuffers();
}


var listeners = {
	onKeyDown:function(sym, mod, char) {
		
		for each ( listener in listenerList )
			listener(sym, mod, char);

		if ( sym == K_ESCAPE )
			endSignal = true;
	},
	onQuit: function() {
	 
		endSignal = true;
	},
	onVideoResize: function(w, h) {

		ProcessEvents( SurfaceReadyEvents() );
		Ogl.Viewport(0, 0, w, h);
	},
};

while ( !endSignal )
	var e = ProcessEvents( SDLEvents(listeners), EndSignalEvents(), SurfaceReadyEvents(SurfaceReady) );

} catch(ex) {

	throw( ex );
}
