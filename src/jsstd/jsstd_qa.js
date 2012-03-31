loadModule('jsstd');

/// exec error

	var filename = 'jsqa_' + QA.randomString(8);
	new File(filename).content = '\n\n\n\nfunction();\n';
	
//	var errorMsg = '';
//	var tmp = host.stderr;
//	host.stderr = function(txt) {
//		
//		errorMsg += txt;
//	}
	
	QA.ASSERTOP(function() { exec(filename, false) }, 'ex', SyntaxError, 'exec handle compilation errors');

	new File(filename).content = undefined;
	
//	host.stderr = tmp;
//	QA.ASSERT( errorMsg.indexOf('function()') != -1, true, 'correct error message' );
	

/// Count object properties [rmtf]

	QA.ASSERT( countProperties({}), 0, 'test with 0 properties' );
	QA.ASSERT( countProperties({a:1}), 1, 'test with 1 properties' );
	QA.ASSERT( countProperties({a:1, b:2}), 2, 'test with 2 properties' );

	var obj = {a:1, b:2};
	delete obj.a;
	delete obj.b;
	QA.ASSERT( countProperties(obj), 0, 'test with deleted properties' );

/// SwitchCase function [ftrm]

	QA.ASSERT( switchCase( 1, [1, '1'], ['num', 'str'] ), 'num', 'SwitchCase type' );
	QA.ASSERT( switchCase( '1', [1, '1'], ['num', 'str'] ), 'str', 'SwitchCase type' );

	QA.ASSERT( switchCase( 2, [1, '1'], ['num', 'str'] ), undefined, 'SwitchCase not found' );
	QA.ASSERT( switchCase( 2, [1, '1'], ['num', 'str'], 'def' ), 'def', 'SwitchCase default value' );


/// IsBoolean function [ftrm]

		QA.ASSERT( isBoolean( false ), true, 'boolean value' );
		QA.ASSERT( isBoolean( 0 ), false, 'not a boolean value' );
		QA.ASSERT( isBoolean( new Number(123) ), false, 'not a boolean object' );
		QA.ASSERT( isBoolean( new Boolean(123) ), true, 'boolean object' );


/// IsNumber function [ftrm]

		QA.ASSERT( isNumber( 123 ), true, 'number value' );
		QA.ASSERT( isNumber( true ), false, 'not a number value' );
		QA.ASSERT( isNumber( new Boolean(123) ), false, 'number object' );
		QA.ASSERT( isNumber( new Number(123) ), true, 'not a number object' );


/// print returns undefined [ftrm]
		QA.ASSERT( print(), undefined, 'print return value' );


/// ObjEx simple access (crash test) [ftrm]

		ObjEx.aux(new ObjEx());
		
		
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

		QA.ASSERT( ObjEx.aux(obj), aux, 'aux object' );


		obj.foo = 123;
		obj.foo = 456;

		QA.ASSERT( addCallbackCalls, 1, 'addCallback calls count' );
		QA.ASSERT( setCallbackCalls, 2, 'setCallback calls count' );


/// ObjEx setter [ftrm]

		function MyException() {}
		function setOnceObject() new ObjEx( undefined,undefined,undefined, function(name, value) this[name] ? (function() { throw new MyException() })() : value );
		var o = setOnceObject();
		o.abc = 123;
		QA.ASSERTOP( function() {  o.abc = 456;  }, 'ex', MyException, 'using setter' );


/// ObjEx data slot [ftrm]

		function newDataNode(parent) new ObjEx(undefined,undefined,newDataNode.get,undefined,{listenerList:[],parent:parent});

		newDataNode.get = function( name, value, aux ) (name in this) ? value : (this[name] = newDataNode(this));

		function addDataListener( path, listener ) {

			ObjEx.aux( path ).listenerList.push(listener);
		}

		function setData( path, data ) {

			var aux = ObjEx.aux(path);
			for each ( let listener in aux.listenerList )
				listener('set', data);
			aux.data = data;
			return data;
		}

		function getData( path ) {

			var aux = ObjEx.aux(path);
			return 'data' in aux ? aux.data : undefined;
		}

		function hasData( path ) {

			return 'data' in ObjEx.aux(path);
		}

		
		var test = newDataNode();
		setData( test.aaa.bbb.ccc.ddd.eee, 1234 );
		QA.ASSERT( getData(test.aaa.bbb.ccc.ddd.eee), 1234, 'check data in the tree' );
		QA.ASSERT( getData(test.aaa.bbb.ccc.ddd), undefined, 'check data in the tree' );


