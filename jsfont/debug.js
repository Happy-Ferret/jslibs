LoadModule('jslang');
LoadModule('jsstd');
LoadModule('jsfont');
LoadModule('jsimage');
LoadModule('jsio');


// DrawChar('1');

var f = new Font('arial.ttf');
f.size = 20;
//f.borderWidth = 100;
//f.useKerning = true;
f.letterSpacing = -10;

Print( f.Draw('123456', true) );
var img = f.Draw('123456');
var img = f.Draw('123456');
new File('text.png').content = EncodePngImage(img);

