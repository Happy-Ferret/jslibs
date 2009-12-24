/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */


/// Blob::concat tests using Mozilla String tests

function TestCase( section, name, expected, result ) QA.ASSERT( result, expected, name );
var SECTION = '';


var aString = new Blob("test string");
var bString = new Blob(" another ");

TestCase( SECTION, "aString.concat(' more')", "test string more",     aString.concat(' more').toString());
TestCase( SECTION, "aString.concat(bString)", "test string another ", aString.concat(bString).toString());
TestCase( SECTION, "aString                ", "test string",          aString.toString());
TestCase( SECTION, "bString                ", " another ",            bString.toString());
TestCase( SECTION, "aString.concat(345)    ", "test string345",       aString.concat(345).toString());
TestCase( SECTION, "aString.concat(true)   ", "test stringtrue",      aString.concat(true).toString());
TestCase( SECTION, "aString.concat(null)   ", "test stringnull",      aString.concat(null).toString());
//TestCase( SECTION, "aString.concat([])     ", "test string[]",          aString.concat([]).toString());
//TestCase( SECTION, "aString.concat([1,2,3])", "test string[1, 2, 3]",     aString.concat([1,2,3]).toString());
TestCase( SECTION, "'abcde'.concat(bString)", "abcde another ", 'abcde'.concat(bString).toString());

QA.ASSERT( (aString instanceof Blob) && !(aString instanceof String), true, 'no mutation' );
QA.ASSERT( (bString instanceof Blob) && !(bString instanceof String), true, 'no mutation' );



/// Blob::indexOf tests using Mozilla String tests

function TestCase( section, name, expected, result ) QA.ASSERT( result, expected, name );
var SECTION = '';

var foo = new Blob('hello');

new TestCase( SECTION, "var foo = new MyObject('hello');foo.indexOf('h')", 0, foo.indexOf("h")  );
new TestCase( SECTION, "var foo = new MyObject('hello');foo.indexOf('e')", 1, foo.indexOf("e")  );
new TestCase( SECTION, "var foo = new MyObject('hello');foo.indexOf('l')", 2, foo.indexOf("l")  );
new TestCase( SECTION, "var foo = new MyObject('hello');foo.indexOf('l')", 2, foo.indexOf("l")  );
new TestCase( SECTION, "var foo = new MyObject('hello');foo.indexOf('o')", 4, foo.indexOf("o")  );
new TestCase( SECTION, "var foo = new MyObject('hello');foo.indexOf('X')", -1,  foo.indexOf("X")  );
new TestCase( SECTION, "var foo = new MyObject('hello');foo.indexOf(5) ", -1,  foo.indexOf(5)  );

var boo = new Blob(true);

new TestCase( SECTION, "var boo = new MyObject(true);boo.indexOf('t')", 0, boo.indexOf("t")  );
new TestCase( SECTION, "var boo = new MyObject(true);boo.indexOf('r')", 1, boo.indexOf("r")  );
new TestCase( SECTION, "var boo = new MyObject(true);boo.indexOf('u')", 2, boo.indexOf("u")  );
new TestCase( SECTION, "var boo = new MyObject(true);boo.indexOf('e')", 3, boo.indexOf("e")  );
new TestCase( SECTION, "var boo = new MyObject(true);boo.indexOf('true')", 0, boo.indexOf("true")  );
new TestCase( SECTION, "var boo = new MyObject(true);boo.indexOf('rue')", 1, boo.indexOf("rue")  );
new TestCase( SECTION, "var boo = new MyObject(true);boo.indexOf('ue')", 2, boo.indexOf("ue")  );
new TestCase( SECTION, "var boo = new MyObject(true);boo.indexOf('oy')", -1, boo.indexOf("oy")  );


var noo = new Blob( Math.PI );
new TestCase( SECTION, "var noo = new MyObject(Math.PI); noo.indexOf('3') ", 0, noo.indexOf('3')  );
new TestCase( SECTION, "var noo = new MyObject(Math.PI); noo.indexOf('.') ", 1, noo.indexOf('.')  );
new TestCase( SECTION, "var noo = new MyObject(Math.PI); noo.indexOf('1') ", 2, noo.indexOf('1')  );
new TestCase( SECTION, "var noo = new MyObject(Math.PI); noo.indexOf('4') ", 3, noo.indexOf('4')  );
new TestCase( SECTION, "var noo = new MyObject(Math.PI); noo.indexOf('1') ", 2, noo.indexOf('1')  );
new TestCase( SECTION, "var noo = new MyObject(Math.PI); noo.indexOf('5') ", 5, noo.indexOf('5')  );
new TestCase( SECTION, "var noo = new MyObject(Math.PI); noo.indexOf('9') ", 6, noo.indexOf('9')  );


