LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsgraphics');
LoadModule('jsprotex');


const RED = [1,0,0,1];
const GREEN = [0,1,0,1];
const BLUE = [0,0,1,1];
const MAGENTA = [1,0,1,1];
const CYAN = [0,1,1,1];
const YELLOW = [1,1,0,1];
const GRAY = [.5,.5,.5,1];
const BLACK = [0,0,0,1];
const WHITE = [1,1,1,1];

const curveLinear = function(v) { return v }
const curveHalfSine = function(v) { return Math.cos(v*Math.PI/2) }
const curveSine = function(v) { return Math.sin(v*Math.PI) }
const curveGaussian = function(c) { return function(x) { return Math.exp( -(x*x)/(2*c*c) ) } }
const curveInverse = function(v) { return 1/v }
const curveSquare = function(v) { return v*v }
const curveDot = function(v,i) { return i%2 }
const curveZero = function(v,i) { return 0 }
const curveOne = function(v,i) { return 1 }

const kernelGaussian = [0,3,10,3,0, 3,16,26,16,3, 10,26,26,26,10, 3,16,26,16,3, 0,3,10,3,0 ];
const kernelGaussian2 = [2,4,5,4,2, 4,9,12,9,4, 5,12,15,12,5, 4,9,12,9,4, 2,4,5,4,2]; // G(r) = pow(E,-r*r/(2*o*o))/sqrt(2*PI*o);
const kernelEmboss = [-1,0,0, 0,0,0 ,0,0,1];
const kernelLaplacian = [-1,-1,-1, -1,8,-1, -1,-1,-1];

//texture.Convolution([0,-1,0, -1,4,-1 ,0,-1,0]); // laplacian 4
//texture.Convolution([0,-1,0, -1,5,-1, 0,-1,0]); // crystals


function AddPixels(t, count) {
	
	while ( count-- > 0 )
		t.SetPixel(Texture.RandInt(), Texture.RandInt(), 1);
}

function Cloud( size, amp ) {

	var octaves = Math.log(size) / Math.LN2;
	var a = 1, s = 1;
	var cloud = new Texture(s, s, 1);
	cloud.ClearChannel();
	while ( octaves-- > 0 ) {
		
		cloud.AddNoise(a);
		a *= amp;
		s *= 2;
		cloud.Resize(s, s, false);
		cloud.BoxBlur(5, 5);
	}
	cloud.NormalizeLevels();
	return cloud;
}

function DesaturateLuminosity( tex ) { // see http://svn.gnome.org/viewcvs/gimp/trunk/libgimpcolor/gimprgb.h?revision=19720&view=markup
	
	tex.Mult([0.2126, 0.7152, 0.0722]);
	var tmp = new Texture(tex.width, tex.height, 1).Desaturate(tex, Texture.desaturateSum);
	tex.Swap(tmp);
	tmp.Free();
}

function GrayToRGB( tex ) {

	if ( tex.channels == 1 )
		new Texture(tex.width, tex.height, 3).SetChannel(0, tex, 0).SetChannel(1, tex, 0).SetChannel(2, tex, 0).Swap(tex);
	else if ( tex.channels == 2 )
		new Texture(tex.width, tex.height, 4).SetChannel(0, tex, 0).SetChannel(1, tex, 0).SetChannel(2, tex, 0).Swap(tex);
}

function AddAlpha( tex ) {

	if ( tex.channels == 1 )
		new Texture(tex.width, tex.height, 2).SetChannel(0, tex, 0).Swap(tex);
	else if ( tex.channels == 3 )
		new Texture(tex.width, tex.height, 4).SetChannel(0, tex, 0).SetChannel(1, tex, 1).SetChannel(2, tex, 2).Swap(tex);
}

function ColorToAlpha( tex, color ) {

	ASSERT( tex.channels == 2 || tex.channels == 4 );
	var alpha = new Texture(tex.width, tex.height, 1);
	alpha.ExtractColor(tex, color);
	tex.SetChannel(tex.channels, alpha, 0);
}


function NoiseChannel( tex, channel ) {
	
	var tmp = new Texture(tex.width, tex.height, 1);
	tmp.AddNoise();
	tex.SetChannel(channel, tmp, 0);
	tmp.Free();
}



