// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

loadModule('jsstd');
loadModule('jsio');
loadModule('jsimage');

loadModule('jsvideoinput');

//print( VideoInput.list.join('\n') ); throw 0;

loadModule('jssdl');
loadModule('jsgraphics');
loadModule('jsprotex');

exec('..\\common\\tools.js');

	
	//	GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
//		GlSetAttribute( GL_DOUBLEBUFFER, 1 );
		glSetAttribute( GL_DEPTH_SIZE, 16 );
//		GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );
		setVideoMode( 640, 480, 32, OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN

		Ogl.hint(Ogl.PERSPECTIVE_CORRECTION_HINT, Ogl.NICEST);
		Ogl.hint(Ogl.POINT_SMOOTH_HINT, Ogl.NICEST);
		Ogl.viewport(0,0,videoWidth,videoHeight);
		Ogl.matrixMode(Ogl.PROJECTION);
		Ogl.perspective(60, 1, 0.1, 100000);
		Ogl.matrixMode(Ogl.MODELVIEW);
		Ogl.clearColor(0.2, 0.1, 0.4, 1);
		Ogl.enable(Ogl.DEPTH_TEST);
		Ogl.enable(Ogl.BLEND);
		Ogl.blendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);



var vi = new VideoInput('QuickCam', 1, 1, 60); // try to get the smallest size and the lowest fps
print('full name: '+vi.name, '\n');

var tmp1 = new Texture(256,256,3).Set(0);
var final = new Texture(256,256,3).Set(0);
var noise = new Texture(256,256,3).Set(0);

print( vi.width + 'x' + vi.height, '\n' );

vi.onImage = function() {
	print('-');
}

var frames = [];
var frame = 0;
var key;
while ( !getKeyState(K_ESCAPE) ) {

	processEvents(SDLEvents({}), vi.events());

//	while( PollEvent({}) );

	glSwapBuffers();

var t1 = timeCounter();
//collectGarbage();
//print( 'GC:',  (TimeCounter() - t1).toFixed(1), 'ms\n' );


	frame++;
	var image = vi.getImage();
	var texture = new Texture(image).resize(256,256);
	image.free();
	
	frames.push(texture);
	if ( frames.length < 3 )
		continue;
	
	if ( getKeyState(K_n) )
		frames[0].NR(frames[1], frames[2]);

//	frames[1] = frames[2];

	var tmp = frames.shift();
	displayTexture(tmp);
	tmp.free();


//	var level = texture.GetGlobalLevel();
//	texture.ClampLevels(level-0.2, level+0.2);

const kernelGaussian = [0,3,10,3,0, 3,16,26,16,3, 10,26,26,26,10, 3,16,26,16,3, 0,3,10,3,0 ];
const kernelGaussian2 = [2,4,5,4,2, 4,9,12,9,4, 5,12,15,12,5, 4,9,12,9,4, 2,4,5,4,2]; // G(r) = pow(E,-r*r/(2*o*o))/sqrt(2*PI*o);
const kernelEmboss = [-1,0,0, 0,0,0 ,0,0,1];
const kernelLaplacian = [-1,-1,-1, -1,8,-1, -1,-1,-1];

	texture.desaturate();
	 
	 texture.normalizeLevels();
	
/*
	tmp1.Set(texture);
	tmp1.shift(1,1);
	texture.add(tmp1);
	tmp1.shift(1,-1);
	texture.add(tmp1, -1);
*/	
	


continue;



	if ( texture.getGlobalLevel() < 0.16 ) {

		noise.add(texture);
		noise.mult(0.84);
		print('...getting noise\n');
		displayTexture(noise);
	} else {

		tmp1.mult(0.92);
		tmp1.add(texture);

		final.Set(tmp1);

		if ( getKeyState(K_n) ) // without noise reduction
			final.add(noise, -1);
		final.mult(0.2);

		if ( getKeyState(K_a) ) // accum image
			displayTexture(final);
		else
			displayTexture(texture);
	}
continue;

	
	if ( frame % 2 ) {
		
		tmp1.Set(texture);
	} else {
		
		tmp1.add(texture, -1);
		
		if ( tmp1.getGlobalLevel() > 0.001 )
			displayTexture(texture);
		
	}
continue;
	
	

}
