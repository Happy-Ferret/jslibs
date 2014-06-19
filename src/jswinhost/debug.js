var loadModule = host.loadModule;

throw 0;


loadModule('jswinshell');
var s = new Systray();
s.icon = new Icon( 0 );

s.onmouseup = function(button) {

	this.popupMenu(['Quit']);
}

s.oncommand = function(name) {

	if ( name == 'Quit' )
		throw 0;
}


var ev = s.events();
for (;;) {

	processEvents(ev);
}



throw 0;



loadModule('jsstd');
exec('qwer');
throw 0;



/*


// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();

loadModule('jswinshell');
host.stderr = messageBox;

loadModule('jsstd');
//loadModule('jssvg');

var s = new Systray();
s.icon = new Icon( 0 );

s.onmouseenter = function() {

	this.text =
		'peakMemoryUsage: '+(peakMemoryUsage/(1024*1024)).toFixed(0) + 'MB\n' +
		'privateMemoryUsage: '+(privateMemoryUsage/(1024*1024)).toFixed(0) + 'MB\n'+
		'processTime: '+processTime.toFixed(0) + 'ms\n';
}

s.onmouseup = function(button) {

	//	registrySet('HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run', host.name, host.path+'\\'+host.name);

	var hasRun = registryGet('HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run', host.name) == (host.path+'\\'+host.name);

	this.popupMenu(['Quit', {label:'run with window', checked:hasRun}]);
}

s.oncommand = function(name) {

	if ( name == 'Quit' )
		throw 0;
}

var ev = s.events();
for (;;) {

	processEvents(ev);
}
*/