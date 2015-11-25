## Purpose ##
JavaScript has always been considered as a second-rate language in spite of its powerful features.
These features like exceptions, closures, garbage collection, generator, ..., can make JavaScript an efficient and modern general-purpose scripting language like python and ruby.

The main issue is that there are no or only few ways to execute JavaScript code outside the web browser.
jslibs is based on Spidermonkey, the mozilla's JavaScript engine.
Usually SpiderMonkey is used to enable scripting capabilities of existing projects but not for a standalone development environment.

jslibs is composed of a set of general-purpose libraries and a host to execute JavaScript source files.

The aim of jslibs is to be simple to use, fast, safe and lightweight.


## Modules ##
In jslibs a module is the name given to a dynamically linked library (.dll or .so) that content a set of related classes an functions.
A module can be a simple wrapper to an existing library.

Modules are loaded using the LoadModule() function witch is the only function that is provided by the script host ([jshost](jshost.md) and [jswinhost](jswinhost.md)).


### jslibs modules ###
  * **[jsstd](jsstd.md)** : provide basic development tools ( Print, Exec, Buffer class, ... )
  * **[jsprotex](jsprotex.md)** : provides tools to create Procedural\_textures.
  * **[jswinshell](jswinshell.md)** : basic support of Windows shell API ( CreateProcess, Console, Systray, clipboard, MessageBox, ... )
  * **[jsobjex](jsobjex.md)** : extended JavaScript object that support add, del, get, set listener of a property.


### Wrappers to third party libraries ###
  * **[jsz](jsz.md)** : support of zlib deflate and inflate.
  * **[jssqlite](jssqlite.md)** : support of SQLite database access.
  * **[jsfastcgi](jsfastcgi.md)** : support server-side applications using FastCGI communication with the web-server.
  * **[jsio](jsio.md)** : support of Files and non-blocking TCP/UDP Sockets using the Netscape Portable Runtime (NSPR) library.
  * **[jsode](jsode.md)** : support of dynamics 3D calculation using the ode the Open Dynamics Engine.
  * **[jsimage](jsimage.md)** : support of png and jpeg image format using libpng and libjpeg.
  * **[jsgraphics](jsgraphics.md)** : support of fast 3D transformations (using SSE instructions) and 3D drawing using the OpenGL library. See Vision Factory in links section.
  * **[jsaudio](jsaudio.md)** : support of 3D audio using OpenAL library.
  * **[jscrypt](jscrypt.md)** : support of RSA, AES, blowfish, twofish, ... ciphers using Tom St. Denis' LibTomCrypt and LibTomMath.
  * **[jsffi](jsffi.md)** : support Foreign function interface with libffi.

By default, modules are loaded in the global namespace, however, it is possible to load a module in a custom namespace:
Load a module in the global namespace (default):
```
LoadModule('jssqlite');
...
var db = new Database('myDatabase');
```

Load a module in a custom namespace:
```
var sqlite = {}; // create the namespace
LoadModule.apply(sqlite, 'jssqlite'); // load the module inside the namespace
...
var db = new sqlite.Database('myDatabase');
```


## Standard module ##
SpiderMonkey is only a language library, and, for example, it does not provide any standard input or output access.
To display something on the screen, at least a Print function is needed.

**[jsstd](jsstd.md)** module provide a minimal set of basic programing functions like Print(), Expand(), Seal(), ... .

Another important basic function is Exec() that allows you to load (compile and execute) other scripts.
The Exec() function saves the compiled version of the script to the disk, to speeds up following loads of the same script.
The format used to store the compiled script version is XDR (External Data Representation XDR IETF standard)


## Simplicity of use ##
Using these libraries is very simple and the modules can be load at run-time, this allows a good modularity of the code.
Each module is loaded only once even if you call LoadModule several times with the same argument.

Deflate a string using zlib:
```
LoadModule('jsz');
deflatedText = new Z(Z.DEFLATE)('This text will be deflated', true);
```

Query the version of a sqlite database file:
```
LoadModule('jssalite');
myDatabaseVersion = new Database('myDatabase').Exec('PRAGMA user_version');
```


## Server-Side scripting ##
One interesting feature of jslibs is its server-side scripting capability.
Using jshost as a FastCGI program allow server-side applications development.
see the page [HOWTO](HOWTO.md) for further information.


## Configuration ##
Each module can use the global configuration object (_configuration) to get some informations about the execution environment.
This object is optional an neither module nor host should rely on it._


### Safe mode ###
Internaly, jslibs supports two execution modes: safe mode and unsafe mode.
This mode is transmitted to other modules using the configuration object.

Modules can use this information to make more tests and assertions in its code.
The safe mode is like a run-time debug mode.


### Standard output and error ###
The global configuration object also stores two javascript functions to write, output messages and error messages.
By default, [jshost](jshost.md) bind these functions do stdout and stderr, and jswinhost do not define them.
These functions can be overrided by redefining them. eg.

```
LoadModule('jswinshell');
_configuration.stderr = MessageBox;
LoadModule('jsstd');
```


## Global object ##
The global namespace object is also defined with 'global'


## Using jslibs in a third-party application ##
(TBD)

## supported platforms ##
Currently, the supported platforms are Windows and Linux, and MacOSX soon.


## License ##
jslibs is an open-source project under the GNU GPL2 license.


## External Links ##
  * http://jslibs.googlecode.com/ Official jslibs website
  * http://www.v3ga.net/blog/ Vision Factory project is using jslibs for drawing and scripting.
  * http://code.google.com/p/jsircbot/ An IRC robot made with jslibs.