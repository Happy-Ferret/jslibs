LoadModule('jsstd');
LoadModule('jsz');


function randomString(size) {

//return 'L\'autre jour, au café, je commande un demi. J\'en bois la moitié. Il ne m\'en restait plus. L\'héroïsme, c\'est encore la meilleure façon de devenir célèbre quand on n\'a pas de talent.';

	var res = '';
	while(size--)	res+=String.fromCharCode(Math.random()*256)
	return res;
}

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



function test2() {

		var deflate = new Z(Z.DEFLATE,9);
		var inflate = new Z(Z.INFLATE);
		
		var str = '';
		var str2 = '';
		for ( var i=10; i>=0; --i ) {
		
		
			str = deflate( randomString( i*1000 +1 ) );
			str2 += inflate(str);
		}
		
		str2 += inflate( deflate() );
		inflate();

		Print( 'adler32:' + (deflate.adler32 == inflate.adler32) ,'\n');
}


function test3() {

		var deflate = new Z(Z.DEFLATE);
		var inflate = new Z(Z.INFLATE);
		var str = deflate('x');
		Print( inflate(str) );
}



function test4() {

	var deflate = new Z(Z.DEFLATE);

	for ( var i = 0; i < 10; i ++ ) {
	
		var str = deflate('abc', true);
		Print( str.length, '\n' );
	}

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

//test2();
//test4();
Print( typeof Z.BEST_SPEED )
Print('done')

//} catch (ex if ex instanceof ZError) {
//	print( ex.const + ' ' + ex.text, '\n' );
//}

