
// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();
//loadModule('jsstd'); exec('../common/tools.js'); RunQATests('-rep 1 -exclude jstask jsz');


loadModule('jsstd');
loadModule('jsz');


//a = new Uint8Array([100,101,102,100,101,102,100,101,102,100,101,102]);
//print( uneval(Array.concat( a )), '\n' );
//throw 0;



var f = new ZipFile('test.zip');
f.Open(ZipFile.CREATE);
f.level = 9;
for ( var i = 0; i < 10; ++i ) {

	f.Select('file'+i+'.txt');
	f.Write('content data '+i+' '+StringRepeat('z',100));
}
f.Close();
print( '---\n' );


var g = new ZipFile('test.zip');
g.Open(ZipFile.READ);

//
g.Select('file9.txt');
g.GoNext();
g.GoTo(10);
g.GoTo(100);



//g.GoTo(8); g.GoNext(); g.GoNext();

//Stringify(g);

//for ( g.GoFirst(); !g.eol; g.GoNext() )  print( ' '+g.filename, ' : ', g.Read(), ' (lvl='+g.level+' eol=', g.eol, ')\n' ); 
//print( ' '+g.filename, ' : ', g.Read(), ' (eol=', g.eol, ')\n' );



g.Close();




throw 0;

var f = new ZipFile('test.zip');
f.Open(ZipFile.CREATE);
f.Select('toto/xxx.txt');
f.date = new Date(2008,6,4);
f.globalComment = 'a comment';
print( 'global comment:', f.globalComment, '\n' );
print( 'date:', f.date, '\n' );
f.extra = 'extra field';
f.Write('content data');
f.Close();

print( '---\n' );

var g = new ZipFile('test.zip');
g.Open(ZipFile.READ);
g.Select('toto/xxx.txt');
print( 'global comment:', g.globalComment, '\n' );
print( g.filename, ' / ', g.date, ' / ', '\n' );
print( g.Read(30), '\n' );
print( g.Read(30), '\n' );
print( g.Read(30), '\n' );
print( g.Read(30), '\n' );
print( g.Read(), '\n' );
print( 'current extra:', g.extra, '\n' );


//print( Stringify(g) );
g.Close();


throw 0;


var g = new ZipFile('test.zip');
g.Open(ZipFile.READ);
g.password = 'aze';

g.Select('toto/UpgradeLog.XML');
print( g.filename, ' / ', g.date, ' / ', g.comment, ' / ', g.Read(), '\n' );

g.Select('toto/xxx.txt');
print( g.filename, ' / ', g.date, ' / ', g.comment, ' / ', g.Read(), '\n' );

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

	print( g.filename, ' / ', g.date, ' / ', g.comment, ' / ', g.Read(), '\n' );
}
g.Close();


throw 0;

var g = new ZipFile('codemirror-0.65.zip');
g.Open(ZipFile.READ);

for ( g.GoFirst(); !g.eol; g.GoNext() ) {

	print( g.filename, '\n' );
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

print( String(data).quote(), '\n' );

throw 0;



Halt(); //////////////////////////////////////////////////////////////////////////

		var data = 'clear data';
		var deflatedData = new Z(Z.DEFLATE)(data, true);
		var deflater = new Z(Z.DEFLATE);

		print( deflater(data,true) ==  deflatedData, '\n' );
		print( deflater(data,true) ==  deflatedData, '\n' );
		print( deflater(data,true) ==  deflatedData, '\n' );




Halt(); //////////////////////////////////////////////////////////////////////////

var deflate = new Z(Z.DEFLATE);
var inflate = new Z(Z.INFLATE);
var source = StringRepeat('x', 100000);
var str = deflate(source);
str += deflate();

var result = inflate(str, true);

print( result == source, '\n' );


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

print( 'adler32:' + (deflate.adler32 == inflate.adler32) ,'\n');




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
		
		print( 'ratio:'+ res.length + ': ' + Math.round( 100 * res.length / str.length ) + '%','\n');
		if ( res2 != str ) {
			print('error\n');
		}
	
}


function test3() {

		var deflate = new Z(Z.DEFLATE);
		var inflate = new Z(Z.INFLATE);
		var str = deflate('x');
		print( inflate(str) );
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

print( '['+res2+']' );
*/

test2();
//test4();

print('done')

//} catch (ex if ex instanceof ZError) {
//	print( ex.const + ' ' + ex.text, '\n' );
//}

