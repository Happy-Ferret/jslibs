LoadModule('jsstd');
LoadModule('jssdl');
LoadModule('jsgraphics');
LoadModule('jssvg');
LoadModule('jsio');
LoadModule('jsimage');


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

SetIcon(image);

//GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync

GlSetAttribute( GL_DOUBLEBUFFER, 1 );

SetVideoMode( 320, 200, 32, HWSURFACE | OPENGL ); // | ASYNCBLIT
//ToggleFullScreen();

var cursor1 = new Cursor(image, 16,16);
SetCursor( cursor1 );


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
	onMouseMotion: function(x,y,dx,dy,button) pos += dx
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

