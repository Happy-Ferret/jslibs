// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');

LoadModule('jsvideoinput');

Exec('..\\common\\tools.js');

CreateOpenGLWindow();

var vi = new VideoInput('QuickCam', 999, 999, 60); // try to get the smallest size and the lowest fps
Print('full name: '+vi.name, '\n');

var t0 = new Texture(128,128,3).Set(0);
var t1 = new Texture(128,128,3).Set(0);

var key;
while ( (key = GetKey()) != 27 ) {

	var texture = new Texture(vi.width, vi.height, vi.channels);
	var img = vi.GetImage();
	texture.Import(img,0,0);
	texture.Resize(128,128);
	
	t0.Set(texture).Mult(0.1);
	t1.Add(t0);
	t1.NormalizeLevels();
	
	DisplayTexture(t1);
	CollectGarbage();
	Sleep(10);
}
