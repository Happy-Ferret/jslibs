# jshost executable #
> [home](http://code.google.com/p/jslibs/) **>** [JSLibs](JSLibs.md) **>** [jshost](jshost.md) - [![](http://jslibs.googlecode.com/svn/wiki/source.png)](http://jslibs.googlecode.com/svn/trunk/jshost/jshost.cpp)

### Description ###

jshost ( javascript host ) is a small executable file that run javascript programs.
The main features are:
  * Lightweight
> > The binary executable file is less than 60KB
  * Minimalist internal API
> > LoadModule is enough, everything else can be added using dynamic loadable modules.

### Command line options ###
  * `-c <0 or 1>` (default = 0)
> > Compile-only. The script is compiled but not executed. This is useful to detect syntax errors.
  * `-u` (disabled by default)
> > Run in unsafe-mode that is a kind of 'release mode'. In unsafe-mode, any runtime checks is avoid and warnings are not reported. This mode allow a better execution speed.
  * `-m <size>` (default: no limit)
> > Specifies the maximum memory usage of the script in megabytes.
  * `-n  <size>` (default: no limit)
> > Specifies the number of allocated megabytes after which garbage collection is run.
  * `-g <time>` (default = 60)
> > This is the frequency (in seconds) at witch the GarbageCollector may be launched (0 for disabled).
  * `-l <case>` (default = 0)
> > This is a temporary option that allow to select function name naming. 0:default, 1:lowerCamelCase, 2:UpperCamelCase
> > ##### node: #####
> > > Default is UpperCamelCase for jslibs version < 1.0 and lowerCamelCase for jslibs version >= 1.0

> > ##### example: #####
```
  loadModule('jsio');
  loadModule('jsstd');
  var f = new File( arguments[0] );
  f.open('r');
  print( f.read(100) );
```

### Exit code ###
  * The exit code of jshost is 1 on error. On success, exit code is the last evaluated expression of the script.
> > If this last expression is a positive integer, its value is returned, in any other case, 0 is returned.
  * If there is a pending uncatched exception and if this exception can be converted into a number (see valueOf), this numeric value is used as exit code.

> ##### example: #####
```
 function Exit(code) {
  throw code;
 }

 Exit(2);
```

### Global functions ###
  * status **LoadModule**( moduleFileName )
> > Loads and initialize the specified module.
> > Do not provide the file extension in _moduleFileName_.
> > ##### example: #####
```
  LoadModule('jsstd');
  Print( 'Unsafe mode: '+configuration.unsafeMode, '\n' );
```
> > ##### note: #####
> > You can avoid LoadModule to use the global object and load the module in your own namespace:
```
  var std = {};
  LoadModule.call( std, 'jsstd' );
  std.Print( std.IdOf(1234), '\n' );
  std.Print( std.IdOf(1234), '\n' );
```

### Global properties ###

  * **arguments**
> > The command-line arguments (given after command line options).
> > ##### example: #####
```
  for ( var i in arguments ) {

   Print( 'argument['+i+'] = '+arguments[i] ,'\n' );
  }
```
> > <pre>
...<br>
c:\>jshost -g 600 -u foo.js bar<br>
argument[0] = foo.js<br>
argument[1] = bar<br>
</pre>

  * **endSignal**
> > Is _true_ if a break signal (ctrl-c, ...) has been sent to jshost. This event can be reset.

### Configuration object ###

> jshost create a global `_configuration` object to provide other modules some useful informations like `stdout` access and `unsafeMode` flag.

## Remarks ##

### Generated filename extensions are ###
  * ".dll" : for windows
  * ".so" : for linux

### Modules entry points signature are ###
| `"ModuleInit"` | `JSBool (*ModuleInitFunction)(JSContext *, JSObject *)` | Called when the module is being load |
|:---------------|:--------------------------------------------------------|:-------------------------------------|
| `"ModuleRelease"` | `void (*ModuleReleaseFunction)(JSContext *cx)`          | Called when the module is not more needed |
| `"ModuleFree"` | `void (*ModuleFreeFunction)(void)`                      | Called to let the module moke some cleanup tasks |


### Example (win32) ###
```
extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

 InitFileClass(cx, obj);
 InitDirectoryClass(cx, obj);
 InitSocketClass(cx, obj);
 InitErrorClass(cx, obj);
 InitGlobal(cx, obj);

 return JS_TRUE;
}

== Embedding JS scripts in your jshost binary ==
 This can only be done at jshost compilation time.
 # Checkout [http://code.google.com/p/jslibs/source/checkout jslibs sources]
 # Save your embbeded script in the file _jslibs/src/jshost/embeddedBootstrapScript.js_
 # [jslibsBuild Compile jslibs] (or only jshost if jslibs has already been compiled once)
```