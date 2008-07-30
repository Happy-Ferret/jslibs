LoadModule('jsstd');
LoadModule('jsio');


var api_DEF = {};

var api = {

	'\t': function(cx, item) {
		
		if ( !isNaN(item.attr.tab) )
			cx.center = StringRepeat(' ', item.attr.tab);
		else {
		
			var offset = cx.left.length;
			Print('AMBIGUOUS TAB at ' + item.filePath + ':' + (CountStr('\n', item.source.substring(0, item.followingSourceTextStart - item.text.length + offset))+1), '\n');
		}
	},

	TBD: function(cx, item) {

		var offset = cx.left.length;
		Print('TBD: ' + item.filePath + ':' + (CountStr('\n', item.source.substring(0, item.followingSourceTextStart - item.text.length + offset))+1), '\n');
	},

	$H: function(cx, item) {

		cx.center = '===== '+ReadEol(cx)+': =====';
	},

	$INAME: function(cx, item) {

		var res = /DEFINE_(\w*)\( *(\w*) *\)/(item.source.substring(item.followingSourceTextStart, item.followingSourceTextEnd));
		if ( res ) {
		
			if ( res[2] == 'Call' ) {
			
				cx.center = '*_call operator_*';
			} else
			if ( res[1] == 'GET_PROPERTY' ) {
		
				cx.center = '*_[N] operator_*';
			} else
			if ( res[1] == 'CONSTRUCTOR' ) {
			
				cx.center = '*_constructor_*';
			} else {
				
				 var identifierName = res[2];
				 
				 if ( res[1].indexOf('PROPERTY') != -1 && ( identifierName.indexOf('Setter') != -1 || identifierName.indexOf('Getter') != -1 ) ) {
				 	
				 	identifierName = identifierName.substring(0, identifierName.length - 6);
				 }

				 if ( identifierName[identifierName.length-1] == '_' )
					  identifierName = identifierName.substring(0, identifierName.length - 1);
				
				cx.center = '*'+identifierName+'*';
			}
		} else
			cx.center = '???';
	},
	
	$INCLUDE: function(cx, item) {
	
		cx.center = new File(item.path+'/'+ReadArg(cx)).content;
	},

	$CLASS_HEADER: function(cx, item) {
		
		var className;

		var res = /BEGIN_CLASS\( *(\w*) *\)/(item.source.substring(item.followingSourceTextStart, item.followingSourceTextEnd));
		if ( res ) {
			className = res[1];
		} else
			className = '???';
		
		var inheritFrom = ReadArg(cx);
		
		cx.center = '----\n== '+item.lastDir+'::'+className+' class ';
		cx.center += inheritFrom ? '^'+item.lastDir+'::'+inheritFrom+'^' : '';
		cx.center += ' ==';
		cx.center += '\n- [http://jslibs.googlecode.com/svn/trunk/'+item.path+'/'+item.fileName+' src] - [#'+item.lastDir+'_module top] -';
	},
	
	$MODULE_HEADER: function(cx, item) {

		cx.center = '#summary '+item.lastDir+' module\n' + '#labels doc\n' + '- [http://jslibs.googlecode.com/svn/trunk/'+item.path+'/ source] - [JSLibs main] -\n'+'= '+item.lastDir+' module =';
	},

	$MODULE_FOOTER: function(cx, item) {

		cx.center = '----\n- [#'+item.lastDir+'_module top] - [JSLibs main] - [http://jslibs.googlecode.com/svn/trunk/'+item.path+'/ source] - [http://jslibs.googlecode.com/svn/trunk/'+item.path+'/qa.js QA] -';
	},

	$READONLY:'http://jslibs.googlecode.com/svn/wiki/readonly.png',

	$WRITEONLY:',,write-only !,,',
	
	$DEPRECATED: '{`DEPRECATED`}',

	$ARG:function(cx, item) {
	
		var type = ReadArg(cx);
		var name = ReadArg(cx);
		cx.center = '# ,,'+type+',, _'+name+'_';
	},


	$RET:function(cx, item) {
	
		cx.center = ',,'+ReadArg(cx)+',,';
	},

	$TYPE:function(cx, item) {
	
		cx.center = ',,'+ReadArg(cx)+',,';
	},
	
//	$OP:'Instances properties',
//	$CP:'Class properties',

// === Static functions ===
// === Static properties ===
// === Constants ===

// === Methods ===
// === Properties ===
// === Examples ===



	$VAL:',,value,,',
	$INT:',,integer,,',
	$REAL:',,real,,',
	$STR:',,string,,',
	$DATA:',,bstring,,',
	$OBJ:',,Object,,',
	$ARRAY:',,Array,,',
	$BOOL:',,boolean,,',
	$VOID:'',
	$THIS:',,this,,',

	$CONST:function(cx, item) {
	
		cx.center = '`'+ReadArg(cx)+'`';
	},

	$LF:'= =',

   $SET: function(cx, item) {

		var key = ReadArg(cx);
		var value = ReadEol(cx, true);
   	api_DEF[key] = value;
   	cx.center = '';
   },

   $: function(cx, item) { // beware: it MUST be the last in the list !!!
   
		var key = ReadArg(cx);
   	cx.center = api_DEF[key];
   },

};

