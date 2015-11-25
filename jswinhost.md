# jswinhost executable #
> [home](http://code.google.com/p/jslibs/) **>** [JSLibs](JSLibs.md) **>** [jswinhost](jswinhost.md) - [![](http://jslibs.googlecode.com/svn/wiki/source.png)](http://jslibs.googlecode.com/svn/trunk/jswinhost/jswinhost.cpp)

### Description ###
> jswinhost (javascript windows host) is a small executable file that run javascript programs under a windows environment.
> The main difference with jshost is the jswinhost does not create any console windows.

### Methods ###

  * status **LoadModule**( moduleFileName )
> > see [jshost](jshost.md)

### Properties ###

  * <sub>Object</sub> **global**  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the global object.

  * <sub>Array</sub> **arguments**  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the host path [0](0.md) and whole command line [1](1.md).

  * <sub>boolean</sub> **isfirstinstance**  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is true ic the current instance is the first one. This can help to avoid jswinhost to be run twice at the same time.

### Configuration object ###

> see [jshost](jshost.md)

### Remarks ###
> There is no way to specify the script to execute using the command line. You have to create a .js file using the name of the host.
> By default, jswinhost.exe is the name of the host and jswinhost.js is the script the host execute.

Because jwinshost do not use a console window, errors and printed messages will not be displayed.

However, you can write your own output system:
```
LoadModule('jswinshell');
_configuration.stdout = new Console().Write;
_configuration.stderr = MessageBox;
LoadModule('jsstd');
Print('toto');
hkqjsfhkqsdu_error();
```

## Embedding JS scripts in your jswinhost binary ##
> see [jshost](jshost.md)