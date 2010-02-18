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
maxFPS = 60;

Ogl.Enable(Ogl.DEPTH_TEST);
Ogl.DepthFunc(Ogl.LESS);

Ogl.Enable( Ogl.LIGHTING ) ;
Ogl.Color( 1 ) ;
Ogl.ColorMaterial ( Ogl.FRONT_AND_BACK, Ogl.AMBIENT_AND_DIFFUSE ) ;
Ogl.Enable(Ogl.LIGHT0);



//Ogl.Enable(Ogl.BLEND);
//Ogl.Enable(Ogl.CULL_FACE);
//Ogl.CullFace(Ogl.BACK /* or GL_BACK or even GL_FRONT_AND_BACK */);
//Ogl.FrontFace(Ogl.CCW);
//Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);

Ogl.Enable(Ogl.TEXTURE_2D);
var f = new Font('c:\\windows\\fonts\\arial.ttf');
var f3d = new Font3D(f, Font3D.SOLID, 48);

f3d.colorCallback = function( pos ) {

//	return [Math.random(),Math.random(),Math.random(),1];
	return [pos[0]/20, pos[1]/20, pos[2]/20, 1];
}

Ogl.ClearColor(0,0,0, 1);
f3d.SetBackgroundColor();

Ogl.PixelStore( Ogl.UNPACK_ALIGNMENT, 1 ); // For the raster styles, it is essential that the pixel store unpacking alignment be set to 1.

var red = Ogl.NewList();
	Ogl.Color(1,0,0);
//	Ogl.Scale(Math.random()+1);
Ogl.EndList();

var green = Ogl.NewList();
	Ogl.Color(0,1,0);
//	Ogl.Scale(Math.random()+1);
Ogl.EndList();

var textStyle = [red, green, red, green, red, green, red, green, red, green, red, green, red, green];

var frame = 0;

function SurfaceReady() {

	frame++;

	var t = TimeCounter();

//	Ogl.ClearColor(0,0,0, 1);

	Ogl.Clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
	Ogl.LoadIdentity();
	Ogl.Rotate(frame/1, 1,1,1);

	Ogl.Color(0,0,0.75);

	Ogl.Translate(-1,0,0);
	Ogl.Scale(0.005);
	
//	Ogl.CallList(textList);

//	f3d.SetColor([Math.sin(frame/5)/2+.5]);
	f3d.SetColor();

	Ogl.RasterPos(0,0);

	Ogl.Scale(1,1,10);	
	Print( f3d.Measure('test', true)[3], '         \r' );
	f3d.Draw('Bob Marley', textStyle);

	GlSwapBuffers(true);
//	Print((1000/(TimeCounter() - t)).toFixed(2), 'fps               \r');
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
