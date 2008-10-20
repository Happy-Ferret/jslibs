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

SetVideoMode( 800, 600, 32, HWSURFACE | OPENGL | FULLSCREEN ); // | ASYNCBLIT
//ToggleFullScreen();

var done = false;

var listeners = {
	onQuit: function() done = true,
	onKeyDown: function(key, mod) done = (key == K_ESCAPE),
}

while ( !done ) {

	WaitEvent();
	PollEvent(listeners);
	
//	Sleep(10);
	Print('.');
	
	with (Ogl) {
		
		Begin( TRIANGLES );
		Vertex(0,0);
		Vertex(0,1);
		Vertex(1,0);
		End();
	};
	
	GlSwapBuffers();
}

Print('Done.');



