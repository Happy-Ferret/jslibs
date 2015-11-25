<table align='right' height='20'><tr><td></td></tr></table>
Jslibs is a standalone JavaScript development runtime environment for using JavaScript as a general-purpose scripting language.

jslibs uses [SpiderMonkey](http://developer.mozilla.org/en/docs/SpiderMonkey) library that is Gecko's JavaScript engine (SpiderMonkey is used in various Mozilla products, including Firefox). The latest version of jslibs uses the new TraceMonkey/JaegerMonkey library with tracing/method JIT enabled.

jslibs provides a set of native modules that contains various general purpose classes and functions.

Some of these modules are simple wrappers for familiar libraries such as: [zlib](http://www.zlib.net/), [SQLite](http://www.sqlite.org/), FastCGI, [NSPR](http://www.mozilla.org/projects/nspr/) <sub>(Netscape Portable Runtime)</sub> , [ODE](http://www.ode.org/) <sub>(Open Dynamics Engine)</sub> , [libpng](http://www.libpng.org/pub/png/libpng.html), [libjpeg](http://freshmeat.net/projects/libjpeg/), librsvg, SDL, libiconv, [OpenGL](http://www.opengl.org/), [OpenAL](http://www.openal.org/), ogg [vorbis](http://xiph.org/vorbis/), [libTomCrypt](http://libtom.org/), libffi <sub>(Foreign function interface)</sub> , [...](http://code.google.com/p/jslibs/wiki/JSLibs)

Other modules provide tools for enhancing JavaScript programming : Print(), Load(), Exec(), Seal(), Expand(), Buffer class, Blob class, Sandbox class, [...](http://code.google.com/p/jslibs/wiki/JSLibs)

The jslibs distribution comes with a small standalone command line access program ([jshost](jshost.md)) that can be used to run JavaScript files.
Note that the modules are quite independent from jshost and can be used in any project that embeds SpiderMonkey.
A Windows binary (without console) is also available ([jswinhost](jswinhost.md)).

[![](http://jslibs.googlecode.com/svn/wiki/source.png)](http://jslibs.googlecode.com/svn/trunk/) jslibs is available under GNU GPL 2.0 license. You can access the source code through the Subversion repository using the '[Source](http://code.google.com/p/jslibs/source)' tab.

[![](http://jslibs.googlecode.com/svn/wiki/wiki.png)](http://code.google.com/p/jslibs/w/list?q=label:doc) [![](http://jslibs.googlecode.com/svn/wiki/calendar2.png)](http://code.google.com/p/jslibs/wiki/Roadmap) Project documentation is available in the '[Wiki](http://code.google.com/p/jslibs/w/list)' tab. In this section, you can find modules and classes [API](JSLibs.md).

[![](http://jslibs.googlecode.com/svn/wiki/issue.png)](http://code.google.com/p/jslibs/issues/entry) [![](http://jslibs.googlecode.com/svn/wiki/idea.png)](http://code.google.com/p/jslibs/issues/entry) If you would like to report a defect or request an enhancement, click the '[Issues](http://code.google.com/p/jslibs/issues/list)' tab and read the existing issues. If no existing issue is relevant, you may enter a new one. You can also use the discussion group to propose new features.

[![](http://jslibs.googlecode.com/svn/wiki/discuss.png)](http://groups.google.com/group/jslibs/post) Feel free to ask any question or make any comments on this project in the [discussion group](http://groups.google.com/group/jslibs) or the [mailing list](mailto:jslibs@googlegroups.com). You can also [submit a suggestion](https://www.google.com/moderator/#16/e=1e5438).

[![](http://jslibs.googlecode.com/svn/wiki/fr.png)](http://fr.wikipedia.org/wiki/Francais) [![](http://jslibs.googlecode.com/svn/wiki/gb.png)](http://en.wikipedia.org/wiki/English_language) You can contact me at [soubok+jslibs@gmail.com](mailto:soubok+jslibs@gmail.com) or leave a [comment](Comments.md).

[![](http://jslibs.googlecode.com/svn/wiki/linux.png)](http://code.google.com/p/jslibs/wiki/jslibsBuild#Linux_Compilation) [![](http://jslibs.googlecode.com/svn/wiki/windows.png)](http://code.google.com/p/jslibs/wiki/jslibsBuild#Windows_Compilation)

![http://jslibs.googlecode.com/svn/wiki/hline.png](http://jslibs.googlecode.com/svn/wiki/hline.png)![http://jslibs.googlecode.com/svn/wiki/hline.png](http://jslibs.googlecode.com/svn/wiki/hline.png)![http://jslibs.googlecode.com/svn/wiki/hlinefade.png](http://jslibs.googlecode.com/svn/wiki/hlinefade.png)

[![](http://jslibs.googlecode.com/svn/wiki/news.png)](http://code.google.com/p/jslibs/wiki/News)

  * **2013.07.06** - updating to the latest JS engine and moving to Visual C++ 2010.

  * **2012.04.03** - re-activating jstask module (kind of webworker).
  * **2012.04.01** - update Linux port.

  * **2012.02.29** - replacing Blob by ArrayBuffer.
  * **2012.02.19** - make jslibs functions and methods lowerCamelCase.
  * **2012.02.12** - update the JavaScript engine to the latest version (mozilla-central-4a9a6ffd1f21)
  * **2012.02.09** - any [suggestion](https://www.google.com/moderator/#16/e=1e5438) ?

  * **2011.09.27** - updating to the latest JS engine (firefox 7)
  * **2011.09.21** - updating to the latest JS engine (firefox 6.0.2)

  * **2011.06.09** - Working on zip files handling (using unzip library).
  * **2011.06.06** - Fixing Linux compilation.
  * **2011.05.25** - Update the JavaScript engine to version 1.8.5 (same as FireFox 4).
  * **2011.05.22** - Make [jslinux](http://bellard.org/jslinux/) run with jslibs (see [tests/jslinux](http://code.google.com/p/jslibs/source/browse/trunk#trunk%2Ftests%2Fjslinux)).

  * **2011.01.04** - Code optimization (speed and size).

  * **2010.12.31** - Fix Win32 compilation.
  * **2010.12.19** - Removing memory leaks.

  * **2010.10.27** - Update to JaegerMonkey engine (huge work).
  * **2010.10.19** - Working on Linux64 port.

  * **2010.05.17** - Switching to visual studio express 2008 (see [Windows Compilation](http://code.google.com/p/jslibs/wiki/jslibsBuild#Windows_Compilation))

  * **[. . . ](http://code.google.com/p/jslibs/wiki/News)**

![http://jslibs.googlecode.com/svn/wiki/hline.png](http://jslibs.googlecode.com/svn/wiki/hline.png)![http://jslibs.googlecode.com/svn/wiki/hlinefade.png](http://jslibs.googlecode.com/svn/wiki/hlinefade.png)

## SVN revision status ##
  * **stable** <sup>(last release + doc in sync)</sup>: **2572** <sup>(May 31, 2009)</sup>
  * **testing** <sup>(compile + QA pass on Win & Linux 32bit)</sup>: **3040** <sup>(Dec 9, 2009)</sup>
  * **unstable** <sup>(may crash)</sup>: **HEAD**
> (use `svn checkout --revision #### http://jslibs.googlecode.com/svn/trunk/ ./jslibs` **or** `svn update --revision ####`)

![http://jslibs.googlecode.com/svn/wiki/hline.png](http://jslibs.googlecode.com/svn/wiki/hline.png)![http://jslibs.googlecode.com/svn/wiki/hlinefade.png](http://jslibs.googlecode.com/svn/wiki/hlinefade.png)
# Some code snippets #

**Capture an image from a webcam (module [jsvideoinput](jsvideoinput.md))**
```
LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsvideoinput'); 

var vi = new VideoInput('QuickCam', 800, 600, 30); // 800x600 at 30fps
var img = vi.GetImage();
new File('myImage.png').content = EncodePngImage(img);
```


**Create a JavaScript thread (module [jstask](jstask.md))**
```
LoadModule('jsstd');
LoadModule('jstask');

function MyTask( request ) {

 for ( var i = 0; i < 200000; i++); // working...
 return 'r#' + request;
}

var myTask = new Task(MyTask);

for ( var i = 0; i < 10; i++ )
 myTask.Request(i);

while ( !myTask.idle )
 Print( myTask.Response(), ', ' );
```


**Create a PNG from SVG (modules [jsimage](jsimage.md), [jssvg](jssvg.md), [jsio](jsio.md))**
```
LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jssvg');

var svgSource = <svg width="100%" height="100%">
 <rect x="0" y="0" width="100" height="100" fill="#FF4422" />
 <circle cx="50" cy="50" r="25" stroke="black" stroke-width="5" fill="none"/>
</svg>

var svg = new SVG();
svg.Write(svgSource);
svgimage = svg.RenderImage();
new File('myImage.png').content = EncodePngImage( svgimage );
```


**Display the content of a file (module [jsio](jsio.md))**
```
Print( new File('test.txt').content );
```


**Compress a string (module [jsz](jsz.md))**
```
var compressedText = new Z(Z.DEFLATE)('This text will be compressed', true);
```


**Call MessageBoxA in User32.dll (module [jsffi](jsffi.md) / jsni)**
```
function Alert( text, caption ) {

 var ret = new NativeData().PU32.Alloc();
 var mod = new NativeModule('C:\\WINDOWS\\SYSTEM32\\User32')
 var MessageBoxA = mod.Proc('MessageBoxA');
 MessageBoxA( ret, DWORD( 0 ), SZ(text), SZ( caption || 'Alert' ), DWORD( 1 ) );
 return ret[0];
}
```


**One-line database query (module [jssqlite](jssqlite.md))**
```
var myDatabaseVersion = new Database('myDatabase').Exec('PRAGMA user_version');
```


**Non-blocking sockets (module [jsio](jsio.md))**
```
var soc = new Socket();
soc.Connect( 'localhost', 8080 );
soc.writable = function(s) {

 delete soc.writable;
 s.Write('GET\r\n');
}
soc.readable = function(s) {

 Print( s.Read() );
}
while(!endSignal)
 Poll([soc],100);
```

**Encrypt data using blowfish (module [jscrypt](jscrypt.md))**
```
var rnd = new Prng('yarrow');
rnd.AutoEntropy(128);

var key = new Hash('sha256')('this is a secret key');
var IV = rnd(Crypt.BlockLength('blowfish'));

var crypt = new Crypt( 'ctr', 'blowfish', key, IV );
var plainText = 'secret string';
var cipherData = crypt.Encrypt(plainText);
```


**Draw "Hello world" in hello.png (module [jsfont](jsfont.md))**
```
var f = new Font('arial.ttf');
f.size = 100;
var img = f.Draw('Hello world');
var file = new File('hello.png');
file.content = EncodePngImage(img);
```


**Ogg Vorbis player (module [jsaudio](jsaudio.md) + [jssound](jssound.md))**
```
LoadModule('jsio');
LoadModule('jsstd');
LoadModule('jssound');
LoadModule('jsaudio');

var decoder = new OggVorbisDecoder(new File('41_30secOgg-q0.ogg').Open(File.RDONLY));

Oal.Open();
var src = Oal.GenSource();

var pcm, decodeSize = 512;
while ( (pcm = decoder.Read(decodeSize)) && !endSignal ) {

	Oal.SourceQueueBuffers(src, Oal.Buffer(pcm));
	if ( Oal.GetSourceInteger(src, Oal.SOURCE_STATE) == Oal.INITIAL )
		Oal.PlaySource(src);
	decodeSize = pcm.frames * 2;
}

var totalTime = decoder.frames/decoder.rate;
var currentTimeOffset = Oal.GetSourceReal(src, Oal.SEC_OFFSET);
for ( i = 0; !endSignal && i < totalTime - currentTimeOffset; i++ )
	Sleep(1000);

Oal.Close();
```


**Use Open Dynamics Engine (module [jsode](jsode.md))**
```
var world = new World();
world.gravity = [0,0,-0.81];

var body1 = new Body(world);
var geo1 = new GeomBox();
geo1.body = body1;


var body2 = new Body(world);
var geo2 = new GeomBox();
geo2.body = body2;

var joint = new JointHinge(world);

joint.body1 = body1;
joint.body2 = body2;
joint.anchor = [4,4,0];
joint.axis = [1,0,0];
joint.loStop = 1;
joint.hiStop = 1.5;

body1.linearVel = [0,0,19];
body1.angularVel = [1,1,0];
...
```


**Load a jpeg image (module [jsimage](jsimage.md))**
```
var texture = new Jpeg(new File('R0010235.JPG').Open(File.RDONLY)).Load().Trim([10,10,20,20]);
Print( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );
```


**A configurable systray shortcut launcher (module [jswinshell](jswinshell.md))**
```
LoadModule('jsstd');
LoadModule('jswinshell');

var s = new Systray();
s.icon = new Icon( 0 );
s.menu = { add:'Add', exit:'Exit', s1:{ separator:true } };
s.onmousedown = s.PopupMenu;

s.oncommand = function( id, button ) {

	switch ( id ) {
		case 'exit':
			return true;
		case 'add':
			var fileName = FileOpenDialog( 'executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*' );
			if ( !fileName )
				return;
			s.menu[fileName] = { 
				icon: ExtractIcon( fileName ), 
				text: fileName.substr( fileName.lastIndexOf( '\\' ) + 1 )
			};
			break;
		default:
			if ( button == 1 ) // left-click to run.
				CreateProcess( id );
			else // right-click to remove.
				if ( MessageBox( 'Remove item: ' + id + '? ', 'Question', MB_YESNO) == IDYES )
					delete s.menu[id];
		}
}

do { Sleep(100) } while ( !s.ProcessEvents() );
```


**A server-side script using E4X with the FastCGI module (module [jsfastcgi](jsfastcgi.md))**
```
Write( "Content-type: text/html\r\n\r\n" );

function CGIVariableList() {

	var fcgiParams = GetParam();
	var list = <ul/>;
	for ( var k in fcgiParams )
		list += <li><b>{k} = </b><pre>{fcgiParams[k]}</pre></li>;
	return list;
}

Write(
<html>
	<head>
		<title>teswt</title>
	</head>
	<body>
		<H1>HELLO WORLD</H1>
		<p>CGI/1.1 variables:</p> {CGIVariableList()}
	</body>
</html>
);
```


**A minimalist HTTP server (modules [jsstd](jsstd.md), [jsio](jsio.md))**
```
LoadModule('jsstd');
LoadModule('jsio');

const CRLF = '\r\n';
var descList = [];
var serv = new Socket(Socket.TCP);
serv.Bind(8081);
serv.Listen();
descList.push(serv);

function Respond() {

  this.httpHeader += this.Read();
  if ( this.httpHeader.indexOf(CRLF+CRLF) == -1 )
    return;

  Print('Received: \n' + this.httpHeader + '\n');
  descList.splice(descList.indexOf(this), 1);

  var writeOp = this.Write(
    'HTTP/1.0 200 OK' + CRLF +
    'Content-Type: text/html; charset=utf-8' + CRLF +
    'Cache-Control: no-cache, must-revalidate' + CRLF +
    'Pragma: no-cache' + CRLF + CRLF +
    '<html><body>Hello from <a href="http://jslibs.googlecode.com/">jslibs</a> at ' + new Date() + '</body></html>' );
  this.Close();
};

serv.readable = function () {

  var desc = this.Accept();
  desc.httpHeader = '';
  desc.readable = Respond;
  descList.push(desc);
}

Print('HTTP server minimal example. Point a web browser at http://localhost:8081. CTRL+C to exit\n');

while ( !endSignal )
 Poll(descList, 50);
```


**Download an image from the web (using blocking sockets)**
```
LoadModule('jsstd');
LoadModule('jsio');
var f = new Socket();
f.Connect('apod.nasa.gov', 80);
f.Write('GET /apod/image/1105/cenAjets_many_1280.jpg HTTP/1.0\r\nAccept:*/*\r\n\r\n');
var image = Stringify(f);
image = image.substr(image.indexOf('\r\n\r\n')+4);
new File('img.jpg').content = image;
```


**A wget function (using non-blocking sockets)**
```
function DownloadFile(host, port, uri) {

	const CRLF = '\r\n';
	var soc = new Socket();
	soc.Connect(host, port);
	soc.writable = true;
	ProcessEvents(Descriptor.Events([soc]), EndSignalEvents());
	delete soc.writable;
	soc.Write('GET '+uri+' HTTP/1.0'+CRLF+'Accept:*/*'+CRLF+'Connection:close'+CRLF+'User-Agent:jslibs'+CRLF+CRLF);
	soc.readable = true;
	var tmp, data = new Blob();
	for (;;) {
	
		ProcessEvents(Descriptor.Events([soc]), EndSignalEvents());
		tmp = soc.Read();
		if ( !tmp )
			break;
		data = data.concat(tmp);
	}
	return data.substr(data.indexOf(CRLF+CRLF)+4);
}

function Wget(url) {

	var urlChunks = ParseUri(url);
	new File(urlChunks.file).content = DownloadFile(urlChunks.host, urlChunks.port || 80, urlChunks.path);
}

Wget('http://www.google.fr/images/logos/ps_logo2.png');
```


<br><br>

Leave a <a href='Comments.md'>comment</a>.<br>
<br>
<br><br><br>

<wiki:gadget url="http://www.ohloh.net/p/6376/widgets/project_partner_badge.xml" height="53" border="0"/><br>
<br>
<br><br><br>

|<a href='http://my4.statcounter.com/project/standard/stats.php?project_id=2192039'><img src='http://c21.statcounter.com/counter.php?sc_project=2192039&java=0&security=4b0a1042&invisible=0&.png' /></a>|  <sub>visitors since 2007.01.24</sub>