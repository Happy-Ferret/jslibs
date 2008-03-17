function( QAASSERT ) ({

	BufferAccess: function() {

		var b = new Buffer();
		b.Write('aaa');
		b.Write('');
		b.Write('bbXb');
		b.Write('');
		b.Write('ccc');
		b.Read(2);
		b.ReadUntil('X');
		b.Skip(2);
		b.Match('cc');
		b.Unread('ddd');
		b.Match('ddd');
		b.Match('ZZZ');
		b.ReadUntil('ZZZ');
		b.Read(1000);
		b.Write('eeee');
		b.Write('ffff');
		b.Write('gggg');
		var t = b.Read();

		QAASSERT( t == 'eeeeffffgggg', 'Invalid Buffer result: '+t );	
	
	}



})