/// ObjectToId and IdToObject [rtd]

		var ids = function() {

			var arr = [];
			for ( var i=0; i < 256; i++ ) {
				
				var obj = { xxx:i };
				var id = objectToId(obj);
				QA.ASSERT( idToObject(id), obj, 'obj->id->obj' );
				arr.push(id);
			}
			return arr;
		}();
		
		var count = 0;
		for each ( id in (ids) ) {

			QA.ASSERTOP( id, 'typeof', 'number', 'id type is good' );
			QA.ASSERT( id >= 1, true, 'id is valid' );
		
			count += ((idToObject(id) == undefined) ? 0 : 1);
		}
		QA.ASSERT( count, ids.length, 'IdToObject validity before GC (may fail if gcZeal > 0)' );
		
		QA.gc();

		var count = 0;
		for each ( id in (ids) ) {

			QA.ASSERTOP( id, 'typeof', 'number', 'id type is good' );
			QA.ASSERT( id >= 1, true, 'id is valid' );
		
			count += ((idToObject(id) == undefined) ? 0 : 1);
		}
		QA.ASSERT( count < ids.length, true, 'IdToObject after GC (may fail if CG is desactivated)' );
		
		for ( var i = 0; i<500; i++ )
			objectToId({});
		QA.gc();
		for ( var i = 0; i<1000; i++ )
			objectToId({});


/// StringRepeat function [ftrm]

		QA.ASSERT( stringRepeat( '', 0 ), '', '0 x empty' );
		QA.ASSERT( stringRepeat( '', 1 ), '', '1 x empty' );
		QA.ASSERT( stringRepeat( '', 10 ), '', '10 x empty' );
		
		QA.ASSERT( stringRepeat( 'a', 0 ), '', '0 x 1 char' );
		QA.ASSERT( stringRepeat( 'a', 1 ), 'a', '1 x 1 char' );
		QA.ASSERT( stringRepeat( 'a', 10 ), 'aaaaaaaaaa', '10 x 1 char' );
		
		QA.ASSERT( stringRepeat( 'abc', 0 ), '', '0 x string' );
		QA.ASSERT( stringRepeat( 'abc', 1 ), 'abc', '1 x string' );
		QA.ASSERT( stringRepeat( 'abc', 10 ), 'abcabcabcabcabcabcabcabcabcabc', '10 x string' );

		QA.ASSERT( stringRepeat( '\u1234', 2 ), '\u1234\u1234', 'UC string' );

/// big StringRepeat[tr]

		QA.ASSERT( stringRepeat( '123', 1024*1024 ).length, 3*1024*1024, '3MB' );


/// buffer and GC []

		var errBuffer = new Buffer();
		QA.gc();
		for ( var i = 0; i < 3; i++ ) {
		
			errBuffer.write(stringRepeat('z', 1000000));
			QA.gc();
		}
		
		var res = errBuffer.toString().indexOf('zzz') 
		QA.gc();
		
		QA.ASSERT( res, 0, 'buffer test' ); 

	

/// Buffer access [ftrm]

		var b = new Buffer();
		b.write('');
		b.write('123');
		b.write('');
		b.write('456');
		b.write('');
		b.write('789');
		b.write('');
		
		var res = b[-1]+b[0]+b[1]+b[2]+b[3]+b[4]+b[5]+b[6]+b[7]+b[8]+b[9]+b[10];


		QA.ASSERT( res.length, 9, 'resulting length' );

		QA.ASSERT( res, '123456789', 'resulting string' );
		QA.ASSERT( b.toString(), res, 'resulting string' );



/// empty Buffer [ftrm]

		var b = new Buffer();
		QA.ASSERT( typeof b.read(), 'string', 'empty buffer read' );

		QA.ASSERT( b.read(), '', 'empty buffer read' );

		b.write('a');
//		QA.ASSERT( typeof b.Read(), 'object', 'buffer read' );
		QA.ASSERT( typeof b.read(), 'string', 'buffer read' );
	
		b.write('a');
