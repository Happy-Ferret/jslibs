LoadModule('jsstd');

/// Print returns undefined [ftrm]
		QA.ASSERT( Print(), undefined, 'Print return value' );

/// ObjEx callback functions [ftrm]
		
		var aux = {};
		
		var addCallbackCalls = 0;
		var setCallbackCalls = 0;
		
		function addCallback( propertyName, propertyValue, auxObject, callbackIndex ) {

			addCallbackCalls++;

			QA.ASSERT( propertyName, 'foo', 'added key name' );
			QA.ASSERT( propertyValue, 123, 'added value' );
			
			QA.ASSERT( auxObject, aux, 'aux object' );
		}

		function setCallback( propertyName, propertyValue, auxObject, callbackIndex ) {
			
			setCallbackCalls++;

			if ( setCallbackCalls == 1 ) {
			
				QA.ASSERT( propertyName, 'foo', 'set key name' );
				QA.ASSERT( propertyValue, undefined, 'set value' );
			} else {
			
				QA.ASSERT( propertyName, 'foo', 'set key name' );
				QA.ASSERT( propertyValue, 456, 'set value' );
			}			
			QA.ASSERT( auxObject, aux, 'aux object' );
		}

		var obj = new ObjEx( addCallback, undefined, undefined, setCallback, aux );

		QA.ASSERT( ObjEx.Aux(obj), aux, 'aux object' );


		obj.foo = 123;
		obj.foo = 456;

		QA.ASSERT( addCallbackCalls, 1, 'addCallback calls count' );
		QA.ASSERT( setCallbackCalls, 2, 'setCallback calls count' );


/// ObjEx setter [ftrm]

		function MyException() {}
		function SetOnceObject() new ObjEx( undefined,undefined,undefined, function(name, value) this[name] ? (function() { throw new MyException() })() : value );
		var o = SetOnceObject();
		o.abc = 123;
		QA.ASSERT_EXCEPTION( function() {  o.abc = 456;  }, MyException, 'using setter' );


/// ObjEx data slot [ftrm]

		function newDataNode(parent) new ObjEx(undefined,undefined,newDataNode.get,undefined,{listenerList:[],parent:parent});

		newDataNode.get = function( name, value, aux ) (name in this) ? value : (this[name] = newDataNode(this));

		function addDataListener( path, listener ) {

			ObjEx.Aux( path ).listenerList.push(listener);
		}

		function setData( path, data ) {

			var aux = ObjEx.Aux(path);
			for each ( let listener in aux.listenerList )
				listener('set', data);
			aux.data = data;
			return data;
		}

		function getData( path ) {

			var aux = ObjEx.Aux(path);
			return 'data' in aux ? aux.data : undefined;
		}

		function hasData( path ) {

			return 'data' in ObjEx.Aux(path);
		}

		
		var test = newDataNode();
		setData( test.aaa.bbb.ccc.ddd.eee, 1234 );
		QA.ASSERT( getData(test.aaa.bbb.ccc.ddd.eee), 1234, 'check data in the tree' );
		QA.ASSERT( getData(test.aaa.bbb.ccc.ddd), undefined, 'check data in the tree' );


/// ObjectToId and IdToObject [rt]

		var obj = { xxx:123 };
		var id = ObjectToId(obj);

		QA.ASSERT_TYPE( id, 'number', 'id type is good' );
		QA.ASSERT( id >= 1, true, 'id is valid' );
		QA.ASSERT( IdToObject(id), obj, 'obj->id->obj' );
		
		QA.ASSERT( IdToObject(id).xxx, 123, 'IdToObject validity before GC' );
		obj = null;
		QA.GC();
		QA.ASSERT( IdToObject(id), undefined, 'IdToObject after GC' );
		
		
		for ( var i = 0; i<500; i++ )
			ObjectToId({});
		QA.GC();
		for ( var i = 0; i<1000; i++ )
			ObjectToId({});


