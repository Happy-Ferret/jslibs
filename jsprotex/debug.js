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
	
//	if ( key == vk.ESC )
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



const RED = [1,0,0,1];
const GREEN = [0,1,0,1];
const BLUE = [0,0,1,1];
const GRAY = [.5,.5,.5,1];
const BLACK = [0,0,0,1];
const WHITE = [1,1,1,1];

const curveLinear = function(v) { return 1-v }
const curveHalfSine = function(v) { return Math.cos(v*Math.PI/2) }
const curveSine = function(v) { return Math.sin(v*Math.PI) }
const curveDot = function(v,i) { return i%2 }
const curveZero = function(v,i) { return 0 }
const curveOne = function(v,i) { return 1 }



function AddChannel(t) {
	
	var tmp = new Texture(t.width, t.height, t.channels+1);
	for ( var i = 0; i < t.channels; i++ )
		tmp.SetChannel(i, t, i);
	t.Free();
	return tmp;
}

function AddPixels(t, count) {
	
	var color = [1,1,1];	
	while ( count-- > 0 )
		t.SetPixel(Texture.RandInt(), Texture.RandInt(), color);
}

function Cloud( size, amp ) {

	var octaves = Math.log(size) / Math.log(2);
	var a = 1, s = 1;
	var cloud = new Texture(s, s, 1);
	cloud.ClearChannel();
	while ( octaves-- > 0 ) {
		
		cloud.AddNoise(a);
		a *= amp;
		s *= 2;
		cloud.Resize(s, s, false);
		cloud.BoxBlur(3, 3);
	}
	cloud.NormalizeLevels();
	return cloud;
}

var t0 = IntervalNow();

draw:{

	
	var size = 128;
	
	var t = new Texture(size, size, 3);
	t.ClearChannel();

	t.AddNoise();
	t.CutLevels( 0.45, 0.55 );
	
	var t = Cloud(size, 1);
	t.CutLevels( 0.8, 1 );
	t.BoxBlur(2,2);
	

break draw; // -----------------------------------------


//	t.Aliasing(5)
//	t.AddGradiantRadial(100, 100, 50, curveDot );
//	t.AddCracks( 1, 10, 0.5, WHITE, curveSine );
	
	var th = new Texture(size, size, 1);
	th.ClearChannel();
	th.AddGradiantLinear( 1, curveDot );

	var tv = new Texture(size, size, 1);
	tv.ClearChannel();
	tv.AddGradiantLinear( curveDot, 1 );
	
	t.Mix(th, tv);
	t.Aliasing(2);






	//t.Normals();
	t.ClearChannel();
	t.AddGradiantQuad( [0], [0], [1], [1] );

	var t = new Texture(128, 128, 3);
	t.ClearChannel();
	t.AddNoise();
	t.RGBToHLS();
	//t.MultLevels([1,1,0.1]);
	//t.BoxBlur(2,2);
	t.HLSToRGB();

	t.ClearChannel();

	for ( var i = 0; i< 10; i++ ) {
		AddPixels(t, 64);
		t.BoxBlur(2,2);
	}
	t.BoxBlur(2,2);
	t.NormalizeLevels();

	var red = new Texture(128,128,3);
	red.SetLevels([1,0,0]);
	var blue = new Texture(128,128,3);
	blue.SetLevels([0,0,1]);

	t.Mix(red,blue);

	t.ClearChannel(2);

	t.ClearChannel();
	t.AddCracks( 100, 100, 0, WHITE, [.1,.2,.3,.5,.6,.7,.8,.9,1] );

	//t.Convolution( [ 0,0.5,0, 0.5,1,0.5, 0,0.5,0 ] );


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
	var texture = new Texture(256,256,1);
	texture.SetLevels([0]);
	texture.SetPixels(100);
	texture.BoxBlur(32,64);
	texture.Convolution([0,0,-1.5, 0,1,0 ,0,0,1]);
	//texture.Convolution([0,0,0, 0,1,0 ,0,0,0]);
	texture.MultLevels([200]);
	//texture.NormalizeLevels();
	*/



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

}

Print( 'time: '+ (IntervalNow() - t0) + ' ms\n' );

gl.Color(1,1,1);
gl.LoadTexture( t );

win.rect = [1700,1000,1900,1200]
//win.rect = [500,500,700,700];
win.ProcessEvents();