//		QA.ASSERT( b.Read() instanceof Blob, true, 'buffer read (instanceof)' );


/// Buffer test 1 [ftrm]
		
		var b = new Buffer();
		b.read(0);
		b.write('aaa');
		b.write('');
		b.write('bbXb');
		b.write('');
		b.write('ccc');
		b.read(2);
		b.readUntil('X');
		b.skip(2);
		b.match('cc');
		b.unread('ddd');
		b.match('ddd');
		b.match('ZZZ');
		b.readUntil('ZZZ');
		b.read(1000);
		b.write('eeee');
		b.write('ffff');
		b.write('gggg');
		b.read(0);
		var t = b.read();

		QA.ASSERT_STR( t, 'eeeeffffgggg', 'buffer match' );


/// Buffer test 2 [ftrm]

		var b = new Buffer();
		b.write("abcdefghi");
		b.read(2);
		b.read(2);
		b.read(2);
		b.read(2);
		QA.ASSERT_STR( b.read(2), 'i', 'buffer match' );


/// Buffer underflow [ftrm]
		
		var times = 0;
		var toto = 'Zz';

		var buf = new Buffer({ read:function(count) { times++; buf.write(toto) } });

		buf.write('1234');
		buf.write('5');
		buf.write('');
		buf.write('6789');

		QA.gc();
		var s = buf.read(5);
		s += 'X'
		QA.gc();
		buf.unread(s);
		s += 'Y'
		QA.gc();

		QA.ASSERT( buf.length, 10, 'length before read 30' );

		QA.ASSERT_STR( buf.read(30), '12345X6789ZzZzZzZzZzZzZzZzZzZz', 'Read(30)' );

		QA.ASSERT( buf.length, 0, 'length after read 30' );

		QA.ASSERT( times, 10, 'times .source stream has been called' );

		buf.write('1234');
		buf.write('5');
		buf.write('');
		buf.write('6789');
		
		QA.ASSERT( buf.read(undefined), '1234', 'undefined read' );
		QA.ASSERT_STR( buf.read(7), '56789Zz', 'read(7)' );
		QA.ASSERT_STR( buf.read(1), 'Z', 'read(1)' );
		QA.ASSERT_STR( buf.read(), 'z', 'read()' );


/// Buffer copy [ftrm]

		var b1 = new Buffer();
		b1.write('1');
		b1.write('');
		b1.write('1');
		b1.write('1');

		QA.ASSERT( b1.length, 3, 'source buffer length' );

		var b2 = new Buffer();
		b2.write('aaa');

		b2.write(b1);
		QA.ASSERT( b1.length, 3, 'source buffer length' );

		b2.write('bbb');
		b2.write(b1);
		QA.ASSERT( b1.length, 3, 'source buffer length' );

		QA.ASSERT_STR( b2.toString(), 'aaa111bbb111', 'buffer content' );	
		
		QA.ASSERT_STR( b1.toString(), '111', 'source buffer content' );	


/// Buffer simple read [ftrm]
	
		var b = new Buffer();
		b.write('123');
		QA.ASSERT_STR( b.read(1), '1', 'buffer read' );	



/// Buffer toString [ftrm]
	
		var b = new Buffer();
		b.write('12345');
		b.write('678');
		b.write('9');
		b.write('');
		QA.ASSERT_STR( b, '123456789', 'buffer toString' );	


/// Buffer toString consumption [ftrm]

		var b = new Buffer();
		b.write('123');
		
		var str1 = b.toString();
		var str2 = b.toString();

		QA.ASSERT( str2.length, str1.length, 'toString Buffer consumption' );
		QA.ASSERT( str1, '123', 'first toString result' );
		QA.ASSERT( str2, '123', 'second toString result' );


/// Buffer valueOf [ftrm]
	
		var b = new Buffer();
		b.write('12345');
		b.write('678');
		b.write('9');
		b.write('');
		QA.ASSERT_STR( b.valueOf(), '123456789', 'buffer valueOf' );	


/// Buffer values store [ftrm]
	
		var b = new Buffer();
		b.write({ toString:function(){return '01'} });
		b.write([2,3]);
		b.write(4567);
		b.write(8.9);
		QA.ASSERT_STR( b.read(), '012,345678.9', 'buffer read stored values' );	


