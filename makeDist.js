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

function GetLatestSVNRevision() {
	
	Print( 'Get latest SVN revision', '\n' );
	var p = new Process(GetEnv('ComSpec'), ['/c', 'svn', 'info', '--xml']); // '-r', 'HEAD', 
	var svnInfo = '';
	for ( let data; data = p.stdout.Read(); )
		svnInfo += data;
	svnInfo = new XML(svnInfo.replace(/<\?.*?\?>/, '')); // remove: <?xml version="1.0"?>
	return svnInfo.entry.@revision;
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


var [type, jslibsVersion, jslibsRevision, changes] = GetLatestChanges();
if ( jslibsRevision[0] == 'r' )
	jslibsRevision = jslibsRevision.substr(1);
changes = changes.replace(/<pre>|<\/pre>/g, ''); // cleanup
changes = changes.replace(/\n/g, '\r\n');
//jslibsRevision = 'r'+GetLatestSVNRevision();


var readme = Expand(new File('./dist/readme.txt.tpl').content, function(id) {
	
	switch(id) {
		case 'type':
			return type;
		case 'version':
			return jslibsVersion;
		case 'jslibsRevision':
			return jslibsRevision;
		case 'changes':
			return IndentText(changes, '  ');
	}
});


Copy('qa.js', './dist/tests');

RecursiveDir('./src', function(file) /_qa\.js$/(file.name) && Copy(file.name, './dist/tests') );

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

RecursiveDir('./Win32_opt', function(file) /\.dll$/(file.name) && Copy(file.name, './dist/bin') );

Copy('./Win32_opt/jshost.exe', './dist/bin');
Copy('./Win32_opt/jswinhost.exe', './dist/bin');

Copy('./libs/openal/sdk/redist/OpenAL32.dll', './dist/bin');
Copy('./libs/openal/sdk/redist/wrap_oal.dll', './dist/bin');

new File('./dist/README.TXT').content = readme;

Zip('./dist/*', 'jslibs_'+type+'_'+jslibsVersion+'_'+'r'+jslibsRevision+'.zip');