/// Map object with new [ftrm]

		var m = new Map();

		QA.ASSERT( '__proto__' in m, false, 'has no __proto__' );
		QA.ASSERT( '__parent__' in m, false, 'has no __parent__' );
		QA.ASSERT( 'toString' in m, false, 'has no toString' );
		QA.ASSERT( 'toSource' in m, false, 'has no toSource' );
		QA.ASSERT( '__count__' in m, false, 'has no __count__' );

		m.__proto__ = 'AAA', 
		m.__parent__ = 'BBB';
		m.toString = 'CCC';
		m.toSource = 'DDD';
		m.__count__ = 'EEE';

		QA.ASSERT( m.__proto__, 'AAA', ' __proto__ custom fvalue' );
		QA.ASSERT( m.__parent__, 'BBB', ' __parent__ custom fvalue' );
		QA.ASSERT( m.toString, 'CCC', 'toString custom fvalue' );
		QA.ASSERT( m.toSource, 'DDD', 'toSource custom fvalue' );
		QA.ASSERT( m.__count__, 'EEE', '__count__ custom fvalue' );
		
		delete m.__proto__;
		delete m.__parent__;
		delete m.toString;
		delete m.toSource;
		delete m.__count__;
		
		QA.ASSERT( '__proto__' in m, false, 'has no __proto__' );
		QA.ASSERT( '__parent__' in m, false, 'has no __parent__' );
		QA.ASSERT( 'toString' in m, false, 'has no toString' );
		QA.ASSERT( 'toSource' in m, false, 'has no toSource' );
		QA.ASSERT( '__count__' in m, false, 'has no __count__' );

		m.__proto__ = { a:1 };
		QA.ASSERT( m.a, undefined, 'no __proto__ behavior' );


/// Map object without new [ftrm]

		var m = Map();

		QA.ASSERT( '__proto__' in m, false, 'has no __proto__' );
		QA.ASSERT( '__parent__' in m, false, 'has no __parent__' );
		QA.ASSERT( 'toString' in m, false, 'has no toString' );
		QA.ASSERT( 'toSource' in m, false, 'has no toSource' );
		QA.ASSERT( '__count__' in m, false, 'has no __count__' );

		m.__proto__ = 'AAA', 
		m.__parent__ = 'BBB';
		m.toString = 'CCC';
		m.toSource = 'DDD';
		m.__count__ = 'EEE';

		QA.ASSERT( m.__proto__, 'AAA', ' __proto__ custom fvalue' );
		QA.ASSERT( m.__parent__, 'BBB', ' __parent__ custom fvalue' );
		QA.ASSERT( m.toString, 'CCC', 'toString custom fvalue' );
		QA.ASSERT( m.toSource, 'DDD', 'toSource custom fvalue' );
		QA.ASSERT( m.__count__, 'EEE', '__count__ custom fvalue' );
		
		delete m.__proto__;
		delete m.__parent__;
		delete m.toString;
		delete m.toSource;
		delete m.__count__;
		
		QA.ASSERT( '__proto__' in m, false, 'has no __proto__' );
		QA.ASSERT( '__parent__' in m, false, 'has no __parent__' );
		QA.ASSERT( 'toString' in m, false, 'has no toString' );
		QA.ASSERT( 'toSource' in m, false, 'has no toSource' );
		QA.ASSERT( '__count__' in m, false, 'has no __count__' );

		m.__proto__ = { a:1 };
		QA.ASSERT( m.a, undefined, 'no __proto__ behavior' );
		

/// StringRepeat function [ftrm]

		QA.ASSERT( StringRepeat( '', 0 ), '', '0 x empty' );
		QA.ASSERT( StringRepeat( '', 1 ), '', '1 x empty' );
		QA.ASSERT( StringRepeat( '', 10 ), '', '10 x empty' );
		
		QA.ASSERT( StringRepeat( 'a', 0 ), '', '0 x 1 char' );
		QA.ASSERT( StringRepeat( 'a', 1 ), 'a', '1 x 1 char' );
		QA.ASSERT( StringRepeat( 'a', 10 ), 'aaaaaaaaaa', '10 x 1 char' );
		
		QA.ASSERT( StringRepeat( 'abc', 0 ), '', '0 x string' );
		QA.ASSERT( StringRepeat( 'abc', 1 ), 'abc', '1 x string' );
		QA.ASSERT( StringRepeat( 'abc', 10 ), 'abcabcabcabcabcabcabcabcabcabc', '10 x string' );