/// Buffer values store [ftrm]
	
		var b = new Buffer();
		b.write({ toString:function(){return '01'} });
		b.write([2,3]);
		b.write(4567);
		b.write(8.9);
		QA.ASSERT_STR( b.read(3), '012', 'buffer read stored values' );	
		QA.ASSERT_STR( b.read(3), ',34', 'buffer read stored values' );	
		QA.ASSERT_STR( b.read(2), '56', 'buffer read stored values' );	
		QA.ASSERT_STR( b.read(3), '78.', 'buffer read stored values' );	
		QA.ASSERT_STR( b.read(1), '9', 'buffer read stored values' );	


/// Buffer and Blob [ftrm]

		// var str = new Blob('123');
		var str = join(['123'], true);

		var b = new Buffer();
		b.write(str);
		str = join([str, '456'], true)
		b.write('7');
		QA.ASSERT_STR( b.read(), '1237', 'buffer containing a Blob' );


/// Buffer IndexOf [ftrm]
	
		var b = new Buffer();
		b.write('abcd');
		b.write('e');
		b.write('');
		b.write('');
		b.write('fghij');
		QA.ASSERT( b.indexOf('def'), 3, 'buffer read' );	


/// Buffer readUntil [ftrm]

		var buf = new Buffer();
		buf.write('xxx');

		buf.write('aaa');
		buf.write('bb1');
		buf.write('14ccc');
		buf.write('buffer2');
		QA.ASSERT_STR( buf.readUntil('114'), 'xxxaaabb', 'ReadUntil' );
		QA.ASSERT( typeof buf, 'object', 'buffer type' );
		QA.ASSERT_STR( buf, 'cccbuffer2', 'remaining' );


/// Buffer misc [ftrm]

		var buf = new Buffer();
		buf.write('12345');
		buf.write('6');
		buf.write('');
		buf.write('789');
		QA.ASSERT_STR( buf.read(2), '12', 'Read(int)' );
		buf.clear();
		QA.ASSERT( buf.length, 0, 'empty length' );
		QA.ASSERT( buf.read(), '', 'empty content' );
		buf.write('4');
		buf.write('45');
		QA.ASSERT( buf.length, 3, 'content length' );


/// Buffer missing unroot [ftrm]

		var buf = new Buffer();
		buf.write('1234');
		buf.read(4);


/// Buffer Source [ftrm]

		var buf = new Buffer(Stream('456'));
		buf.write('123');

		QA.ASSERT( buf.length, 3, 'length' );
		QA.ASSERT_STR( buf.read(6), '123456', 'read' );

		var buf1 = new Buffer(Stream('456'));
		buf1.write('123');
		
		QA.ASSERT( buf1.length, 3, 'length' );
		QA.ASSERT_STR( buf1.read(6), '123456', 'read' );

		var buf2 = new Buffer({
			read: function(count) { return stringRepeat('x',count) }
		});
		buf2.write('123');
		QA.ASSERT( buf2.length, 3, 'length' );
		QA.ASSERT_STR( buf2.read(6), '123xxx', 'read' );


		var buf3 = new Buffer({
			read: function(count) { buf3.write( stringRepeat('x',count) ) }
		});
		buf3.write('123');
		QA.ASSERT( buf3.length, 3, 'length' );
		QA.ASSERT_STR( buf3.read(6), '123xxx', 'read' );


/// Pack int64 [rmtf]

	var buf = new Buffer();
	var pack = new Pack(buf);

	pack.writeInt(-9007199254740991, 8, true);
	var v = pack.readInt(8, true);
	QA.ASSERT(v, -9007199254740991, "min value");

	pack.writeInt(9007199254740991, 8);
	var v = pack.readInt(8);
	QA.ASSERT(v, 9007199254740991, "max value");

	QA.ASSERTOP( function() { pack.writeInt(9007199254740992, 8, true) }, 'ex', RangeError, "value overflow" );


/// Pack endian [ftrm]
	
		if ( !Pack.systemIsBigEndian ) {
		
			var buf = new Buffer();
			var pack = new Pack(buf);
			var v = 0x71727374;
			pack.writeInt(v, 4, false);
			QA.ASSERT_STR( buf.read(), 'tsrq', 'check stored data' );
		} else {
		
			QA.FAILED('this test is missing');
		}


