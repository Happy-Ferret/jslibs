# Frequently Asked Questions #

### Does jslibs requires Mozilla Firefox ? ###
> No, jslibs is stand-alone and only requires spidermonkey, the Mozilla's javascript engine. Spidermonkey (js32.dll) is provided within any jslibs package.

### Is there a Linux version of jslibs available ? ###
> Yes, you just have to checkout the current source code and type make all. All modules do not run on Linux (like jswinshell and jswinhost). To have a complete list about witch modules are available under Linux, see the [status](Status.md) page. If you have issues about building jslibs under Linux, please refer [this page](jslibsBuild.md) or create a new [issue](http://code.google.com/p/jslibs/issues/list).

### How to contribute to this project ? ###
> Contact me at [soubok+jslibs@gmail.com](mailto:soubok+jslibs@gmail.com).

### What skill are you looking for ? ###
  * Javascript programmer ( for coding demos, frameworks, tools, tests )
  * C/C++ programmer ( wrapping new libraries, enhance existing ones )
  * Linux/unix port ( makefiles, compilation, tests )

### Why did you release jslibs under GPL license ? ###
> Because I think it is the best way to get contributions to the project.

### Can I test jslibs ? ###
> Yes, you can download a jslibs packages in the [download](http://code.google.com/p/jslibs/downloads/list) section or in the [project group](http://groups-beta.google.com/group/jslibs). Any feedback will be greatly appretiated ! Contact me at [soubok+jslibs@gmail.com](mailto:soubok+jslibs@gmail.com).

### How can I give feedback about jslibs ? ###
> By e-mail : [soubok+jslibs@gmail.com](mailto:soubok+jslibs@gmail.com).
> #  #
> Or by posting in the group : http://groups-beta.google.com/group/jslibs ( jslibs@googlegroups.com ).

### Where should I report bugs or enhancements ? ###
> Go in the [Issues](http://code.google.com/p/jslibs/issues/list) section and click [New Issue](http://code.google.com/p/jslibs/issues/entry).

### What is the aim of this project ? ###
> The first aim of this project is to prove programmers how much powerful Javascript language is. Using Javascript in a web page is quite nice but this don't show it as a versatile language.
> > "By 2011, we will recognize JavaScript as a language with a better set of features for developing modern applications." - Stuart Halloway <sub>(</sub>[...](http://www-128.ibm.com/developerworks/java/library/j-cb12196/)<sub>)</sub>
#  #

> The second aim of this project is to prepare the basis of a 3D game engine. One Year ago I started to create a 3D engine in C++ but the code became complex and not flexible enough. This is why I try with this new approach :
> > Everything that is not time-critical can be written in Javascript.