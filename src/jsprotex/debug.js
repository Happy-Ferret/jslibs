var loadModule = host.loadModule;
 //loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();
//loadModule('jsstd'); exec('../common/tools.js'); runQATests('jsprotex'); throw 0; // -inlineOnly

loadModule('jsstd');
loadModule('jsio');
loadModule('jssdl');
loadModule('jsgraphics');
loadModule('jsprotex');
loadModule('jsimage');

//halt(); //////////////////////////////////////////////////////////////////////


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


function addPixels(t, count) {
	
	while ( count-- > 0 )
		t.setPixel(Texture.randInt(), Texture.randInt(), 1);
}

function cloud( size, amp ) {

	var octaves = Math.log(size) / Math.LN2;
	var a = 1, s = 1;
	var cloud = new Texture(s, s, 1);
	cloud.clearChannel();
	while ( octaves-- > 0 ) {
		
		cloud.addNoise(a);
		a *= amp;
		s *= 2;
		cloud.resize(s, s, false);
		cloud.boxBlur(3,3);
	}
	cloud.normalizeLevels();
	return cloud;
}

function desaturateLuminosity( tex ) { // see http://svn.gnome.org/viewcvs/gimp/trunk/libgimpcolor/gimprgb.h?revision=19720&view=markup
	
	tex.mult([0.2126, 0.7152, 0.0722]);
	var tmp = new Texture(tex.width, tex.height, 1).desaturate(tex, Texture.desaturateSum);
	tex.swap(tmp);
	tmp.free();
}

function grayToRGB( tex ) {

	if ( tex.channels == 1 )
		new Texture(tex.width, tex.height, 3).setChannel(0, tex, 0).setChannel(1, tex, 0).setChannel(2, tex, 0).swap(tex);
	else if ( tex.channels == 2 )
		new Texture(tex.width, tex.height, 4).setChannel(0, tex, 0).setChannel(1, tex, 0).setChannel(2, tex, 0).swap(tex);
}

function addAlpha( tex ) {

	if ( tex.channels == 1 )
		new Texture(tex.width, tex.height, 2).setChannel(0, tex, 0).swap(tex);
	else if ( tex.channels == 3 )
		new Texture(tex.width, tex.height, 4).setChannel(0, tex, 0).setChannel(1, tex, 1).setChannel(2, tex, 2).swap(tex);
}

function colorToAlpha( tex, color ) {

	ASSERT( tex.channels == 2 || tex.channels == 4 );
	var alpha = new Texture(tex.width, tex.height, 1);
	alpha.extractColor(tex, color);
	tex.setChannel(tex.channels, alpha, 0);
}


function noiseChannel( tex, channel ) {
	
	var tmp = new Texture(tex.width, tex.height, 1);
	tmp.addNoise();
	tex.setChannel(channel, tmp, 0);
	tmp.free();
}


////


var size = 256;
var texture = new Texture(size, size, 3);

//	var myImage = DecodeJpegImage( new File('301_0185.jpg').Open('r') );
//	var file = new File('button2.png').Open('r');
//	var myImage = DecodePngImage( file );
//	file.Close();
//	texture.Set(0);
//	texture.Import( myImage, 0, 0 );

var t0 = intervalNow();
function time() intervalNow() - t0;
var z = 0;
randSeed(1);
perlinNoiseReinit();

//exec('liveconsole.js');

