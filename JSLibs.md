> [home](http://code.google.com/p/jslibs/) **>** [JSLibs](JSLibs.md) - [![](http://jslibs.googlecode.com/svn/wiki/source.png)](http://jslibs.googlecode.com/svn/trunk/)

# Executables #
  * [jshost](jshost.md) - command line JavaScript runtime.
  * [jswinhost](jswinhost.md) - Windows JavaScript runtime.

# Modules #
  * [jslang](jslang.md) - Common classes used by other jslibs modules.
  * [jsstd](jsstd.md) - Basic functions set.
  * [jsdebug](jsdebug.md) - Basic debugging and introspection tools.
  * [jsobjex](jsobjex.md) - Hierarchical data storage tool.
  * [jsio](jsio.md) - File and socket support based on [NSPR](http://www.mozilla.org/projects/nspr/) (Netscape Portable Runtime) library.
  * [jssqlite](jssqlite.md) - Database support based on [SQLite](http://www.sqlite.org/).
  * [jsz](jsz.md) - Data compression support using [zlib](http://www.zlib.net/).
  * [jsffi](jsffi.md) - foreign function interface support.
  * [jscrypt](jscrypt.md) - complete cryptographic toolkit based on [libtomcrypt](http://libtom.org/) library.
  * [jswinshell](jswinshell.md) - Basic Windows shell functions support.
  * [jsfastcgi](jsfastcgi.md) - fastcgi protocol support.
  * [jsode](jsode.md) - Rigid body dynamics management based on [ODE](http://www.ode.org/) (Open Dynamics Engine).
  * [jsaudio](jsaudio.md) - Support 2D and 3D sound source and listener using [OpenAL](http://www.openal.org/) library.
  * [jssound](jssound.md) - Support various sound format using [libsndfile](http://www.mega-nerd.com/libsndfile/) and [libvorbis](http://xiph.org/vorbis/) librarye.
  * [jsgraphics](jsgraphics.md) - 3D graphics management using [OpenGL](http://www.opengl.org/) API.
  * [jsimage](jsimage.md) - png and jpg image codec using [libjpeg](http://freshmeat.net/projects/libjpeg/) and [libpng](http://www.libpng.org/pub/png/libpng.html) librarys.
  * [jsfont](jsfont.md) - Support text to image rendering using [FreeType](http://www.freetype.org/) library.
  * [jsprotex](jsprotex.md) - Procedural texture generator.
  * [jssvg](jssvg.md) - render Scalable Vector Graphics (SVG) to an image buffer.
  * [jssdl](jssdl.md) - wraps the Simple DirectMedia Layer (SDL) library API.
  * [jstask](jstask.md) - manage simultaneous JavaScript function execution using threads.
  * [jsjabber](jsjabber.md) - manage XMPP protocol (Jabber Instant Messaging).
  * [jsiconv](jsiconv.md) - conversion between different character encoding.
  * [jstrimesh](jstrimesh.md) - manage triangle based 3D objects.



---


# Notational Conventions #


## myModule::MyClass class <sup>MyParentClass class</sup> ##

### Functions ###
  * <sub>myReturn type</sub> **MyFunction**( myArgument, myOtherArgument [, myOptionalArgument]  )
> > The functions description. _myReturn type_ is the type of data returned by the function
> > #  #
> > _myArgument_ is the first argument of my function.


### Properties ###
  * **myProperty**
> > The property description

  * **myReadOnlyProperty** ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Description...

### Static functions ###

> A static function is only available on the class itself (not its instance) like ` Math.cos(1,23) `

### Static properties ###
> A static function is only available on the class itself (not its instance) like ` Math.PI `

### Constants ###
  * `MY_CONSTANT1`
  * `MY_CONSTANT2`
  * `MY_CONSTANT3`

### Callback functions ###


### examples ###
```

My example of use of this class.

```