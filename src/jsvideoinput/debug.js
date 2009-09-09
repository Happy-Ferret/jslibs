// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');

LoadModule('jsvideoinput');

Exec('..\\common\\tools.js');

CreateOpenGLWindow();

var vi = new VideoInput('QuickCam', 60); // try to get the smallest size and the lowest fps
Print('full name: '+vi.name, '\n');

var tmp1 = new Texture(128,128,3).Set(0);
var final = new Texture(128,128,3).Set(0);
var noise = new Texture(128,128,3).Set(0);

var frame = 0;
var key;
while ( (key = GetKey()) != 27 ) {

	CollectGarbage();
	frame++;
	var texture = new Texture(vi.GetImage()).Resize(128,128);
	
	if ( frame % 2 ) {
		
		tmp1.Set(texture);
	} else {
		
		tmp1.Add(texture, -1);
		
		if ( tmp1.GetGlobalLevel() > 0.001 )
			DisplayTexture(texture);
		
	}
	
	
continue;
	
	
	if ( globalLevel < 0.16 ) {

		noise.Add(texture);
		noise.Mult(0.84);
		Print('...Getting noise\n');
		DisplayTexture(noise);
	} else {

		tmp1.Mult(0.92);
		tmp1.Add(texture);

		final.Set(tmp1);

		if ( !GetKeyState(K_n) ) // without noise reduction
			final.Add(noise, -1);
		final.Mult(0.1);

		if ( GetKeyState(K_s) ) // source image only
			DisplayTexture(texture);
		else
			DisplayTexture(final);
	}

}
