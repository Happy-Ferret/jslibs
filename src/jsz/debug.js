
// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();
//LoadModule('jsstd'); Exec('../common/tools.js'); RunQATests('-rep 1 -exclude jstask jsz');


LoadModule('jsstd');
LoadModule('jsz');


var f = new ZipFile('test.zip');
f.Open(ZipFile.CREATE);
f.Select('toto/xxx.txt');
f.date = new Date(2008,6,4);
f.globalComment = 'a comment';
Print( 'global comment:', f.globalComment, '\n' );
Print( 'date:', f.date, '\n' );
f.extra = 'extra field';
f.Write('content data');
f.Close();

Print( '---\n' );

var g = new ZipFile('test.zip');
g.Open(ZipFile.READ);
g.Select('toto/xxx.txt');
Print( 'global comment:', g.globalComment, '\n' );
Print( g.filename, ' / ', g.date, ' / ', '\n' );
Print( g.Read(30), '\n' );
Print( g.Read(30), '\n' );
Print( g.Read(30), '\n' );
Print( g.Read(30), '\n' );
Print( g.Read(), '\n' );
Print( 'current extra:', g.extra, '\n' );


//Print( Stringify(g) );
g.Close();


throw 0;


var g = new ZipFile('test.zip');
g.Open(ZipFile.READ);
g.password = 'aze';

g.Select('toto/UpgradeLog.XML');
Print( g.filename, ' / ', g.date, ' / ', g.comment, ' / ', g.Read(), '\n' );

g.Select('toto/xxx.txt');
Print( g.filename, ' / ', g.date, ' / ', g.comment, ' / ', g.Read(), '\n' );

g.Close();


throw 0;


var f = new ZipFile('test.zip');
f.Open(ZipFile.CREATE, 0);
f.Select('toto/xxx.txt');
f.date = new Date(2008,6,4);
f.Write('content');
f.Write(' ');
f.Write('data');
f.comment = 'toto';
f.Close();


var g = new ZipFile('test.zip');
g.Open(ZipFile.READ);
for ( g.GoFirst(); !g.eol; g.GoNext() ) {

	Print( g.filename, ' / ', g.date, ' / ', g.comment, ' / ', g.Read(), '\n' );
}
g.Close();


throw 0;

var g = new ZipFile('codemirror-0.65.zip');
g.Open(ZipFile.READ);

for ( g.GoFirst(); !g.eol; g.GoNext() ) {

	Print( g.filename, '\n' );
}

g.Close();


throw 0;



var f = new ZipFile('test.zip');
f.Open(ZipFile.CREATE);
f.Select('toto/xxx.txt');
f.Write('content data');
f.Close();

var g = new ZipFile('test.zip');
g.Open(ZipFile.READ);
g.Select('toto/xxx.txt');
var data = g.Read();
g.Close();

Print( String(data).quote(), '\n' );

throw 0;



Halt(); //////////////////////////////////////////////////////////////////////////

		var data = 'clear data';
		var deflatedData = new Z(Z.DEFLATE)(data, true);
		var deflater = new Z(Z.DEFLATE);

		Print( deflater(data,true) ==  deflatedData, '\n' );
		Print( deflater(data,true) ==  deflatedData, '\n' );
		Print( deflater(data,true) ==  deflatedData, '\n' );




Halt(); //////////////////////////////////////////////////////////////////////////

var deflate = new Z(Z.DEFLATE);
var inflate = new Z(Z.INFLATE);
var source = StringRepeat('x', 100000);
var str = deflate(source);
str += deflate();

var result = inflate(str, true);

Print( result == source, '\n' );


Halt(); //////////////////////////////////////////////////////////////////////////


function randomString(size) {

	var res = '';
	while(size--)	res+=String.fromCharCode(Math.random()*256)
	return res;
}

var deflate = new Z(Z.DEFLATE);
var inflate = new Z(Z.INFLATE);
var source = randomString(10000);
var str = deflate(source, true);	
var result = inflate(str, true);


Halt(); //////////////////////////////////////////////////////////////////////////




function randomString(size) {

	var res = '';
	while(size--)	res+=String.fromCharCode(Math.random()*256)
	return res;
}



var deflate = new Z(Z.DEFLATE, 9);
var inflate = new Z(Z.INFLATE);

var str2 = '';
for ( var i=10; i>0; --i ) {

	str = deflate( randomString( i*1000 +1 ) );
	str2 += inflate(str);
}

str2 += inflate( deflate() );
inflate();

Print( 'adler32:' + (deflate.adler32 == inflate.adler32) ,'\n');




Halt(); //////////////////////////////////////////////////////////////////////////


function test1() {


		var deflate = new Z(Z.DEFLATE,9);
		
		var str = '';
		var res = '';
		for ( var i=10; i>=0; --i ) {
			var chunk = randomString(10000);
			res += deflate( chunk );
			str += chunk;
		}
		res += deflate();

		var inflate = new Z(Z.INFLATE);
		var res2 = inflate( res );
		res2 += inflate();
		
		Print( 'ratio:'+ res.length + ': ' + Math.round( 100 * res.length / str.length ) + '%','\n');
		if ( res2 != str ) {
			Print('error\n');
		}
	
}


function test3() {

		var deflate = new Z(Z.DEFLATE);
		var inflate = new Z(Z.INFLATE);
		var str = deflate('x');
		Print( inflate(str) );
}



function test4() {

		var data = 'clear data';
		var deflatedData = new Z(Z.DEFLATE)(data, true);
		var inflater = new Z(Z.INFLATE);
		inflater(deflatedData,true);

}




//try {

/*
var compressor = new Z(Z.DEFLATE);
var res = compressor.Transform( 'hello ' );
res += compressor.Transform( 'world.' );
res += compressor.Transform();


var compressor2 = new Z(Z.INFLATE);
var res2 = compressor2.Transform( res );
res2 += compressor2.Transform();

Print( '['+res2+']' );
*/

test2();
//test4();

Print('done')

//} catch (ex if ex instanceof ZError) {
//	print( ex.const + ' ' + ex.text, '\n' );
//}