QA.ASSERT( (foo instanceof Blob) && !(foo instanceof String), true, 'no mutation' );
QA.ASSERT( (boo instanceof Blob) && !(boo instanceof String), true, 'no mutation' );
QA.ASSERT( (noo instanceof Blob) && !(noo instanceof String), true, 'no mutation' );



/// Blob::lastIndexOf tests using Mozilla String tests

var TEST_STRING = new String( " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~" );

function TestCase( section, name, expected, result ) QA.ASSERT( result, expected, name );
var SECTION = '';

function LastIndexOf( string, search, position ) arguments.length == 3 ? string.lastIndexOf(search, position) : string.lastIndexOf(search);



//QA.ASSERT( Blob.prototype.lastIndexOf.length, 1, "Blob.prototype.lastIndexOf.length (argc)" );
QA.ASSERT( delete Blob.prototype.lastIndexOf.length, false, "delete Blob.prototype.lastIndexOf.length" );
//QA.ASSERT( eval("delete Blob.prototype.lastIndexOf.length; Blob.prototype.lastIndexOf.length" ), 1, 'eval("delete Blob.prototype.lastIndexOf.length; Blob.prototype.lastIndexOf.length" )' );


TestCase( SECTION, "var s = new Blob(''); s.lastIndexOf('', 0)",          LastIndexOf("","",0),  eval("var s = new Blob(''); s.lastIndexOf('', 0)") );
TestCase( SECTION, "var s = new Blob(''); s.lastIndexOf('')",             LastIndexOf("",""),  eval("var s = new Blob(''); s.lastIndexOf('')") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('', 0)",     LastIndexOf("hello","",0),  eval("var s = new Blob('hello'); s.lastIndexOf('',0)") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('')",        LastIndexOf("hello",""),  eval("var s = new Blob('hello'); s.lastIndexOf('')") );

TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll')",     LastIndexOf("hello","ll"),  eval("var s = new Blob('hello'); s.lastIndexOf('ll')") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', 0)",  LastIndexOf("hello","ll",0),  eval("var s = new Blob('hello'); s.lastIndexOf('ll', 0)") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', 1)",  LastIndexOf("hello","ll",1),  eval("var s = new Blob('hello'); s.lastIndexOf('ll', 1)") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', 2)",  LastIndexOf("hello","ll",2),  eval("var s = new Blob('hello'); s.lastIndexOf('ll', 2)") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', 3)",  LastIndexOf("hello","ll",3),  eval("var s = new Blob('hello'); s.lastIndexOf('ll', 3)") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', 4)",  LastIndexOf("hello","ll",4),  eval("var s = new Blob('hello'); s.lastIndexOf('ll', 4)") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', 5)",  LastIndexOf("hello","ll",5),  eval("var s = new Blob('hello'); s.lastIndexOf('ll', 5)") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', 6)",  LastIndexOf("hello","ll",6),  eval("var s = new Blob('hello'); s.lastIndexOf('ll', 6)") );

TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', 1.5)", LastIndexOf('hello','ll', 1.5), eval("var s = new Blob('hello'); s.lastIndexOf('ll', 1.5)") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', 2.5)", LastIndexOf('hello','ll', 2.5),  eval("var s = new Blob('hello'); s.lastIndexOf('ll', 2.5)") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', -1)",  LastIndexOf('hello','ll', -1), eval("var s = new Blob('hello'); s.lastIndexOf('ll', -1)") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', -1.5)",LastIndexOf('hello','ll', -1.5), eval("var s = new Blob('hello'); s.lastIndexOf('ll', -1.5)") );

TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', -Infinity)",    LastIndexOf("hello","ll",-Infinity), eval("var s = new Blob('hello'); s.lastIndexOf('ll', -Infinity)") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', Infinity)",    LastIndexOf("hello","ll",Infinity), eval("var s = new Blob('hello'); s.lastIndexOf('ll', Infinity)") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', NaN)",    LastIndexOf("hello","ll",NaN), eval("var s = new Blob('hello'); s.lastIndexOf('ll', NaN)") );
TestCase( SECTION, "var s = new Blob('hello'); s.lastIndexOf('ll', -0)",    LastIndexOf("hello","ll",-0), eval("var s = new Blob('hello'); s.lastIndexOf('ll', -0)") );


var i, k, j = 0;