/// big StringRepeat[tr]

		QA.ASSERT( StringRepeat( '123', 1024*1024 ).length, 3*1024*1024, '3MB' );


/// buffer and GC []

		var errBuffer = new Buffer();
		QA.GC();
		for ( var i = 0; i < 3; i++ ) {
		
			errBuffer.Write(StringRepeat('z', 1000000));
			QA.GC();
		}
		
		var res = errBuffer.toString().indexOf('zzz') 
		QA.GC();
		
		QA.ASSERT( res, 0, 'buffer test' ); 

	

/// Buffer access [ftrm]

		var b = new Buffer();
		b.Write('');
		b.Write('123');
		b.Write('');
		b.Write('456');
		b.Write('');
		b.Write('789');
		b.Write('');
		
		var res = b[-1]+b[0]+b[1]+b[2]+b[3]+b[4]+b[5]+b[6]+b[7]+b[8]+b[9]+b[10];


		QA.ASSERT( res.length, 9, 'resulting length' );

		QA.ASSERT( res, '123456789', 'resulting string' );
		QA.ASSERT( b.toString(), res, 'resulting string' );



/// empty Buffer [ftrm]

		var b = new Buffer();
		QA.ASSERT( typeof b.Read(), 'string', 'empty buffer read' );

		QA.ASSERT( b.Read(), '', 'empty buffer read' );

		b.Write('a');
		QA.ASSERT( typeof b.Read(), 'object', 'buffer read' );
	
		b.Write('a');
		QA.ASSERT( b.Read() instanceof Blob, true, 'buffer read' );


/// Buffer test 1 [ftrm]
		
		var b = new Buffer();
		b.Read(0);
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
		b.Read(0);
		var t = b.Read();

		QA.ASSERT_STR( t, 'eeeeffffgggg', 'buffer match' );


/// Buffer test 2 [ftrm]

		var b = new Buffer();
		b.Write("abcdefghi");
		b.Read(2);
		b.Read(2);
		b.Read(2);
		b.Read(2);
		QA.ASSERT_STR( b.Read(2), 'i', 'buffer match' );


/// Buffer underflow [ftrm]
		
		var times = 0;
		var toto = 'Zz';

		var buf = new Buffer({ Read:function(count) { times++; buf.Write(toto) } });

		buf.Write('1234');
		buf.Write('5');
		buf.Write('');
		buf.Write('6789');

		QA.GC();
		var s = buf.Read(5);
		s += 'X'
		QA.GC();
		buf.Unread(s);
		s += 'Y'
		QA.GC();

		QA.ASSERT( buf.length, 10, 'length before read 30' );

		QA.ASSERT_STR( buf.Read(30), '12345X6789ZzZzZzZzZzZzZzZzZzZz', 'Read(30)' );

		QA.ASSERT( buf.length, 0, 'length after read 30' );

		QA.ASSERT( times, 10, 'times .source stream has been called' );

		buf.Write('1234');
		buf.Write('5');
		buf.Write('');
		buf.Write('6789');
		
		QA.ASSERT( buf.Read(undefined), '1234', 'undefined read' );
		QA.ASSERT_STR( buf.Read(7), '56789Zz', 'read(7)' );
		QA.ASSERT_STR( buf.Read(1), 'Z', 'read(1)' );
		QA.ASSERT_STR( buf.Read(), 'z', 'read()' );


/// Buffer copy [ftrm]

		var b1 = new Buffer();
		b1.Write('1');
		b1.Write('');
		b1.Write('1');
		b1.Write('1');

		QA.ASSERT( b1.length, 3, 'source buffer length' );

		var b2 = new Buffer();
		b2.Write('aaa');

		b2.Write(b1);
		QA.ASSERT( b1.length, 3, 'source buffer length' );

		b2.Write('bbb');
		b2.Write(b1);
		QA.ASSERT( b1.length, 3, 'source buffer length' );

		QA.ASSERT_STR( b2.toString(), 'aaa111bbb111', 'buffer content' );	
		
		QA.ASSERT_STR( b1.toString(), '111', 'source buffer content' );	


