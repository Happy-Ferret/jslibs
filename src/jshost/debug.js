LoadModule('jsdebug');
LoadModule('jsstd');
//LoadModule('jsio');
//LoadModule('jscrypt');
//LoadModule('jsaudio');
//LoadModule('jsimage');
//LoadModule('jsfont');
//LoadModule('jsiconv');

LoadModule('jsode');


_host.stdin = function(){};

var done = {__proto__:null};
done[ObjectGCId(done)] = true;
done[ObjectGCId(DebugBreak)] = true;
done[ObjectGCId(Halt)] = true;
done[ObjectGCId(DumpHeap)] = true;
done[ObjectGCId(Object.__proto__.__proto__)] = true;

done[ObjectGCId(Iterator)] = true;


function fct(obj, left) {

	if ( endSignal )
		Halt();
	if ( IsPrimitive(obj) )
		return;
	done[ObjectGCId(obj)] = true;
	var list = Object.getOwnPropertyNames(obj);
	for each ( var name in list ) {
		
		if ( name == 'arguments' )
			continue;

		var nextObj;
		try {
			nextObj = obj[name];
		} catch(ex) {
			continue;
		}
		
		if ( done[ObjectGCId(nextObj)] )
			continue;

		try {
			obj[name]();
		} catch(ex) {
		}

		try {
			nextObj();
		} catch(ex) {
		}

		fct(nextObj);
	}
}

fct(global);

Print('\nDone.\n');
