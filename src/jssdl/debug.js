// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

try { 

LoadModule('jsstd');
LoadModule('jssdl');
LoadModule('jsgraphics');


SetVideoMode( 320, 200, 32, HWACCEL | OPENGL | RESIZABLE ); // | ASYNCBLIT

GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DEPTH_SIZE, 16 );
GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );


var listeners = {
	onQuit: function() { 
		endSignal = true;
	},
	onVideoResize: function(w, h) {

//		SetVideoMode(w, h);
		Ogl.Viewport(0, 0, w, h);
		Print( w, ' ', h, '\n' );
	},	
};

Ogl.Viewport(0, 0, 32, 20);

Ogl.ClearColor(0,0,0, 1);


var angle = 0.;
while ( !endSignal ) {

	var e = ProcessEvents( SDLEvents(listeners), EndSignalEvents() );
	//Print(e.toString(2), '  ');

	Ogl.Clear(Ogl.COLOR_BUFFER_BIT);
	Ogl.Rotate(1,1,1, angle++);
	Ogl.Begin(Ogl.TRIANGLES);
	Ogl.Vertex(0,0);
	Ogl.Vertex(0,1);
	Ogl.Vertex(1,0);
	Ogl.End();
	
	var t0 = TimeCounter();
	GlSwapBuffers();
	ProcessEvents(BuffersSwappedEvents());
	Print( (TimeCounter() - t0).toFixed(2), '\n'  )
}

} catch(ex) {

	Print( ex );
}

Halt();


//var count = MetaPoll( CoPool([s1, s2]), CoPollEvent(), CoWinCOMPoll(xhr), CoEndSignal() );



LoadModule('jsstd');
LoadModule('jssdl');
LoadModule('jsgraphics');
LoadModule('jssvg');
LoadModule('jsio');
LoadModule('jsimage');


SetVideoMode( 320, 200, 32, HWACCEL | OPENGL | RESIZABLE ); // | ASYNCBLIT


//count = MetaPoll( ConfIOPool([s1, s2]), ConfSDLPoll(), ConfWinCOMPoll(xhr), ConfTimeout(100) );


//SDLDebugTest();


//Halt();


var svgIcon = <svg width="100%" height="100%" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
	<rect x="0" y="0" width="100" height="100" fill="#FF4422" />
	<circle id="a" cx="50" cy="50" r="25" stroke="black" stroke-width="16" fill="none"/>
	<use xlink:href="#a" stroke="#FF4422" stroke-width="10" x="8" />
	<use xlink:href="#a" stroke="#black" stroke-width="6" x="8" />
</svg>


var svg = new SVG();
svg.Write('<?xml version="1.0" encoding="utf-8"?>'+svgIcon);
var image = svg.RenderImage(32,32);
//new File('test.png').content = EncodePngImage( image );

Print( GetVideoModeList(8, FULLSCREEN).join('\n') );
//Halt();

icon = image;

//GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync


GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_DEPTH_SIZE, 16 );

SetVideoMode( 320, 200, 32, HWACCEL | OPENGL | RESIZABLE ); // | ASYNCBLIT
//ToggleFullScreen();

var cursor1 = new Cursor(image, 16,16);
SetCursor( cursor1 );

try {

//	SetGamma(1,1,1);
	
} catch(ex) {

	Print('error:'+ex.text);
}

showCursor = true;

var done = false;
var pos = 0;

var listeners = {
	onQuit: function() done = true,
	onKeyDown: function(key, mod) {
		
		switch (key) {
			case K_ESCAPE: 
				done = true;
				break;
			case K_g:
				grabInput = !grabInput;
				showCursor = !showCursor;
				break;
		}
	},
	onVideoResize: function(w,h) {

		SetVideoMode(w, h);
		Ogl.Viewport(0, 0, w, h);
	},
	onMouseMotion: function(x,y,dx,dy,button) {
		
		pos += dx;
	}
};

Print( videoWidth+'x'+videoHeight, '\n' );

while ( !done ) {

	PollEvent(listeners);
	
	with (Ogl) {
		
		LoadIdentity();
		Rotate( pos / Math.PI*2, 0,0,1 );
		Clear(COLOR_BUFFER_BIT);

		Begin( TRIANGLES );
		Vertex(0,0);
		Vertex(0,1);
		Vertex(1,0);
		End();

	};
	
	GlSwapBuffers();
}