var apiRe = new RegExp([RegQuote(p) for ( p in api ) ].join('|'), 'g');




function ParseArguments(str) {

    var args = [], reg = /"((?:\\?.)*?)"|[^ ]+/g;
    for (var res; res = reg(str); args.push(res[1] != null ? res[1] : res[0] ));
    return args;
}

function ReadCx(cx, re) {

    res = re(cx.right);
    if (res) {
        cx.right = cx.right.substr(res[0].length);
        return res;
    } else {
        return [];
    }
}

function ReadArg(cx) ReadCx(cx, / *([\w_]*)/)[1]||'';
function ReadEol(cx, eatEol) ReadCx(cx, eatEol ? / *([^\n]*)\n?/ : / *([^\n]*)/ )[1]||'';


function RegQuote(str) {

    var quote = '\\^$*+?.|()[]{}';
    var res = '';
    for each ( c in str )
        res += quote.indexOf(c) != -1 ? '\\' + c : c;
    return res;
}


function ExpandText(str, api, apiRe, item) {

    var cx = {left: '', center: '', right: str};
    for(;;) {

        apiRe.lastIndex = 0;
        var res = apiRe(cx.right);
        if (!res) {
            cx.left += cx.right;
            break;
        }
        cx.center = res[0];
        cx.left += cx.right.substr(0, apiRe.lastIndex - cx.center.length);
        cx.right = cx.right.substr(apiRe.lastIndex);
        
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


function CountStr(str, source) {

	var count = 0;
	if (str)
		for ( var pos = 0; (pos = source.indexOf(str, pos)) != -1; pos += str.length, count++ );
	return count;		
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

function CreateDocItemList(startDir, api) {

	var docExpr = /\/\*\*doc([^]*?)(?:\n([^]*?))?\*\*\//g;
	var hidden = /\/\./;
	var sourceCode = /\.(h|cpp|c)$/;

	var itemList = [];
	var index = 0;

	RecursiveDir( startDir, function(file) {

		if ( !hidden(file.name) && sourceCode(file.name) ) {

			var source = String(file.content);
			source = source.replace(/\r\n|\r/g, '\n'); // cleanup
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
//				item.docTextStart = docExpr.lastIndex - res[2].length;
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
				
				// cleanup item.text
				
			
				
				item.text = ExpandText(item.text, api, apiRe, item);
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




var itemList = CreateDocItemList('./src', api);

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
var fileIndexValues = { topmost:-Infinity, top:-1, bottom:1, bottommost:+Infinity }; // 0 is for undefined items
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
	var f = new File( moduleName+'.wiki' );
	f.Open('w');
	for each ( var file in module )
		for each ( var item in file ) {
		
			if ( !('hidden' in item.attr) )
				f.Write( item.text + '\n' ); 
		}
	f.Close();
}

Print('Done.');
