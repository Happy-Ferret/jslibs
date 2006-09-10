exec('deflib.js');
LoadModule('jsz');


function randomString(size) {

//return 'L\'autre jour, au caf�, je commande un demi. J\'en bois la moiti�. Il ne m\'en restait plus. L\'h�ro�sme, c\'est encore la meilleure fa�on de devenir c�l�bre quand on n\'a pas de talent.';

	var res = '';
	while(size--)	res+=String.fromCharCode(Math.random()*256)
	return res;
}

function test1() {


		var deflate = new Z(Z.DEFLATE,9);
		
		var str = '';
		var res = '';
		for ( var i=100; i>=0; --i ) {
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

		print( 'adler32:' + (deflate.adler32 == inflate.adler32) ,'\n');
}


function test3() {

		var deflate = new Z(Z.DEFLATE);
		var inflate = new Z(Z.INFLATE);
		var str = deflate('x');
		print( inflate(str) );
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

test1();
print('done')

//} catch (ex if ex instanceof ZError) {
//	print( ex.const + ' ' + ex.text, '\n' );
//}