for ( k = 0, i = 0x0021; i < 0x007e; i++, j++, k++ ) {
  new TestCase( SECTION,
		"String.lastIndexOf(" +String.fromCharCode(i)+ ", 0)",
		-1,
		TEST_STRING.lastIndexOf( String.fromCharCode(i), 0 ) );
}

for ( k = 0, i = 0x0020; i < 0x007e; i++, j++, k++ ) {
  new TestCase( SECTION,
		"String.lastIndexOf("+String.fromCharCode(i)+ ", "+ k +")",
		k,
		TEST_STRING.lastIndexOf( String.fromCharCode(i), k ) );
}

for ( k = 0, i = 0x0020; i < 0x007e; i++, j++, k++ ) {
  new TestCase( SECTION,
		"String.lastIndexOf("+String.fromCharCode(i)+ ", "+k+1+")",
		k,
		TEST_STRING.lastIndexOf( String.fromCharCode(i), k+1 ) );
}

for ( k = 9, i = 0x0021; i < 0x007d; i++, j++, k++ ) {
  new TestCase( SECTION,

		"String.lastIndexOf("+(String.fromCharCode(i) +
				       String.fromCharCode(i+1)+
				       String.fromCharCode(i+2)) +", "+ 0 + ")",
		LastIndexOf( TEST_STRING, String.fromCharCode(i) +
			     String.fromCharCode(i+1)+String.fromCharCode(i+2), 0),
		TEST_STRING.lastIndexOf( (String.fromCharCode(i)+
					  String.fromCharCode(i+1)+
					  String.fromCharCode(i+2)),
					 0 ) );
}

for ( k = 0, i = 0x0020; i < 0x007d; i++, j++, k++ ) {
  new TestCase( SECTION,
		"String.lastIndexOf("+(String.fromCharCode(i) +
				       String.fromCharCode(i+1)+
				       String.fromCharCode(i+2)) +", "+ k +")",
		k,
		TEST_STRING.lastIndexOf( (String.fromCharCode(i)+
					  String.fromCharCode(i+1)+
					  String.fromCharCode(i+2)),
					 k ) );
}

for ( k = 0, i = 0x0020; i < 0x007d; i++, j++, k++ ) {
  new TestCase( SECTION,
		"String.lastIndexOf("+(String.fromCharCode(i) +
				       String.fromCharCode(i+1)+
				       String.fromCharCode(i+2)) +", "+ k+1 +")",
		k,
		TEST_STRING.lastIndexOf( (String.fromCharCode(i)+
					  String.fromCharCode(i+1)+
					  String.fromCharCode(i+2)),
					 k+1 ) );
}

for ( k = 0, i = 0x0020; i < 0x007d; i++, j++, k++ ) {
  new TestCase( SECTION,
		"String.lastIndexOf("+
		(String.fromCharCode(i) +
		 String.fromCharCode(i+1)+
		 String.fromCharCode(i+2)) +", "+ (k-1) +")",
		LastIndexOf( TEST_STRING, String.fromCharCode(i) +
			     String.fromCharCode(i+1)+String.fromCharCode(i+2), k-1),
		TEST_STRING.lastIndexOf( (String.fromCharCode(i)+
					  String.fromCharCode(i+1)+
					  String.fromCharCode(i+2)),
					 k-1 ) );
}


new TestCase( SECTION,  "String.lastIndexOf(" +TEST_STRING + ", 0 )", 0, TEST_STRING.lastIndexOf( TEST_STRING, 0 ) );
new TestCase( SECTION,  "String.lastIndexOf(" +TEST_STRING + ", 1 )", 0, TEST_STRING.lastIndexOf( TEST_STRING, 1 ));
new TestCase( SECTION,  "String.lastIndexOf(" +TEST_STRING + ")", 0, TEST_STRING.lastIndexOf( TEST_STRING ));



// print( "TEST_STRING = new String(\" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\")" );



/// Blob::charAt tests using Mozilla String tests

function TestCase( section, name, expected, result ) QA.ASSERT( result, expected, name );
var SECTION = '';

var x;

new TestCase( SECTION,     "x = new Blob(); x.charAt(0)",    "",     eval("x=new Blob();x.charAt(0)") );
new TestCase( SECTION,     "x = new Blob(); x.charAt(1)",    "",     eval("x=new Blob();x.charAt(1)") );
new TestCase( SECTION,     "x = new Blob(); x.charAt(-1)",   "",     eval("x=new Blob();x.charAt(-1)") );

