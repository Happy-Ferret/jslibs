exec('deflib.js');
LoadModule('jsobjex');

function newNode(parent) {

	return new objex(undefined,undefined,get,undefined,{parent:parent});
}

function get( what, obj, id, val, aux ) {
	
	return id in obj ? val : obj[id] = newNode(obj);
}

function addListener( path, listener ) {

	objex.Aux( path ).listener = listener;
}

function setData( path, data ) {
	
	var aux = objex.Aux( path );
	aux.listener && aux.listener('set', data);
	aux.data = data;
	
	while ( path = (aux=objex.Aux(path)).parent || undefined )
		aux.listener && aux.listener('set (children)');
}

function getData( path, data ) {

	return objex.Aux( path ).data || undefined;
}

function moveData( path, newPath ) {
	
	setData( newPath, getData(path));
	delete objex.Aux(path).data;
	for ( i in path )
		moveData( path[i], newPath[i] );		
}

function delData( path ) { // delete datas but not listeners

	var aux = objex.Aux(path);
	aux.listener && aux.listener('del');
	delete aux.data;
	for ( var [k,v] in path )
		delData( v );
}


///////////////////////


var root = new objex(undefined,undefined,get,undefined,{});

addListener( root.server.port.x.y, function(info) { print(info+'\n') } );

setData( root.server.port, 8080 );
//setData( root.server.ip, '127.0.0.1' );

delData( root.server.port );
print( getData( root.server.port ), '\n' );

setData( root.server.port, 8081 );
print( getData( root.server.port ), '\n' );


moveData( root.server.port, root.server2.port );

print( getData( root.server2.port ), '\n' );


setData( root.server.port.x.y.z, 'serverNode' );

