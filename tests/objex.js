exec('deflib.js');
LoadModule('jsobjex');

function basicTest() {

	function setter(id,val,aux) {
		print('setter: '+id+','+val+','+aux+'\n');
		return val;
	}


	function getter(id,val,aux) {
		print('getter: '+id+','+val+','+aux+'\n');
		return val;
	}

	function add(id,val,aux) {
		print('add: '+id+','+val+','+aux+'\n');
		return val;
	}


	function del(id,val,aux) {
		print('add: '+id+','+val+','+aux+'\n');
		return val;
	}


	var o = new objex(add,del,getter, setter, {});

	o.a = 123;

	print('write o.a = 123\n');
	o.a = 123;
	print( 'read a.o:'+o.a, '\n' )


	print('list:');
	for ( i in o ) 
		print(i);
}





function add(obj, id, val, aux) {

	id != 'add' && id != 'del' && id != 'set' && 'add' in obj && obj['add'](id);
	return val;
}

function set(obj, id, val, aux) {

	id != 'add' && id != 'del' && id != 'set' && 'set' in obj && obj['set'](id);
	return val;
}

function del(obj, id, val, aux) {

	id != 'add' && id != 'del' && id != 'set' && 'del' in obj && obj['del'](id);
	return val;
}


function get(obj, id, val, aux) {

	return id in obj ? val : obj[id] = new objex( add,del,get,set,{} );
}




var o = new objex( add,del,get,set,{} );


//o.a = 4;

o.a.b.c.d.e.add = function(v) { print('add'+v+'\n') }
o.a.b.c.d.e.del = function(v) { print('del'+v+'\n') }
o.a.b.c.d.e.set = function(v) { print('set'+v+'\n') }

o.a.b.c.d.e.toto = 123;
o.a.b.c.d.e.tata = 123;
o.a.b.c.d.e.tata = 123;
delete o.a.b.c.d.e.toto




//for ( var [k,v] in o.a.b.c.d.e ) print( v );

