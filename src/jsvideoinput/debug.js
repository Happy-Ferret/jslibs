// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');

LoadModule('jsvideoinput');

Print("start\n");

Print( VideoInput.hasDevice );

Halt();

var vi = new VideoInput('QuickCam', 1, 1, 1); // try to get the smallest size and the lowest fps
Print('full name: '+vi.name, '\n');

while (!endSignal) {

	var img = vi.GetImage();
	var t1 = IntervalNow();
	Print( 1000/(t1 - t0), '\n' );
	var t0 = t1;

	new File('myImage.png').content = EncodePngImage(img);
}

