LoadModule('jsstd');
LoadModule('jsnspr');
LoadModule('jsgraphics');
LoadModule('jsprotex');

var glc = Exec('OpenGL.js'); // OpenGL constants

var win = new Window();
var gl = new Gl(win);

win.onidle = function() {

	gl.Clear( glc.COLOR_BUFFER_BIT | glc.DEPTH_BUFFER_BIT );

	gl.Translate(0,0,-1);
	gl.Quad(-1,-1,1,1);
	
	Sleep(1);
	gl.SwapBuffers();
}

win.onkeydown = function( key, l ) {
	
	if ( ket == vk.ESC )
		win.Exit();
}

win.onsize = function( w, h ) {

	gl.Viewport([0,0,w,h]);
//	gl.Perspective( 60, 0.01, 10000 );	
	gl.Ortho(0,0,10,10);
	win.onidle();
}

gl.Init();
//gl.Perspective( 60, 0.01, 10000 );
gl.Ortho(-1, 1, -1, 1);


var texture = new Texture(256,256);

texture.Flat(0,0,0,1);
texture.Noise(false,1234);
//texture.Pixels(10000,1234);
//texture.Cells(16,0,1);


gl.Color(1,1,1);
gl.LoadTexture( texture );

win.rect = [1700,1000,1900,1200]
win.ProcessEvents();

