LoadModule('jsstd');
LoadModule('jsfont');
LoadModule('jsimage');
LoadModule('jsprotex');
LoadModule('jsio');

var f = new Font(GetEnv('windir')+'\\Fonts\\' + 'arial.ttf');
f.size = 100;
f.verticalPadding = -16;
var img = f.DrawString('Hello world', true);

var t = new Texture(img);
var t1 = new Texture(t);

t1.AddNoise(0.6);
t1.BoxBlur(5,1);
t1.OppositeLevels();

t.BoxBlur(15,15);
t.Add(t1);
t.OppositeLevels();
t.Add(0.8);

Print('Writing "HelloWorld.png" \n');
new File('HelloWorld.png').content = EncodePngImage(t.Export());