/// Buffer simple read [ftrm]
	
		var b = new Buffer();
		b.Write('123');
		QA.ASSERT_STR( b.Read(1), '1', 'buffer read' );	



/// Buffer toString [ftrm]
	
		var b = new Buffer();
		b.Write('12345');
		b.Write('678');
		b.Write('9');
		b.Write('');
		QA.ASSERT_STR( b, '123456789', 'buffer toString' );	


/// Buffer toString consumption [ftrm]

		var b = new Buffer();
		b.Write('123');
		
		var str1 = b.toString();
		var str2 = b.toString();

		QA.ASSERT( str2.length, str1.length, 'toString Buffer consumption' );
		QA.ASSERT( str1, '123', 'first toString result' );
		QA.ASSERT( str2, '123', 'second toString result' );


/// Buffer valueOf [ftrm]
	
		var b = new Buffer();
		b.Write('12345');
		b.Write('678');
		b.Write('9');
		b.Write('');
		QA.ASSERT_STR( b.valueOf(), '123456789', 'buffer valueOf' );	


/// Buffer values store [ftrm]
	
		var b = new Buffer();
		b.Write({ toString:function(){return '01'} });
		b.Write([2,3]);
		b.Write(4567);
		b.Write(8.9);
		QA.ASSERT_STR( b.Read(), '012,345678.9', 'buffer read stored values' );	


/// Buffer values store [ftrm]
	
		var b = new Buffer();
		b.Write({ toString:function(){return '01'} });
		b.Write([2,3]);
		b.Write(4567);
		b.Write(8.9);
		QA.ASSERT_STR( b.Read(3), '012', 'buffer read stored values' );	
		QA.ASSERT_STR( b.Read(3), ',34', 'buffer read stored values' );	
		QA.ASSERT_STR( b.Read(2), '56', 'buffer read stored values' );	
		QA.ASSERT_STR( b.Read(3), '78.', 'buffer read stored values' );	
		QA.ASSERT_STR( b.Read(1), '9', 'buffer read stored values' );	


/// Buffer and Blob [ftrm]

		var str = new Blob('123');
		var b = new Buffer();
		b.Write(str);
		str.concat('456');
		b.Write('7');
		QA.ASSERT_STR( b.Read(), '1237', 'buffer containing a Blob' );


/// Buffer IndexOf [ftrm]
	
		var b = new Buffer();
		b.Write('abcd');
		b.Write('e');
		b.Write('');
		b.Write('');
		b.Write('fghij');
		QA.ASSERT( b.IndexOf('def'), 3, 'buffer read' );	


/// Buffer readUntil [ftrm]

		var buf = new Buffer();
		buf.Write('xxx');

		buf.Write('aaa');
		buf.Write('bb1');
		buf.Write('14ccc');
		buf.Write('buffer2');
		QA.ASSERT_STR( buf.ReadUntil('114'), 'xxxaaabb', 'ReadUntil' );
		QA.ASSERT( typeof buf, 'object', 'buffer type' );
		QA.ASSERT_STR( buf, 'cccbuffer2', 'remaining' );


/// Buffer misc [ftrm]

		var buf = new Buffer();
		buf.Write('12345');
		buf.Write('6');
		buf.Write('');
		buf.Write('789');
		QA.ASSERT_STR( buf.Read(2), '12', 'Read(int)' );
		buf.Clear();
		QA.ASSERT( buf.length, 0, 'empty length' );
		QA.ASSERT( buf.Read(), '', 'empty content' );
		buf.Write('4');
		buf.Write('45');
		QA.ASSERT( buf.length, 3, 'content length' );


/// Buffer missing unroot [ftrm]

		var buf = new Buffer();
		buf.Write('1234');
		buf.Read(4);


