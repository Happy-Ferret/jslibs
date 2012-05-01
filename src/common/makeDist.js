LoadModule('jsstd');
LoadModule('jsio');

var svnDir = 'C:/Program Files/TortoiseSVN/bin/';

function zip(dir, destFilename) {

	print( 'zip '+dir+' to '+destFilename, '\n' );
	// eXclude Recursively .svn directories
	// eXclude .tpl files
	var p = new Process('7z', ['a', '-tzip', '-mx9', '-xr!*.tpl', '-xr!*.svn', destFilename, dir]);
	for ( let data; data = p.stdout.read(); )
		print(data);
	for ( let data; data = p.stderr.read(); )
		print(data);
}

function getLatestRemoteSVNRevision() {
	
	print( 'Get latest remote SVN revision', '\n' );
	var p = new Process(svnDir+'svn', ['--xml', 'info', '-r', 'HEAD']);
	var svnInfo = '';
	for ( let data; data = p.stdout.read(); )
		svnInfo += data;
	svnInfo = new XML(svnInfo.replace(/<\?.*?\?>/, ''));
	return svnInfo.entry.@revision;
}

function getSVNWorkingCopyVersion() {
	
	var p = new Process(svnDir+'svnversion');
	var info = /(?:(\d+):)?(\d+)([MSP]*)/(p.stdout.read());
	return {
		mixed: Number(info[1]),
		wc: Number(info[2]),
		modified: info.indexOf('M') > 0,
		switched: info.indexOf('S') > 0,
		partial: info.indexOf('P') > 0
	};
}

function indentText(text, indent) [ indent+line for each (line in text.split('\n')) ].join('\n');

function getLatestChanges() {

	var host = 'jslibs.googlecode.com';
	var page = '/svn/wiki/ReleaseNotes.wiki';

	var response = '';
	var soc = new Socket();
	soc.nonblocking = true;
	soc.connect(host, 80);
	soc.writable = function(s) {

		delete soc.writable;
		s.write( 'GET '+page+' HTTP/1.0\r\nHost: '+host+'\r\nnUser-Agent: jslibs\r\nAccept: */*\r\nAccept-Charset: ISO-8859-1\r\nConnection: close\r\n\r\n' );
	}
	soc.readable = function(s) {

		var res = s.read();
		if ( res )
			response += res;
		else
			delete soc.readable;
	}
	while( poll([soc], 500) );
	soc.close();
	
	var res = /== jslibs (\w+) ([0-9.?]+) (r[0-9?]+)? .*?==\n((?:.|\n)*?)\n==/img(response);
	if ( res == null )
		return [];
	return res.slice(1);		
}

function getClassRevision(moduleName, className) {

  var proc = new Process('jshost', [ '-u', '-i', 'var loadModule = host.loadModule; loadModule("'+moduleName+'"); host.stdout('+className+'._sourceId)']);
  return Number(proc.stdout.read());
}

var copiedFilesDest = [];

function copy(fromFilename, toFilename) {
	
	print( 'Copying '+fromFilename+' to '+toFilename+'\n' );
	var fromFile = new File(fromFilename).open(File.RDONLY);
	var toFile = new File(toFilename);
	
	if ( toFile.exist && toFile.info.type == File.FILE_DIRECTORY ) {
	
		var pos = fromFilename.lastIndexOf('/');
		if ( pos == -1 )
			toFile = new File(toFilename+'/'+fromFilename);
		else
			toFile = new File(toFilename+'/'+fromFilename.substr(pos+1));
	}
	toFile.open(File.WRONLY | File.CREATE_FILE | File.TRUNCATE);
	for ( var buf; buf = fromFile.read(65536); )
		toFile.write(buf);
	toFile.close();
	copiedFilesDest.push(toFile.name);
	fromFile.close();
}


function recursiveDir(path, callback) {
	
	(function(path) {

		var dir = new Directory(path);
		dir.open();
		for ( var entry; ( entry = dir.read(Directory.SKIP_BOTH) ); ) {

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
		dir.close();
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
		var info = getSVNWorkingCopyVersion();
		jslibsRevision = info.wc;
		changes = 'http://code.google.com/p/jslibs/source/list';
		break;
		
	case '--version':
		var [type, jslibsVersion, jslibsRevision, changes] = getLatestChanges();
		changes = changes.replace(/<pre>|<\/pre>/g, ''); // cleanup
		changes = changes.replace(/\n/g, '\r\n');
		if ( jslibsRevision[0] == 'r' )
			jslibsRevision = jslibsRevision.substr(1);
		//jslibsRevision = 'r'+getLatestRemoteSVNRevision();
		break;
	default:
		print( 'choose a type\n' );
		throw 0;
}



var readme = expand(new File('./dist/readme.txt.tpl').content, function(id) {
	
	switch ( id ) {
		case 'type':
			return type;
		case 'version':
			return jslibsVersion;
		case 'sourceId':
			return jslibsRevision;
		case 'changes':
			return indentText(changes, '  ');
	}
});



copy('qa.js', './dist/tests');

recursiveDir('./src', function(file) /_qa\.js$/(file.name) && copy(file.name, './dist/tests') );

copy('./tests/version.js', './dist/examples');
copy('./tests/helloworld.js', './dist/examples');
copy('./tests/systray.js', './dist/examples');
copy('./tests/miniInteractiveConsole.js', './dist/examples');
copy('./tests/3d.js', './dist/examples');
copy('./tests/ffiTest.js', './dist/examples');
copy('./tests/md5.js', './dist/examples');
copy('./tests/cs.js', './dist/examples');
copy('./tests/sqlite.js', './dist/examples');
copy('./tests/ode.js', './dist/examples');
copy('./tests/proceduralTextures.js', './dist/examples');
copy('./tests/oggFilePlayer.js', './dist/examples');
copy('./tests/41_30secOgg-q0.ogg', './dist/examples');
copy('./tests/testForDebugger.js', './dist/examples');

copy('./tests/svg.js', './dist/examples');
copy('./src/jssvg/fonts.conf', './dist/examples');

copy('./src/jsdebug/debugger.js', './dist/bin');
copy('./src/jsdebug/debugger.xul', './dist/bin');

copy('./src/jsprotex/liveconsole.xul', './dist/bin');


recursiveDir('./Win32_opt', function(file) /\.dll$/(file.name) && copy(file.name, './dist/bin') );

copy('./Win32_opt/jshost.exe', './dist/bin');
copy('./Win32_opt/jswinhost.exe', './dist/bin');

copy('./libs/openal/sdk/redist/OpenAL32.dll', './dist/bin');
copy('./libs/openal/sdk/redist/wrap_oal.dll', './dist/bin');

new File('./dist/README.TXT').content = readme;
copiedFilesDest.push('./dist/README.TXT');


zip('./dist/*', 'jslibs_'+type+'_'+jslibsVersion+'_'+'r'+jslibsRevision+'.zip');

// cleanup

for each ( var f in copiedFilesDest )
	new File(f).delete()
	
