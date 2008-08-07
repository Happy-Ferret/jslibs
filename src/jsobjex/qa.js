LoadModule('jsobjex');

/// callback functions [ftr]
		
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


/// setter [ftr]

		function MyException() {}
		function SetOnceObject() new ObjEx( undefined,undefined,undefined, function(name, value) this[name] ? (function() { throw new MyException() })() : value );
		var o = SetOnceObject();
		o.abc = 123;
		QA.ASSERT_EXCEPTION( function() {  o.abc = 456;  }, MyException, 'using setter' );


/// data slot [ftr]

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
