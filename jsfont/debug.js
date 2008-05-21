LoadModule('jslang');
LoadModule('jsstd');
LoadModule('jsfont');
LoadModule('jsimage');
LoadModule('jsio');


// DrawChar('1');

var f = new Font('arial.ttf', true);

f.SetSize(0, 50);

var img = f.Draw('The ascender is the vertical distance from the ...');

new File('text.png').content = EncodePngImage(img);




