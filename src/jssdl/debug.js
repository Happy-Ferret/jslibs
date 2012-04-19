var loadModule = host.loadModule;

//loadModule('jsstd'); exec('../../tests/explodebox.js');

loadModule('jssdl');


host.stdout('press ctrl-c');
processEvents(timeoutEvents(2000), host.endSignalEvents());
host.stdout('done.');
