
//RunJsircbot(false); throw 0;
//loadModule('jsstd'); loadModule('jsio'); var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/exec(currentDirectory)[0] + '_qa.js');  halt();
//loadModule('jsstd'); exec('../common/tools.js'); var QA = fakeQAApi;  runLocalQAFile();

loadModule('jsstd'); exec('../common/tools.js'); runQATests('-exclude jstask'); throw 0; // -inlineOnly

//loadModule('jsstd'); loadModule('jsio'); currentDirectory += '/../../tests/jslinux'; exec('start.js'); throw 0;

loadModule('jsstd');

_jsapiTests();

loadModule('jsdebug');
//print( new Date(Handle._buildDate + new Date().getTimezoneOffset() * 60 * 1000), '\n' );




throw 0;


//SetPerfTestMode();

//loadModule('jscrypt');
//loadModule('jsio');

loadModule('jsstd');

jslangTest(); throw 0;



var err = new TypeError();
var ser = new Serializer();
ser.write(err);
var unser = new Unserializer(ser.done());
err = unser.read();
print( err, '\n' );
print( err.fileName, '\n' );
print( err.lineNumber, '\n' );
print( err.stack, '\n' );



throw 0;



halt();


print(isBoolean.length); halt();

/*
loadModule('jswinshell');
var fso = createComObject('Scripting.fileSystemObject');
var tmp = fso.GetFolder('c:\\');
print( uneval(join(tmp.SubFolders)) );
//jslangTest(res);
halt();
*/


var arr = ( new ArrayBuffer(4) );

arr[0] = 97;
arr[1] = 98;
arr[2] = 99;
arr[3] = 100;

function Gen() {

	yield arr;
	yield arr;
	yield arr;
	yield arr;
	yield arr;
}


//print( join(['a', 'b', 'c']) );
//print( join(Gen()) );

//print( join( [arr, arr, arr, ' ', arr, arr, arr] ), '\n' );


var str = join(['123'], true);

var b = new Buffer();
b.write(str);
str = join([str, '456'], true)
b.write('7');

print( b.read(), '1237', 'buffer containing a Blob' );




//jslangTest();
halt();

	var rdv = new Socket(); rdv.bind(9999, '127.0.0.1'); rdv.listen(); rdv.readable = true;
	var cl = new Socket(); cl.connect('127.0.0.1', 9999);
	processEvents( Descriptor.events([rdv]), timeoutEvents(2000) );
	var sv = rdv.accept(); rdv.close();


halt();





/*
function camelify(str) {
	
	return str[0].toLowerCase() + str.substr(1);
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

var jslibsRoot = '../../tmp2';

var classList = {__proto__:null};

var exclude = <><![CDATA[
NativeInterface
NaN
H1
U32
S32
OpenGl
R0010235
Gecko
Firefox
System32
No
Linux
Windows_NT
No
Non
License
See
Software
You
Mozilla
Version
Cannot
Ab
Bad
UpgradeLog
Program
TypeLib
Scriptlet
Bonjour
Microsoft
Scripting
SubFolders
GetFolder
CreateTextFile
Already
Image_Tectonic_plates
D90000
M50
C0
L150
C200
Masked
Example
Stack
StrChr
My
Big
Page
Failed
Bindings
F00
InMemory
IterMore
Content
Type
Authenticate
Basic
Authorization
Required
Of
Left
Bottom
The
Right
Top
StrStrStrStrStrStrStrStrStrStr
Str
ReadOnlyGlobalClasses
General
Public
ComSpec
Memory
ZzZ
Line3
That
Coordinates
Device
Normalized
Clip
Eye
Tutorial9
Tutorials
Transitional
Expanded
W3C
S3
Flow
Typical
Protocol
After
Alice
Bob
Zip
Intersection
Cone
Each
TexShadowReflectLight
This
To
Force
MesaDemos
Mesa
First
]]></>.toString();

exclude.split(/\r?\n/).forEach(function(item) { classList[item] = true; });

// jslibs classes
recursiveDir(jslibsRoot, function(f) {

	if ( /\.cpp$/.test(f.name) ) {

		var content = f.content.toString();
		( content.match(/BEGIN_CLASS\( ?\w+ ?\)/g) || [] ).forEach(function(item) {
			
			classList[ /BEGIN_CLASS\( ?(\w+) ?\)/.exec(item)[1] ] = true;
		});
	}
});


// custom js classes
recursiveDir(jslibsRoot, function(f) {

	if ( /\.js$/.test(f.name) ) {

		var content = f.content.toString();
		( content.match(/\Wnew \w+/g) || [] ).forEach(function(item) {
			
			classList[/\Wnew (\w+)/.exec(item)[1] ] = true;
		});

	}
});


var changes = '';
 
var i = 0;
recursiveDir(jslibsRoot, function(f) {
	
	if ( /\.js$/.test(f.name) ) {
		
		print( f.name, '...\n' );

		var content = f.content.toString();
		
		var newContent = content.replace(/([^\w'"])([A-Z][a-z0-9]\w*)/g, function(all, p1, token, offset, str) {
			
			if ( !classList[token] && !global[token] ) {
		
				var before = str.substring( str.lastIndexOf('\n', offset)+1, offset );				
				if ( before.indexOf('//') == -1 )
					token = camelify(token);
			}
			
			return p1 + token;
		});
		
		f.content = newContent;
	}
});

*/

