//RunJsircbot(false); throw 0;
//var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();
//Exec('../common/tools.js'); var QA = FakeQAApi;  RunLocalQAFile();
//LoadModule('jsstd'); Exec('../common/tools.js'); RunQATests('-exclude jstask');
//LoadModule('jsstd'); LoadModule('jsio'); currentDirectory += '/../../tests/jslinux'; Exec('start.js'); throw 0;

LoadModule('jsstd');




throw 0;

var b = Blob('123');

var obj = b; //[1, '2', { abcd:1 }, /qwe/, b ];

var obj2 = Deserialize(Serialize(obj));

Print(uneval(obj2));

