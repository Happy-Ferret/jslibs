// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');

LoadModule('jsvideoinput');

Print( uneval(VideoInput.list) );

var vi = new VideoInput('QuickCam', 1,1, 25);
var img = vi.GetImage();
new File('myImage.png').content = EncodePngImage(img);