/// Pack read integer [ftrm]

		var buf = new Buffer();
		buf.write('\xAA\xBB\xCC\xDD');
		var pack = new Pack(buf);
		QA.ASSERT( pack.readInt(4, false, true).toString(16), 'aabbccdd', 'ReadInt' );


/// Pack read string [ftrm]

		var buf = new Buffer();
		buf.write('\xAA\xBB\xCC\xDD');
		var pack = new Pack(buf);
		QA.ASSERT_STR( pack.readString(4), "\xAA\xBB\xCC\xDD", 'ReadString' );


/// Pack class [ftrm]

		var buf = new Buffer();
		var pack = new Pack(buf);

		var v = 12345678;

		pack.writeInt(v, 4, true);
		QA.ASSERT( pack.buffer.length, 4, 'buffer length' );
		QA.ASSERT( v, pack.readInt(4, true), 'data validity' );

		pack.writeInt(v, 4);
		QA.ASSERT( v, pack.readInt(4, false), 'data validity' );

		pack.writeInt(v, 4, false);
		QA.ASSERT( v, pack.readInt(4), 'data validity' );

		v = 65432;
		pack.writeInt(v, 2);
		QA.ASSERT( v, pack.readInt(2), 'data validity' );


/// Garbage collector [rd]
		
		loadModule('jsdebug');
		
		var str = QA.randomString(1024*1024);
		
		QA.gc();
	
		for ( var i = 0; i < 4; i++ )
			str += str;
			
//		QA.ASSERT( gcBytes, str.length, 'lot of allocated memory' );
		
		QA.gc();


/// hide properties [ftrm]
	
		var o = { a:1, b:2, c:3, d:4 };

		//setPropertyEnumerate(o, 'b', false);
		Object.defineProperty(o, 'b', {enumerable : false});  
		//setPropertyEnumerate(o, 'c', false);
		Object.defineProperty(o, 'c', {enumerable : false});  
		
		QA.ASSERT( o.b, 2, 'do not delete' );
		QA.ASSERT( [p for each (p in o)].join(','), '1,4', 'visible properties' );


/// SetScope function [ftrmd]

		var data = 55;
		function bar() { QA.ASSERT( data, 7, 'modified scope' ); }
		var old = setScope( bar, {data:7, QA:QA} );
		bar();



