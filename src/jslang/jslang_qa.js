loadModule('jsstd');

/// crash

	processEvents( timeoutEvents(1) );


/// Stream prototype JL_ASSERT_INSTANCE [p]

	QA.IS_UNSAFE || QA.ASSERTOP( function() Stream.prototype.available, 'ex', TypeError );
	QA.IS_UNSAFE || QA.ASSERTOP( function() Stream.prototype.read.call({}), 'ex', TypeError );


/// Handle prototype [p]

	QA.ASSERT_STR( Handle._serialize, undefined, '_serialize access' );


/// NativeInterface [p]

	var stream = new Stream('456');
	QA.ASSERT( !isNaN(stream._NI_StreamRead), true, 'NativeInterface system' )


/// NativeInterface security [p]

	var stream = new Stream('456');
	var prev = stream._NI_StreamRead;
	stream._NI_StreamRead = 123456;
	QA.ASSERT( stream._NI_StreamRead, prev, 'NativeInterface security' )
	stream._NI_StreamRead = 987654;
	QA.ASSERT( stream._NI_StreamRead, prev, 'NativeInterface security' )


/// Stream test [p]

	var s = new Stream( String('hello') );

	QA.ASSERT_STR( s.read(1), 'h', 'stream reading' );
	QA.ASSERT_STR( s.read(0), '', 'stream reading 0' );
	QA.ASSERT_STR( s.read(1), 'e', 'stream reading' );
	QA.ASSERT_STR( s.read(3), 'llo', 'stream reading' );
	QA.ASSERT_STR( s.read(0), '', 'stream reading' );
	QA.ASSERT( s.read(1), undefined, 'stream reading EOF' );


/// Stream [p]

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


/// Stream add [p]

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


/// non-native Stream [p]

		function Buffer() {
			
			_buf = [];
			this.write = function(data) {

				_buf.push(String(data));
			}
			this.read = function(amount) {

				var tmp = '';
				while ( tmp.length < amount && _buf.length > 0 )
					tmp += _buf.shift();
				var res = tmp.substr(0, amount);
				_buf.push(tmp.substr(amount));
				return res;
			}
		}

		var buf = new Buffer();
		buf.write('abcdefghi');

		function MyStream() {
		
			this.read = function(amount) {
				
				return buf.read(2);
			}
		}
		
		QA.ASSERT( stringify( new MyStream() ), 'abcdefghi', 'force string conversion' );



/// another non-native Stream [p]

		function Buffer() {
			
			_buf = [];
			this.write = function(data) {

				_buf.push(String(data));
			}
			this.read = function(amount) {

				var tmp = '';
				while ( tmp.length < amount && _buf.length > 0 )
					tmp += _buf.shift();
				var res = tmp.substr(0, amount);
				_buf.push(tmp.substr(amount));
				return res;
			}
		}

		var buf = new Buffer();
		buf.write('abcdefghijklmnopqrstuvwxyz');
		var res = stringify({ read:function(n) { return buf.read(3); } });
		QA.ASSERT_STR( res, 'abcdefghijklmnopqrstuvwxyz', 'stringified stream' );


/// Stringify function [p]

		QA.ASSERT( 'test', stringify('test'), 'force string conversion' );
		
		var b = '';
		for ( var i = 0; i < 100; i++ )
			b += 'q68erg4b6qer8b4';

		var s = stringify(new Stream(b));

		QA.ASSERT( s, b, 'strings are the same' );


/// blob serialization [pd]

		var b = Blob("my blob");
		b.aPropertyOfMyBlob = 12345;
		var xdrData = xdrEncode(b);
		var val = xdrDecode(xdrData);
		QA.ASSERT( val.length, 7, 'blob length' );
		QA.ASSERT( val.aPropertyOfMyBlob, 12345, 'blob property' );
		QA.ASSERT_STR( val, "my blob", 'blob content' );


/// map serialization [pd]

		var obj = {a:1, b:2, c:3};
		var m = Map(obj);
		var xdrData = xdrEncode(m);
		var val = xdrDecode(xdrData);
		QA.ASSERT_STR( uneval(obj), uneval(val), 'map content' );


/// Stringify StreamRead [p]

	var i = 10;
	var res = stringify({ read: function(count) i-- ? 'Str' : '' } );
	QA.ASSERT_STR( res, 'StrStrStrStrStrStrStrStrStrStr', 'Stringify streamRead object' );


/// Stringify BufferGet [p]

	QA.ASSERT_STR( stringify({ get: function() { return 'ABCD' } }) , 'ABCD', 'Stringify bufferGet object' );


/// Stringify TypedArray [p]

	QA.ASSERT_STR( stringify(Int8Array([100,101,102])), 'def', 'Int8Array to string' );