function updateTexture(imageIndex) { // <<<<<<<<<<<<<<<<<-----------------------------------




	// disco effect 2
	var t = cloud(size, 0.2);
	grayToRGB( t );
	t.addGradiantQuad(BLUE, GREEN, RED, YELLOW);
	t.mirrorLevels( 0.5 );
	t.mult(2);
	t.cutLevels(0.6, 0.8);
	t.colorize( BLACK, RED, 0 );
	t.colorize( WHITE, BLUE, 0 );
	texture.set(t);
return;

	// dilate test
	var mx = texture.width/2, my = texture.height/2;
	texture.set(0).setPixel(mx-mx/2, my, [1,0,0]).setPixel(mx, my, [0,0.7,0]).setPixel(mx+mx/2, my, [0,0,1]).boxBlur(mx/2,mx/2, 5);
	
	texture.dilate(5);
	texture.normalizeLevels();
return;


	// gozilla's skin
	randSeed(1);
	var bump = cloud( size, 0.6 );
	bump.normals();
	texture.set(1);
	texture.light( bump, [-1, -1, 0.5], 0, [0.4, 0.6, 0.0], 0.2, 0.7, 10 );

return;

	// perlin noise
	texture.set(0.5);
	texture.addPerlin2(z,0,0, 32, 1);
	texture.addPerlin2(z,0,0, 16, 0.5);
	texture.addPerlin2(z,0,0, 8, 0.25);
	texture.addPerlin2(z,0,0, 4, 0.125);
	texture.setChannel(1, texture, 0);
	texture.setChannel(2, texture, 0);
	z += 1;
return;


	// perlin noise
	texture.set(0);
	texture.addGradiantQuad(RED, GREEN, BLUE, WHITE);
	texture.addPerlin2([z/20,0,0], [10,0,0], [0,10,0], 2);
return;

	// ???
	randSeed(1);
	texture.set(0); // clears the texture
	texture.addCracks( 1000, 10, 0.1, 1, function(v) { return randReal() } );
return;


	// disco effect
	tmp = new Texture(size, size, 4);
	tmp.set(0);
	tmp.addGradiantQuad('#ff0000ff', GREEN, BLUE, BLACK);
	var tr = new Transformation(null);
	tr.scale(5);
	tr.rotate(intervalNow()/5, 1,1,1);
	tmp.applyColorMatrix(tr);
	texture.setChannel(0, tmp, 0).setChannel(1, tmp, 1).setChannel(2, tmp, 2);
return;


	// live coding test
	live.poll();
	live.Function(texture);
return;




function fractalCubeFace(px, py, pz,  x1, y1, z1,  x2, y2, z2) {

	var face = new Texture(128,128,1).set(0);
	for ( var scale = 2; scale <= 32; scale *= 2 )
		face.addPerlin2([px*scale, py*scale, pz*scale], [x1*scale, y1*scale, z1*scale], [x2*scale, y2*scale, z2*scale], 1/scale);
	return face;
}

	var t1 = fractalCubeFace(z/10,0,0, -1,0,0, 0,-1,0);

//	t1.Dilate(1,1);
	t1.add(0.25);
	t1.mult(1.5);
	
	texture = t1;

	z += 1;
return;



	// ???
	var bump = new Texture(texture.width, texture.height, 3).cells(8, 0).add( new Texture(size, size, 3).cells(8, 1).oppositeLevels() ); // broken floor
	bump.normals();

	var t = new Texture(size, size, 3);
	t.set(1);
	t.Light( bump, [-1, -1, 0.5], [1,1,1], 0.5, 1 );

	
/*	
	t.normals();
	var t1 = new Texture(size, size, 3);
	t1.setLevels(0.5);
	t1.Light( t, 0, 0.5, '#ff0000', [1,1,3], 1, 1 );
	t.swap(t1);
*/
return;







	// the sun
	texture.set(0);
	texture.forEachPixel(function(pixel, x, y) {
		
		var val = perlinNoise(1.5, 0.75, 5, x/4, y/4, z/16);
		pixel[0] = val;
		pixel[1] = val;
	});
	texture.addGradiantRadial( curveGaussian( 0.5 ), true ).add(-1);
	texture.addGradiantRadial( function(v) v > 0.4 ? 1 : 0, true ).add(-1);
	z += 1;
return;
	

	// gaussian like fast blur
	var mx = texture.width/2, my = texture.height/2;
	texture.set(0).setPixel(mx-mx/2, my, [1,0,0]).setPixel(mx, my, [0,1,0]).setPixel(mx+mx/2, my, [0,0,1]).boxBlur(mx/2,mx/2, 5).normalizeLevels();
return;


	// movable perlin texture
	texture.set(1);
	var mx = texture.width, my = texture.height;
	for ( var y = 0; y < my; y++ )
		for ( var x = 0; x < mx; x++ ) {
		
			var val = perlinNoise2((x/24 + -offsetx/30) * -offsetz/2, (y/24 + -offsety/30) * -offsetz/2, z/100);
			texture.setPixel(x,y, val);
		}

	z += 1;
return;


	// smooth colored noise
	randSeed(1);
	texture.set(0).addNoise(1).boxBlur(7,7, 3).normalizeLevels();
return;


	// smoke effect
	texture.set(1);
	texture.forEachPixel(function(pixel, x, y) {
		
		var ry = (y/texture.height);
//		val = PerlinNoise(2, 0.5, 1, x,y,z);
		var val = improvedPerlinNoise(x/24, (y/24 + z/24), z/246) * ry;
		pixel[0] = val;
		pixel[1] = val;
		pixel[2] = val;
	});
	z += 1;
return;


	// nice clouds effect
	texture.set(1);
	texture.forEachPixel(function(pixel, x, y) {

		var dis = perlinNoise(2.1, 0.5, 6, x-z/2, y);
		var val = perlinNoise(2, 0.5, 5, (x-z+dis*24)/2, y/2);
		pixel[0] = 1-val;
		pixel[1] = 1-val;
	});
	z += 1;
return;

	// binary stream
	texture.set(0);
	texture.forEachPixel(function(pixel, x, y) {

		var val = perlinNoise2(x+z, y/3, 0);
		pixel[1] = val;
	});
	z += 1;
return;

	// strange effect
	texture.set(0);
	texture.forEachPixel(function(pixel, x, y) {

		var dx= perlinNoise2(y*f, (x+z)*f, 0);
		val = perlinNoise2((x*dx+z)*f, (y*dx)*f, 0);

		pixel[0] = val;
		pixel[1] = val;
		pixel[2] = val;

	});
	z += 1;

return;



	
	texture = cloud( size, 0.5 * Math.sin(time()/1000) );
	texture.cutLevels(0.49,0.51);
	texture.boxBlur(4,4);
	texture.mirrorLevels(0.5);
	texture.powLevels(0.5)
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
//	print( tex.width );
//	var tmp = new Texture(tex.width, tex.height, 1);
//	tmp.AddNoise();
//	tex.SetChannel(channel, tmp, 0);
//	tex.Free();


//	NoiseChannel( displace, 0 );
//	NoiseChannel( displace, 1 );
//	displace.ClearChannel(2);
return;





//	t.AddGradiantRadial(curveHalfSine);

//	t.AddGradiantRadial(curveSine);
//	t.OppositeLevels();
//	t.Add(1);
	
	var c = cloud(size, 0.5)
	t.mult( c );
	t.add(-0.05);
	t.mult(3)
	

	var tmp = new Texture(size, size, 1);
	tmp.clearChannel();
	tmp.setRectangle(10,10,size-10,size-10,1);
	t.blend(tmp,0.7);

	
return;
	
	
//	t.set(GRAY);
	t.addGradiantLinear(curveHalfSine, curveOne);
	
	var bump = new Texture(size, size, 1);
	bump.clearChannel();
	bump.addGradiantRadial( curveHalfSine );
	bump.normals();
//	t.Light( bump, [1, 1, 0.01], 0, [1,0,0], 1, 1, 1 );
//t.Displace( bump, 1 );
	
//	t.Resize(256,256);
//	t.AddNoise(0.1);

//	t.set(BLUE);
	
//	t.Blend(t,RED);
	
return;

	var bump = new Texture(size, size, 3);
	bump.clearChannel();
	for ( var i = 0; i < 20; i++ ) {
		var x = Texture.randReal() * bump.width;
		var y = Texture.randReal() * bump.height;
		var s = Texture.randReal() * 60 + 10;
		bump.setRectangle(x, y, x+s, y+s, Texture.randReal() );
	}
	bump.normals();
	
//	t.AddGradiantQuad(BLUE, GREEN, RED, YELLOW);
//	t.Colorize(BLUE, WHITE, 1);

//	t.Light( bump, [1, 1, 0.01], RED, BLUE, 1, 0.25, 0 );


return;
	
//	t.AddGradiantRadial( [0, 1] );
//	t.Resize(256,256);
//	t.AddNoise(0.5);
	
	t.addGradiantQuad(BLUE, GREEN, RED, YELLOW);
//	t.set( YELLOW )

	t.colorize(BLUE, WHITE);
	
//	t.NormalizeVectors();
//	t.NormalizeLevels();

return;
	
	var t1 = new Texture(size, size, 1);
	
	t1.extractColor(t,BLACK, 1);
	t1.cutLevels(0.5);
	
	t = t1;
	
	
return;

	t.colorize(RED, BLACK);
	t.colorize(GREEN, BLACK);
	t.colorize(BLUE, BLACK);
	t.normalizeLevels();
	
	
//	DesaturateLuminosity(t);

return;

	var t = cloud(size, 0.5);
	t.aliasing(8,curveLinear);
	t.boxBlur(3,3)

return;


return;

	t.setLevels('#FFDDEE');
	
	var t1 = new Texture(t);
	t1.addLevels(-0.5);
	t1.multLevels(0.5);
	
	t.paste( t1, size/2, 0, -1 );
	
return;
	var t = new Texture(size, size, 1);
	t.clearChannel();
	t.addNoise(1);
	grayToRGB(t);

	t.cutLevels( 0.5, 0.5 );
	t.multLevels([0.5,1,1]);
	
	t.boxBlur(2,2);
	

return;

//	t.Aliasing(5)
//	t.AddGradiantRadial(100, 100, 50, curveDot );
//	t.AddCracks( 1, 10, 0.5, WHITE, curveSine );
	
	var th = new Texture(size, size, 1);
	th.clearChannel();
	th.addGradiantLinear( 1, curveDot );

	var tv = new Texture(size, size, 1);
	tv.clearChannel();
	tv.addGradiantLinear( curveDot, 1 );
	
	t.mix(th, tv);
	t.aliasing(2);






	//t.Normals();
	t.clearChannel();
	t.addGradiantQuad( [0], [0], [1], [1] );

	var t = new Texture(128, 128, 3);
	t.clearChannel();
	t.addNoise();
	t.RGBToHLS();
	//t.MultLevels([1,1,0.1]);
	//t.BoxBlur(2,2);
	t.HLSToRGB();

	t.clearChannel();

	for ( var i = 0; i< 10; i++ ) {
		addPixels(t, 64);
		t.boxBlur(2,2);
	}
	t.boxBlur(2,2);
	t.normalizeLevels();

	var red = new Texture(128,128,3);
	red.setLevels([1,0,0]);
	var blue = new Texture(128,128,3);
	blue.setLevels([0,0,1]);

	t.mix(red,blue);

	t.clearChannel(2);

	t.clearChannel();
	t.addCracks( 100, 100, 0, WHITE, [.1,.2,.3,.5,.6,.7,.8,.9,1] );

	//t.Convolution( [ 0,0.5,0, 0.5,1,0.5, 0,0.5,0 ] );


	/*
	var sx = 16; 
	var sy = 16; 

	var noise = new Texture(sx,sy,1).setNoise();
	noise.resize( 64, 64, true );

	var texture = new Texture(noise.width,noise.height,4);
	texture.setChannel(0, noise, 0).setChannel(1, noise, 0).setChannel(2, noise, 0);
	*/

	/*
	var texture = new Texture(2,2,4);
	texture.setValue([0,0,0,1]);
	texture.setPixel(0,0,[1,0,0,1]);
	texture.setPixel(1,0,[1,0,0,1]);
	texture.setPixel(0,1,[0,0,1,1]);
	texture.setPixel(1,1,[1,1,1,1]);
	texture.resize(64,64,true);
	texture.shift(10,0);
	*/


	/* test normals
	var texture = new Texture(128,128,1);
	texture.setNoise();
	texture.setRectangle( 0, 0, 10, 10, [1]);
	texture.normals(1);
	*/

	/* spaceship hud
	var texture = new Texture(256,256,1);
	texture.setLevels([0]);
	texture.setPixels(100);
	texture.boxBlur(32,64);
	texture.convolution([0,0,-1.5, 0,1,0 ,0,0,1]);
	//texture.Convolution([0,0,0, 0,1,0 ,0,0,0]);
	texture.multLevels([200]);
	//texture.NormalizeLevels();
	*/



	//texture.Convolution([-1,0,1, 0,1,0 ,-1,0,1]);
	//texture.NormalizeLevels();
	//texture.NormalizeVectors();


	//texture.Rect(0,0,100,100,1,0,0,1);
	//texture.Shift(-50,-10);

	//texture.setValue(0,0,0,1);
	//texture.setNoise(1234);
	//texture.pixels(10000,1234);
	//texture.cells(10,0,1);

	//texture.contrast(2.5);

	//texture.resize(256,256,true);

	//texture.convolution([0,-1,0, -1,4,-1 ,0,-1,0]); // laplacian 4

	//texture.convolution([0,0,0, 0,0,0 ,0,0,1]); // shift

	//texture.convolution([-1,0,0, 0,0,0 ,0,0,1]); // emboss
	//texture.convolution([0,-1,0, -1,5,-1, 0,-1,0]); // crystals
	//texture.convolution([1,1,1, -2,-2,-2, 1,1,1]);
	//texture.convolution([1,1,1, 1,1,1, 1,1,1]);
	//texture.normalize();
	//texture.multValue(20,20,20,20);

	//texture.displace(new Texture(512,512).SetNoise(true, 1), 32);


	//var texture2 = new Texture(256,256);
	//texture2.setValue(1,0,0,1);
	//texture.pasteAt( texture2, 300, 300, true );

	//texture.multValue(3,2,2,1);
	//texture.clamp(0.5, 1, false);
	//texture.normalize();
	//texture.aliasing(10);
	//texture.invert();
}


