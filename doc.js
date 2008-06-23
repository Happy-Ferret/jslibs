LoadModule('jsstd');
LoadModule('jsio');

//function Map() ({ __proto__:null });
function Map() ({});

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


function CreateDocItemList() {

	var docExpr = /\/\*\*doc(?: (.*)\r?\n)?((.*\r?\n)*?)\*\*\//g;
	var hidden = /\/\./;
	var sourceCode = /\.(h|cpp|c)$/;

	var itemList = [];
	var index = 0;

	RecursiveDir( './src', function(file) {

		if ( !hidden(file.name) && sourceCode(file.name) ) {

			var source = file.content;
			docExpr.lastIndex = 0;
			var res;
			while( res = docExpr(source) ) {

				var item = Map();
				itemList.push(item);
				
				item.index = index++;
				item.filePath = file.name;
				item.path = file.name.substr(0, file.name.lastIndexOf('/'));
				item.lastDir = item.path.substr(item.path.lastIndexOf('/')+1);
				item.fileName = file.name.substr(file.name.lastIndexOf('/')+1);
				item.text = res[2];
				item.source = source;
				item.sourceIndex = docExpr.lastIndex - item.text.length;
				
				var attr = Map();
				item.attr = attr;
				
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


function MakeGroups( list, groupFunction ) {

	var group, grouped = {};
	for each ( var item in list ) {

		group = groupFunction(item, group);
		(grouped[group] || (grouped[group] = [])).push(item);
	}
	return [ g for each ( g in grouped ) ];
}


function FlattenGroup( groupList ) {

	var flat = [];
	for each ( var group in groupList )
		flat = flat.concat(group);
	return flat;
}


var itemList = CreateDocItemList();

// make module and file group
var moduleList = MakeGroups( itemList, function(item) item.lastDir );
for ( var module in moduleList )
	moduleList[module] = MakeGroups( moduleList[module], function(item) item.filePath );

// order doc items by group ( g:* )
for each ( var module in moduleList )
	for ( var filePath in module )
		module[filePath] = FlattenGroup( MakeGroups( module[filePath], function(item, prevGroup) item.attr.g || prevGroup ) );

/*
// move to the top of its module the file that contains t:header	
for each ( var module in moduleList )
	module.sort(function(a,b) {
		for each ( var item in a )
			if ( item.attr.t == 'header' )
				return -1;
		return 1;
	} );
*/


// sort files in each module
for each ( var module in moduleList )

	module.sort(function(a,b) {
		for each ( var item in a )
			if ( item.fileName == 'static.cpp' )
				return -1;
		return 1;
	} );




// move to the top of its module the docitem that contains t:header	
for each ( var module in moduleList )
	for each ( var file in module )
		for ( var i in file ) {
			if ( file[i].attr.t == 'header' ) {

				module.unshift([file[i]]);
				file.splice(i,1);
				break;
			}
			if ( file[i].attr.t == 'footer' ) {

				module.push([file[i]]);
				file.splice(i,1);
				break;
			}
		}


// write the doc
for each ( var module in moduleList ) {

	var moduleName = module[0][0].lastDir;
	var f = new File( moduleName+'_.wiki' );
	f.Open('w');
	for each ( var file in module )
		for each ( var item in file )
			f.Write( item.text ); 
	f.Close();
}


Print('Done.');
