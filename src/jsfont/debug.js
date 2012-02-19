// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

loadModule('jsstd');
loadModule('jsfont');
loadModule('jsimage');
loadModule('jsprotex');
loadModule('jsio');

var f = new Font('c:\\windows\\fonts\\arial.ttf');
f.size = 32;
f.verticalPadding = 10;
f.horizontalPadding = 10;
f.letterSpacing = 5;

//f.GetCharOutline('o'); Halt();

//var bump = new Texture(f.DrawString('Hello World', true));


f.DrawChar({__proto__:null});

Halt();

bump.OppositeLevels();
bump.Add(1);

//bump.BoxBlur(3,3);
bump.Resize( bump.width*3, bump.height*3, true );
//bump.BoxBlur(3,3);

var kernelGaussian = [2,4,5,4,2, 4,9,12,9,4, 5,12,15,12,5, 4,9,12,9,4, 2,4,5,4,2];
bump.Convolution(kernelGaussian);
bump.NormalizeLevels();

bump.Normals();

var texture = new Texture(bump.width, bump.height, 3);
texture.Set(0.2);

var ambiant = [1,0.8,1];
var diffuse = [0, 1, 0];
var specular =[1, 0.7, 1];
texture.Light(bump, [-2,1,2], ambiant, diffuse, specular, 1, 0.1);

new File('text.png').content = EncodePngImage(texture.Export());

Halt();

// DrawChar('1');

var f = new Font('C:\\WINDOWS\\Fonts\\arial.ttf');
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
