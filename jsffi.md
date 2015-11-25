# jsffi module #
> [home](http://code.google.com/p/jslibs/) **>** [JSLibs](JSLibs.md) **>** [jsffi](jsffi.md) - [![](http://jslibs.googlecode.com/svn/wiki/source.png)](http://jslibs.googlecode.com/svn/trunk/jsffi/ffi.cpp)

## Description ##

The original name of this project was **JSNI** (JavaScript Native Interface).
It has been renamed into **jsffi** to remain consistent with other modules naming.

jsffi is a module that allow you to call any symbol in a native module (dll/so).
This project is a simple file that you can link with any spidermonkey embeding project.

### Example 1 ###

In this example, jsffi call the symbol MessageBoxA in user32.dll :

```
function MyAlert( text, caption ) {

	var ret = new NativeData;
	ret.PU32.Alloc();
	new NativeModule( 'C:\\WINDOWS\\SYSTEM32\\User32').Proc('MessageBoxA')( ret.PU32, DWORD( 0 ), SZ(text), SZ( caption || 'Alert' ), DWORD( 1 ) );
	return ret.PU32[0];
}

MyAlert('Bonjour tout le monde.', 'message');
```

note: DWORD, SZ, ... are defined [here](http://jslibs.googlecode.com/svn/trunk/jsffi/jsffi.js).

### Example 2 ###

note: some casts are missing in the c-style code representation

goal: create an array of 2 strings

```
// creates the root of the native data structure
// void *nd;
var nd = new NativeData();

// casts the NativeData object to a pointer, and allocate the space to store 2 pointer ( .Alloc(1) is equivalent to .Alloc() )
// nd = new void*[2];
nd.PP.Alloc(2);

// access to the first pointer
// nd[0] = new char[10];
nd.PP[0].PS8.Alloc(10);

// fills the first string with "test" ( for a more simple syntax, see example 2 )
// ((char*)nd[0])[0] = "t";
// ((char*)nd[0])[1] = "e";
// ((char*)nd[0])[2] = "s";
// ((char*)nd[0])[3] = "t";
// ((char*)nd[0])[4] = 0; 
nd.PP[0].PS8[0] = 't';
nd.PP[0].PS8[1] = 'e';
nd.PP[0].PS8[2] = 's';
nd.PP[0].PS8[3] = 't';
nd.PP[0].PS8[4] = '\x00';

nd.PP[1].PS8[0] = '\x00';

...call( ret.VOID, nd.PP );
```

### Example 3 ###

goal: same as example 2 but with a more advanced syntax.

```
var ndpp = new NativeData().PP;
nd.PP.Alloc(3);
nd.PP[0].String = "test";
nd.PP[1].String = ...
```


### Tools functions ###

These functions allows a simpler handling of usuals native types:

```
function NULL() {

	var nat = new NativeData().PP;
	nat.Alloc()[0] = 0;
	return nat;
}

function DWORD( value ) {

	var nat = new NativeData().PU32;
	nat.Alloc()[0] = value;
	return nat;
}

function BYTE( value ) {

	var nat = new NativeData().PU8;
	nat.Alloc()[0] = value;
	return nat;
}

function SZ( value ) {

	var nat = new NativeData;
	nat.String = value;
	return nat.PP;
}

function VOID( value ) {

	return new NativeData().VOID;
}

function PDATA( size, initByte ) {

	var nat = new NativeData().PP;
	nat.Alloc()[0].PS8.Alloc( size, initByte );
	return nat;
}
```

### Call a native function in a dynamic library (.dll/.so) ###

```
function CreateProcess( fileName, commandLine ) {

	ret = new NativeData()
	ret.PU32.Alloc();
	new NativeModule('C:\\WINDOWS\\SYSTEM32\\kernel32').Proc('CreateProcessA')( ret.PU32, SZ( fileName ), SZ( commandLine || "" ), NULL(), NULL(), DWORD( 0 ), DWORD( 0x20 ), NULL(), NULL(), PDATA(68, 0), PDATA(16) );
	return ret.PU32[0] == 1;
}

function Sleep( time ) {

	if ( !Sleep.proc ) { // cache NativeData objects

		Sleep.proc = new NativeModule('C:\\WINDOWS\\SYSTEM32\\kernel32').Proc('Sleep');
		Sleep.arg1 = new NativeData()
		Sleep.arg1.PU32.Alloc()[0] = time
		Sleep.ret = new NativeData().VOID;
	}
	Sleep.proc( Sleep.ret, Sleep.arg1.PU32 );
}
```

# Quick reference #

### NativeData object ###
  * **VOID**
> > getter that cast the [NativeData](NativeData.md) object to `void`
  * **PI**
> > getter that cast the [NativeData](NativeData.md) object to a `int*` (system dependant !)
  * **PU8**
> > getter that cast the [NativeData](NativeData.md) object to a `unsigned char*`
  * **PS8**
> > getter that cast the [NativeData](NativeData.md) object to a `char*`
  * **PU16**
> > getter that cast the [NativeData](NativeData.md) object to a `unsigned short*`
  * **PS16**
> > getter that cast the [NativeData](NativeData.md) object to a `short*`
  * **PU32**
> > getter that cast the [NativeData](NativeData.md) object to a `unsigned long*`
  * **PS32**
> > getter that cast the [NativeData](NativeData.md) object to a `long*`
  * **PU64**
> > getter that cast the [NativeData](NativeData.md) object to a `...`
  * **PS64**
> > getter that cast the [NativeData](NativeData.md) object to a `...`
  * **PP**
> > getter that cast the [NativeData](NativeData.md) object to a `void**`
  * **String**
> > getter/setter that accept string with terminal '\0' ( C string )

### NativeType object ###
  * **Alloc**()
> > allocates one element of the type specified by it's parent,
  * **Alloc**( size )
> > allocates _size_ element of the type specified by it's parent,
  * **Alloc**( size, init )
> > allocates _size_ element of the type specified by it's parent,
> > and initialize the memory with _init_ value.
  * **Free**()
> > force an allocation to be freed,
  * **[**index**]**
> > getter/setter that allow to access the _index_ th element of the array.
> > There is a special treatment for `PP[]`,
> > getter/setter get and return a [NativeData](NativeData.md) object ( see advances examples )
> > or get a pointer value ( see NULL() ).

### NativeModule object ###
  * <sub>constructor</sub> **NativeModule**( fileName, closeType )
> > Open a handler to a dynamic linked library ( _fileName_ without .dll/.so, it is automaticaly added ).


> _closeType_ :
    * true: close the library when the object is GC'ed.
    * false/undefined: close the library on exit.

  * void **Close**()
> > release the dynamic linked library handler.

  * [NativeProc](NativeProc.md) **Proc**( symbol )
> > Once opened, you can acces a _symbol_ in the dll using it's name (String) or ordinal number (Number).


> Example:
```
  lib.Proc('AddOne')( VOID(), pi );
```
> or
```
  lib.Proc(5)( VOID(), pi );
```

### NativeProc objet ###
  * **call operator**( returnValue [, arg1 [, arg2 [, ... ] ] ] )
> > the call operator calls the symbol.


## Advanced examples ##

A C function in a dll/so :
```
__declspec(dllexport) void AddOne( int *i ) {

	*i = *i + 1;
}
```

then, the javascript code to call this function :
```
 var i = new NativeData();
i.PI.Alloc();
i[0] = 123;

...
  var pi = new NativeData().PP.Alloc();
  pi[0] = i;

...
lib.Proc('AddOne')( VOID(), pi );

Print( i[0] ); // returns 124
```


### Advanced informations ###

[NativeData](NativeData.md) internal:
  * parent  : <not used>
  * private : pointer to `[` the pointer to the data `]`
  * slot[0](0.md) : root NativeData Object ( this is used when we have to store pointers that have to be freed )

[NativeType](NativeType.md) internal:
  * parent  : it's [NativeData](NativeData.md) ( `nd.PP[0]` ==> `nd.PP.0` ==> `NativeData.NativeType.NativeData` )
  * private : &ffi\_type_...
  * slot[0](0.md) : reference to a [NativeData](NativeData.md) object, only in the case `PP[0]` = nativeData object..._

[NativeProc](NativeProc.md) internal:
  * parent  : reference to it's [NativeModule](NativeModule.md) object
  * private : <not used>
  * slot[0](0.md) : name of the proc to call

[NativeModule](NativeModule.md) internal:
  * parent  : <not used>
  * private : pointer / handle to the loaded library
  * slot[0](0.md) : <not used>

### Lib dependances ###

  * spidermonkey
  * libffi / libffi\_msvc


### Notes ###

the memory allocated with .Alloc is automaticaly freed when the root NativeData object is released / garbage collected

```
char  	1  	character or integer 8 bits length.  	signed: -128 to 127 unsigned: 0 to 255
short 	2 	integer 16 bits length. 	signed: -32768 to 32767 unsigned: 0 to 65535
long 	4 	integer 32 bits length. 	signed:-2147483648 to 2147483647 unsigned: 0 to 4294967295
int 	* 	Integer. Its length traditionally depends on the length of the system's Word type, thus in MSDOS it is 16 bits long, whereas in 32 bit systems (like Windows 9x/2000/NT and systems that work under protected mode in x86 systems) it is 32 bits long (4 bytes). 	See short, long
float 	4 	floating point number. 	3.4e + / - 38 (7 digits)
double 	8 	double precision floating point number. 	1.7e + / - 308 (15 digits)
long double 	10 	long double precision floating point number. 	1.2e + / - 4932 (19 digits)
bool 	1 	Boolean value. It can take one of two values: true or false NOTE: this is a type recently added by the ANSI-C++ standard. Not all compilers support it. Consult section bool type for compatibility information. 	true or false
wchar_t 	2 	Wide character. It is designed as a type to store international characters of a two-byte character set. NOTE: this is a type recently added by the ANSI-C++ standard. Not all compilers support it. 	wide characters
```


### Some links ###

**spidermonkey**

> home
> > http://www.mozilla.org/js/spidermonkey/

> developer mozilla
> > http://developer.mozilla.org/en/docs/JSAPI_Reference

> updated API reference
> > http://www.sterlingbates.com/jsref/sparse-frameset.html

> search in files:
> > http://lxr.mozilla.org/mozilla/

**libffi**

> http://sources.redhat.com/libffi/

**libffi\_msvc**
> old
> > http://sourceforge.net/projects/ctypes/

> new
> > http://svn.python.org/view/ctypes/trunk/ctypes/source/libffi_msvc/ ( svn checkout http://svn.python.org/projects/ctypes/trunk/ctypes/source/libffi_msvc )

**ffi for php5**

> http://pecl.php.net/package-info.php?package=ffi

**other x86 port**
> http://pnet.nu6.org/scvs/module_libffi_src_x86.html



### My TODO list ###

  1. add offset capability to data ( or extend .Data() to support an offset in the array )
  1. allow to test if a pointer is null without using casting
  1. add a method to read not zero-ended strings  ( like .Data(100) )
  1. check and apply GT tips ( http://www.mozilla.org/js/spidermonkey/gctips.html )
  1. port all of this on linux
  1. manages natives structures
> > really needed ? libffi seems to not implement the access to a native structure data
  1. var nd = new NativeData( [[ "test" ](.md), [""] ] );
> > the same result can be done in javascript
  1. NativeModule.onUse
> > allow the dll to be load later ( lazy ... )
  1. create a fast mode, where no checks are done ( datatype, ... )