/// Expand function [ftrm]

		QA.ASSERT( expand('\u1234', {} ), '\u1234', 'unicode' );
		QA.ASSERT( expand('$(\u1234)', { '\u1234':'ok' } ), 'ok', 'unicode key' );
		QA.ASSERT( expand('$(foo)', { foo:'\u1234' } ), '\u1234', 'unicode value' );

		QA.ASSERT( expand('abcde', {} ), 'abcde', 'no expand' );
		QA.ASSERT( expand('ab$()cde', {} ), 'abcde', 'empty expand' );
		QA.ASSERT( expand('ab$(xx)cde', {} ), 'abcde', 'unknown expand' );
		QA.ASSERT( expand('$(foo)', {foo:'$(foo)'} ), '$(foo)', 'kind of escape for $()' );
		QA.ASSERT( expand('', { h:'Hello', w:'World' }), '', 'expanding an empty string' );
		
		QA.ASSERT( expand('Hello World'), 'Hello World', 'expanding a simple string' );
		
		QA.ASSERT( expand(' $(h) $(w)', { h:'Hello', w:'World' }), ' Hello World', 'expanding a string' );
		QA.ASSERT( expand(' $(h) $(w', { h:'Hello', w:'World' }), ' Hello ', 'expanding a bugous string' );
		QA.ASSERT( expand(' $(h) $(', { h:'Hello', w:'World' }), ' Hello ', 'expanding a bugous string' );
		QA.ASSERT( expand(' $(h) $', { h:'Hello', w:'World' }), ' Hello $', 'expanding a string' );
		QA.ASSERT( expand(' $(h)', { h:'Hello', w:'World' }), ' Hello', 'expanding a string' );

		QA.ASSERT( expand('$(c)'), '', 'expanding a string' );
		QA.ASSERT( expand('$(c)a'), 'a', 'expanding a string' );
		QA.ASSERT( expand('a$(c'), 'a', 'expanding a string' );
		QA.ASSERT( expand('a$()c'), 'ac', 'expanding a string' );
		QA.ASSERT( expand('a$(b)c'), 'ac', 'expanding a string' );
		QA.ASSERT( expand('a$(b)c', {}), 'ac', 'expanding a string' );
		QA.ASSERT( expand('a$(b)c', { b:'' }), 'ac', 'expanding a string' );
		QA.ASSERT( expand('$(b)c', { b:'' }), 'c', 'expanding a string' );
		QA.ASSERT( expand('a$(b)', { b:'' }), 'a', 'expanding a string' );
		QA.ASSERT( expand('a$()', { b:'' }), 'a', 'expanding a string' );
		QA.ASSERT( expand('$()a', { b:'' }), 'a', 'expanding a string' );
		QA.ASSERT( expand('$(b)$(b)$(b)', { b:'' }), '', 'expanding a string' );
		QA.ASSERT( expand('$(b)$(b)$(b)', { b:'x' }), 'xxx', 'expanding a string' );
		QA.ASSERT( expand('$(b)$(b$(b)', { b:'x' }), 'x', 'expanding a string' );
		QA.ASSERT( expand('$(b)$(b$(b)'), '', 'expanding a string' );

	
		QA.ASSERT( expand('a$(b()c)d', function(key) key), 'ab(c)d', 'expanding a string' );
		QA.ASSERT( expand('$(b)$(b$(b)', function(key) key), 'bb$(b', 'expanding a string' );
		QA.ASSERT( expand('$(b)$(b$(b', function(key) key), 'b', 'expanding a string' );
		QA.ASSERT( expand('$()', function(key) key), '', 'expanding a string' );
		QA.ASSERT( expand('a$(b)c$(d)e', function(key) key), 'abcde', 'expanding a string' );
		QA.ASSERT( expand('$(b)c$(d)', function(key) key), 'bcd', 'expanding a string' );
		QA.ASSERT( expand('$(a)', function(key) undefined), '', 'expanding a string' );
		QA.ASSERT( expand('$(a)', function(key) null), '', 'expanding a string' );
		QA.ASSERT( expand('', function(key) key), '', 'expanding a string' );
		QA.ASSERT( expand('', function(key) key), '', 'expanding a string' );

		QA.ASSERTOP( function() expand('$()', function(key) { throw 123 }), 'ex', 123, 'Expand function throw exception' );

		var obj = {
			toString: function() {
				return 'Hello';
			}
		}
		QA.ASSERT( expand('$(h)', { h:obj }), 'Hello', 'expanding a string' );
		
		var obj1 = {
			
			expand:expand,
			x:123,
			y:'456',
		}
		QA.ASSERT( obj1.expand('$(x)$(y)', obj1), '123456', 'expanding a string using this as map' );
		
		var o = { title:'My HTML Page', titi:1234, toString:function() { return expand( this.text, this ) } };
		o.text = '<html><title>$(title)</title>\n'
		QA.ASSERT_STR( o, '<html><title>My HTML Page</title>\n', 'expand string using this object' );

/// expand bug1 [tr]

	var source = 'gl_LightSource[$(xxx)';
	QA.ASSERT_STR( expand(source, { xxx:1 }), 'gl_LightSource[1' );
		
		
/// Big expand [tr]
		
	var exp = stringRepeat('$(X)', 1000);
	for ( var i = 0 ; i < 10; ++i )
		expand(exp, {X:'123'});


/// Expand using a callback function [ftrm]

		var ids = '';
		var i = 0;
		var res = expand('ab$(c)$(d)e$(f)g$(h)ij', function(id) { ids+=id; return i++ } );
		QA.ASSERT_STR( res +'-'+ ids, 'ab01e2g3ij-cdfh', 'Expand result is correct' );


/// exec error [ftrm]

		QA.ASSERTOP( function() exec('e654ser65t'), 'ex', ReferenceError, 'exec unknown file' );

		
