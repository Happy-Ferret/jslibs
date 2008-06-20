LoadModule('jsstd');
LoadModule('jsio');

function Map() ({ __proto__:null });

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


function CreateItemList() {

	var docExpr = /\/\*\*doc(?: (.*)\r?\n)?((.*\r?\n)*?)\*\*\//g;
	var hidden = /\/\./;
	var sourceCode = /\.(h|cpp|c)$/;

	var itemList = [];

	RecursiveDir( './src', function(file) {

		if ( !hidden(file.name) && sourceCode(file.name) ) {

			var source = file.content;
			docExpr.lastIndex = 0;
			var res;
			while( res = docExpr(source) ) {

				var item = Map();
				itemList.push(item);

				item.filePath = file.name;
				item.path = file.name.substr(0, file.name.lastIndexOf('/'));
				item.lastDir = item.path.substr(item.path.lastIndexOf('/')+1);
				item.fileName = file.name.substr(file.name.lastIndexOf('/')+1);
				item.wikiText = res[2];
				item.source = source;
				item.sourceIndex = docExpr.lastIndex - item.wikiText.length;
				
//				var attr = Map();
				var attr = {};
				item.attributs = attr;
				
				if ( res[1] )
					for each ( var a in res[1].split(' ') ) {

						var s = a.split(':');
						attr[s[0]] = s[1];
					}
			}
		}
	});

	return itemList;
}


function docBuilder(itemList) {

	itemList.sort(function(a,b) (b.attributs.o||0) - (a.attributs.o||0)); // o||0 manages: x > undefined and undefined > x
	
	
	var doc = <doc/>;

	var wikiFile;
	for ( var [i,item] in Iterator(itemList) ) {
		
		if ( item.attributs.f )
			wikiFile = item.attributs.f;

		doc[wikiFile]

		var currentFile = doc[wikiFile] || (doc[wikiFile] = []);
		
		if ( item.attributs.g )
		
		currentFile
	
		
		Print( item.path,' ', item.attributs.toSource(), '\n' );
	

	}
}

docBuilder(CreateItemList());



/**doc #HideProperties
 * void *ASSERT*( expression [, failureMessage ] )
  If the argument expression compares equal to zero, the failureMessage is written to the standard error device and the program stops its execution.
**/


/**doc #example #HideProperties 
  var foo = ['a', 'b', 'c'];
  ASSERT( i >= 0 || i < 3, 'Invalid value.' );
  Print( foo[i] );
**/

