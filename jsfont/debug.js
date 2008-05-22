LoadModule('jslang');
LoadModule('jsstd');
LoadModule('jsfont');
LoadModule('jsimage');
LoadModule('jsio');


// DrawChar('1');

var f = new Font('arial.ttf');
f.size = 100;
//f.borderWidth = 100;
//f.italic = true;
//f.bold = true;
f.letterSpacing = 4;

var img = f.Draw('AWA1');
new File('text.png').content = EncodePngImage(img);

