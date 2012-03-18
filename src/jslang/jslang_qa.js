loadModule('jsstd');

/// Handle prototype [ftrm]

	_jsapiTests();


/// Handle prototype [ftrm]

	QA.ASSERT( Handle._serialize, undefined, '_serialize access' );


/// NativeInterface [ftrm]

		var stream = new Stream('456');
		QA.ASSERT( !isNaN(stream._NI_StreamRead), true, 'NativeInterface system' )


/// NativeInterface security [ftrm]

		var stream = new Stream('456');
		var prev = stream._NI_StreamRead;
		stream._NI_StreamRead = 123456;
		QA.ASSERT( stream._NI_StreamRead, prev, 'NativeInterface security' )
		stream._NI_StreamRead = 987654;
		QA.ASSERT( stream._NI_StreamRead, prev, 'NativeInterface security' )


/// Stream [ftrm]

		var blob = stringify("1234567", true);
		var stream = Stream(blob);
		QA.ASSERT( stream.source, blob, 'source object' )

		QA.ASSERT( stream.position, 0, 'initial stream position' )
		QA.ASSERT( stringify(stream.read(3)), '123', 'stream read()' )
		QA.ASSERT( stream.position, 3, 'stream position after read()' )
		QA.ASSERT( stringify(stream.read(0)), '', 'read 0 bytes on the stream' )
		QA.ASSERT( stream.position, 3, 'stream position after read(0)' )
		QA.ASSERT( stringify(stream.read(100)), '4567', 'read more than the stream size' )
		QA.ASSERT( stream.position, 7, 'stream position after reading more than the stream size' )

		stream.position = 0;
		QA.ASSERT( stringify(stream.read(blob.byteLength)), '1234567', 'read the exact length' )


/// Stream add [ftrm d]

		var blob = stringify("1234", true);
		var stream = Stream(blob);
		blob = stringify("1234ABC", true);
		QA.ASSERT( stringify(stream.read(3)), '123', 'stream read()' )
		blob = stringify("1234ABCDEF", true);
		QA.ASSERT( blob.byteLength, 10, 'stream source length' )
		QA.ASSERT( stringify(stream.read(3)), '4', 'stream read()' )
		QA.ASSERT( stream.position, 4, 'stream position' )

		var s1 = Stream(Int8Array(blob));
		QA.ASSERT( s1.position, 0, 'stream position' )
		
		QA.ASSERT( s1.available, 10, 'stream source length' )

		QA.ASSERT( stringify(s1.read(3)), '123', 'stream read()' )


/// non-native Stream [ftrm]

		var buf = new Buffer();
		buf.write('abcdefghi');

		function MyStream() {
		
			this.read = function(amount) {
				
				return buf.read(2);
			}
		}
		
		QA.ASSERT( stringify( new MyStream() ), 'abcdefghi', 'force string conversion' );



/// another non-native Stream [ftrm]

		var buf = new Buffer();
		buf.write('abcdefghijklmnopqrstuvwxyz');
		var res = stringify({ read:function(n) { return buf.read(3); } });
		QA.ASSERT_STR( res, 'abcdefghijklmnopqrstuvwxyz', 'stringified stream' );


/// Stringify function [ftr]
	
		QA.ASSERT( 'test', stringify('test'), 'force string conversion' );
		
		var len = 0;
		var b = new Buffer();
		for ( var i = 0; i < 500; i++ ) {
			
			len += i;
			b.write(QA.randomString(i));
		}

		QA.ASSERT( b.length, len, 'buffer length' );
		
		var s = stringify(b);

		QA.ASSERT( s.length, len, 'string length' );


/// blob serialization [ftrm d]

		var b = Blob("my blob");
		b.aPropertyOfMyBlob = 12345;
		var xdrData = xdrEncode(b);
		var val = xdrDecode(xdrData);
		QA.ASSERT( val.length, 7, 'blob length' );
		QA.ASSERT( val.aPropertyOfMyBlob, 12345, 'blob property' );
		QA.ASSERT_STR( val, "my blob", 'blob content' );



/// map serialization [ftrm d]

		var obj = {a:1, b:2, c:3};
		var m = Map(obj);
		var xdrData = xdrEncode(m);
		var val = xdrDecode(xdrData);
		QA.ASSERT_STR( uneval(obj), uneval(val), 'map content' );


/// Stringify StreamRead [ftmr]

	var i = 10;
	var res = stringify({ read: function(count) i-- ? 'Str' : '' } );
	QA.ASSERT_STR( res, 'StrStrStrStrStrStrStrStrStrStr', 'Stringify streamRead object' );


/// Stringify BufferGet [ftmr]

	QA.ASSERT_STR( stringify({ get: function() { return 'ABCD' } }) , 'ABCD', 'Stringify bufferGet object' );


