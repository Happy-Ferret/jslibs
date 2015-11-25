<pre>
0        1         2         3         4         5         6         7         8<br>
12345678901234567890123456789012345678901234567890123456789012345678901234567890<br>
</pre>

## jslibs devsnapshot [r2702](https://code.google.com/p/jslibs/source/detail?r=2702) ##
<pre>

jsgraphics:<br>
- Add static functions: FrustumSphere(), BoxToCircumscribedSphere() and several Vector3D handeling functions.<br>
- Add Transformation.Scale().<br>
- Add Ogl.MultMatrix(), .ColorMaterial(), .LookAt(), .DrawSphere(), .DrawPoint(), .DrawDisk(), .PixelWidthFactor(), .KeepTranslation(), .DrawBuffer(), .ReadBuffer(), .StencilFunc(), .ColorMask().<br>
- Add target argument in Ogl.CopyTexImage2D().<br>
- Add compileOnly argument to Ogl.NewList().<br>
- Add mode argument to Ogl.DrawTrimesh().<br>
<br>
jsode:<br>
- Improve object management system. Now JavaScript objects are only wrappers to ode objects. If a JavaScript object is GCed, its ode object is not destroyed.<br>
- Improve collision system. In impact callback function, replace depth with velocity (that is the velocity projection onto the impact normal).<br>
- Change Step() argument unit from s to ms.<br>
- Add JointGroup, JointPiston, JointUniversal, JointAMotor, JointLMotor, GeomCylinder classes.<br>
- Add methods Body.AddTorque(), JointPlane.ResetBody(), JointSlider.AddForce(), World.ScaleImpulse().<br>
- Add properties GeomPlane.param property (get and set), JointBall.anchor2 (get and set), Geom.aabb property (get), Goem.boundarySphere (get).<br>
- Add Geom.disabled and remove Geom.enable properties.<br>
- Add support of group 2 and 3 joint parameters.<br>
<br>
jsprotex:<br>
- Rename ForEachPixels() into ForEachPixel(). Enhance memory management.<br>
- Move RandSeed(), RandInt(), RandReal() from Texture class to static scome.<br>
- Add static functions PerlinNoise(), PerlinNoiseReinit().<br>
- Add methods Texture.AddPerlin2() (for KEN PERLIN IMPROVED NOISE), .Dilate(), .GetBorderLevelRange(), .ApplyColorMatrix(), GetPixelAt(), GetLevelRange().<br>
- Change ForEachPixel() argument order.<br>
- Manage borderMode in GetPixelAt().<br>
- Texture.Add() and Texture.Mult() now supports 4-channels against 1-channel textures.<br>
<br>
jssdl:<br>
- Allow no parameter in PollEvent().<br>
- Fix onMouseButtonDown event.<br>
- Add BUTTON_* constants.<br>
<br>
jsdebug:<br>
- Add processTime and cpuLoad static properties (win32 only).<br>
- Remove DumpStats() static function.<br>
<br>
other modules:<br>
- jswinshell: add RegistryGet() API function.<br>
- jstrimesh: now trimesh objects can be used to draw points only.<br>
- jsstd: allow JIT in SandboxEval().<br>
- jsio: add reverse DNS lookup support through GetHostsByAddr() API function.<br>
- jsaudio: add OalEffectSlot class (link effects and sources).<br>
<br>
other modifications:<br>
- Add jsvideoinput module to manage video input devices (for windows only).<br>
- Enable low fragmentation heap on Windows.<br>
- Add liveconsole.xul for live programming.<br>
- jshost: allow maybeGCInterval (-g option) to be a float value.<br>
- jstrimesh: update Blender exporter.<br>
- JavaScript debugger: show JavaScript assembly in the debugger client (in DEBUG version only).<br>
<br>
</pre>

## jslibs version 0.95 [r2572](https://code.google.com/p/jslibs/source/detail?r=2572) ##
<pre>