/// exec basic test [ftrm]
		
		loadModule('jsio');
		var f = new File('qa_exec_test.js');
		f.content = '((1234))';
		var res = exec('qa_exec_test.js', false);
		QA.ASSERT_STR( res, eval( stringify(f.content) ), 'content validity' );

		var res = exec.call(this, 'qa_exec_test.js', false);
		QA.ASSERT_STR( res, eval( stringify(f.content) ), 'content validity' );

		f.content = undefined;


/// exec using XDR [ftrm]
		
		loadModule('jsio');
	
		var f = new File('qa_exec_test.js');
		f.content = '(1234)';
		
		var res = exec(f.name, true);
		QA.ASSERT( res, 1234, 'exec return value' );

		var fxdr = new File('qa_exec_test.jsxdr');
		QA.ASSERT( fxdr.exist, true, 'XDR file exist' );
		
		f.delete();

		QA.ASSERT( f.exist, false, 'do not have source file' );

		var res = exec(f.name, true);
		QA.ASSERT( res, 1234, 'exec using XDR file' );

		fxdr.delete();
		QA.ASSERT( fxdr.exist, false, 'XDR file is deleted' );
		
		try {
			
			var res = exec(f.name, false);
			QA.FAILED('exec do not detect missing file');
			
		} catch(ex) {
			
			QA.ASSERT( ex.constructor, ReferenceError, 'exec exception' );
			QA.ASSERT( ex.message.indexOf('xdr cannot be found') != -1, true, 'error message' );
		}


/// XDR serialization [ftrmd]

		var s = new Script('/./');
		QA.ASSERTOP( function() xdrEncode(s), 'ex', TypeError );
		//var res = XdrDecode(XdrEncode(s));
		//QA.ASSERT_STR( s.toSource(), res.toSource(), 'XDR->unXDR a Script' );

		QA.ASSERT( xdrDecode(xdrEncode(Map({a:1, b:2, c:3}))).a, 1, 'XDR->unXDR a Map' );
		QA.ASSERT( xdrDecode(xdrEncode(Map({a:1, b:2, c:3}))).b, 2, 'XDR->unXDR a Map' );
		QA.ASSERT( xdrDecode(xdrEncode(Map({a:1, b:2, c:3}))).c, 3, 'XDR->unXDR a Map' );

		QA.ASSERT( xdrDecode(xdrEncode('6r54aze6r5')), '6r54aze6r5', 'XDR->unXDR a string' );
		QA.ASSERT( xdrDecode(xdrEncode(1234567)), 1234567, 'XDR->unXDR a number' );
		QA.ASSERT( xdrDecode(xdrEncode(1.234567)), 1.234567, 'XDR->unXDR a double' );


/// Clear function [ftrm]

		var o = { x:5, y:6, z:7 };
		QA.ASSERT( 'z' in o, true, 'has z property' );
		clearObject(o);
		QA.ASSERT( 'z' in o, false, 'property z is cleared' );


/// Clear on an array [ftrm]
	
		var o = { x:5, y:6, z:7 };
		QA.ASSERT( 'z' in o, true, 'has z property' );
		clearObject(o);
		QA.ASSERT( 'z' in o, false, 'property z is cleared' );



/// Seal function [ftrm]
		
		var o = { a:1, b:{c:2} };
		deepFreezeObject(o);
		o.a = 5;
		QA.ASSERT( o.a, 1, 'freezed object' );
		o.b.c = 6;
		QA.ASSERT( o.b.c, 2, 'deep freezed object' );


/// IsStatementValid  function [ftrm]
		
		QA.ASSERT( isStatementValid( 'for ( var i; i<10; i++ )' ), false, 'invalid statement' );
		QA.ASSERT( isStatementValid( 'for ( var i; i<10; i++ );' ), true, 'valid statement' );
		QA.ASSERT( isStatementValid( '{a,b,c} = { a:1, b:2, c:3 }' ), true, 'valid statement' );


/// StrChr function [ftr]

		var str1 = stringRepeat('y', 100);
		QA.ASSERT( [ c for each ( c in str1 ) if (c == 'y') ].length, 100, 'all chars are good' );
		
		var str = stringRepeat('x', 10000);
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
	
		var res = sandboxEval('Math');
		QA.ASSERT( res.toString(), Math.toString(), 'Math object' );
		QA.ASSERT( res == Math, false, 'Global objects' );