var offsetx = 0, offsety = 0, offsetz = 0;

var end=false, pause = false;

var listeners = {
	onVideoResize: function(w,h) { Ogl.viewport(0, 0, w, h) },
	onQuit: function() { end = true },
	onKeyDown: function(key, mod) { end = key == K_ESCAPE },
	onMouseButtonDown: function(button, x, y) {
		if ( button == BUTTON_LEFT ) {
			showCursor = false;
			grabInput = true;
		}
	},
	onMouseButtonUp: function(button, x, y) {
		if ( button == BUTTON_LEFT ) {
			showCursor = true;
			grabInput = false;
		}
		if ( button == BUTTON_WHEELUP ) {
		
			offsetz++;
		}
		if ( button == BUTTON_WHEELDOWN ) {
		
			offsetz--;
		}
		
	},
	onMouseMotion: function(px,py,dx,dy,button) {
		if ( grabInput ) {
			offsetx += dx;
			offsety += dy;
		}
	}
}



glSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
glSetAttribute( GL_DOUBLEBUFFER, 1 );
glSetAttribute( GL_DEPTH_SIZE, 16 );
setVideoMode( 200, 200, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN

with (Ogl) {

	matrixMode(PROJECTION);
	shadeModel(FLAT);
	enable(TEXTURE_2D);
	texParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST); // GL_LINEAR
	texParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
  scale(1, -1, 1);
}

while (!end) {

	while (pollEvent(listeners));
	if ( pause ) {
		
		sleep(100);
		continue;
	}

	var t = timeCounter();
	updateTexture();
	t = timeCounter() - t;
	print( t.toFixed(), 'ms     \r' );

	with (Ogl) {
		
		try {
			defineTextureImage(TEXTURE_2D, undefined, texture);
		} catch (ex) {}
		
		color(1,1,1,1);
		begin(QUADS);
		texCoord( 0, 0 );	vertex( -1, -1 );
		texCoord( 1, 0 ); vertex( 1, -1 );
		texCoord( 1, 1 ); vertex( 1, 1 );
		texCoord( 0, 1 ); vertex( -1, 1 );
		end();
	}

	maybeCollectGarbage();
	glSwapBuffers();
}

