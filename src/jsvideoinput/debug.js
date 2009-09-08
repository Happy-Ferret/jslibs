// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');

LoadModule('jsvideoinput');

Exec('..\\common\\tools.js');

CreateOpenGLWindow();

var vi = new VideoInput('QuickCam', 800, 600, 60); // try to get the smallest size and the lowest fps
Print('full name: '+vi.name, '\n');

var tmp1 = new Texture(128,128,3).Set(0);
var tmp2 = new Texture(128,128,3).Set(0);
var noise = new Texture(128,128,3).Set(0);

var key;
while ( (key = GetKey()) != 27 ) {

	var texture = new Texture(vi.GetImage()).Resize(128,128, true);
	
	var globalLevel = texture.GetGlobalLevel();
	Print('Global level: ', globalLevel, '\n');
	
	if ( globalLevel < 0.20 ) {

		noise.Add(texture);
		noise.Mult(0.88);
		Print('...Getting noise\n');
		DisplayTexture(noise);
	} else {

		tmp1.Mult(0.92);
		tmp1.Add(texture);

		tmp2.Set(tmp1);

		if ( !GetKeyState(K_n) ) // without noise reduction
			tmp2.Add(noise, -1);

//		tmp2.NormalizeLevels();
		tmp2.Mult(0.1);
		
		if ( GetKeyState(K_s) ) // source image only
			DisplayTexture(texture);
		else
			DisplayTexture(tmp2);
	}

	CollectGarbage();
	Sleep(10);
}