/// Sandbox external access [tfm]

		loadModule('jsio');
		var res = sandboxEval('typeof File');
		QA.ASSERT( res == typeof File, false, 'forbidden File class access' );
		var res = sandboxEval('typeof loadModule');
		QA.ASSERT( res == typeof loadModule, false, 'forbidden loadModule function access' );


/// Sandbox basic query [tfm]

	QA.ASSERTOP( sandboxEval('', function() 123), '===', undefined );
	QA.ASSERTOP( sandboxEval('typeof query'), '===', 'undefined' );
	QA.ASSERTOP( sandboxEval('query()', function() 123), '===', 123 );
	

/// Sandbox Query [tfm]

		var res = Function("var v = 567; return sandboxEval('query()', function(val) v)")();
		QA.ASSERT( res, 567, 'SandboxEval result using Function( query function )' );

		var res = sandboxEval('query(123)', function(val) val + 1 );
		QA.ASSERT( res, 124, 'SandboxEval result using query function' );
		
		var obj = {};
		QA.ASSERTOP( function() { sandboxEval('query()', function(val) obj) }, 'ex', 'TypeError', 'Query returns a non-primitive value');

//		var obj = { abc: 321 };
//		QA.ASSERT( SandboxEval('query()', function(val) obj).abc, 321, 'Query return value');


/// Disabled GC [rd]

		var prev = disableGarbageCollection;
		disableGarbageCollection = true;
		QA.ASSERT( disableGarbageCollection, true, 'GC is disabled' );
		
		QA.gc();
		var mem0 = privateMemoryUsage;
		var str = stringRepeat('x', 1000000);
		str = undefined;
		QA.gc();
		var mem1 = privateMemoryUsage;
		QA.ASSERT( mem1 >= mem0 + 1000000, true, 'without GC' );
		
		disableGarbageCollection = prev;

/*	
		QA.gc();
		var mem0 = privateMemoryUsage;
		var str = stringRepeat('x', 1000000);
		str = undefined;
		QA.gc();
		var mem1 = privateMemoryUsage;
		QA.ASSERT( Math.abs(mem1/mem0) < 1.1, true, 'with GC' );
*/

/// Sandbox misc [rmtf]

	var s = sandboxEval('var a = Math.abs(-123); a');
	QA.ASSERT( s, 123, 'abs' );


/// Sandbox watchdog [rmt]
	
	QA.ASSERTOP( function() { sandboxEval('for (var i=0; i<10000000000; ++i);', undefined, 250) }, 'ex', OperationLimit, 'OperationLimit detection' );


/// OperationLimit is not extensible [fmtr d]

	try {
		
		sandboxEval('for (var i = 0; i < 100000; ++i);', undefined, 0);
		QA.FAILED('OperationLimit exception not thrown');
	} catch (ex) {

		QA.ASSERTOP( function() { 'use strict'; ex.__proto__.test = 1 }, 'ex', TypeError, 'OperationLimit extensibility' );
	}


/// Sandbox stack overflow [rmt]

	QA.ASSERTOP( function() { sandboxEval('(function f(){f();})();') }, 'ex', 'InternalError', 'Stack overflow detection' );


/// exec function [f]
	
	var filename = QA.randomString(10);
	new File(filename).content = '_exectest++';
	
	_exectest = 5;
	exec(filename);
	QA.ASSERT( _exectest, 6, 'exec an expression script (1)' );
	exec(filename);
	QA.ASSERT( _exectest, 7, 'exec an expression script (2)' );
		
	new File(filename).content = undefined;
	QA.ASSERT( new File(filename).content, undefined, 'exec etest file deletion' );
	new File(filename+'xdr').content = undefined;

	delete global._exectest;

/// warning messages [ft]

	if ( !host.safeMode ) // warning messages are disabled in unsafe mode
		return;
		
	var buffer = '';
	var prev = host.stderr;
	host.stderr = function(chunk) buffer += chunk;
	try {
		warning('testing warning messages');
	} catch (ex) {}
	host.stderr = prev;
	
	//print(buffer.length)
	QA.ASSERT_STR( buffer.indexOf('warning: testing warning messages') != -1, true, 'stderr redirection result' ); 

