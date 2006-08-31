exec('deflib.js');
LoadModule('jsobjex');


/*
function add( what, obj, id, val, aux ) {

	id != 'add' && id != 'del' && id != 'set' && 'add' in obj && obj.add(id);
	return val;
}

function set( what, obj, id, val, aux ) {

	id != 'add' && id != 'del' && id != 'set' && 'set' in obj && obj.set(id);
	return val;
}

function del( what, obj, id, val, aux ) {

	id != 'add' && id != 'del' && id != 'set' && 'del' in obj && obj.del(id);
	return val;
}

function get( what, obj, id, val, aux ) {

	return id in obj ? val : obj[id] = new objex( add,del,get,set );
}

var o = new objex( add,del,get,set,{} );
*/


var handlerName = '__handler';

function handler( what, obj, id, val, aux ) {
//print('('+id+' '+val+')');

	if ( id != handlerName && handlerName in obj && !(obj instanceof objex)  )
		for ( var [i,v] in obj[handlerName] )
			v.apply( this, arguments)
	return val;
}

function get( what, obj, id, val, aux ) {

	return id in obj ? val : obj[id] = new objex( handler,handler,get,handler );
}

function listen( obj, callback ) {
	
	handlerName in obj || ( obj[handlerName] = []);
	obj[handlerName].push( callback );
	hideProperties( obj, handlerName );
}


var root = new objex( handler,handler,get,handler );

listen( root.server, function( what, obj, id, val, aux ) { print('event '+val+'!\n'); } );

root.server.port = 654;

print(root.server.ip);
root.server.ip = 6667;

/*
o.a.b.c.d.e.add = function(v) { print('add'+v+'\n') }
o.a.b.c.d.e.del = function(v) { print('del'+v+'\n') }
o.a.b.c.d.e.set = function(v) { print('set'+v+'\n') }

hideProperties( o.a.b.c.d.e, 'add', 'del', 'set' );

o.a.b.c.d.e.toto = 123;
o.a.b.c.d.e.tata = 555;
o.a.b.c.d.e.tata = 555;
delete o.a.b.c.d.e.toto


for ( var [k,v] in o.a.b.c.d.e ) print( v );
*/
