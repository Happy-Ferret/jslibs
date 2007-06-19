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
	
	if ( key == vk.ESC )
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
//texture.Rect(0,0,100,100,1,0,0,1);
//texture.Shift(-50,-10);

//texture.SetValue(0,0,0,1);
//texture.Noise(false,1234);
//texture.Pixels(10000,1234);
texture.Cells(10,0,1);

texture.Contrast(2.5);

//texture.Resize(256,256,true);

//texture.Convolution([0,-1,0, -1,4,-1 ,0,-1,0]); // laplacian 4

//texture.Convolution([0,0,0, 0,0,0 ,0,0,1]); // shift

//texture.Convolution([-1,0,0, 0,0,0 ,0,0,1]); // emboss
//texture.Convolution([0,-1,0, -1,5,-1, 0,-1,0]); // crystals
//texture.Convolution([1,1,1, -2,-2,-2, 1,1,1]);
//texture.Convolution([1,1,1, 1,1,1, 1,1,1]);
//texture.Normalize();
//texture.MultValue(20,20,20,20);

//texture.Displace(new Texture(512,512).SetNoise(true, 1), 32);


//var texture2 = new Texture(256,256);
//texture2.SetValue(1,0,0,1);
//texture.PasteAt( texture2, 300, 300, true );

//texture.MultValue(3,2,2,1);
//texture.Clamp(0.5, 1, false);
//texture.Normalize();
//texture.Aliasing(10);
//texture.Invert();

gl.Color(1,1,1);
gl.LoadTexture( texture );

win.rect = [1700,1000,1900,1200]
//win.rect = [500,500,700,700];
win.ProcessEvents();

