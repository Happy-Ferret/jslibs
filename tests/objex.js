LoadModule('jsstd');
LoadModule('jsobjex');

function newNode(parent) {

	return new ObjEx(undefined,undefined,get,undefined,{parent:parent});
}

function get( id, val, aux ) {
	
	return id in this ? val : this[id] = newNode(this);
}

function addListener( path, listener ) {

	ObjEx.Aux( path ).listener = listener;
}

function setData( path, data ) {
	
	var aux = ObjEx.Aux( path );
	aux.listener && aux.listener('set', data);
	aux.data = data;
	
	while ( path = (aux=ObjEx.Aux(path)).parent || undefined )
		aux.listener && aux.listener('set (children)');
}

function getData( path, data ) {

	return ObjEx.Aux( path ).data || undefined; //  || undefined avoids strict warning
}

function moveData( path, newPath ) {
	
	setData( newPath, getData(path));
	delete ObjEx.Aux(path).data;
	for ( i in path )
		moveData( path[i], newPath[i] );		
}

function delData( path ) { // delete datas but not listeners

	var aux = ObjEx.Aux(path);
	aux.listener && aux.listener('del');
	delete aux.data;
	for ( var [k,v] in path )
		delData( v );
}


///////////////////////


var root = new ObjEx(undefined,undefined,get,undefined,{});

addListener( root.server.port.x.y, function(info) { Print('event: '+info+'\n') } );

setData( root.server.port, 8080 );
//setData( root.server.ip, '127.0.0.1' );

delData( root.server.port );
Print( getData( root.server.port ), '\n' );

setData( root.server.port, 8081 );
Print( getData( root.server.port ), '\n' );


moveData( root.server.port, root.server2.port );

Print( getData( root.server2.port ), '\n' );


setData( root.server.port.x.y.z, 'serverNode' );

