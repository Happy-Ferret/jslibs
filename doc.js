LoadModule('jsstd');
LoadModule('jsio');

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

var docExpr = /\/\*\*doc( .*\r?\n)?((.*\r?\n)*?)\*\*\//g;
var hidden = /\/\./;
var sourceCode = /\.(h|cpp|c)$/;

RecursiveDir( './src', function(file) {

	if ( !hidden(file.name) && sourceCode(file.name) ) {
		
		var code = file.content;
		docExpr.lastIndex = 0;
		var res;
		while( res = docExpr(code) ) {

			 Print('attr:'+res[1], '\n');
			 Print('text:'+res[2], '\n');
		}
	}
});


/*




/**doc #HideProperties
 * void *ASSERT*( expression [, failureMessage ] )
  If the argument expression compares equal to zero, the failureMessage is written to the standard error device and the program stops its execution.
**/


/**doc #example #HideProperties
  var foo = ['a', 'b', 'c'];
  ASSERT( i >= 0 || i < 3, 'Invalid value.' );
  Print( foo[i] );
**/











*/
