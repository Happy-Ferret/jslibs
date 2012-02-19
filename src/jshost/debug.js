//RunJsircbot(false); throw 0;
//loadModule('jsstd'); loadModule('jsio'); var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/exec(currentDirectory)[0] + '_qa.js');  Halt();
//loadModule('jsstd'); exec('../common/tools.js'); var QA = FakeQAApi;  RunLocalQAFile();
//loadModule('jsstd'); exec('../common/tools.js'); RunQATests('jshost -exclude jstask');
//loadModule('jsstd'); loadModule('jsio'); currentDirectory += '/../../tests/jslinux'; exec('start.js'); throw 0;
//SetPerfTestMode();

loadModule('jsstd');
loadModule('jsio');

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

var jslibsRoot = '..';

var classList = {__proto__:null};

recursiveDir(jslibsRoot, function(f) {

	if ( /\.cpp$/.test(f.name) ) {

		var content = f.content.toString();
		( content.match(/BEGIN_CLASS\( ?\w+ ?\)/g) || [] ).forEach(function(item) {
			
			classList[ /BEGIN_CLASS\( ?(\w+) ?\)/.exec(item)[1] ] = true;
		});
	}
});


var i = 0;
recursiveDir(jslibsRoot, function(f) {
	
	if ( /\.js$/.test(f.name) ) {

		var content = f.content.toString();
		
		var newContent = content.replace(/([A-Z][a-z0-9]\w*)/g, function(all, token) {
			
			if ( !classList[token] ) {
			
				print(token,'\n');
				if ( ++i > 100 )
					halt();
			}
		});
		
		//print(newContent, '\n');
	}
});