New modules:<br>
- jssvg - render Scalable Vector Graphics (SVG) to an image buffer.<br>
- jssdl - wraps the Simple DirectMedia Layer (SDL) library API.<br>
- jstask - manage simultaneous JavaScript function execution using threads.<br>
- jsjabber - manage XMPP protocol (Jabber Instant Messaging).<br>
- jsiconv - conversion between different character encoding.<br>
- jstrimesh - manage triangle based 3D objects.<br>
- jsvst - a new host that can be load as a VST plug-in.<br>
<br>
Enhancements:<br>
- use TraceMonkey as JavaScript engine.<br>
- add a JavaScript debugger written in XUL (runnable in Firefox).<br>
- complete OpenAL API.<br>
- add OpenAL EFX (Effects Extension) support.<br>
- add a static property '_revision' to each jslibs class.<br>
- jshost: add -g option to control GC call interval (default is 15 seconds).<br>
- jshost: add -l option to control function name upper or lower camel case.<br>
- enhance Blob and String behavior similarity.<br>
- jshost exit code can be throw from the JavaScript code (see issue#69)<br>
- add _NI_BufferGet support to mutated Blob objects.<br>
<br>
API and behavior modifications:<br>
- remove the Window class from jsgraphics. jssdl module must be use instead.<br>
- jsobjex module has been merged into jsstd module.<br>
- jssqlite: support "for each..in" loop on database results.<br>
- jsio: default mode for Open() finction is read+write for user and group.<br>
- jsio: rename currentWorkingDirectory into currentDirectory.<br>
- jsio: rename pathSeparator into directorySeparator.<br>
- jsio: rename listSeparator into pathSeparator.<br>
- jsio: add the AvailableSpace() static function.<br>
- jsio: add numberOfProcessors static property.<br>
- jsio: add static listSeparator and pathSeparator properties.<br>
- jsio: add the class Process to replace CreateProcess() static function.<br>
- jsio: make currentWorkingDirectory property read/write.<br>
- jsio/Semaphore: add constants values for the 'mode' argument of the constructor.<br>
- jsstd: remove HideProperties(), add SetPropertyEnumerate(), SetPropertyReadonly() functions.<br>
- jsstd: add IsBoolean(), IsPrimitive(), IsFunction(), IsVoid() functions.<br>
- jsstd: replace IdOf and FromId with ObjectToId and IdToObject and manage GCed objects.<br>
- jsstd: add XdrEncode() and XdrDecode() static functions.<br>
- jsstd: replace Sandbox.Eval() function with a static SandboxEval() function.<br>
- jsstd: rename ASSERT() into Assert().<br>
- jsstd: add a callback support to the Expand() function.<br>
- jslang/Blob: add Free(wipe) method.<br>
- jslang/Blob: add substring() method.<br>
- jsprotex/Texture::Convolution(): remove the autoGain argument.<br>
- jsprotex/Texture: add ForEachPixels() and LevelRange() methods.<br>
- jsgl: add LoadTrimesh() and DrawTrimesh() to manage trimesh drawing using VBO.<br>
- jsgraphics/Transformation: add RotationFromQuaternion() function.<br>
- jsaudio: add Open() and Close() static function to manage OpenAL initialization.<br>
- jsaudio: add OalBuffer, OalSource and OalListener classes.<br>
- jssound: add SoundFileDecoder and OggVorbisDecoder classes to manage sound decoding streaming.<br>
- jssound: add SplitChannels() static function.<br>
- jswinshell: add RegistryGet() function to retrieve values from the Windows registry.<br>
- jsz: make inflate and deflate objects reusable. Remove eof property. Add Z.BEST_SPEED and Z.BEST_COMPRESSION constants.<br>
- make 'undefined' value immutable.<br>
- add exception location through fileName and lineNumber properties for each jslibs error class (safe mode only).<br>
- _configuration.unsafeMode is now read-only.<br>
- rename global class name (not the property) from "global" into "Global".<br>
<br>
Fixed bugs:<br>
- fix issue#59 - jssqlite crash.<br>
- fix issue#61 - SharedMemory not closed properly.<br>
- fix jsfont/Font .ascender, .descender and .width properties.<br>
- fix jsio/Descriptor::Import() behavior.<br>
- fix jsprotex/Texture::Normal() arguments.<br>
- fix jsprotex/Texture::CopyTexImage2D() behavior.<br>
- fix jsprotex/Texture constructor when importing an image object.<br>
- fix jsprotex/Texture::AddCracks() when variation argument is missing.<br>
- fix jslang/blob::substr() function.<br>
- fix jslang/blob::toSource()/::uneval() functions.<br>
- fix jswinshell/CreateProcess() function.<br>
- fix jswinshell/ExtractIcon() function.<br>
- fix a bug about missing _NI_BufferGet on non-constructed jslang/Blob.<br>
- fix several memory leaks.<br>
- fix several GC hazard.<br>
<br>
...among 225 fixes and 137 enhancements.<br>
<br>
</pre>

## jslibs version 0.9 [r1881](https://code.google.com/p/jslibs/source/detail?r=1881) ##
```
New modules:
-----------
jslang:
  Automatically loaded by jshost and jswinhost (LoadModule call is not needed).
  This new module contains all common classes used by other jslibs modules.

jssound:
  Support wav, aiff, au, voc, sd2, flac, ... sound format using libsndfile 
  and ogg vorbis using libogg and libvorbis.

jsaudio:
  Support 2D and 3D sound source and listener using OpenAL library.

jsfont:
  Support text rendering (text to image) from the following font format:
  TrueType, Type 1, CID-keyed Type 1, CFF, OpenType TrueType, OpenType CFF,
  SFNT-based bitmap, X11 PCF, Windows FNT, BDF, PFR, Type 42


New classes:
-----------
jssound/<static> class: sound decoding.
  functions: DecodeSound, DecodeOggVorbis

jsaudio/Oal class: sound playback.
  functions: PlaySound

jslang/Blob class: immutable binary data support. May self transform into a String object.
  functions: <constructor>, [] operator, concat, substr, valueOf, toString, indexOf, lastIndexOf, charAt
  properties: length

jslang/Stream class: String/Blob to readable stream adapter.
  functions: <constructor>, Read
  properties: position, available, source

jsfont/Font class: text rendering.
  functions: <constructor>, DrawString, DrawChar
  properties: poscriptName, size, useKerning, horizontalPadding, verticalPadding, letterSpacing, italic, bold, encoding, ascender, descender, width

jsio/MemoryMapped class: memory mapped file.
  functions: <constructor>
  properties: file

jsstd/Sandbox class: protected context to eval a script.
  static function: Eval


API and behavior modifications:
------------------------------
global:
 - disabling Javascript "Stript" object.
 - make jslibs classes read-only and permanent.
 - most jslibs API use Blob as resulting string value or '' for empty string value.
 - rename Data class into Map.

jsio/<static>:
  - remove timeout for IsReadable and IsWritable.
  - add currentWorkingDirectory property.
  - CreateProcess() function adds NIStreamRead support to the output pipe.
jsio/Directory class:
  - Add Directory.SKIP_FILE, SKIP_DIRECTORY and SKIP_OTHER constants for List() function.
  - Make() function uses 766 permission on Linux (issue#37).
jsio/Socket class:
  - GetHostsByName() returns an empty array when the directory lookup on a network address has failed.
jsio/SharedMemory class:
  - add NIBufferGet support.
  - add Close() method.
jsio/File class:
  - add 'position' property (f. seek() function).
  - add Move() method.

jssqlite:
  - replace Blob class with Blob class.
  - Upgrade to sqlite 3.5.9, use sqlite3_open_v2, sqlite3_prepare_v2 functions.
  - Enable sqlite3_enable_shared_cache.
jssqlite/Database class:
  - construction without arguments creates a :memory: database.
  - construction using an empty string as argument creates a temporary database.
  - add .memoryUsed static property.

jsgraphics:
  - rename Gl class into Ogl static class (like Math).
jsgraphics/Ogl class:
  - supports all OpenGL functions and some extensions like multi textures, ...
  - add all OpenGL constants.
  - Remove some non-primitive functions (Axis, Quad, Line, Cube, Init).
  - Add RenderToImage() function
  - GetDouble() and GetInteger() can return an array.
  - supports NI_READ_MATRIX44.
  - Merge CallList() and CallLists() functions.
jsgraphics/Transformation class:
  - rename RevertProduct() with ReverseProduct()
  - allow transformation matrix element access via [] operator.
  - TransformVector() function accepts a 3 or 4 item length vector.
  - enable Translate() function.
jsgraphics/Window class:
  - Add clientRect property.
  - ProcessEvents is no longer an infinite loop ( see Open() and Close() functions )
  - remove Exit() function.

jsimage:
  - replace Jpeg an Png classes with DecodePngImage() and DecodeJpegImage() static functions.
  - add EncodePngImage() static function.

jsstd:
  - add StringRepeat() function.
  - add FromId() function that is the reciprocal of IdOf() function.
  - add disableGarbageCollection property.
jsstd/Buffer:
  - replace 'onunderflow' callback with an argument of the constructor.
  - add property getter ([] operator) to access the Nth char.
  - Write() method supports a Buffer object as argument.

jsprotex/Texture class:
  - constructor supports Blob image.
  - stop curveData support Mult() function.
  - add PixelAt() to read a pixel value.
  - Support Blob as InitCurveData.
  - add Import() and Export() functions to support Blob images.

jswinhost:
  - global property 'argument' has been renamed into the array 'arguments'.

jswinshell:
  - prefix all modal dialog box style constants with MB_
  
jsdebug:
  - add LocateLine() static function.


Miscellaneous:
-------------
- Add a QA framework (cf. qarun.js).
- Add a documentation generator system (cf. doc.js).
- use FAST_NATIVE functions for jsprotex/texture, jsgl/Ogl, jstransformation/Transformation.
- jshost: check reportWarnings status dynamically from the global configuration object.
- jshost: enable the "LAZY_STANDARD_CLASSES" mechanism.
- jsprotex: choose fast floating point option for windows build.
- jssqlite: enhance error management in bound functions.


Bugs fix:
--------
- jscrypt: Hash, Cipher, Prng initialization in unsafe mode.
- jscrypt/AsymmetricCipher: Don't finalize key if the key have not been created.
- jscrypt/AsymmetricCipher::Decrypt() "lparam" management.
- jsio/<static>::CreateProcess()
- jsio/Descriptor::closed property condition.
- jsio/Socket::TransmitFile() headers management.
- jssqlite/Database::Exec() miss to close the statement in some cases.
- jssqlite: issue#41 - too many SQL statement
- jsprotex/Texture::InitCurveData() function. 
- jsprotex/Texture::Aliasing() function.
- jsstd/Expand() function.
- jswinshell/Console: fix console stdin/stdout handlers management.

```


## jslibs devsnapshot 2008.03.15 [r1402](https://code.google.com/p/jslibs/source/detail?r=1402) ##
```
- Documentation has been updated.
- Several memory leak fix and performance enhancements.
- jsimage.dll is back.
- Enable DESTRUCTURING_SHORTHAND SpiderMonkey option.

- jshost
  - Fix a bug that avoid any finalize to be called at exit.

- jsstd
  - Fix bug in Print function.
  - Change IsConstructing function into isConstructing property.
  - Add InternString function.
  - fix a bug in Buffer::Read function.

- jsio
  - Optimize descriptor allocation and fix a bug in Poll function.
  - Propagates exceptions throws in Poll function events callbacks.
  - Add SharedMemory class.
  - Fix a bug in File class that make jsio to crash when unloaded at exit.
  - Add CreateProcess function.
  - Add Semaphore class.
  - Fix Directory error management.
  - Add Directory.List function to list all entries at once.
  - Add systemInfo property that contains architecture, name and release information.
  - Change HostName function into hostName property.
  
- jscrypt
  - Renaming crypt* into cypher* (except Enctypt and Decrypt JS functions)
  - Manages all encryption modes: ecb, cfb, ofb, cbc, ctr, lrw, f8
  - Add AsymmetricCipher class to manage RSA, ECC, DSA asymmetric ciphers. (Rsa class is now merged into AsymmetricCipher)
  - Enhance Hash.list, Prng.list and Cipher.list static properties that now returns more information.
  - Fix prng argument in CreateKeys function.
  - Add optional numRound argument to cipher constructor.
  - Add Prng::state property to manage inport/export.
```

## jslibs devsnapshot 2008.02.26 [r1315](https://code.google.com/p/jslibs/source/detail?r=1315) ##
```
- in jshost:
  - remove -w, -s flags from the command line.
  - add -c command line option for only compiling the script.
  - by default, memory usage is unlimited but maxMem and maxAlloc are configurable with -m and -n command line flags (use megabytes as unit).
  - the global property 'endSignal' is retriggerable (it is set to <true> each time a break signal is detected)
  - on win32, endSignal is trigged by: CTRL_C_EVENT, CTRL_BREAK_EVENT, CTRL_CLOSE_EVENT, CTRL_LOGOFF_EVENT, CTRL_SHUTDOWN_EVENT.
  - reset signals at exit (SIG_DFL)
  - stop using BranchCallback limit to break the script execution but enable BranchCallback to run JS_MaybeGC periodically.
  - enhance module absolute path detection (arguments[0]).
  - in unsafe mode (-u 1 command line option), show only errors (no warnings) and stop reporting exceptions.
  - stop using atexit when jshost exists.
  - call the GC before compiling a JS file.
  - fix executable file name detection (argv[0]).

- in jsstd module:
  - Do not failed when Clear() something that is not an object, just return <false>.
  - disable NativeInterface for Buffer class to fix some memory leaks.
  - Pack class supports host/big/little endian conversion.
  - add TimeCounter(), a very accurate time counter (win32 only).
  - add MaybeCollectGarbage(), that call the GC only if needed.
  - fix some memory leaks in Buffer class.
  - fix issue #26 (crash in std:Buffer class).
  - in Buffer, avoid empty chunks to be added in the chunk list (avoid waste of memory).
  - in Buffer, onunderflow is called until it do not fill the buffer any more (see ReadRawAmount).
  - Buffer constructor accepts a buffer object (kind of copy constructor) or a first chunk of data.
  - add Buffer::toString() function (cast to string).
  - add Buffer::IndexOf() function to locate a string in the buffer (returns -1 if not found).
  - add Buffer::Match().
  - add Buffer::Skip().
  - add Buffer::ReadUntil() function that read data until a boundaryString is reash (boundaryString is skip by default).
  - add Pack::WriteInt().

- in jsio module
  - a Descriptor::Read or Socket::RecvFrom operation on a eof or a disconnected descriptor returns <undefined> instead of an empty string.
  - before Poll() returns, callback functions are called in this order: error, exception, hangup, readable, writable.
  - optimize Poll() when descriptor list is empty.
  - File::content property returns <undefined> if the file do not exists.
  - Socket::Listen() supports <undefined> value as first argument (same as no arguments).
  - enhance Socket::connectContinue function.
  - Socket::Bind() returns <false> if the address/port is already in use and <true> otherwise, and now, ip argument may be <undefined>.
  - IoError::os property is the operating system error code.
  - add UIntervalNow() that uses IntervalToMicroseconds.
  - add IoError::toString() (cast to string).
  - add Descriptor::closed property is <true> if the descriptor is closed (NULL).
  - Socket::Accept() and Socket::TransmitFile() have a timeout argument.
  - new Socket::SendTo() and jsio::Socket::RecvFrom() functions.
  - add Socket::maxSegment, Socket::broadcast and Socket::multicastLoopback socket options.
  - add Socket::peerPort and jsio::Socket::sockPort properties.
  - add Socket::connectContinue and Socket::connectionClosed from the old jsnspr module.
  - add Socket::GetHostsByName().

Miscellaneous:
- use sqlite-amalgamation sources instead sqlite sources.
- jsfastcgi now use fastcgi library.
- in jsobjex, make object used by objex more generic: remove __parent__, __proto__, __count__ to avoid conflicts with user data.
```

## jslibs devsnapshot 2007.08.27 ##
```
- use JavaScript 1.8 ( see. http://developer.mozilla.org/en/docs/New_in_JavaScript_1.8 )
- the use of jsnspr is deprecated. jsio will take must be used instead.
  - The API is slightly different: Read() without arguments = ReadAll()
- add jsfastcgi module ( doc. http://code.google.com/p/jslibs/wiki/jsfastcgi )
  - see http://code.google.com/p/jslibs/wiki/HOWTO : "How to configure jshost to run as FastCGI"
- add a Pack class in jsstd to encode/decode binary data
- support shebang (#!)
- JS_THREADSAFE desactivated
- a lot of minor bugfix
```

## jsode module devsnapshot 2007.08.02 ##
```
```

## jslibs devsnapshot 2007.08.01 ##
```
  - jshost: on exit, returns a value
  - jswinhost: base the mutexName (to detect multiple instances running) on the module path
  - support of JS_THREADSAFE compilation option
  - support dynamic link of Visual C++ runtime libraries (msvcr80.dll)
  - jsgraphics: move OpenGL context creation from jsgl to jswindow
  - jsstd: add ASSERT() function ( mapped on RT_ASSERT() )
  - jsstd: add IsConstruction()
  - jsgl: rename function Texture into LoadImage
  - jsgl: create function LoadTexture see jsprotex/texture.h
  - jsgl: add function Ortho
  - jsprotex: remove channels management ( default is 4: RGBA )
  - new module: jsprotex for javascript procedural textures
  - jssqlite: support '?' in SQL text ( allow use of array as Query/Exec argument )
  - jsstd: add Halt function
  - jsnspr: by default, Open() use mode 700
  - jssqlite: use SQLite 3.3.17
  - jswinhost: add global isfirstinstance property to jswinhost
  - jsstd: add Buffer::Clear() method
  - jsnspr: add Event class
  - jsnspr: add GetRandomNoise() function
  - jsnspr: support of UDP protocol
  - jsnspr: split Socket::Listen function into Socket::Bind and Socket::Listen functions
  - jsstd: new functions SetScope and IsStatementValid
  - jsobjex: rename objex into ObjEx and enhance the Class API
  - jsstd: optimization of HideProperties function
  - jsnspr: add ReadAll function to File objects ( good for File.stdio.ReadAll() )
  - jsnspr: add content property to File objects
  - jsnspr: File class manage .content = undefined as delete file
  - jsstd: add IdOf() function
  - jsstd: enhance Buffer management, add Unread(), Read one chunk with Read(undefined)
  - jsstd: enhance Seal() and Clear() functions
  - jsshell: add Sleep(), MessageBeep() and Beep() functions
  - jsshell: in the menu object, if the value of text, checked, grayed or icon is a function, it is called and the return value is used.
  - jsstd: add a Buffer class
  - jsstd: add Buffer onunderflow management
  - jsstd: manage Buffer Read() and Buffer Write()
```

## jslibs devsnapshot 2007.03.02 ##
```
```

## jslibs devsnapshot 2007.02.12 ##
```
```

## jslibs 0.1 ##
```
```