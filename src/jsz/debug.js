var loadModule = host.loadModule;
// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();
 loadModule('jsstd'); exec('../common/tools.js'); runQATests('jsz'); throw 0;



loadModule('jsstd');
loadModule('jsz');


	var deflate = new Z(Z.DEFLATE);
	var inflate = new Z(Z.INFLATE);

	var source = '123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789';

	var str = deflate.process(source);

	var result = inflate.process(str, true);
	
	print( result == source , '\n' ); // see toString

	print( deflate.process().length )
	


//	QA.ASSERT_STR( result, source, 'inflate result' );


throw 0;



loadModule('jsstd');
loadModule('jsio');
loadModule('jsz');


	var deflate = new Z(Z.DEFLATE);
	var inflate = new Z(Z.INFLATE);

	var data = inflate.process(new File('testdata.txt').content, true);


	print(data.quote());

 throw 0;




var g = new ZipFile('test.zip');
g.open(ZipFile.READ);
g.password = 'aze';

g.select('toto/UpgradeLog.XML');
print( g.filename, ' / ', g.date, ' / ', g.comment, ' / ', g.read(), '\n' );

g.select('toto/xxx.txt');
print( g.filename, ' / ', g.date, ' / ', g.comment, ' / ', g.read(), '\n' );

g.close();


throw 0;


var f = new ZipFile('test.zip');
f.open(ZipFile.CREATE, 0);
f.select('toto/xxx.txt');
f.date = new Date(2008,6,4);
f.write('content');
f.write(' ');
f.write('data');
f.comment = 'toto';
f.close();


var g = new ZipFile('test.zip');
g.open(ZipFile.READ);
for ( g.goFirst(); !g.eol; g.goNext() ) {

	print( g.filename, ' / ', g.date, ' / ', g.comment, ' / ', g.read(), '\n' );
}
g.close();


throw 0;

var g = new ZipFile('codemirror-0.65.zip');
g.open(ZipFile.READ);

for ( g.goFirst(); !g.eol; g.goNext() ) {

	print( g.filename, '\n' );
}

g.close();


throw 0;



var f = new ZipFile('test.zip');
f.open(ZipFile.CREATE);
f.select('toto/xxx.txt');
f.write('content data');
f.close();

var g = new ZipFile('test.zip');
g.open(ZipFile.READ);
g.select('toto/xxx.txt');
var data = g.read();
g.close();

print( String(data).quote(), '\n' );

throw 0;



halt(); //////////////////////////////////////////////////////////////////////////

		var data = 'clear data';
		var deflatedData = new Z(Z.DEFLATE)(data, true);
		var deflater = new Z(Z.DEFLATE);

		print( deflater(data,true) ==  deflatedData, '\n' );
		print( deflater(data,true) ==  deflatedData, '\n' );
		print( deflater(data,true) ==  deflatedData, '\n' );




halt(); //////////////////////////////////////////////////////////////////////////

var deflate = new Z(Z.DEFLATE);
var inflate = new Z(Z.INFLATE);
var source = stringRepeat('x', 100000);
var str = deflate(source);
str += deflate();

var result = inflate(str, true);

print( result == source, '\n' );


halt(); //////////////////////////////////////////////////////////////////////////


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


halt(); //////////////////////////////////////////////////////////////////////////



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




halt(); //////////////////////////////////////////////////////////////////////////


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
var res = compressor.transform( 'hello ' );
res += compressor.transform( 'world.' );
res += compressor.transform();


var compressor2 = new Z(Z.INFLATE);
var res2 = compressor2.transform( res );
res2 += compressor2.transform();

print( '['+res2+']' );
*/

test2();
//test4();

print('done')

//} catch (ex if ex instanceof ZError) {
//	print( ex.const + ' ' + ex.text, '\n' );
//}

