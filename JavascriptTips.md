<table align='right' height='20'><tr><td></td></tr></table>
<a href='Hidden comment: 
<hr/>
<font size="4"><font color="red"><b>ANNOUNCEMENT

Unknown end tag for &lt;/b&gt;



Unknown end tag for &lt;/font&gt;

 - jslibs 0.95 has been [http://code.google.com/p/jslibs/downloads/list released]

Unknown end tag for &lt;/font&gt;

 (see [http://code.google.com/p/jslibs/ jslibs project])
<hr/><br/>
'></a>

# JavaScript language advanced Tips & Tricks #

![http://jslibs.googlecode.com/svn/wiki/warning.png](http://jslibs.googlecode.com/svn/wiki/warning.png) These tips and tricks are not related to any web browser or any Document Object Model (DOM), they are only general purpose tips and tricks for the JavaScript language.

Some of these tricks are using a latest version of JavaScript language (v1.8) and cannot run with the Microsoft Implementation of JavaScript (v1.5).

All these tricks has been tested with the Mozilla [SpiderMonkey/TraceMonkey](http://developer.mozilla.org/en/docs/SpiderMonkey) JavaScript engine (v1.8).

![http://code.google.com/images/dl_arrow.gif](http://code.google.com/images/dl_arrow.gif) You can try these examples using [jshost](jshost.md), a command-line JavaScript interpreter. ([download it](http://code.google.com/p/jslibs/downloads/list)).

![http://jslibs.googlecode.com/svn/wiki/mail.png](http://jslibs.googlecode.com/svn/wiki/mail.png) If you need more explanation about one of the following tips, don't hesitate to [ask](mailto:soubok+jslibs@gmail.com) me or use the comment section at the end of this page.

See the TOC at the end of the page.

![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)



---

## Append an array to another array ##
```
var a = [4,5,6];
var b = [7,8,9];
Array.prototype.push.apply(a, b);

uneval(a); // is: [4, 5, 6, 7, 8, 9]
```


---

## Milliseconds since epoch ##
```
+new Date() // 1259359833574
```


---

## Simulate threads using yield operator ##
<sup>JavaScript 1.7</sup>
```
//// thread definition
function Thread( name ) {

    for ( var i = 0; i < 5; i++ ) {

        Print(name+': '+i);
        yield;
    }
}

//// thread management
var threads = [];

// thread creation
threads.push( new Thread('foo') );
threads.push( new Thread('bar') );

// scheduler
while (threads.length) {

    var thread = threads.shift();
    try {
        thread.next();
        threads.push(thread);
    } catch(ex if ex instanceof StopIteration) {}
}
```
prints:
```
foo: 0
bar: 0
foo: 1
bar: 1
foo: 2
bar: 2
foo: 3
bar: 3
foo: 4
bar: 4
```



---

## prefix an integer with zeros ##
```
function PrefixInteger(num, length) {

    return (num / Math.pow(10, length)).toFixed(length).substr(2);
}
```


And more efficient:
```
function PrefixInteger(num, length) {
  
  return ( "0000000000000000" + num ).substr( -length );
}
```
Thanks to fritzthe...


And even better:
```
function PrefixInteger(num, length) {
  
  return (Array(length).join('0') + num).slice(-length);
}
```
Thanks to tobeytai...


---

## shuffle the Array ##
<sup>JavaScript 1.?</sup>
```
var list = [1,2,3,4,5,6,7,8,9];

list = list.sort(function() Math.random() - 0.5);

Print(list); // prints something like: 4,3,1,2,9,5,6,7,8 
```


---

## multi-line text ##
<sup>JavaScript 1.6</sup>
```
var text = <>
this
is
my
multi-line
text
</>.toString();
Print(text);
```
prints:
```

this
is
my
multi-line
text

```

#### note ####
If you want to support special XML chars, you can use a CDATA section:
```
<><![CDATA[

>>> hello

]]></>.toString();
```


---

## Escape and unescape HTML entities ##

```
const entityToCode = { __proto__: null,
apos:0x0027,quot:0x0022,amp:0x0026,lt:0x003C,gt:0x003E,nbsp:0x00A0,iexcl:0x00A1,cent:0x00A2,pound:0x00A3,
curren:0x00A4,yen:0x00A5,brvbar:0x00A6,sect:0x00A7,uml:0x00A8,copy:0x00A9,ordf:0x00AA,laquo:0x00AB,
not:0x00AC,shy:0x00AD,reg:0x00AE,macr:0x00AF,deg:0x00B0,plusmn:0x00B1,sup2:0x00B2,sup3:0x00B3,
acute:0x00B4,micro:0x00B5,para:0x00B6,middot:0x00B7,cedil:0x00B8,sup1:0x00B9,ordm:0x00BA,raquo:0x00BB,
frac14:0x00BC,frac12:0x00BD,frac34:0x00BE,iquest:0x00BF,Agrave:0x00C0,Aacute:0x00C1,Acirc:0x00C2,Atilde:0x00C3,
Auml:0x00C4,Aring:0x00C5,AElig:0x00C6,Ccedil:0x00C7,Egrave:0x00C8,Eacute:0x00C9,Ecirc:0x00CA,Euml:0x00CB,
Igrave:0x00CC,Iacute:0x00CD,Icirc:0x00CE,Iuml:0x00CF,ETH:0x00D0,Ntilde:0x00D1,Ograve:0x00D2,Oacute:0x00D3,
Ocirc:0x00D4,Otilde:0x00D5,Ouml:0x00D6,times:0x00D7,Oslash:0x00D8,Ugrave:0x00D9,Uacute:0x00DA,Ucirc:0x00DB,
Uuml:0x00DC,Yacute:0x00DD,THORN:0x00DE,szlig:0x00DF,agrave:0x00E0,aacute:0x00E1,acirc:0x00E2,atilde:0x00E3,
auml:0x00E4,aring:0x00E5,aelig:0x00E6,ccedil:0x00E7,egrave:0x00E8,eacute:0x00E9,ecirc:0x00EA,euml:0x00EB,
igrave:0x00EC,iacute:0x00ED,icirc:0x00EE,iuml:0x00EF,eth:0x00F0,ntilde:0x00F1,ograve:0x00F2,oacute:0x00F3,
ocirc:0x00F4,otilde:0x00F5,ouml:0x00F6,divide:0x00F7,oslash:0x00F8,ugrave:0x00F9,uacute:0x00FA,ucirc:0x00FB,
uuml:0x00FC,yacute:0x00FD,thorn:0x00FE,yuml:0x00FF,OElig:0x0152,oelig:0x0153,Scaron:0x0160,scaron:0x0161,
Yuml:0x0178,fnof:0x0192,circ:0x02C6,tilde:0x02DC,Alpha:0x0391,Beta:0x0392,Gamma:0x0393,Delta:0x0394,
Epsilon:0x0395,Zeta:0x0396,Eta:0x0397,Theta:0x0398,Iota:0x0399,Kappa:0x039A,Lambda:0x039B,Mu:0x039C,
Nu:0x039D,Xi:0x039E,Omicron:0x039F,Pi:0x03A0,Rho:0x03A1,Sigma:0x03A3,Tau:0x03A4,Upsilon:0x03A5,
Phi:0x03A6,Chi:0x03A7,Psi:0x03A8,Omega:0x03A9,alpha:0x03B1,beta:0x03B2,gamma:0x03B3,delta:0x03B4,
epsilon:0x03B5,zeta:0x03B6,eta:0x03B7,theta:0x03B8,iota:0x03B9,kappa:0x03BA,lambda:0x03BB,mu:0x03BC,
nu:0x03BD,xi:0x03BE,omicron:0x03BF,pi:0x03C0,rho:0x03C1,sigmaf:0x03C2,sigma:0x03C3,tau:0x03C4,
upsilon:0x03C5,phi:0x03C6,chi:0x03C7,psi:0x03C8,omega:0x03C9,thetasym:0x03D1,upsih:0x03D2,piv:0x03D6,
ensp:0x2002,emsp:0x2003,thinsp:0x2009,zwnj:0x200C,zwj:0x200D,lrm:0x200E,rlm:0x200F,ndash:0x2013,
mdash:0x2014,lsquo:0x2018,rsquo:0x2019,sbquo:0x201A,ldquo:0x201C,rdquo:0x201D,bdquo:0x201E,dagger:0x2020,
Dagger:0x2021,bull:0x2022,hellip:0x2026,permil:0x2030,prime:0x2032,Prime:0x2033,lsaquo:0x2039,rsaquo:0x203A,
oline:0x203E,frasl:0x2044,euro:0x20AC,image:0x2111,weierp:0x2118,real:0x211C,trade:0x2122,alefsym:0x2135,
larr:0x2190,uarr:0x2191,rarr:0x2192,darr:0x2193,harr:0x2194,crarr:0x21B5,lArr:0x21D0,uArr:0x21D1,
rArr:0x21D2,dArr:0x21D3,hArr:0x21D4,forall:0x2200,part:0x2202,exist:0x2203,empty:0x2205,nabla:0x2207,
isin:0x2208,notin:0x2209,ni:0x220B,prod:0x220F,sum:0x2211,minus:0x2212,lowast:0x2217,radic:0x221A,
prop:0x221D,infin:0x221E,ang:0x2220,and:0x2227,or:0x2228,cap:0x2229,cup:0x222A,int:0x222B,
there4:0x2234,sim:0x223C,cong:0x2245,asymp:0x2248,ne:0x2260,equiv:0x2261,le:0x2264,ge:0x2265,
sub:0x2282,sup:0x2283,nsub:0x2284,sube:0x2286,supe:0x2287,oplus:0x2295,otimes:0x2297,perp:0x22A5,
sdot:0x22C5,lceil:0x2308,rceil:0x2309,lfloor:0x230A,rfloor:0x230B,lang:0x2329,rang:0x232A,loz:0x25CA,
spades:0x2660,clubs:0x2663,hearts:0x2665,diams:0x2666
};

var charToEntity = {};
for ( var entityName in entityToCode )
	charToEntity[String.fromCharCode(entityToCode[entityName])] = entityName;

function UnescapeEntities(str) str.replace(/&(.+?);/g, function(str, ent) String.fromCharCode( ent[0]!='#' ? entityToCode[ent] : ent[1]=='x' ? parseInt(ent.substr(2),16): parseInt(ent.substr(1)) ) );

function EscapeEntities(str) str.replace(/[^\x20-\x7E]/g, function(str) charToEntity[str] ? '&'+charToEntity[str]+';' : str );
```


---

## Remove an object from an array ##
<sup>JavaScript 1.8</sup>
```
function RemoveArrayElement( array, element ) !!let (pos=array.lastIndexOf(element)) pos != -1 && array.splice(pos, 1);
```


---

## Creates a random alphabetic string ##
```
function RandomString(length) {
    
    var str = '';
    for ( ; str.length < length; str += Math.random().toString(36).substr(2) );
    return str.substr(0, length);
}
```


---

## Brainfuck interpreter ##
```

var code = '++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.';
var inp = '23\n';
var out = '';

var codeSize = code.length;
var i = 0, ip = 0, cp = 0, dp = 0, m = {};

var loopIn = {}, loopOut = {};
var tmp = [];
for ( var cp = 0; cp < codeSize ; cp++ )
    if ( code[cp] == '[' )
        tmp.push(cp);
    else
        if ( code[cp] == ']' )
            loopOut[loopIn[cp] = tmp.pop()] = cp;

for (var cp = 0; cp < codeSize && i < 100000; cp++, i++) {

    switch(code[cp]) {

        case '>': dp++; break;
        case '<': dp--; break;
        case '+': m[dp] = ((m[dp]||0)+1)&255; break
        case '-': m[dp] = ((m[dp]||0)-1)&255; break;
        case '.': out += String.fromCharCode(m[dp]); break;
        case ',': m[dp] = inp.charCodeAt(ip++)||0; break;
        case '[': m[dp]||(cp=loopOut[cp]); break;
        case ']': cp = loopIn[cp]-1; break;
    }
}
Print(out);
```

This [Brainfuck](http://en.wikipedia.org/wiki/Brainfuck) program just prints 'Hello World!'


---

## Optional named function arguments ##
```
function foo({ name:name, project:project}) {

    Print( project );
    Print( name );
}

foo({ name:'soubok', project:'jslibs' })
foo({ project:'jslibs', name:'soubok'})
```



---

## String converter ##
<sup>JavaScript 1.8</sup>
```
function CreateTranslator(translationTable) function(s) s.replace(new RegExp([k for (k in translationTable)].join('|'), 'g'), function(str) translationTable[str]);
```
exemple of use:
```
var translationTable = { a:1, bb:2, b:3, c:4 };
var MyTranslater = CreateTranslator( translationTable );
MyTranslater('aabbbc'); // returns: 11234
```


---

## Display the current call stack ##
```
function Stack() { try { throw Error() } catch(ex) { return ex.stack } }

print( Stack() );
```
prints:
```
Error()@:0
Stack()@test.js:1
@test.js:3
```



---

## Change the primitive value of an object with _valueOf_ ##
```
function foo() {

   this.valueOf = function() {

     return 'this is my value';
   }
}

var bar = new foo();

Print( bar ); // prints: this is my value

Print( bar == 'this is my value' ) // prints: true

Print( bar === 'this is my value' ) // prints: false

```



---

## Transform the _arguments_ object into an array ##
<sup>JavaScript 1.6</sup>
```
function foo() {

  var argArray = Array.slice(arguments); // is ['aa',11] 
}

foo('aa',11);
```

use also
```
var argArray = Array.prototype.slice.call(arguments);
```



---

## Convert a string into a charcode list ##
Method 1:
<sup>JavaScript 1.6</sup>
```
Array.map('foo', function(x) { return String.charCodeAt(x) }) // is [112,111,111]
```

Method 2:
<sup>JavaScript 1.7</sup>
```
[ String.charCodeAt(x) for each ( x in 'foo' ) ] // is [112,111,111]
```



---

## Array iteration pitfall ##
```
var foo = [3,4,5];

for ( var i in foo ) {

    if ( i == 1 ) {

        foo.unshift(6);
    }
    Print('item: '+foo[i])
}
Print( 'array: '+foo.toSource() )
```
output:
```
item: 3
item: 3
item: 4
array: [6, 3, 4, 5]
```



---

## exceptions for non-fatal errors ##
<sup>JavaScript 1.7</sup>
```
function ERR() { throw ERR }
function CHK( v ) { return v || ERR() }

try {
	
  var data1 = 'a/b/c';
  var arr1 = CHK(data1).split('/');

  var data2 = '';
  var arr2 = CHK(data2).split('/'); // the exception is throw here

} catch(ex if ex == ERR) {

  Print('non fatal error while decoding the data')
}
```
prints:
```
a b c
```



---

## A Switch function ##
<sup>JavaScript 1.8</sup>
```
function Switch(i) arguments[++i];
```
usage:
```
Print( Switch(2,'aa','bb','cc','dd') ); // prints: cc
Print( Switch(100,'aa','bb','cc','dd') ); // prints: undefined
```



---

## A Match function ##
<sup>JavaScript 1.8</sup>
```
function Match(v) Array.indexOf(arguments,v,1)-1;
```
usage:
```
Print( Match('aa', '0', 'b', 123, 'aa', 8.999 ) ); // prints: 3
Print( Match('Z', 'b', 123, 'aa', 8.999 ) ); // prints: -2 (< 0 is not found)
```



---

## new object override ##
<sup>JavaScript 1.5</sup>
```
function foo() {

  return new Array(5,6,7);
}

var bar = new foo();
bar.length // is 3
```



---

## Kind of destructuring assignments ##
<sup>JavaScript 1.7</sup>
```
 var { a:x, b:y } = { a:7, b:8 };
 Print(x); // prints: 7
 Print(y); // prints: 8
```



---

## Generator Expressions ##
<sup>JavaScript 1.7</sup>
```
[ y for ( y in [5,6,7,8,9] ) ] // is [0,1,2,3,4]
```

**and**

```
[ y for each ( y in [5,6,7,8,9] ) ] // is [5,6,7,8,9]
```

Because in **for** extracts index names, and **for each** extracts the values.



---

## Advanced use of iterators ##
<sup>JavaScript 1.7</sup>
```
Number.prototype.__iterator__ = function() {

 for ( let i = 0; i < this; i++ )
  yield i;
};

for ( let i in 5 )
 print(i);
```
prints:
```
1
2
3
4
5
```

This make Number object to act as a generator.



---

## Expression Closures ##
<sup>JavaScript 1.8</sup>
```
function(x) x * x;
```
Note that braces **{**...**}** and **return** are implicit



---

## Function declaration and expression ##

Function declaration:
```
bar(); // prints: bar
function bar() {

   Print('bar');
}

function foo() {

   Print('foo');
}
foo(); // prints: foo

```



Function expression:
```
(function foo() {
  Print( foo.name );
})(); // prints: foo
foo(); // rise a ReferenceError

!function foo() {
}
foo(); // rise a ReferenceError

var bar = function foo() {
}
foo(); // rise a ReferenceError
```


---

## Factory method pattern ##
```
Complex = new function() {

	function Complex(a, b) {
		// ...
	}

	this.fromCartesian = function(real, mag) {

		return new Complex(real, imag);
	}

	this.fromPolar = function(rho, theta) {

		return new Complex(rho * Math.cos(theta), rho * Math.sin(theta));
	}
}


var c = Complex.fromPolar(1, Math.pi); // Same as fromCartesian(-1, 0);
```

<sub>see factory pattern on</sub> [wikipedia](http://en.wikipedia.org/wiki/Factory_method_pattern)


---

## Closures by example ##
<sup>JavaScript 1.5</sup>

```
function CreateAdder( add ) {

  return function( value ) {
  
    return value + add;
  }
}
```

usage:
```
var myAdder5 = CreateAdder( 5 );
var myAdder6 = CreateAdder( 6 );

Print( myAdder5( 2 ) ); // prints 7
Print( myAdder6( 4 ) ); // prints 10
```

<sub>Further information about</sub> [Nested functions and closures](http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Reference:Functions#Nested_functions_and_closures)


---

## SyntaxError ##
<sup>JavaScript 1.5</sup>

raised when a syntax error occurs while parsing code in eval()
```
try {

  eval('1 + * 5'); // will rise a SyntaxError exception
	
} catch( ex ) {

	Print( ex.constructor == SyntaxError ); // Prints true
}
```

<sup>JavaScript 1.7</sup>
```
try {

 eval('1 + * 5');

} catch( ex if ex instanceof SyntaxError  ) {

 Print( 'SyntaxError !' ); // prints: SyntaxError !
}
```



---

## ReferenceError ##

raised when de-referencing an invalid reference.
```
try {

 fooBar(); // will rise a ReferenceError exception

} catch( ex ) {

 Print( ex.constructor == ReferenceError ); // Prints true
}
```

<sup>JavaScript 1.7</sup>
```
try {

 fooBar();

} catch( ex if ex instanceof ReferenceError ) {

 Print( 'ReferenceError !' ); // prints: ReferenceError !
}
```




---

## JavaScript Minifier / comment remover ##
<sup>jslibs</sup>
```
var script = new Script('var a; /* this is a variable */ var b; // another variable');
Print( script.toString() );
```
prints:
```
var a;
var b;
```


<sup>javascript</sup>
```
function foo() {
 
 var a; // a variable
 var b = [1, 2, 3]; // an array
 var c = {x: {y: 1}};
 
 function bar() { // my function
  
  return 1 /* 2 */;
 }
}

Print( foo.toSource() );
```



---

## JavaScript code beautifier (un-minifier) ##
```
function foo() {var a;var b=[1,2,3];var c={x:{y:1}};function bar(){return 1}}
Print( foo.toSource(1) );
```
prints:
```
function foo() {
 var a;
 var b = [1, 2, 3];
 var c = {x: {y: 1}};
 
 function bar() {
  return 1;
 }
}
```



---

## Auto indent JavaScript code / unobfuscator ##
<sup>Spidermonkey JavaScript engine only</sup>
```
function foo() { function bar(){};var i=0;for(;i<10;++i) bar(i) }
Print(foo.toSource(2));
```
prints:
```
function foo() {

      function bar() {
      }

      var i = 0;
      for (; i < 10; ++i) {
          bar(i);
      }
  }
```



---

## Objects, constructor and instanceof ##
<sup>JavaScript 1.5</sup>
```
function Foo() {
  // Foo class
}

var a = new Foo();


Bar.prototype = new Foo();
function Bar() {
  // Bar class
}

var b = new Bar();


Print( a.constructor == Foo ); // true
Print( a instanceof Foo ); // true

Print( b.constructor == Foo ); // true
Print( b instanceof Foo ); // true

Print( b.constructor == Bar ); // false !
Print( b instanceof Bar ); // true
```



---

## Objects private and public members ##

```
function MyConstructor( pub, priv ) {

  var privateVariable = priv;
  this.publicVariable = pub;

  this.SetThePrivateVariable = function( value ) {

    privateVariable = value;
  }

  this.GetThePrivateVariable = function() {

    return privateVariable;
  }

  this.PrintAllVariables = function() {

    Print( privateVariable + ',' + this.publicVariable );
  }
}

var myObject = new MyConstructor( 123, 456 );

Print( myObject.privateVariable ); // prints: undefined
Print( myObject.publicVariable ); // prints: 123
myObject.PrintAllVariables(); // prints 456,123
myObject.SetThePrivateVariable( 789 );
myObject.PrintAllVariables(); // prints 789,123

```



---

## Stack data structure ##
<sup>JavaScript 1.5</sup>
Array object has all needed methods to be used as a stack.
```
  var stack = [];
  stack.push( 111 );
  stack.push( 2.22 );
  stack.push( 'ccc' );

  Print( stack ); // prints: 111,2.22,ccc

  Print( stack.pop() ); // prints: ccc
  Print( stack.pop() ); // prints: 2.22
```



---

## Singleton pattern ##
<sup>JavaScript 1.5</sup>
The singleton pattern is a [design pattern](http://en.wikipedia.org/wiki/Design_pattern_%28computer_science%29) that is used to restrict instantiation of a class to one object.
This is useful when exactly one object is needed to coordinate actions across the system.
```
function MySingletonClass() {

  if ( arguments.callee._singletonInstance )
    return arguments.callee._singletonInstance;
  arguments.callee._singletonInstance = this;

  this.Foo = function() {
    // ...
  }
}

var a = new MySingletonClass()
var b = MySingletonClass()
Print( a === b ); // prints: true
```



---

## Use Functions as an Object ##
<sup>JavaScript 1.5</sup>
```
function Counter() {
   
   if ( !arguments.callee.count ) {
   
        arguments.callee.count = 0;
    }
    return arguments.callee.count++;
}
Print( Counter() ); // prints: 0
Print( Counter() ); // prints: 1
Print( Counter() ); // prints: 2
```



---

## E4X add nodes ##
<sup>JavaScript 1.6</sup>
```
var x = <store/>;
x.* += <item price="10" />;
x.* += <item price="15" />;
Print( x.toXMLString() );
```
prints:
```
<store>
  <item price="10"/>
  <item price="15"/>
</store>
```



---

## E4X dynamic document creation ##
<sup>JavaScript 1.6</sup>
```
var html = <html/>;
html.head.title = "My Page Title";
html.body.@bgcolor = "#e4e4e4";
html.body.form.@name = "myform";
html.body.form.@action = "someurl.jss";
html.body.form.@method = "post";
html.body.form.@onclick = "return somejs();";
html.body.form.input[0] = "";
html.body.form.input[0].@name = "test";
Print(html.toXMLString());
```
prints:
```
<html>
  <head>
    <title>My Page Title</title>
  </head>
  <body bgcolor="#e4e4e4">
    <form name="myform" action="someurl.jss" method="post" onclick="return somejs();">
      <input name="test"></input>
    </form>
  </body>
</html>
```



---

## E4X dynamic tag name ##
<sup>JavaScript 1.6</sup>
```
var nodeName = 'FOO';
if ( isXMLName(nodeName) ) { // assert that nodeName can be used as a node name
  var x = <{nodeName}>test</{nodeName}>
  Print( x.toXMLString() ); // prints: <FOO>test</FOO>
}
```



---

## E4X dynamic content ##
<sup>JavaScript 1.6</sup>
```
var content = 'FOO';
var x = <item>{content}</item>
Print( x.toXMLString() ); // prints: <item>FOO</item>
```


---

## E4X iteration ##
<sup>JavaScript 1.6</sup>
```
var sales = 
<sales vendor="John">
  <item type="peas" price="4" quantity="5"/>
  <item type="carrot" price="3" quantity="10"/>
  <item type="chips" price="5" quantity="3"/>
</sales>;
 
for each( var price in sales..@price ) {
  Print( price + '\n' );
}
```
prints:
```
4
3
5
```



---

## Listen a property for changes ##
```
function onFooChange( id, oldval, newval ) {

  Print( id + " property changed from " + oldval + " to " + newval );
  return newval;
}

var o = { foo:5 };
o.watch( 'foo', onFooChange );

o.foo = 6;

delete o.foo;

o.foo = 7;

o.unwatch('foo');
o.foo = 8;
```
prints:
```
foo property changed from 5 to 6
foo property changed from undefined to 7
```



---

## Logical operators tricks ##
```
var a = 5;
a == 5 && Print( 'a is 5 \n' );
a == 7 || Print( 'a is not 7 \n' );
```
prints:
```
a is 5 
a is not 7
```



---

## Functions argument default value ##
```
function foo( a, b ) {

  a = a || '123';
  b = b || 55;
  Print( a + ',' + b );
}

foo(); // prints: 123,55
foo('bar'); // prints: bar,55
foo('x', 'y'); // prints x,y
```
![http://jslibs.googlecode.com/svn/wiki/warning.png](http://jslibs.googlecode.com/svn/wiki/warning.png) but:
```
foo(0,''); // prints: 123,55
```
because 0 and '' are evaluated as false !



---

## Remove an item by value in an Array object ##
```
var arr = ['a', 'b', 'c', 'd'];
var pos = arr.indexOf( 'c' );
pos > -1 && arr.splice( pos, 1 );
Print( arr ); // prints: a,b,d
```



---

## Multiple-value returns ##
<sup>JavaScript 1.7</sup>
```
function f() {

  return [1, 2];
}

var [a, b] = f();

Print( a + ' ' + b ); // prints: 1 2
```



---

## Operator [ ] and strings ( like charAt() ) ##
<sup>JavaScript 1.6</sup>
```
var str = 'foobar';
Print( str[4] );
```
prints:
```
a
```



---

## indexOf() and lastIndexOf() Works on Array ##
<sup>JavaScript 1.6</sup>
```
var obj = {};
var arr = [ 'foo', 567, obj, 12.34 ];
Print( arr.indexOf(obj) ); // prints: 2
```



---

## Using Array functions on a non-Array object ##
<sup>JavaScript 1.7</sup>
```
var obj = {};
Array.push(obj, 'foo');
Array.push(obj, 123);
Array.push(obj, 5.55);
Print( obj.toSource() ); // prints: ({0:"foo", length:3, 1:123, 2:5.55})
```



---

## Change current object (this) of a function call ##
```
function test(arg) {

  Print( this[0]+' '+this[1]+' '+arg );
}

var arr = ['foo', 'bar'];
test.call(arr, 'toto'); // prints: foo bar toto
```



---

## Filter / intercept a function call ##
```
function bar(a, b, c, d, e, f) {

  Print(a, b, c, d, e, f)
}

function foo() {

  bar.apply(this, arguments);
}

foo(1, 2, 3, 4, 5, 6); // prints: 123456
```



---

## E4X Query ( like XPath ) ##
<sup>JavaScript 1.6</sup>
```
var xml = <mytree>
 <data id="1" text="foo"/>
 <data id="2" text="bar"/>
</mytree>

Print( xml.data.(@id==1).@text );
var myid = 2; 
Print( xml.data.(@id==myid).@text );
```
prints:
```
foo
bar
```



---

## swap two variables ##
<sup>JavaScript 1.7</sup>
```
var a = 1;
var b = 2;
[a,b] = [b,a];
```



---

## Destructuring assignment with function arguments ##
<sup>JavaScript 1.7</sup>
```
function foo( [a,b] ) {

	Print(a);
	Print(b);
}

foo( [12,34] );
```
Prints:
```
12
34
```



---

## JavaScript scope is not C/C++ scope ##
```
if ( false ) { // never reach the next line

	var foo = 123;
}

Print(foo); // prints: undefined ( but is defined )
Print(bar); // failed: ReferenceError: bar is not defined
```



---

## JavaScript scope and LET instruction ##
<sup>JavaScript 1.7</sup>
```
var x = 5;
var y = 0;
let (x = x+10, y = 12) {
  Print(x+y);
}
Print(x+y);
```
prints:
```
27
5
```

or,
```
for ( let i=0 ; i < 10 ; i++ ) {
  Print(i + ' ');
}
Print(i);
```
prints:
```
0 1 2 3 4 5 6 7 8 9 test.js:4: ReferenceError: i is not defined
```



---

## Defer function calls ##
```
var opList = [];
function deferCall( text, value ) { 
	opList.push(arguments)
}
function doCall(func) { 
	while (opList.length) 
		func.apply(this,opList.shift());
}

deferCall( 'one', 1 );
deferCall( 'titi', 'toto' );
deferCall( 5, 'foo' );

function callLater(a,b) {
  
Print(a+', '+b);
}

doCall( callLater )
```
Prints:
```
one, 1
titi, toto
5, foo
```



---

## Insert an array in another array ##
```
var a = [1,2,3,7,8,9]

var b = [4,5,6]
var insertIndex = 3;

a.splice.apply(a, Array.concat(insertIndex, 0, b));
Print(a); // prints: 1,2,3,4,5,6,7,8,9
```



---

## Multiple string concatenation ##
```
var html = ['aaa', 'bbb', 'ccc', ...].join('');
```
Is faster than:
```
var html = 'aaa' + 'bbb' + 'ccc' + ...;
```
This is true with a big number of elements ( > 5000 )



---

## HTTP headers parser ##
```
var rexp_keyval = /(.*?): ?(.*?)\r?\n/g;
function headersToObject( allHeaders ) {

	var res, hdrObj = {};
	for ( rexp_keyval.lastIndex = 0; res = rexp_keyval.exec(allHeaders); hdrObj[res[1]] = res[2])
	return hdrObj;
}
```



---

## Using 'with' scope ##
```
with({ a:5 }) function toto() { return a }
toto() // returns 5
```



---

## (object).toString() ##
```
var a = { toString:function() { return '123'; }  }
Print(a); // prints '123', and not [Object object]
```



---

## RegExpr.$1 ##
```
var re = /a(.*)/
'abcd'.match(re)
Print( RegExp.$1 ) // prints 'bcd'
```



---

## Binary with XmlHTTPRequest ##
<sup>browser related example</sup>
```
var req = new XMLHttpRequest();
req.open('GET', "http://code.google.com/images/code_sm.png",false);
req.overrideMimeType('text/plain; charset=x-user-defined');
//req.overrideMimeType('application/octet-stream');
req.send(null);
var val = req.responseText;
Print( escape(val.substr(0,10)) );
```



---

## Iterate on values ##
<sup>JavaScript 1.6</sup>
```
for each ( var i in [3,23,4] )
	Print(i)
```
Prints:
```
3
23
4
```



---

## Exceptions Handling / conditional catch (try catch if) ##
```
function Toto(){}
function Titi(){}

try {

	throw new Titi()

} catch ( err if err instanceof Toto ) {
	Print('toto')
} catch ( err if err instanceof Titi ) {
	Print('titi')
} catch(ex) {
	throw(ex);
}
```



---

## Special chars ##
```
$=4
_=5
Print( _+$)
```
prints:
```
9
```



---

## object's eval method ##
```
var foo = { bar:123 };
foo.eval('bar') // returns 123
```

```
var foo = { bar:123 };
with ( foo )
 var val = eval( 'bar' );
Print( val ); // returns 123
```


---

## eval this ##
```
function test() {

	Print(eval('this'));
}
test.call(123)
```
prints:
```
123
```



---

## No Such Method ( noSuchMethod ) ##
```
var o = {}
o.__noSuchMethod__ = function(arg){ Print('unable to call "'+arg+'" function') }
o.foo(234)
```
prints:
```
unable to call "foo" function
```



---

## RegExp replace ##
```
function Replacer( conversionObject ) {

	var regexpStr = '';
	for ( var k in conversionObject )
		regexpStr += (regexpStr.length ? '|' : '') + k;
	var regexpr = new RegExp(regexpStr,'ig'); // g: global, m:multi-line i: ignore case
	return function(s) { return s.replace(regexpr, function(str, p1, p2, offset, s) { var a = conversionObject[str]; return a == undefined ? str : a }) }
}

var myReplacer = Replacer( { '<BR>':'\n', '&amp;':'&', '&lt;':'<', '&gt;':'>', '&quot;':'"' } );

Print( myReplacer('aa<BR>a &amp;&amp;&amp;&lt;') );
```
prints:
```
aa
a &&&<
```



---

## Values comparison ##
```
[4] === 4 // is: false
[4] == 4 // is: true

'0' == 0 // is: true
'0' === 0 // is: false
```



---

## undefined, null, 0, false, '', ... ##
```
var a = { b:undefined, c:null, d:0, f:'' }

a['b'] // is: undefined
a['e'] // is: undefined

'b' in a // is: true
'e' in a // is: false

Boolean(a.b) // is: false
Boolean(a.c) // is: false
Boolean(a.d) // is: false
Boolean(a.e) // is: false !
Boolean(a.f) // is: false

typeof( asvqwfevqwefq ) == 'undefined' // =true

Print( '' == false ); // prints: true
Print( 0 == false ); // prints: true
Print( [] == false ); // prints: true

Print( [] == '' ); // prints: true
Print( '' == 0 ); // prints: true
Print( [] == 0 ); // prints: true

```



---

## constructor property ( InstanceOf + Type ) ##
```
var a = {};
a.constructor === Object // is: true
```



---

## AJAX evaluation ##
```
var a = {

	b:function() { Print(123) }
}

var body = 'b();';

with(a) { eval(body) }; // Prints 123
```



---

## Comma operator ##
```
var a = 0;
var b = ( a++, 99 );

a // is: 1
b // is: 99

var i = 0;
while( Print('x '), i++<10 )
	Print(i + ' ')
```
prints:
`x 1 x 2 x 3 x 4 x 5 x 6 x 7 x 8 x 9 x 10 x `


---

## closures pitfall ##
```
var a = [];

for ( var i=0; i<10; i++ ) {

 a[i] = function() { Print(i); }
}

a[0](); // is: 10
a[1](); // is: 10
a[2](); // is: 10
a[3](); // is: 10
```

simpler case:
```
var i = 5;
function foo() { Print(i) }
foo(); // prints 5
i = 6;
foo(); // prints 6
```

#### Closure definition ####
A closure occurs when one function appears entirely within the body of another, and the inner function refers to local variables of the outer function.

#### links ####
[obligated lecture to understand closures](http://www.jibbering.com/faq/faq_notes/closures.html)


In the first example, you can avoid the behavior using the let instruction:
```
var a = [];

for ( var i=0; i<10; i++ ) {
 let j = i;
 a[i] = function() { Print(j); }
}
```
In this case, each instances of the (anonymous) inner function refer to different instances of variable j.


---

## sharp variable ##
```
var a = { titi:#1={}, toto:#1# };
a.titi === a.toto; // is: true

var a = { b:#1={ c:#1# } }
// a.b.c.c.c.c.c.c...
```



---

## common object between 2 objects ##
```
function a() {}

a.prototype = { b:{} }

c = new a;
d = new a;

c.b.e = 2;

c.b === d.b // is: true !
d.b.e // is: 2
```



---

## constructor ##
```
function a( b ) {

	Print(b);
}

c.prototype = new a;

function c( arg ) {

	this.constructor.apply( this, arg );
};

o = new c( [1,2,3] );
```
prints:
```
undefined
1
```



---

## JavaScript string can contain null chars ##
```
var test = '\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00';
test.length // is: 10
```



---

## 'new' operator precedence ##
```
function test() {

	this.toto = 1234;
}

(new test()).toto // ok ( logical way )

new test().toto // ok

new test.toto // bad !

(new test).toto // ok
```



---

## constructor usage ##
```
function toto( val ) { // parent class

	this.print = function() {

		Print( val );
	}
}

titi.prototype = new toto;

function titi( val ) { // child class
  
	this.constructor(val);
}

(new titi(7)).print(); // prints 7
```



---

## To object ##
```
var obj = Object(123);
obj.foo = 678;
Print( typeof( obj ) + ', ' + obj + ', ' + obj.foo ); // prints: object, 123, 678
```



---

## Serialize or uneval a variable or an object ( can be nested ) ##
```
var o = { a:123, b:'test', c:function() { return 987; } }

Print( o.toSource() ); // prints: ({a:123, b:"test", c:(function () {return 987;})})

Print( uneval(o) ); // prints: ({a:123, b:"test", c:(function () {return 987;})})
```



---

## JavaScript labels ##
```
xxx: {
	Print( 111 )
	break xxx;
	Print( 222 )
}
```
prints:
```
111
```



---

## for in loop ##
```
for ( var i in function(){ return [1, 2, 3] }() )
  Print( i );
```
prints:
```
0
1
2
```



---

## proto ( prototype property ) ##
```
var a = {}
var b = {}
a.__proto__ == b.__proto__ // is: true
```



---

## Numbers and floating point error ##
```
Print( 1000000000000000128 ); // prints 1000000000000000100
```


---

## Number base conversion ( hexadecimal ) ##
```
(255).toString(16); // is: ff

parseInt( 'ff', 16 ) // is: 255

parseInt('0xff'); // is 255
```


---

## try / catch / finally ##
```
try {

} catch(ex) {

	Print('catch')
} finally {

	Print('finally')
}
```
prints:
```
finally
```


---

## Object argument ##
```
var proto = {

	x: function() { return 'x' }
}

var o1 = new Object( proto );
var o2 = new Object( proto );

o1 == o2 // is: true
```



---

## object and its prototype ##
```
function obj() {}

obj.prototype = { x:1 };
var b = new obj;

obj.prototype = { x:2 };
var c = new obj;

c.x == b.x // is: false
```



---

## Runtime prototype object ##
```
var proto1 = {
	a:function(){ return 1 }
}

var proto2 = {
	a:function(){ return 2 }
}

function createObject( proto ) {

	var cl = function() {}
	cl.prototype = proto;
	return new cl;
}

var v1 = createObject( proto1 ).a
var v2 = createObject( proto2 ).a

Print( v1() );
Print( v2() );

Print( createObject( proto1 ).a === createObject( proto1 ).a );
```
prints:
```
1
2
true
```



---

## for in loop and undefined value ##
```
var o = { x:undefined }
for ( var i in o )
	Print(i)
```
prints:
```
x
```



---

## Call function in parent class ##
```
toto.prototype = new function() {

	this.a = function() {

		Print(456)
	}
};


function toto() {

	this.a=function(){

		Print(123)
		toto.prototype.a.call(this); // or: this.__proto__.a();
	}
}

var o = new toto;
o.a();
```
prints:
```
123
456
```



---

## getter and setter function ##
```
abcd getter = function() {

	Print(345);
}

abcd;
abcd;
abcd;
```
prints:
```
345
345
345
```

#### Beware ####
![http://jslibs.googlecode.com/svn/wiki/warning.png](http://jslibs.googlecode.com/svn/wiki/warning.png) The previous syntax used to define a getter is **highly deprecated**, and should be replaced with:
```
__defineGetter__('abcd', function() {

	Print(345);
});
```


---

## defineGetter ( define getter ) ##
```
o = {}
o.__defineGetter__('x', function(){ Print('xxx')} )
o.x
```
prints:
```
xxx
```


---

## check if an object or its prototype has a propery (hasOwnProperty) ##
```
var o = { a:1 }
o.__proto__ = { b:2 }

o.hasOwnProperty('a'); // is: true
o.hasOwnProperty('b'); // is: false
```

check this [article](http://yuiblog.com/blog/2006/09/26/for-in-intrigue/) about hasOwnProperty function



---

## check if a property is enumerable (propertyIsEnumerable) ##
```
var o = { a:1 }

o.propertyIsEnumerable( 'a' ); // is: true

[].propertyIsEnumerable( 'splice' ); // is: false
```



---

## find the getter/setter function of an object ( lookup getter ) ##
```
function test() {

	this.x getter = function(){ Print( 'getter' ) }
	this.x setter = function(){ Print( 'setter' ) }
}

var t = new test
Print( t.__lookupGetter__( 'x' ) );
```
prints:
```
function () {
    Print("getter");
}
```


---

## suppress array element while iterating it ##

the following example will failed :
```
var a = [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ];
for ( var i in a ) {

	if ( a[i] == 1 || a[i] == 2 )
	a.splice( i, 1 );
}
Print( a ); // prints: 0,2,3,4,5,6,7,8,9
Print( a.length ); // prints: 9
```

We can use : ` a[i] == undefined; `
Or start from the end :
```
var a = [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ];

for ( var i = a.length - 1; i >= 0; i-- ) {

	if ( a[i] == 0 || a[i] == 8 )
	a.splice( i, 1 );
}
Print( a ); // prints: 1,2,3,4,5,6,7,9
Print( a.length ); // prints: 8
```


---

## JavaScript arrays ##
```
var a = [ 1,2,3 ];
a.x = 'xxx';

for ( var i = 0; i < a.length; i++ )
	Print('a: '+a[i]);

for ( var i in a )
	Print('b: '+a[i]);
```
prints:
```
a: 1
a: 2
a: 3
b: 1
b: 2
b: 3
b: xxx
```


---

## delete array element ##
![http://jslibs.googlecode.com/svn/wiki/warning.png](http://jslibs.googlecode.com/svn/wiki/warning.png) The following do not work:
```
var ti = [5,6,7,8];
ti.length // is: 4
delete ti[3]
ti.length // is: 4
```

Use 'splice' instead:
```
ti.splice(3, 1);
```



---

![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)





![http://jslibs.googlecode.com/svn/wiki/vspace.png](http://jslibs.googlecode.com/svn/wiki/vspace.png)

| ![http://c25.statcounter.com/counter.php?sc_project=2492374&java=0&security=5656caab&invisible=0&.png](http://c25.statcounter.com/counter.php?sc_project=2492374&java=0&security=5656caab&invisible=0&.png) |  <sub>visitors since 2007.04.25</sub>
