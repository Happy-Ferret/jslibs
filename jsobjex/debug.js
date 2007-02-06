LoadModule('jsstd');
LoadModule('jsobjex');

function newDataNode(parent) {

	return new objex( undefined, undefined, get, undefined, { listenerList:[], parent:parent, data:undefined } );
}

function get( what, obj, id, val, aux ) {

	return id in obj ? val : obj[id] = newDataNode(obj);
}

var d = newDataNode();
objex.Aux(d.aaa.bbb).data = 5;

for ( var [k,v] in d.aaa.bbb ) {
	delData( v );
}


Print( objex.Aux(d.aaa.bbb).data, '\n' );
Print( objex.Aux(d.aaa.bbbb).data, '\n' );
