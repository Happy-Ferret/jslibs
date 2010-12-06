LoadModule('jsstd');

var obj = {
	_serialize: function(ser) {
	
		ser.Write('ser obj');
	},
	_unserialize: function(unser) {
	
		return {};
		
	}
};


var s = new Serializer();
s.Write('test');
s.Write(100);
s.Write([,undefined,1,2,3]);
//s.Write(obj);
s.Write(Blob('12345678'));

var buffer = s.GetBuffer();
//Print( buffer, '\n' );


var s = new Unserializer(buffer);

Print( uneval( s.Read() ) );
Print( uneval( s.Read() ) );
Print( uneval( s.Read() ) );

Print( uneval( s.Read() ) );

Print( '-=-' );

throw 0;