/// Stringify Array [p]

	QA.ASSERT_STR( stringify([100,101,102]), '100,101,102', 'JS Array to string' );


/// Handle constructor []

	Handle != '';
	

/// Handle object [p]

	var h = timeoutEvents(1);

	QA.ASSERT(Handle instanceof Object, true, 'instance test');
	QA.ASSERTOP(h, 'instanceof', Handle, 'handle object type');
	QA.ASSERT_STR(h, '[Handle  pev]', 'handle type string');
//	QA.ASSERT(h.constructor, Math.constructor, 'constructor test');
	QA.ASSERT(h.prototype, Math.prototype, 'prototype test');


/// Test InheritFrom() [pd]

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


/// processEvents() []

	var timeout = timeoutEvents(123);
	var t = timeCounter();
	processEvents(timeout);
	t = timeCounter() - t;
	QA.ASSERT( t > 122 && t < 130, true, 'TimeoutEvents time ('+t+')' );
//	QA.ASSERTOP( function() processEvents(timeout), 'ex', Error, 'processEvents reused' );



/// unserializer EOF

	var s = new Serializer();
	s.write(1);
	s.write(2);
	var u = new Unserializer( s.done() );

	QA.ASSERTOP( u.eof, '==', false, 'unserialized data' );
	QA.ASSERTOP( u.read(), '==', 1, 'unserialized data' );
	QA.ASSERTOP( u.eof, '==', false, 'unserialized data' );
	QA.ASSERTOP( u.read(), '==', 2, 'unserialized data' );
	QA.ASSERTOP( u.eof, '==', true, 'unserialized data' );
	QA.ASSERTOP( function() u.read(), 'ex', InternalError, 'unserialized data' );


/// Serialization / Unserialization crash 1

	var s = new Serializer();
	s.write(1);
	s.write(2);
	var u = new Unserializer( s.done() );
	QA.ASSERT( uneval(u.read())+uneval(u.read()), '12', 'unserialized data' );


/// ArrayBuffer Serialization / Unserialization

	var ab = stringify('test', true);
	ab.foo = 123;
	QA.ASSERT( ab.foo, 123, 'unserialized data' );
	var s = new Serializer();
	s.write(ab);
	var s = new Unserializer(s.done());
	
	var unser = s.read();
	QA.ASSERT( stringify(unser), 'test', 'unserialized data' );
	QA.ASSERT( unser.foo, undefined, 'unserialized data property' );


/// ArrayBuffer Serialization / Unserialization

	var o = { data:stringify('test', true), x:123, y:456 };

	var s = new Serializer();
	s.write(o);
	var buffer = s.done();
	// ...
	var u = new Unserializer(buffer);
	var o1 = u.read();

	QA.ASSERTOP( o.data, 'instanceof', ArrayBuffer, 'source object data property type' );
	QA.ASSERT_STR( o.data, 'test', 'source object data property' );

	QA.ASSERTOP( o1, 'has', 'data', 'unserialized object properties' );
	QA.ASSERTOP( o1, 'has', 'x', 'unserialized object properties' );
	QA.ASSERTOP( o1, 'has', 'y', 'unserialized object properties' );
	QA.ASSERTOP( o1.data, 'instanceof', ArrayBuffer, 'unserialized object data property type' );
	QA.ASSERT_STR( o1.data, 'test', 'unserialized object data property' );
	QA.ASSERTOP( o1.x, '===', 123, 'unserialized object properties value' );
	QA.ASSERTOP( o1.y, '===', 456, 'unserialized object properties value' );


/// function Serialization

	var s = new Serializer();
	s.write(function(){});
	s.done();


/// function Serialization / Unserialization

	var s = new Serializer();
	s.write(function(){});
	var s = new Unserializer(s.done());
	s.read();


/// Serialization / Unserialization of custom class

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

	var myobj = [ ob ];

	var s = new Serializer();
	s.write(myobj);
	var s = new Unserializer(s.done());

	var str = uneval(myobj);
	var str1 = uneval(s.read());
	QA.ASSERT_STR( str, str1, 'several serialized / unserialized objects' );


/// Serialization / Unserialization

	function genReferenceError() {
		
		try {
			dgsdfgfvf6z5ef4sdfg();
		} catch(ex) {
			return ex;
		}
		return undefined;
	}
	
	var typedArray = new Uint32Array(10);
	for ( var i = 0; i < 10 + 5; ++i )
		typedArray[i] = i*100;

	var emptyTypedArray = new Uint32Array(10);

	var myobj = [
		emptyTypedArray,
		typedArray,
		genReferenceError(),
		new Error('error test'), 
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


/// issue with jslibs serializer

	var s = new Serializer();
	s.write(new URIError()); var ln = currentLineNumber;
	var u = new Unserializer(s.done());
	QA.ASSERT( u.read().lineNumber, ln );
