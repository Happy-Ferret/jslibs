LoadModule('jsstd');  LoadModule('jsio');
//RunJsircbot(false); throw 0;
// var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();
//Exec('../common/tools.js'); var QA = FakeQAApi;  RunLocalQAFile();
Exec('../common/tools.js'); RunQATests('-exclude jstask jslang');
// LoadModule('jsstd'); LoadModule('jsio'); currentDirectory += '/../../tests/jslinux'; Exec('start.js'); throw 0;