new TestCase( SECTION,     "x = new Blob(); x.charAt(NaN)",  "",     eval("x=new Blob();x.charAt(Number.NaN)") );
new TestCase( SECTION,     "x = new Blob(); x.charAt(Number.POSITIVE_INFINITY)",   "",     eval("x=new Blob();x.charAt(Number.POSITIVE_INFINITY)") );
new TestCase( SECTION,     "x = new Blob(); x.charAt(Number.NEGATIVE_INFINITY)",   "",     eval("x=new Blob();x.charAt(Number.NEGATIVE_INFINITY)") );

new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(0)",  "1",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(0)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(1)",  "2",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(1)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(2)",  "3",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(2)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(3)",  "4",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(3)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(4)",  "5",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(4)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(5)",  "6",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(5)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(6)",  "7",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(6)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(7)",  "8",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(7)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(8)",  "9",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(8)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(9)",  "0",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(9)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(10)",  "",       eval("var MYOB = new Blob(1234567890); MYOB.charAt(10)") );

new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(Math.PI)",  "4",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(Math.PI)") );




/// Blob::charCodeAt tests using Mozilla String tests

function TestCase( section, name, expected, result ) QA.ASSERT( result, expected, name );
var SECTION = '';


var foo = new Blob('hello');

new TestCase( SECTION, "var foo = new Blob('hello');foo.charCodeAt(0)", 0x0068, foo.charCodeAt(0)  );
new TestCase( SECTION, "var foo = new Blob('hello');foo.charCodeAt(1)", 0x0065, foo.charCodeAt(1)  );
new TestCase( SECTION, "var foo = new Blob('hello');foo.charCodeAt(2)", 0x006c, foo.charCodeAt(2)  );
new TestCase( SECTION, "var foo = new Blob('hello');foo.charCodeAt(3)", 0x006c, foo.charCodeAt(3)  );
new TestCase( SECTION, "var foo = new Blob('hello');foo.charCodeAt(4)", 0x006f, foo.charCodeAt(4)  );
new TestCase( SECTION, "var foo = new Blob('hello');foo.charCodeAt(-1)", Number.NaN,  foo.charCodeAt(-1)  );
new TestCase( SECTION, "var foo = new Blob('hello');foo.charCodeAt(5)", Number.NaN,  foo.charCodeAt(5)  );

var boo = new Blob(true);

new TestCase( SECTION, "var boo = new Blob(true);boo.charCodeAt(0)", 0x0074, boo.charCodeAt(0)  );
new TestCase( SECTION, "var boo = new Blob(true);boo.charCodeAt(1)", 0x0072, boo.charCodeAt(1)  );
new TestCase( SECTION, "var boo = new Blob(true);boo.charCodeAt(2)", 0x0075, boo.charCodeAt(2)  );
new TestCase( SECTION, "var boo = new Blob(true);boo.charCodeAt(3)", 0x0065, boo.charCodeAt(3)  );

var noo = new Blob( Math.PI );

new TestCase( SECTION, "var noo = new Blob(Math.PI);noo.charCodeAt(0)", 0x0033, noo.charCodeAt(0)  );
new TestCase( SECTION, "var noo = new Blob(Math.PI);noo.charCodeAt(1)", 0x002E, noo.charCodeAt(1)  );
new TestCase( SECTION, "var noo = new Blob(Math.PI);noo.charCodeAt(2)", 0x0031, noo.charCodeAt(2)  );
new TestCase( SECTION, "var noo = new Blob(Math.PI);noo.charCodeAt(3)", 0x0034, noo.charCodeAt(3)  );
new TestCase( SECTION, "var noo = new Blob(Math.PI);noo.charCodeAt(4)", 0x0031, noo.charCodeAt(4)  );
new TestCase( SECTION, "var noo = new Blob(Math.PI);noo.charCodeAt(5)", 0x0035, noo.charCodeAt(5)  );
new TestCase( SECTION, "var noo = new Blob(Math.PI);noo.charCodeAt(6)", 0x0039, noo.charCodeAt(6)  );

var noo = new Blob( null );

new TestCase( SECTION, "var noo = new Blob(null);noo.charCodeAt(0)", 0x006E, noo.charCodeAt(0)  );
new TestCase( SECTION, "var noo = new Blob(null);noo.charCodeAt(1)", 0x0075, noo.charCodeAt(1)  );
new TestCase( SECTION, "var noo = new Blob(null);noo.charCodeAt(2)", 0x006C, noo.charCodeAt(2)  );
new TestCase( SECTION, "var noo = new Blob(null);noo.charCodeAt(3)", 0x006C, noo.charCodeAt(3)  );
new TestCase( SECTION, "var noo = new Blob(null);noo.charCodeAt(4)", NaN, noo.charCodeAt(4)  );

