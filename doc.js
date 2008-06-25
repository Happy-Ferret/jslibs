LoadModule('jsstd');
LoadModule('jsio');


var api = {
	$H: function(cx, item) {

		cx.center = '===== '+ReadArg(cx)+': =====';
	},

	$FNAME: function(cx, item) {

		var res = /DEFINE_(\w*)\( *(\w*) *\)/(item.source.substring(item.followingSourceTextStart, item.followingSourceTextEnd));
		if ( res ) {
			cx.center = '*'+res[2]+'*';
		} else
			cx.center = '???';
	},
	
	$INCLUDE: function(cx, item) {
	
		cx.center = new File(ReadArg(cx)).content;
	},

	$CLASS_HEADER: function(cx, item) {
		
		var className;

		var res = /BEGIN_CLASS\( *(\w*) *\)/(item.source.substring(item.followingSourceTextStart, item.followingSourceTextEnd));
		if ( res ) {
			className = res[1];
		} else
			className = '???';
		
		var inheritFrom = ReadArg(cx);
		
		cx.center = '== '+item.lastDir+'::'+className+' class '+(inheritFrom ? '^'+item.lastDir+'::'+inheritFrom+'^ ==';
	},
	
	$MHEADER: function(cx, item) {

		cx.center = '#summary '+item.lastDir+' module\n' + '#labels doc\n' + '- [http://jslibs.googlecode.com/svn/trunk/'+item.lastDir+'/ source] - [JSLibs main] -\n'+'= '+item.lastDir+' module =';
	},

	$MFOOTER: function(cx, item) {

		cx.center = '----\n- [http://jslibs.googlecode.com/svn/trunk/'+item.lastDir+'/ source] - [#'+item.lastDir+'_module top] - [JSLibs main] -';
	},

	$READONLY:'http://jslibs.googlecode.com/svn/wiki/readonly.png',
	$VAL:',,value,,',
	$INT:',,integer,,',
	$REAL:',,real,,',
	$STR:',,string,,',
	$OBJ:',,object,,',
	$BOOL:',,boolean,,',
	$VOID:'',
	$LF:'= =',
};



function ReadCx(cx, re) {

    re.lastIndex = 0;
    res = re(cx.right);
    if (res) {
        cx.right = cx.right.substr(re.lastIndex);
        return res;
    } else {
        return [];
    }
}

function ReadArg(cx) ReadCx(cx, /([^\s]+)/g)[1]||'';


function RegQuote(str) {

    var quote = '$^*+?.()[]{}|\\';
    var res = '';
    for each ( c in str )
        res += quote.indexOf(c) != -1 ? ('\\' + c) : c;
    return res;
}

function ExpandText(str, api, item) {

//    var re = /\$\w*/g;
    var re = new RegExp([RegQuote(p) for ( p in api ) ].join('|'),'g');
    var cx= {left: "", center: "", right: str};
    for (var i = 0; i < 10; i++) {

        re.lastIndex = 0;
        var res = re(cx.right);
        if (!res) {
            cx.left += cx.right;
            break;
        }
        cx.center = res[0];
        cx.left += cx.right.substr(0, re.lastIndex - cx.center.length);
        cx.right = cx.right.substr(re.lastIndex);
        
        if ( cx.center in api ) {
        
				var apiItem = api[cx.center];
				if ( apiItem instanceof Function )
					apiItem(cx, item);
				else
					cx.center = apiItem;
			}
        
        cx.left += cx.center;
    }
    return cx.left;
}



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

function CreateDocItemList(api) {

	var docExpr = /\/\*\*doc([^]*?)(?:\r?\n([^]*?))?\*\*\//g;
	var hidden = /\/\./;
	var sourceCode = /\.(h|cpp|c)$/;

	var itemList = [];
	var index = 0;

	RecursiveDir( './src', function(file) {

		if ( !hidden(file.name) && sourceCode(file.name) ) {

			var source = file.content;
			docExpr.lastIndex = 0;
			var res, item;
			while( res = docExpr(source) ) {

				if ( item ) // adjust the previous followingTextEnd
					item.followingSourceTextEnd = docExpr.lastIndex - res[0].length;
				
				item = Map();
				itemList.push(item);
				
				item.index = index++;
				item.filePath = file.name;
				item.path = file.name.substr(0, file.name.lastIndexOf('/'));
				item.lastDir = item.path.substr(item.path.lastIndexOf('/')+1);
				item.fileName = file.name.substr(file.name.lastIndexOf('/')+1);
				item.text = res[2]||'';
				item.source = source;
				item.followingSourceTextStart = docExpr.lastIndex;
				item.followingSourceTextEnd = source.length;
				
				var attr = Map();
				item.attr = attr;
				
				if ( res[1] )
					for each ( var a in res[1].split(' ') ) {

						var s = a.split(':');
						attr[s[0]] = s[1];
					}
				delete attr[''];
				
				item.text = ExpandText(item.text, api, item);
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




var itemList = CreateDocItemList(api);

// group docitems by module then by file.
var moduleList = MakeGroups( itemList, function(item) item.lastDir );
for ( var module in moduleList )
	moduleList[module] = MakeGroups( moduleList[module], function(item) item.filePath );

// in each file, group docitems based on their 'group' attribute ( any following docitems without a group name is moved in the group too ).
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

// sort files in each module according to the value of 'fileIndex' attribute. Only the first docitem of each file is tested.
var fileIndexValues = { top:-Infinity, bottom:+Infinity }; // 0 is for undefined items
function NormalizeFileIndexValue(value) isNaN(value) ? fileIndexValues[value]||0 : value;
for each ( var module in moduleList )
	module.sort(function(a,b) NormalizeFileIndexValue(a[0].attr.fileIndex) - NormalizeFileIndexValue(b[0].attr.fileIndex) );

// move to the top of its module the docitem that contains t:header ( and footer to the bottom )
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
