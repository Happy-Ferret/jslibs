## How to configure jshost memory usage ? ##

jshost has two arguments that acts on the memory usage:
  * -m 

<size\_in\_MB>

 is used to set the maximum memory (in megabyte) a script can use. Beyond this limit, the script will crash with an "Out of memory" error. (default setting is no limit)
  * -n 

<size\_in\_MB>

 is the number of megabytes a script can allocate before the garbage collector is called. (default setting is no limit)

It is important to note that the value used by -m and -n flags are only related to the memory allocations made internally by SpiderMonkey.
If a native class or function wrapped by a JavaScript object allocates memory, this amount of memory it is not taken in account for -m and -n calculation.
This can result in a bigger memory usage that you have specified with -m and -n flags.

**Note**: when SpiderMonkey allocates system memory, it does not necessarily free it instantly. To enhance performances, it will keep it for later use.
For example, the following script will allocate a lot of memory without returning it to the system:
```
var a=[];
for (var i=0; i<1024*1024; i++)
 a.push([[]]);
a=undefined;
CollectGarbage();
```


## How to avoid loading a module in the global scope ? ##

To achieve this, you have to change the current object of LoadModule's call:
```
var jsstd = {};
LoadModule.call( jsstd, 'jsstd' );
jsstd.Print( jsstd.IdOf(1234), '\n' );
```

This acts as namespaces.

You are free to define your own LoadModule function.
eg.
```
function MyLoadModule( moduleName ) {

  var ns = global[moduleName] = {};
  return LoadModule.call( ns, moduleName );
}
```

## How to configure jshost to run as FastCGI ? ##

#### for Apache web server ####

  1. Used the last build of mod\_fcgi "Version2.1 ( Feb 15th 2007 )" :
> > http://fastcgi.coremail.cn/download.htm
  1. Configured Apache 2.2.4 (httpd.conf) with this:
```
 ...
 LoadModule fcgid_module modules/mod_fcgid.so
 ...
 <Directory "C:/Program Files/Apache Software Foundation/Apache2.2/cgi-bin">
   SetHandler fcgid-script
   Options execCGI
   AllowOverride None
   Order allow,deny
   Allow from all
   FCGIWrapper "C:\fcgitest\jshost.exe C:\fcgitest\fcgi.js" .js
 </Directory>
 ...
```
  1. for the moment, fcgi.js is quite basic and do not process any script:
```
 LoadModule('jsio');
 new File('C:\\fcgitest\\test.txt').content = File.stdin.Read (9999);
```
  1. Created a dummy script in .../Apache2.2/cgi-bin, and when you run this script, fcgi.js is executed via jshost.exe and test.txt is filled with consistent data.
  1. Last, learn this:
> > http://www.fastcgi.com/devkit/doc/fcgi-spec.html

#### for Abyss Web Server ####


> > Host > Configure > Scripting Parameters
> Check "Enable Scripts Execution"

> > Interpreters
> Interface: FastCGI (Local - Pipes)
> Interpreters: C:\...\jslibs\jswinhost.exe  (using an absolute path !)
> Type: Standard
> Associated Extensions: js

### Scripts ###

> Because [jshost](jshost.md) is a generic script host, it needs a FastCGI support script to run FastCGI programs:
```
 LoadModule('jsstd');
 LoadModule('jsfastcgi');
 while ( Accept() >= 0 )
 try {
  Exec(GetParam('SCRIPT_FILENAME'));
 } catch(ex) {
  var errorMessage = ex.name + ': ' + ex.message + ' (' + ex.fileName + ':' + ex.lineNumber + ')';
  Log( 'jslibs: '+errorMessage );
  Print( 'jslibs: '+errorMessage );
  Write( 'Status: 500\r\n\r\n' + errorMessage );
 }
```

> Then, server-side programs looks like:
```
 Write( "Content-type: text/html\r\n\r\n" );
 function CGIVariableList() {
  var fcgiParams = GetParam();
  var list = <ul/>;
  for ( var k in fcgiParams )
   list += <li><b>{k} = </b><pre>{fcgiParams[k]}</pre></li>;
  return list;
 }
 
 Write(
  <html>
   <head><title>teswt</title></head>
   <body>
    <H1>HELLO WORLD</H1>
    <p>CGI/1.1 variables:</p> {CGIVariableList()}
   </body>
  </html>
 );
```

> Note that like in the previous example, the use of E4X can make HTML page easy to generate.