var noo = new Blob( void 0 );

new TestCase( SECTION, "var noo = new Blob(void 0);noo.charCodeAt(0)", 0x0075, noo.charCodeAt(0)  );
new TestCase( SECTION, "var noo = new Blob(void 0);noo.charCodeAt(1)", 0x006E, noo.charCodeAt(1)  );
new TestCase( SECTION, "var noo = new Blob(void 0);noo.charCodeAt(2)", 0x0064, noo.charCodeAt(2)  );
new TestCase( SECTION, "var noo = new Blob(void 0);noo.charCodeAt(3)", 0x0065, noo.charCodeAt(3)  );
new TestCase( SECTION, "var noo = new Blob(void 0);noo.charCodeAt(4)", 0x0066, noo.charCodeAt(4)  );



var aString = new Blob("tEs5");

TestCase( SECTION, "aString.charCodeAt(-2)", NaN, aString.charCodeAt(-2));
TestCase( SECTION, "aString.charCodeAt(-1)", NaN, aString.charCodeAt(-1));
TestCase( SECTION, "aString.charCodeAt( 0)", 116, aString.charCodeAt( 0));
TestCase( SECTION, "aString.charCodeAt( 1)",  69, aString.charCodeAt( 1));
TestCase( SECTION, "aString.charCodeAt( 2)", 115, aString.charCodeAt( 2));
TestCase( SECTION, "aString.charCodeAt( 3)",  53, aString.charCodeAt( 3));
TestCase( SECTION, "aString.charCodeAt( 4)", NaN, aString.charCodeAt( 4));
TestCase( SECTION, "aString.charCodeAt( 5)", NaN, aString.charCodeAt( 5));
TestCase( SECTION, "aString.charCodeAt( Infinity)", NaN, aString.charCodeAt( Infinity));
TestCase( SECTION, "aString.charCodeAt(-Infinity)", NaN, aString.charCodeAt(-Infinity));

QA.ASSERT( (aString instanceof Blob) && !(aString instanceof String), true, 'no mutation' );




/// Blob::substring tests using Mozilla String tests

function TestCase( section, name, expected, result ) QA.ASSERT_STR( result, expected, name );
function AssertBlob(b) b instanceof Blob;
var SECTION = '';


new TestCase(   SECTION,
		"var s = new Blob('this is a string object'); typeof s.substring()",
		"object",
		eval("var s = new Blob('this is a string object'); typeof s.substring()") );

new TestCase(   SECTION,
		"var s = new Blob(''); s.substring(1,0)",
		"",
		eval("var s = new Blob(''); s.substring(1,0)") );

new TestCase(   SECTION,
		"var s = new Blob('this is a string object'); s.substring(true, false)",
		"t",
		eval("var s = new Blob('this is a string object'); s.substring(false, true)") );

new TestCase(   SECTION,
		"var s = new Blob('this is a string object'); s.substring(NaN, Infinity)",
		"this is a string object",
		eval("var s = new Blob('this is a string object'); s.substring(NaN, Infinity)") );


new TestCase(   SECTION,
		"var s = new Blob('this is a string object'); s.substring(Infinity, NaN)",
		"this is a string object",
		eval("var s = new Blob('this is a string object'); s.substring(Infinity, NaN)") );


new TestCase(   SECTION,
		"var s = new Blob('this is a string object'); s.substring(Infinity, Infinity)",
		"",
		eval("var s = new Blob('this is a string object'); s.substring(Infinity, Infinity)") );

new TestCase(   SECTION,
		"var s = new Blob('this is a string object'); s.substring(-0.01, 0)",
		"",
		eval("var s = new Blob('this is a string object'); s.substring(-0.01,0)") );


new TestCase(   SECTION,
		"var s = new Blob('this is a string object'); s.substring(s.length, s.length)",
		"",
		eval("var s = new Blob('this is a string object'); s.substring(s.length, s.length)") );

new TestCase(   SECTION,
		"var s = new Blob('this is a string object'); s.substring(s.length+1, 0)",
		"this is a string object",
		eval("var s = new Blob('this is a string object'); s.substring(s.length+1, 0)") );


new TestCase(   SECTION,
		"var s = new Blob('this is a string object'); s.substring(-Infinity, -Infinity)",
		"",
		eval("var s = new Blob('this is a string object'); s.substring(-Infinity, -Infinity)") );