/// Buffer Source [ftrm]

		var buf = new Buffer(Stream('456'));
		buf.Write('123');


		QA.ASSERT( buf.length, 3, 'length' );
		QA.ASSERT_STR( buf.Read(6), '123456', 'read' );

		var buf1 = new Buffer(Stream('456'));
		buf1.Write('123');
		
		QA.ASSERT( buf1.length, 3, 'length' );
		QA.ASSERT_STR( buf1.Read(6), '123456', 'read' );

		var buf2 = new Buffer({
			Read: function(count) { return StringRepeat('x',count) }
		});
		buf2.Write('123');
		QA.ASSERT( buf2.length, 3, 'length' );
		QA.ASSERT_STR( buf2.Read(6), '123xxx', 'read' );


		var buf3 = new Buffer({
			Read: function(count) { buf3.Write( StringRepeat('x',count) ) }
		});
		buf3.Write('123');
		QA.ASSERT( buf3.length, 3, 'length' );
		QA.ASSERT_STR( buf3.Read(6), '123xxx', 'read' );


/// Pack endian [ftrm]
	
		if ( !Pack.systemIsBigEndian ) {
		
			var buf = new Buffer();
			var pack = new Pack(buf);
			var v = 0x71727374;
			pack.WriteInt(v, 4, false);
			QA.ASSERT_STR( buf.Read(), 'tsrq', 'check stored data' );
		} else {
		
			QA.FAILED('this test is missing');
		}


/// Pack read integer [ftrm]

		var buf = new Buffer();
		buf.Write('\xAA\xBB\xCC\xDD');
		var pack = new Pack(buf);
		QA.ASSERT( pack.ReadInt(4, false, true).toString(16), 'aabbccdd', 'ReadInt' );


/// Pack read string [ftrm]

		var buf = new Buffer();
		buf.Write('\xAA\xBB\xCC\xDD');
		var pack = new Pack(buf);
		QA.ASSERT_STR( pack.ReadString(4), "\xAA\xBB\xCC\xDD", 'ReadString' );


/// Pack class [ftrm]

		var buf = new Buffer();
		var pack = new Pack(buf);

		var v = 12345678;

		pack.WriteInt(v, 4, true);
		QA.ASSERT( pack.buffer.length, 4, 'buffer length' );
		QA.ASSERT( v, pack.ReadInt(4, true), 'data validity' );

		pack.WriteInt(v, 4);
		QA.ASSERT( v, pack.ReadInt(4, false), 'data validity' );

		pack.WriteInt(v, 4, false);
		QA.ASSERT( v, pack.ReadInt(4), 'data validity' );

		v = 65432;
		pack.WriteInt(v, 2);
		QA.ASSERT( v, pack.ReadInt(2), 'data validity' );


/// Garbage collector [rd]
		
		LoadModule('jsdebug');
		
		var str = QA.RandomString(1024*1024);
		
		QA.GC();
	
		for ( var i = 0; i < 4; i++ )
			str += str;
			
//		QA.ASSERT( gcBytes, str.length, 'lot of allocated memory' );
		
		QA.GC();


/// hide properties [ftrm]
	
		var o = { a:1, b:2, c:3, d:4 };

		SetPropertyEnumerate(o, 'b', false);
		SetPropertyEnumerate(o, 'c', false);
		
		QA.ASSERT( o.b, 2, 'do not delete' );
		QA.ASSERT( [p for each (p in o)].join(','), '1,4', 'visible properties' );


/// SetScope function [ftrm]

		var data = 55;
		function bar() { QA.ASSERT( data, 7, 'modified scope' ); }
		var old = SetScope( bar, {data:7, QA:QA} );
		bar();



/// Expand function [ftrm]

		QA.ASSERT( Expand('', { h:'Hello', w:'World' }), '', 'expanding an empty string' );
		QA.ASSERT( Expand('Hello World'), 'Hello World', 'expanding a simple string' );
		QA.ASSERT( Expand(' $(h) $(w)', { h:'Hello', w:'World' }), ' Hello World', 'expanding a string' );
		QA.ASSERT( Expand(' $(h) $(w', { h:'Hello', w:'World' }), ' Hello ', 'expanding a bugous string' );
		QA.ASSERT( Expand(' $(h) $(', { h:'Hello', w:'World' }), ' Hello ', 'expanding a bugous string' );
		QA.ASSERT( Expand(' $(h) $', { h:'Hello', w:'World' }), ' Hello $', 'expanding a string' );
		QA.ASSERT( Expand(' $(h)', { h:'Hello', w:'World' }), ' Hello', 'expanding a string' );
		
		var obj = {
			toString: function() {
				return 'Hello';
			}
		}
		QA.ASSERT( Expand('$(h)', { h:obj }), 'Hello', 'expanding a string' );
		
		var obj1 = {
			
			Expand:Expand,
			x:123,
			y:'456',
		}
		QA.ASSERT( obj1.Expand('$(x)$(y)', obj1), '123456', 'expanding a string using this as map' );
		
		var o = { title:'My HTML Page', titi:1234, toString:function() { return Expand( this.text, this ) } };
		o.text = '<html><title>$(title)</title>\n'
		QA.ASSERT_STR( o, '<html><title>My HTML Page</title>\n', 'expand string using this object' );

		var aaa = 123;
		function foo() {
			var bbb = 456;
			return Expand('$(aaa) $(bbb)');
		}
		QA.ASSERT_STR( foo(), '123 456', 'expand string using scope chain' );
		