/// Stringify TypedArray [ftmr]

	QA.ASSERT_STR( stringify(Int8Array([100,101,102])), 'def', 'Int8Array to string' );


/// Stringify Array [ftmr]

	QA.ASSERT_STR( stringify([100,101,102]), '100,101,102', 'JS Array to string' );


/// Handle constructor [ft]

	Handle != '';
	

/// Handle object [ft]

	var h = timeoutEvents(100);

	QA.ASSERT(Handle instanceof Object, true, 'instance test');
	QA.ASSERT_TYPE(h, Handle, 'handle object type');
//	QA.ASSERT_STR(h, '[Handle ????]', 'handle type string');
//	QA.ASSERT(h.constructor, Math.constructor, 'constructor test');
//	QA.ASSERT(h.prototype, Math.prototype, 'prototype test');


/// Test InheritFrom() [ft d]

	function A() {}
	function B() {}
	A.prototype = new B;
	var a = new A;
	var b = new B;
	
	QA.ASSERT(inheritFrom(a, A), false, 'InheritFrom is not instanceof a,B');
	QA.ASSERT(inheritFrom(a, B), true, 'InheritFrom test a,B');
	QA.ASSERT(inheritFrom(b, B), false, 'InheritFrom is not instanceof b,A');
	QA.ASSERT(inheritFrom(b, A), false, 'InheritFrom test b,A');
	QA.ASSERT(inheritFrom(a, b), false, 'InheritFrom test a,b');
	QA.ASSERT(inheritFrom(b, a), false, 'InheritFrom test b,a');
	QA.ASSERT(inheritFrom(A, B), false, 'InheritFrom test A,B');
	QA.ASSERT(inheritFrom(B, A), false, 'InheritFrom test B,A');


/// processEvents() [t]

	var timeout = timeoutEvents(123);
	var t = timeCounter();
	processEvents(timeout);
	t = timeCounter() - t;
	QA.ASSERT( t > 122 && t < 130, true, 'TimeoutEvents time ('+t+')' );
//	QA.ASSERT_EXCEPTION( function() processEvents(timeout), Error, 'processEvents reused' );


/// Serialization / Unserialization crash 1

	var s = new Serializer();
	s.write(1);
	s.write(2);
	var u = new Unserializer( s.done() );
	QA.ASSERT( uneval(u.read())+uneval(u.read()), '12', 'unserialized data' );



/// Serialization / Unserialization

	function JsClass() {
	
		this.a = 5;
		this._serialize = function(ser) {
		
			ser.write(this.a);
		}
		this._unserialize = function(unser) {
			
			var o = new JsClass();
			o.a = unser.read();
			return o;
		}
	}

	var ob = new JsClass();
	ob.a = 7;
	
	function genReferenceError() {
		
		try {
			dgsdfgfvf6z5ef4sdfg();
		} catch(ex) {
			return ex;
		}
		return undefined;
	}
	
	var myobj = [
		genReferenceError(),
		new Error('error test'), 
		ob, 
		function() [,1,{__proto__:null}],
		'',
		'string', 
		{__proto__:null, a:2}, 
		[], 
		[,,,,,], 
		[,undefined,'arrayelt'], 
		true, 
		false,
		(void 0), 
		null, 
		0, 
		0.0, 
		-0.0, 
		1,234, 
		NaN, 
		-Infinity, 
		+Infinity, 
		{a:1, b:2, c:{d:3}}, 
		{},
		,
		new Date(),
		new Number(123),
		new String(123),
		<A>B</A>,
		-2147483647-1,
		0xffffffff,
		'a', 
		String(),
		new Error(),
		new SyntaxError(),
		new URIError(),
		function(x,y) { return x+y+1; }
	];
	
	
	var s = new Serializer();
	s.write(myobj);
	var s = new Unserializer(s.done());

	var str = uneval(myobj);
	var str1 = uneval(s.read());
	QA.ASSERT_STR( str, str1, 'several serialized / unserialized objects' );


/// Unserialization crash

	var s = new Serializer();
	s.write(stringify('1', true));
	var s = new Unserializer(s.done());
	s.read();


/// Stringify TypedArray

	QA.ASSERT_STR( stringify(new Float32Array()), '', 'toString empty float array' );
	QA.ASSERT_STR( stringify(new Int8Array()), '', 'toString empty Int8Array' );
	QA.ASSERT_STR( stringify(new Int8Array([100,100,100])), 'ddd', 'toString Int8Array "ddd"' );
	QA.ASSERT_STR( stringify(new Uint16Array([100,100,100])), 'ddd', 'toString Uint16Array "ddd"' );
