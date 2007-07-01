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

const RED = [1,0,0];
const GREEN = [0,1,0];
const BLUE = [0,0,1];
const GRAY = [.5,.5,.5];
const BLACK = [0,0,0];
const WHITE = [1,1,1];

/*
var sx = 16; 
var sy = 16; 


var noise = new Texture(sx,sy,1).SetNoise();
noise.Resize( 64, 64, true );

var texture = new Texture(noise.width,noise.height,4);
texture.SetChannel(0, noise, 0).SetChannel(1, noise, 0).SetChannel(2, noise, 0);
*/

/*
var texture = new Texture(2,2,4);
texture.SetValue([0,0,0,1]);
texture.SetPixel(0,0,[1,0,0,1]);
texture.SetPixel(1,0,[1,0,0,1]);
texture.SetPixel(0,1,[0,0,1,1]);
texture.SetPixel(1,1,[1,1,1,1]);
texture.Resize(64,64,true);
texture.Shift(10,0);
*/


/* test normals
var texture = new Texture(128,128,1);
texture.SetNoise();
texture.SetRectangle( 0, 0, 10, 10, [1]);
texture.Normals(1);
*/

/* spaceship hud
*/
var texture = new Texture(256,256,1);
texture.SetLevels([0]);
texture.SetPixels(100);
texture.BoxBlur(32,64);
texture.Convolution([0,0,-1.5, 0,1,0 ,0,0,1]);
//texture.Convolution([0,0,0, 0,1,0 ,0,0,0]);
texture.MultLevels([200]);
//texture.NormalizeLevels();


//texture.Convolution([-1,0,1, 0,1,0 ,-1,0,1]);
//texture.NormalizeLevels();
//texture.NormalizeVectors();


//texture.Rect(0,0,100,100,1,0,0,1);
//texture.Shift(-50,-10);

//texture.SetValue(0,0,0,1);
//texture.SetNoise(1234);
//texture.Pixels(10000,1234);
//texture.Cells(10,0,1);

//texture.Contrast(2.5);

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

//win.rect = [1700,1000,1900,1200]
win.rect = [500,500,700,700];
win.ProcessEvents();

