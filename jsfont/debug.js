LoadModule('jslang');
LoadModule('jsstd');
LoadModule('jsfont');
LoadModule('jsimage');
LoadModule('jsio');


// DrawChar('1');

var f = new Font('arial.ttf');
f.size = 50;
//f.horizontalPadding = 50;
//f.verticalPadding = 0;
//f.italic = true;
//f.bold = true;
f.letterSpacing = f.size / 2;
//f.useKerning = false;

//f.encoding = Font.UNICODE;

var img = f.DrawString('AVA bien');
//var img = f.DrawChar('x');

new File('text.png').content = EncodePngImage(img);
