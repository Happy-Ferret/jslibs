LoadModule('jslang');
LoadModule('jsstd');
LoadModule('jsfont');
LoadModule('jsimage');
LoadModule('jsio');


// DrawChar('1');

var f = new Font('arial.ttf');
f.size = 100;
//f.borderWidth = 100;
//f.useKerning = true;
var img = f.Draw('123');
new File('text.png').content = EncodePngImage(img);




