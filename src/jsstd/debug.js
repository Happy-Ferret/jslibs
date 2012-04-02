//loadModule('jsstd'); exec('../common/tools.js'); runQATests('jsstd -exclude jstask'); throw 0;
loadModule = host.loadModule;
loadModule('jsstd');

var buf = new Buffer();
buf.write('\xAA\xBB\xCC\xDD');
var pack = new Pack(buf);

pack.readString(4)