/// Expand using a callback function [ftrm]

		var ids = '';
		var i = 0;
		var res = Expand('ab$(c)$(d)e$(f)g$(h)ij', function(id) { ids+=id; return i++ } );
		QA.ASSERT_STR( res +'-'+ ids, 'ab01e2g3ij-cdfh', 'Expand result is correct' );


/// Exec error [ftrm]

		QA.ASSERT_EXCEPTION( function() Exec('e654ser65t'), Error, 'Exec unknown file' );

		
/// Exec basic test [ftrm]
		
		LoadModule('jsio');
		var f = new File('qa_exec_test.js');
		f.content = '((1234))';
		var res = Exec('qa_exec_test.js', false);
		QA.ASSERT_STR( res, eval( f.content.toString() ), 'content validity' );

		var res = Exec.call(this, 'qa_exec_test.js', false);
		QA.ASSERT_STR( res, eval( f.content.toString() ), 'content validity' );

		f.content = undefined;


/// Exec using XDR [ftrm]
		
		LoadModule('jsio');
	
		var f = new File('qa_exec_test.js');
		f.content = '(1234)';
		
		var res = Exec(f.name, true);
		QA.ASSERT( res, 1234, 'Exec return value' );

		var fxdr = new File('qa_exec_test.jsxdr');
		QA.ASSERT( fxdr.exist, true, 'XDR file exist' );
		
		f.Delete();

		QA.ASSERT( f.exist, false, 'do not have source file' );

		var res = Exec(f.name, true);
		QA.ASSERT( res, 1234, 'Exec using XDR file' );

		fxdr.Delete();
		QA.ASSERT( fxdr.exist, false, 'XDR file is deleted' );
		
		try {
			
			var res = Exec(f.name, false);
			QA.FAILED('Exec do not detect missing file');
			
		} catch(ex) {
			
			QA.ASSERT( ex.constructor, Error, 'Exec exception' );
			QA.ASSERT( ex.message.substr(0,21), 'Unable to load Script', 'error message' );
		}


/// XDR serialization [ftrm]

		var s = new Script('/./');
		QA.ASSERT_EXCEPTION( function() XdrEncode(s), TypeError );
		//var res = XdrDecode(XdrEncode(s));
		//QA.ASSERT_STR( s.toSource(), res.toSource(), 'XDR->unXDR a Script' );

		QA.ASSERT( XdrDecode(XdrEncode(Map({a:1, b:2, c:3}))).a, 1, 'XDR->unXDR a Map' );
		QA.ASSERT( XdrDecode(XdrEncode(Map({a:1, b:2, c:3}))).b, 2, 'XDR->unXDR a Map' );
		QA.ASSERT( XdrDecode(XdrEncode(Map({a:1, b:2, c:3}))).c, 3, 'XDR->unXDR a Map' );

		QA.ASSERT( XdrDecode(XdrEncode('6r54aze6r5')), '6r54aze6r5', 'XDR->unXDR a string' );
		QA.ASSERT( XdrDecode(XdrEncode(1234567)), 1234567, 'XDR->unXDR a number' );
		QA.ASSERT( XdrDecode(XdrEncode(1.234567)), 1.234567, 'XDR->unXDR a double' );


/// Clear function [ftrm]

		var o = { x:5, y:6, z:7 };
		QA.ASSERT( 'z' in o, true, 'has z property' );
		Clear(o);
		QA.ASSERT( 'z' in o, false, 'property z is cleared' );


