try { 

LoadModule('jsdebug');
LoadModule('jsstd');
LoadModule('jssdl');
LoadModule('jsgraphics');
LoadModule('jsfont');
LoadModule('jsoglft');

GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DEPTH_SIZE, 16 );
GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );

SetVideoMode(100, 100, 32, HWACCEL | OPENGL | RESIZABLE, false);

Ogl.Enable(Ogl.TEXTURE_2D);
var f = new Font('c:\\windows\\fonts\\arial.ttf');
var f3d = new Oglft(f, Oglft.GRAYSCALE, 48);

Ogl.ClearColor(0,1,0, 1);
f3d.SetBackgroundColor();

Ogl.PixelStore( Ogl.UNPACK_ALIGNMENT, 1 ); // For the raster styles, it is essential that the pixel store unpacking alignment be set to 1.


var frame = 0;

function SurfaceReady() {

	frame++;

	var t = TimeCounter();

//	Ogl.ClearColor(0,0,0, 1);
	Ogl.Clear(Ogl.COLOR_BUFFER_BIT);
	Ogl.LoadIdentity();
//	Ogl.Rotate(frame, 0,0,1);

	Ogl.Color(1,1,0);

	Ogl.Translate(-1,0,0);
	Ogl.Scale(0.1);
	
//	Ogl.CallList(textList);

	f3d.SetColor([Math.sin(frame/5)/2+.5]);

	Ogl.RasterPos(10,-10);

	f3d.Draw('test');

	GlSwapBuffers(true);
	Print((1000/(TimeCounter() - t)).toFixed(2), 'fps               \r');
}


var listeners = {
	onKeyDown:function(sym) {
		
		if ( sym == K_ESCAPE )
			endSignal = true;
	},
	onQuit: function() {
	 
		endSignal = true;
	},
	onVideoResize: function(w, h) {

		ProcessEvents( SurfaceReadyEvents() );
		Ogl.Viewport(0, 0, w, h);
//		Print(w,'x',h, '\n');
	}
};

while ( !endSignal )
	var e = ProcessEvents( SDLEvents(listeners), EndSignalEvents(), SurfaceReadyEvents(SurfaceReady) );

} catch(ex) {

	Print( ex );
}
