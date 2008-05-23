LoadModule('jslang');
LoadModule('jsstd');
LoadModule('jsfont');
LoadModule('jsimage');
LoadModule('jsio');


// DrawChar('1');

var f = new Font('arial.ttf');
f.size = 100;
//f.horizontalPadding = 50;
//f.verticalPadding = 0;
//f.italic = true;
f.bold = true;
f.letterSpacingFactor = -0.1;
f.useKerning = false;

//f.encoding = Font.UNICODE;

var img = f.DrawString(f.poscriptName, true);
//var img = f.DrawChar('x');

new File('text.png').content = EncodePngImage(img);
