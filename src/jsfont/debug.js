var loadModule = host.loadModule;
 //loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();
loadModule('jsstd'); exec('../common/tools.js'); runQATests('jsfont'); throw 0; // -inlineOnly

loadModule('jsstd');
loadModule('jsfont');
loadModule('jsimage');
loadModule('jsprotex');
loadModule('jsio');

var QA = {cx:{item:{path:'.'}}};

var f = new Font(QA.cx.item.path + '/AnarchySans.otf');
f.size = 10;
var image = f.drawString('Hello World', true);



throw 0;

//f.getCharOutline('o'); halt();

//var bump = new Texture(f.drawString('Hello World', true));


f.drawChar({__proto__:null});

halt();

bump.oppositeLevels();
bump.add(1);

//bump.BoxBlur(3,3);
bump.resize( bump.width*3, bump.height*3, true );
//bump.BoxBlur(3,3);

var kernelGaussian = [2,4,5,4,2, 4,9,12,9,4, 5,12,15,12,5, 4,9,12,9,4, 2,4,5,4,2];
bump.convolution(kernelGaussian);
bump.normalizeLevels();

bump.normals();

var texture = new Texture(bump.width, bump.height, 3);
texture.Set(0.2);

var ambiant = [1,0.8,1];
var diffuse = [0, 1, 0];
var specular =[1, 0.7, 1];
texture.Light(bump, [-2,1,2], ambiant, diffuse, specular, 1, 0.1);

new File('text.png').content = encodePngImage(texture.export());

halt();

// DrawChar('1');

var f = new Font('C:\\WINDOWS\\fonts\\arial.ttf');
f.size = 50;
//f.horizontalPadding = 50;
//f.verticalPadding = 0;
//f.italic = true;
//f.bold = true;
f.letterSpacing = f.size / 2;
//f.useKerning = false;

//f.encoding = Font.UNICODE;

var img = f.drawString('AVA bien');
//var img = f.DrawChar('x');

new File('text.png').content = encodePngImage(img);
