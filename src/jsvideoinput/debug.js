// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');

LoadModule('jsvideoinput');

Exec('..\\common\\tools.js');

CreateOpenGLWindow();

var vi = new VideoInput('QuickCam', 1, 1, 60); // try to get the smallest size and the lowest fps
Print('full name: '+vi.name, '\n');

var tmp1 = new Texture(256,256,3).Set(0);
var final = new Texture(256,256,3).Set(0);
var noise = new Texture(256,256,3).Set(0);

Print( vi.width + 'x' + vi.height, '\n' );

var frames = [];
var frame = 0;
var key;
while ( (key = GetKey()) != 27 ) {

	CollectGarbage();
	frame++;
	var texture = new Texture(vi.GetImage()).Resize(256,256);
	
	frames.push(texture);
	if ( frames.length < 3 )
		continue;
	
	if ( GetKeyState(K_n) )
		frames[0].NR(frames[1], frames[2]);

//	frames[1] = frames[2];

	DisplayTexture(frames.shift());


//	var level = texture.GetGlobalLevel();
//	texture.ClampLevels(level-0.2, level+0.2);

//	texture.Convolution([-1,-1,-1, -1,8,-1, -1,-1,-1]);
	
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
