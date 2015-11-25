# Introduction #

This page describes the procedure to build jslibs on Linux and Windows platforms.

# Linux Compilation #

## using make ##
This method use libraries provided in jslibs/libs directory.

### prerequisite ###

You need to install the following tools to download and build jslibs:
  * Python
  * zip
  * subversion (to download source files) **optional**
  * gcc and g++ 3 or 4
  * GNU make 3.8+
  * -or- build-essential
  * patch (needed to compile libtomcrypt)
  * autoconf2.13 (needed to compile the JavaScript engine)
  * automake 1.8.2+ (needed to compile ode) **optional**
  * libtool 1.5.22+ (needed to compile ode) **optional**
  * gcc-4.1-multilib (needed to compile on 64-bit platforms) **optional**
  * pkg-config (required by glib configure)
  * gettext (required by glib configure)


### Compilation ###
  1. `svn checkout http://jslibs.googlecode.com/svn/trunk/ jslibs`
  1. `cd jslibs`
  1. `make all`
  1. `make copy`

> _This command will compile jslibs and copy binaries in `./<YOUR_OS_NAME>/` directory._


### Check ###
  * `cd <YOUR_OS_NAME>`
  * `../qalaunch.sh`

## using waf ##
This method uses libraries installed on your system.

### prerequisite ###
  * Same prerequisite as _using make_
  * All external libraries (Debian-based names).
    * `apt-get install libfreetype6-dev`
    * `apt-get install libpng12-dev`
    * `apt-get install libjpeg62-dev`
    * `apt-get install libnspr4-dev`
    * `apt-get install libsqlite3-dev`
    * `apt-get install libtomcrypt-dev`
    * `apt-get install libfcgi-dev`

> ( `apt-get install libfreetype6-dev libpng12-dev libjpeg62-dev libnspr4-dev libsqlite3-dev libtomcrypt-dev libfcgi-dev` )

### Compilation ###
  1. `svn checkout http://jslibs.googlecode.com/svn/trunk/ jslibs`
  1. `cd jslibs`
  1. `./waf configure`
  1. `./waf build`


### Quick test ###

```
./waf install --destdir jslibs_inst
LD_LIBRARY_PATH=$PWD/jslibs_inst/usr/local/lib:$LD_LIBRARY_PATH PATH=$PWD/jslibs_inst/usr/local/bin:$PATH /bin/sh

```



---


# Windows Compilation #

### prerequisite ###

You need to install the following tools to download and build jslibs:
  * [Visual C++ 9 SP1 (VS2008) Express](http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=14597) or Professional
  * [Microsoft Windows SDK for Windows 7 and .NET Framework 3.5 SP1](https://developer.mozilla.org/En/Windows_SDK_versions)
  * [Latest MozillaBuild package](http://ftp.mozilla.org/pub/mozilla.org/mozilla/libraries/win32/MozillaBuildSetup-Latest.exe)) ([info](https://developer.mozilla.org/en/Windows_Build_Prerequisites#MozillaBuild))
  * **optional** [Microsoft DirectX SDK](https://developer.mozilla.org/en/Windows_Build_Prerequisites#Microsoft_DirectX.C2.A0SDK) (needed to compile jsaudio only)


### Configuration ###
  1. check and configure paths in `jslibs\build.cmd`

### Compilation ###
  1. `svn checkout http://jslibs.googlecode.com/svn/trunk/ jslibs`
  1. `cd jslibs`
  1. `build.cmd`
  1. type `build.sh`

### Result ###
  * `dir .\Win32_opt\*.*`

### Check ###
  * `qalaunch.cmd`