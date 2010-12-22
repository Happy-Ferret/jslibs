// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');

LoadModule('jsvideoinput');

//Print( VideoInput.list.join('\n') ); throw 0;

LoadModule('jssdl');
LoadModule('jsgraphics');
LoadModule('jsprotex');

Exec('..\\common\\tools.js');

	
	//	GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
//		GlSetAttribute( GL_DOUBLEBUFFER, 1 );
		GlSetAttribute( GL_DEPTH_SIZE, 16 );
//		GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );
		SetVideoMode( 640, 480, 32, OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN

		Ogl.Hint(Ogl.PERSPECTIVE_CORRECTION_HINT, Ogl.NICEST);
		Ogl.Hint(Ogl.POINT_SMOOTH_HINT, Ogl.NICEST);
		Ogl.Viewport(0,0,videoWidth,videoHeight);
		Ogl.MatrixMode(Ogl.PROJECTION);
		Ogl.Perspective(60, 1, 0.1, 100000);
		Ogl.MatrixMode(Ogl.MODELVIEW);
		Ogl.ClearColor(0.2, 0.1, 0.4, 1);
		Ogl.Enable(Ogl.DEPTH_TEST);
		Ogl.Enable(Ogl.BLEND);
		Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);



var vi = new VideoInput('QuickCam', 1, 1, 60); // try to get the smallest size and the lowest fps
Print('full name: '+vi.name, '\n');

var tmp1 = new Texture(256,256,3).Set(0);
var final = new Texture(256,256,3).Set(0);
var noise = new Texture(256,256,3).Set(0);

Print( vi.width + 'x' + vi.height, '\n' );

vi.onImage = function() {
	Print('-');
}

var frames = [];
var frame = 0;
var key;
while ( !GetKeyState(K_ESCAPE) ) {

	ProcessEvents(SDLEvents({}), vi.Events());

//	while( PollEvent({}) );

	GlSwapBuffers();

var t1 = TimeCounter();
//CollectGarbage();
//Print( 'GC:',  (TimeCounter() - t1).toFixed(1), 'ms\n' );


	frame++;
	var image = vi.GetImage();
	var texture = new Texture(image).Resize(256,256);
	image.Free();
	
	frames.push(texture);
	if ( frames.length < 3 )
		continue;
	
	if ( GetKeyState(K_n) )
		frames[0].NR(frames[1], frames[2]);

//	frames[1] = frames[2];

	var tmp = frames.shift();
	DisplayTexture(tmp);
	tmp.Free();


//	var level = texture.GetGlobalLevel();
//	texture.ClampLevels(level-0.2, level+0.2);

const kernelGaussian = [0,3,10,3,0, 3,16,26,16,3, 10,26,26,26,10, 3,16,26,16,3, 0,3,10,3,0 ];
const kernelGaussian2 = [2,4,5,4,2, 4,9,12,9,4, 5,12,15,12,5, 4,9,12,9,4, 2,4,5,4,2]; // G(r) = pow(E,-r*r/(2*o*o))/sqrt(2*PI*o);
const kernelEmboss = [-1,0,0, 0,0,0 ,0,0,1];
const kernelLaplacian = [-1,-1,-1, -1,8,-1, -1,-1,-1];

	texture.Desaturate();
	 
	 texture.NormalizeLevels();
	
/*
	tmp1.Set(texture);
	tmp1.Shift(1,1);
	texture.Add(tmp1);
	tmp1.Shift(1,-1);
	texture.Add(tmp1, -1);
*/	
	


continue;



	if ( texture.GetGlobalLevel() < 0.16 ) {

		noise.Add(texture);
		noise.Mult(0.84);
		Print('...Getting noise\n');
		DisplayTexture(noise);
	} else {

		tmp1.Mult(0.92);
		tmp1.Add(texture);

		final.Set(tmp1);

		if ( GetKeyState(K_n) ) // without noise reduction
			final.Add(noise, -1);
		final.Mult(0.2);

		if ( GetKeyState(K_a) ) // accum image
			DisplayTexture(final);
		else
			DisplayTexture(texture);
	}
continue;

	
	if ( frame % 2 ) {
		
		tmp1.Set(texture);
	} else {
		
		tmp1.Add(texture, -1);
		
		if ( tmp1.GetGlobalLevel() > 0.001 )
			DisplayTexture(texture);
		
	}
continue;
	
	

}
