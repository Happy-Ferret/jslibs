LoadModule('jsstd');
LoadModule('jsio');


function Zip(dir, destFilename) {

	Print( 'zip '+dir+' to '+destFilename, '\n' );
	// eXclude Recursively .svn directories
	// eXclude .tpl files
	var p = new Process('D:/Tools/mozilla-build/7zip/7z.exe', ['a', '-tzip', '-mx9', '-xr!*.tpl', '-xr!*.svn', destFilename, dir]);
	for ( let data; data = p.stdout.Read(); )
		Print(data);
	for ( let data; data = p.stderr.Read(); )
		Print(data);
}

function GetLatestRemoteSVNRevision() {
	
	Print( 'Get latest remote SVN revision', '\n' );
	var p = new Process('svn', ['--xml', 'info', '-r', 'HEAD']);
	var svnInfo = '';
	for ( let data; data = p.stdout.Read(); )
		svnInfo += data;
	svnInfo = new XML(svnInfo.replace(/<\?.*?\?>/, ''));
	return svnInfo.entry.@revision;
}

function GetSVNWorkingCopyVersion() {
	
	var p = new Process('svnversion');
	var info = /(?:(\d+):)?(\d+)([MSP]*)/(p.stdout.Read());
	return { mixed:Number(info[1]), wc:Number(info[2]), modified:info.indexOf('M')>0, switched:info.indexOf('S')>0, partial:info.indexOf('P')>0 };
}


function IndentText(text, indent) [ indent+line for each (line in text.split('\n')) ].join('\n');

function GetLatestChanges() {

	var host = 'jslibs.googlecode.com';
	var page = '/svn/wiki/ReleaseNotes.wiki';

	var response = '';
	var soc = new Socket();
	soc.nonblocking = true;
	soc.Connect(host, 80);
	soc.writable = function(s) {

		delete soc.writable;
		s.Write( 'GET '+page+' HTTP/1.0\r\nHost: '+host+'\r\nnUser-Agent: jslibs\r\nAccept: */*\r\nAccept-Charset: ISO-8859-1\r\nConnection: close\r\n\r\n' );
	}
	soc.readable = function(s) {

		var res = s.Read();
		if ( res )
			response += res;
		else
			delete soc.readable;
	}
	while( Poll([soc], 500) );
	soc.Close();
	
	var res = /== jslibs (\w+) ([0-9.?]+) (r[0-9?]+)? .*?==\n((?:.|\n)*?)\n==/img(response);
	if ( res == null )
		return [];
	return res.slice(1);		
}


function GetClassRevision(moduleName, className) {

  var proc = new Process('jshost', [ '-u', '-i', 'LoadModule("'+moduleName+'"); _configuration.stdout('+className+'._revision)']);
  return Number(proc.stdout.Read());
}

var copiedFilesDest = [];

function Copy(fromFilename, toFilename) {
	
	Print( 'Copying '+fromFilename+' to '+toFilename+'\n' );
	var fromFile = new File(fromFilename).Open(File.RDONLY);
	var toFile = new File(toFilename);
	
	if ( toFile.exist && toFile.info.type == File.FILE_DIRECTORY ) {
	
		var pos = fromFilename.lastIndexOf('/');
		if ( pos == -1 )
			toFile = new File(toFilename+'/'+fromFilename);
		else
			toFile = new File(toFilename+'/'+fromFilename.substr(pos+1));
	}
	toFile.Open(File.WRONLY | File.CREATE_FILE | File.TRUNCATE);
	for ( var buf; buf = fromFile.Read(65536); )
		toFile.Write(buf);
	toFile.Close();
	copiedFilesDest.push(toFile.name);
	fromFile.Close();
}


function RecursiveDir(path, callback) {
	
	(function(path) {

		var dir = new Directory(path);
		dir.Open();
		for ( var entry; ( entry = dir.Read(Directory.SKIP_BOTH) ); ) {

			var file = new File(dir.name+'/'+entry);
			switch ( file.info.type ) {
				case File.FILE_DIRECTORY:
					arguments.callee(file.name);
					break;
				case File.FILE_FILE:
					callback(file);
					break;
			}
		}
		dir.Close();
	})(path);
}



// main ======================================================================

var dist = new Directory('./dist');
if ( !dist.exist )
	throw 'Unable to find '+dist.name;

switch ( arguments[1] ) {
	case '--devsnapshot':
		type = 'DEVSNAPSHOT';
		jslibsVersion = '';
		var info = GetSVNWorkingCopyVersion();
		jslibsRevision = info.wc;
		changes = 'http://code.google.com/p/jslibs/source/list';
		break;
		
	case '--version':
		var [type, jslibsVersion, jslibsRevision, changes] = GetLatestChanges();
		changes = changes.replace(/<pre>|<\/pre>/g, ''); // cleanup
		changes = changes.replace(/\n/g, '\r\n');
		if ( jslibsRevision[0] == 'r' )
			jslibsRevision = jslibsRevision.substr(1);
		//jslibsRevision = 'r'+GetLatestRemoteSVNRevision();
		break;
	default:
		Print( 'choose a type\n' );
		throw 0;
}



var readme = Expand(new File('./dist/readme.txt.tpl').content, function(id) {
	
	switch(id) {
		case 'type':
			return type;
		case 'version':
			return jslibsVersion;
		case 'revision':
			return jslibsRevision;
		case 'changes':
			return IndentText(changes, '  ');
	}
});



Copy('qa.js', './dist/tests');

RecursiveDir('./src', function(file) /_qa\.js$/(file.name) && Copy(file.name, './dist/tests') );

Copy('./tests/version.js', './dist/examples');
Copy('./tests/helloworld.js', './dist/examples');
Copy('./tests/systray.js', './dist/examples');
Copy('./tests/miniInteractiveConsole.js', './dist/examples');
Copy('./tests/3d.js', './dist/examples');
Copy('./tests/ffiTest.js', './dist/examples');
Copy('./tests/md5.js', './dist/examples');
Copy('./tests/cs.js', './dist/examples');
Copy('./tests/sqlite.js', './dist/examples');
Copy('./tests/ode.js', './dist/examples');
Copy('./tests/proceduralTextures.js', './dist/examples');
Copy('./tests/oggFilePlayer.js', './dist/examples');
Copy('./tests/41_30secOgg-q0.ogg', './dist/examples');
Copy('./tests/testForDebugger.js', './dist/examples');

Copy('./tests/svg.js', './dist/examples');
Copy('./src/jssvg/fonts.conf', './dist/examples');

Copy('./src/jsdebug/debugger.js', './dist/bin');
Copy('./src/jsdebug/debugger.xul', './dist/bin');

Copy('./src/jsprotex/liveconsole.xul', './dist/bin');


RecursiveDir('./Win32_opt', function(file) /\.dll$/(file.name) && Copy(file.name, './dist/bin') );

Copy('./Win32_opt/jshost.exe', './dist/bin');
Copy('./Win32_opt/jswinhost.exe', './dist/bin');

Copy('./libs/openal/sdk/redist/OpenAL32.dll', './dist/bin');
Copy('./libs/openal/sdk/redist/wrap_oal.dll', './dist/bin');

new File('./dist/README.TXT').content = readme;
copiedFilesDest.push('./dist/README.TXT');


Zip('./dist/*', 'jslibs_'+type+'_'+jslibsVersion+'_'+'r'+jslibsRevision+'.zip');

// cleanup

for each ( var f in copiedFilesDest )
	new File(f).Delete()
	