var t0 = IntervalNow();

function time() IntervalNow() - t0;

var size = 256;
var texture = new Texture(size, size, 3);

function UpdateTexture(imageIndex) {

//texture.SetRectangle(0,0,1,1, [1,0,0]);	

	Texture.RandSeed(0);
	texture.ClearChannel();
	
	texture.AddGradiantRadial( [1], 0 );
	
	return;
	
	texture = Cloud( size, 0.5 * Math.sin(time()/1000) );
	

	
	texture.CutLevels(0.49,0.51);
	texture.BoxBlur(4,4);
	
	
	texture.MirrorLevels(0.5);
	texture.PowLevels(0.5)
	
//texture.AddGradiantQuad(BLUE, GREEN, RED, YELLOW);


//	texture.SetRectangle( 1*size/4, 1*size/4, 3*size/4, 3*size/4, WHITE );
//	texture.RotoZoom( 0, 0, 1, 1, time()/10000 );
	//texture.RotoZoom( 0, 0, 1, 0.5, 0 );



//texture.Shift(time()/10,0);

//	var displace = new Texture(size, size, 2);
//	var d = Cloud( size, Math.random()/100 );
//	NoiseChannel( displace, 0 );
//	NoiseChannel( displace, 1 );
//	d.NormalizeVectors();
	//d.Aliasing(2);
//	d.Normals();
//	d.BoxBlur(3,3);
//	texture.Displace(d, 50);

	
//	t.AddGradiantRadial( curveGaussian( 0.5 ) );
//	t.AddGradiantRadial( [1,0], 1 );
//	t.RotoZoom( 0.5,0.5, 10,1, 0 );
//	t.OppositeLevels();
//	t.Add(1);
//	t.PowLevels( 0.1 );

	
//	var displace = new Texture(size, size, 2);
//	NoiseChannel( displace, 0 );
//	NoiseChannel( displace, 1 );
//	displace.NormalizeVectors();
	
//	t.Displace(displace, 20);
//	t.RotoZoom( 0.5,0.5, 5,5, 0 );
//	t.BoxBlur(3,3);


//	var tex = t;
//	var channel = 1;
//	Print( tex.width );
//	var tmp = new Texture(tex.width, tex.height, 1);
//	tmp.AddNoise();
//	tex.SetChannel(channel, tmp, 0);
//	tex.Free();


//	NoiseChannel( displace, 0 );
//	NoiseChannel( displace, 1 );
//	displace.ClearChannel(2);
	
	

return;
	var t = Cloud(size, 0.2);
	GrayToRGB( t );
	
//	t.AddGradiantQuad(BLUE, GREEN, RED, YELLOW);
	t.MirrorLevels( 0.5 );
	t.Mult(2);
	t.CutLevels(0.6, 0.8);
	t.Colorize( BLACK, RED, 0 );
	t.Colorize( WHITE, BLUE, 0 );

	
	
//	
		
return;
//	t.AddGradiantRadial(curveHalfSine);

//	t.AddGradiantRadial(curveSine);
//	t.OppositeLevels();
//	t.Add(1);
	
	var c = Cloud(size, 0.5)
	t.Mult( c );
	t.Add(-0.05);
	t.Mult(3)
	

	var tmp = new Texture(size, size, 1);
	tmp.ClearChannel();
	tmp.SetRectangle(10,10,size-10,size-10,1);
	t.Blend(tmp,0.7);

	
return;
	
	
//	t.Set(GRAY);
	t.AddGradiantLinear(curveHalfSine, curveOne);
	
	var bump = new Texture(size, size, 1);
	bump.ClearChannel();
	bump.AddGradiantRadial( curveHalfSine );
	bump.Normals();
//	t.Light( bump, [1, 1, 0.01], 0, [1,0,0], 1, 1, 1 );
//t.Displace( bump, 1 );
	
//	t.Resize(256,256);
//	t.AddNoise(0.1);

//	t.Set(BLUE);
	
//	t.Blend(t,RED);
	
return;

	var bump = new Texture(size, size, 3);
	bump.ClearChannel();
	for ( var i = 0; i < 20; i++ ) {
		var x = Texture.RandReal() * bump.width;
		var y = Texture.RandReal() * bump.height;
		var s = Texture.RandReal() * 60 + 10;
		bump.SetRectangle(x, y, x+s, y+s, Texture.RandReal() );
	}
	bump.Normals();
	
//	t.AddGradiantQuad(BLUE, GREEN, RED, YELLOW);
//	t.Colorize(BLUE, WHITE, 1);

//	t.Light( bump, [1, 1, 0.01], RED, BLUE, 1, 0.25, 0 );


return;
	
//	t.AddGradiantRadial( [0, 1] );
//	t.Resize(256,256);
//	t.AddNoise(0.5);
	
	t.AddGradiantQuad(BLUE, GREEN, RED, YELLOW);
//	t.Set( YELLOW )

	t.Colorize(BLUE, WHITE);
	
//	t.NormalizeVectors();
//	t.NormalizeLevels();

return;
	
	var t1 = new Texture(size, size, 1);
	
	t1.ExtractColor(t,BLACK, 1);
	t1.CutLevels(0.5);
	
	t = t1;
	
	
return;

	t.Colorize(RED, BLACK);
	t.Colorize(GREEN, BLACK);
	t.Colorize(BLUE, BLACK);
	t.NormalizeLevels();
	
	
//	DesaturateLuminosity(t);

return;

	var t = Cloud(size, 0.5);
	t.Aliasing(8,curveLinear);
	t.BoxBlur(3,3)

return;
	var bump = new Texture(size, size, 3).Cells(8, 0).Add( new Texture(size, size, 3).Cells(8, 1).OppositeLevels() ); // broken floor
	bump.Normals();

	var t = new Texture(size, size, 3);
	t.Set(1);
	t.Light( bump, 0, 1, 1, [1,1,1], 0.5, 1 );

	
/*	
	t.Normals();
	var t1 = new Texture(size, size, 3);
	t1.SetLevels(0.5);
	t1.Light( t, 0, 0.5, '#ff0000', [1,1,3], 1, 1 );
	
	t.Swap(t1);
*/

return;

	t.SetLevels('#FFDDEE');
	
	var t1 = new Texture(t);
	t1.AddLevels(-0.5);
	t1.MultLevels(0.5);
	
	t.Paste( t1, size/2, 0, -1 );
	
return;
	var t = new Texture(size, size, 1);
	t.ClearChannel();
	t.AddNoise(1);
	GrayToRGB(t);

	t.CutLevels( 0.5, 0.5 );
	t.MultLevels([0.5,1,1]);
	
	t.BoxBlur(2,2);
	

return;

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



var end = false;

var win = new Window();
win.Open();
win.CreateOpenGLContext();

function ResizeWindow(w, h) {

	with (Ogl) {
		Viewport(0,0,w,h);
		MatrixMode(PROJECTION);
		LoadIdentity();
		Ortho(0,0,10,10, -1, 1);
	}
	Render();
}

with (Ogl) {

	ShadeModel(FLAT);
	FrontFace(CCW);
	ClearColor(0, 0, 0, 0);
	Enable(TEXTURE_2D);
	var tid = GenTexture();
	BindTexture(TEXTURE_2D, tid);
	TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST); // GL_LINEAR
	TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
	Clear( COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT );
}

function Render(imageIndex) {

	with (Ogl) {
	
		MatrixMode(MODELVIEW);

		UpdateTexture();
		DefineTextureImage(TEXTURE_2D, texture);
		
		LoadIdentity();
		Translate(0,0,0);
		Color(1,1,1);
		Begin(QUADS);
		TexCoord( 0, 0 );
		Vertex( -1, -1, 0 );
		TexCoord( 1, 0, 0 );
		Vertex( 1, -1 );
		TexCoord( 1, 1 );
		Vertex( 1, 1 );
		TexCoord( 0, 1 );
		Vertex( -1, 1 );
		End();
	}
	win.SwapBuffers();
	
	MaybeCollectGarbage();

}

win.onkeydown = function( key, l ) {

		end = ( key == 0x1B );
}

win.onsize = ResizeWindow;


//win.rect = [1700,1000,1900,1200]
win.rect = [200,200,800,800];

while (!end) {

	win.ProcessEvents();
	Render();
}

win.Close();