/// Clear on an array [ftrm]
	
		var o = { x:5, y:6, z:7 };
		QA.ASSERT( 'z' in o, true, 'has z property' );
		Clear(o);
		QA.ASSERT( 'z' in o, false, 'property z is cleared' );



/// Seal function [ftrm]
		
		var o = { a:1 };
		Seal(o);
		
		try {
			
			o.a = 123;
			QA.FAILED('seal do not work');
			
		} catch(ex) {
			
			QA.ASSERT( ex.constructor, Error, 'object access exception' );
			QA.ASSERT( ex.message, 'o.a is read-only', 'error message' );
		}


/// IsStatementValid  function [ftrm]
		
		QA.ASSERT( IsStatementValid( 'for ( var i; i<10; i++ )' ), false, 'invalid statement' );
		QA.ASSERT( IsStatementValid( 'for ( var i; i<10; i++ );' ), true, 'valid statement' );
		QA.ASSERT( IsStatementValid( '{a,b,c} = { a:1, b:2, c:3 }' ), true, 'valid statement' );


/// StrChr function [ftr]

		var str1 = StringRepeat('y', 100);
		QA.ASSERT( [ c for each ( c in str1 ) if (c == 'y') ].length, 100, 'all chars are good' );
		
		var str = StringRepeat('x', 10000);
		QA.ASSERT( str.length, 10000, 'string length' );
		QA.ASSERT( str[0], 'x', 'first char' );
		QA.ASSERT( str[9999], 'x', 'last char' );



/// MultiLineStringUsingE4X [ftrm]
				
		var t = <text>
		this is
		a multiline

		text
		</text>
		
		QA.ASSERT( typeof t, 'xml', 'text type' );
		QA.ASSERT_STR( t, "\n\t\tthis is\n\t\ta multiline\n\n\t\ttext\n\t\t", 'text' );


/// Sandbox global objects [tfm]
	
		var res = SandboxEval('Math');
		QA.ASSERT( res.Math == Math, false, 'Global objects' );


/// Sandbox external access [tfm]

		LoadModule('jsio');
		var res = SandboxEval('typeof File');
		QA.ASSERT( res == typeof File, false, 'forbidden File class access' );
		var res = SandboxEval('typeof LoadModule');
		QA.ASSERT( res == typeof LoadModule, false, 'forbidden LoadModule function access' );


/// Sandbox Query [tfm]

		var res = Function("var v = 567; return SandboxEval('Query()', function(val) v)")();
		QA.ASSERT( res, 567, 'SandboxEval result using Function( Query function )' );

		var res = SandboxEval('Query(123)', function(val) val + 1 );
		QA.ASSERT( res, 124, 'SandboxEval result using Query function' );
		
		var obj = {};
		QA.ASSERT_EXCEPTION( function() {  SandboxEval('Query()', function(val) obj)  }, Error, 'Query returns a non-primitive value');


/// Disabled GC [r]

		var prev = disableGarbageCollection;
		disableGarbageCollection = true;
		QA.ASSERT( disableGarbageCollection, true, 'GC is disabled' );
		
		QA.GC();
		var mem0 = privateMemoryUsage;
		var str = StringRepeat('x', 1000000);
		str = undefined;
		QA.GC();
		var mem1 = privateMemoryUsage;
		QA.ASSERT( mem1 >= mem0 + 1000000, true, 'without GC' );
		
		disableGarbageCollection = prev;

/*	
		QA.GC();
		var mem0 = privateMemoryUsage;
		var str = StringRepeat('x', 1000000);
		str = undefined;
		QA.GC();
		var mem1 = privateMemoryUsage;
		QA.ASSERT( Math.abs(mem1/mem0) < 1.1, true, 'with GC' );
*/

/// Sandbox misc

	var s = SandboxEval('var a = Math.abs(-123); a');
	QA.ASSERT( s, 123, 'abs' );


/// Sandbox watchdog
	
//		QA.ASSERT_EXCEPTION( function() { SandboxEval('for (var i=0; i<10000000; ++i);') }, OperationLimit, 'OperationLimit detection' );

