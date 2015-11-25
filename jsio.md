<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jsio/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jsio/qa.js) -
# jsio module #

> This module is based on Netscape Portable Runtime (NSPR) that provides a platform-neutral API for system level and libc like functions.
> NSPR API is used in the Mozilla client, many of Netscape/AOL/iPlanet's and other software offerings.
> 



---

## jsio static members ##
- [top](#jsio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsio/static.cpp?r=2577) -

### Static functions ###

#### <font color='white' size='1'><b>Poll</b></font> ####
> <sub>integer</sub> <b>Poll</b>( _descriptorArray_ [[.md](.md), _timeout_ = undefined ] )
> > This function listen for a readable, writable or exception event on each descriptor in _descriptorArray_.
> > When an event occurs, the function tries to call the corresponding property (function) on the descriptor.
> > <br />
> > The function returns the number of events that occurs or 0 if the function timed out.
> > ##### note: #####
> > > The property is NOT cleared by the function.
> > > If you want to stop receiving an event, you have to remove the property by yourself. eg. `delete socket1.writable`.

> > <br />
> > The second argument, _timeout_, defines the number of milliseconds the function has to wait before returning if no event has occured.
> > _timeout_ can be undefined OR omited, in this case, no timeout is used.
> > ##### <font color='red'>beware</font>: #####
> > > No timeout means that the function will wait endless for events.

> > ##### example: #####
```
  socket1.readable = function(soc) { Print( soc.Recv(); ) }
  var count = Poll( [socket1, socket2], 1000 );
  if ( count == 0 )
    Print( 'Nothing has been recived' );
```

#### <font color='white' size='1'><b>IsReadable</b></font> ####

> <sub>boolean</sub> <b>IsReadable</b>( _descriptor_ )
> > Returns true if the _descriptor_ can be read without blocking.

#### <font color='white' size='1'><b>IsWritable</b></font> ####

> <sub>boolean</sub> <b>IsWritable</b>( descriptor )
> > Returns _true_ if the _descriptor_ can be write without blocking.

#### <font color='white' size='1'><b>IntervalNow</b></font> ####

> <sub>integer</sub> <b>IntervalNow</b>()
> > Returns the milliseconds value of NSPR's free-running interval timer.

#### <font color='white' size='1'><b>UIntervalNow</b></font> ####

> <sub>integer</sub> <b>UIntervalNow</b>()
> > Returns the microseconds value of NSPR's free-running interval timer.

#### <font color='white' size='1'><b>Sleep</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Sleep</b>( _milliseconds_ )
> > Sleeps _milliseconds_ milliseconds.

#### <font color='white' size='1'><b>GetEnv</b></font> ####

> <sub>string</sub> <b>GetEnv</b>( name )
> > Retrieve the value of the given environment variable.

#### <font color='white' size='1'><b>GetRandomNoise</b></font> ####

> <sub>string</sub> <b>GetRandomNoise</b>( size )
> > Provides, depending on platform, a random value.
> > The length of the random value is dependent on platform and the platform's ability to provide a random value at that moment.
> > ##### <font color='red'>beware</font>: #####
> > > Calls to <b>GetRandomNoise</b>() may use a lot of CPU on some platforms.
> > > Some platforms may block for up to a few seconds while they accumulate some noise.
> > > Busy machines generate lots of noise, but care is advised when using <b>GetRandomNoise</b>() frequently in your application.
> > > <br />
> > > [NSPR API](http://developer.mozilla.org/en/docs/PR_GetRandomNoise)

#### <font color='white' size='1'><b>WaitSemaphore</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>WaitSemaphore</b>( semaphoreName )
> > Tests the value of the semaphore.
> > If the value of the semaphore is > 0, the value of the semaphore is decremented and the function returns.
> > If the value of the semaphore is 0, the function blocks until the value becomes > 0, then the semaphore is decremented and the function returns.
> > ##### note: #####
> > > The "test and decrement" operation is performed atomically.

#### <font color='white' size='1'><b>PostSemaphore</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>PostSemaphore</b>( semaphoreName )
> > Increments the value of a specified semaphore.

#### <font color='white' size='1'><b>AvailableSpace</b></font> ####

> <sub>real</sub> <b>AvailableSpace</b>( path )
> > Returns the available storage space (in bytes) at the given path.
> > ##### example: #####
```
  Print( AvailableSpace('/var') );
```

### Static properties ###

#### <font color='white' size='1'><b>hostName</b></font> ####

> <sub>string</sub> <b>hostName</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the host name with the domain name (if any).

#### <font color='white' size='1'><b>physicalMemorySize</b></font> ####

> <sub>integer</sub> <b>physicalMemorySize</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the amount of physical RAM in the system in bytes.

#### <font color='white' size='1'><b>systemInfo</b></font> ####

> <sub>Object</sub> <b>systemInfo</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Returns an object that contains the following properties:
      * <sub>string</sub> **architecture**: x86, ...
      * <sub>string</sub> **name**: Linux, Windows\_NT, ...
      * <sub>string</sub> **release**: 2.6.22.18, 5.1, ...
> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jsio');
  Print( systemInfo.toSource() );
```
> > prints:
      * on coLinux: ` ({architecture:"x86", name:"Linux", release:"2.6.22.18-co-0.7.3"}) `
      * on WinXP: ` ({architecture:"x86", name:"Windows_NT", release:"5.1"}) `

#### <font color='white' size='1'><b>processPriority</b></font> ####

> <sub>integer</sub> <b>processPriority</b>
> > Is the current process priority among the following values:
      * `-1`: low
      * ` 0`: normal
      * ` 1`: high
      * ` 2`: urgent

#### <font color='white' size='1'><b>numberOfProcessors</b></font> ####

> <sub>string</sub> <b>numberOfProcessors</b>
> > Is the number of processors (CPUs available in an SMP system).

#### <font color='white' size='1'><b>currentDirectory</b></font> ####

> <sub>string</sub> <b>currentDirectory</b>
> > Gets or sets the current working directory.
> > ##### example: #####
```
  currentDirectory = '/home/foobar';
```

#### <font color='white' size='1'><b>directorySeparator</b></font> ####

> <sub>string</sub> <b>directorySeparator</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Get the host' directory separator.
> > ##### example: #####
```
  var isRootDir = (currentDirectory == directorySeparator);
```

#### <font color='white' size='1'><b>pathSeparator</b></font> ####

> <sub>string</sub> <b>pathSeparator</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Get the host' path separator.
> > ##### example: #####
```
  Print( GetEnv('PATH').split(<b>pathSeparator</b>) )
```

#### <font color='white' size='1'><b>version</b></font> ####

> <b>version</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Hold the current version of NSPR.


---

## class jsio::Descriptor ##
- [top](#jsio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsio/descriptor.cpp?r=2577) -

### Methods ###

#### <font color='white' size='1'><b>Close</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Close</b>()
> > Close the descriptor.

#### <font color='white' size='1'><b>Read</b></font> ####

> <sub>value</sub> <b>Read</b>( [[.md](.md)amount] )
> > Read _amount_ bytes of data from the current descriptor. If _amount_ is ommited, the whole available data is read.
> > If the descriptor is exhausted (eof or disconnected), this function returns _undefined_.
> > If the descriptor is a blocking socket and _amount_ is set to the value _undefined_ value, the call blocks until data is available.
> > ##### <font color='red'>beware</font>: #####
> > > This function returns a Blob or a string literal as empty string.

> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jsio');

  var soc = new Socket();
  soc.Connect('www.google.com', 80);
  soc.Write('GET\r\n\r\n');
  Print( soc.Read(undefined), '\n' );
```

#### <font color='white' size='1'><b>Write</b></font> ####

> <sub>string</sub> <b>Write</b>( data )
> > If the whole data cannot be written, Write returns that have not be written.

#### <font color='white' size='1'><b>Sync</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Sync</b>()
> > Sync any buffered data for a fd to its backing device.

### Properties ###

#### <font color='white' size='1'><b>available</b></font> ####

> <b>available</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Determine the amount of data in bytes available for reading on the descriptor.

#### <font color='white' size='1'><b>type</b></font> ####

> <b>type</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > see constants below.

#### <font color='white' size='1'><b>closed</b></font> ####

> <b>closed</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is _true_ if the file descriptor is closed.
> > ##### <font color='red'>beware</font>: #####
> > > Do not confuse with disconnected.<br /> eg. A socket descriptor can be open but disconnected.

### Static functions ###

#### <font color='white' size='1'><b>Import</b></font> ####

> <b>Import</b>( nativeDescriptor, type )

### Constants ###
> <b><code>DESC_FILE</code></b>

> <b><code>DESC_SOCKET_TCP</code></b>

> <b><code>DESC_SOCKET_UDP</code></b>

> <b><code>DESC_LAYERED</code></b>

> <b><code>DESC_PIPE</code></b>


### Native Interface ###
  * **NIStreamRead**
> > Read the file as a stream.


---

## class jsio::Directory ##
- [top](#jsio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsio/directory.cpp?r=2577) -

> This class manages directory I/O Functions.

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( directoryName )
> > Creates a new Directory object.
> > ##### arguments: #####
      1. <sub>string</sub> _directoryName_

### Methods ###

#### <font color='white' size='1'><b>Open</b></font> ####

> <sub>this</sub> <b>Open</b>()
> > Open the directory.

#### <font color='white' size='1'><b>Close</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Close</b>()
> > Close the specified directory.

#### <font color='white' size='1'><b>Read</b></font> ####

> <sub>string</sub> <b>Read</b>( [[.md](.md), flags = Directory.SKIP\_NONE ] )
> > Reads an item of the current directory and go to the next.
> > ##### arguments: #####
      1. <sub>enum</sub> _flags_: specifies how special files are processed.
> > ##### return value: #####
> > > A single directory item.

#### <font color='white' size='1'><b>Make</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Make</b>()
> > Create the directory given in the constructor.

#### <font color='white' size='1'><b>Remove</b></font> ####

> <sub>boolean</sub> <b>Remove</b>()
> > Removes the directory given in the constructor.
> > ##### return value: #####
> > > returns _false_ If the directory is not empty else it returns _true_.

### Properties ###

#### <font color='white' size='1'><b>exist</b></font> ####

> <b>exist</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Check if the directory exists.

#### <font color='white' size='1'><b>name</b></font> ####

> <b>name</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Returns the name of the directory.

### Static functions ###

#### <font color='white' size='1'><b>List</b></font> ####

> <sub>Array</sub> <b>List</b>( dirName [[.md](.md), flags = Directory.SKIP\_DOT] )
> > Read all entries of a directory at once.
> > ##### arguments: #####
      1. <sub>string</sub> _dirName_: is the path of the directory.
      1. <sub>enum</sub> _flags_: specifies how special files are processed.
> > ##### return value: #####
> > > All entries in the directory _name_.

> > ##### note: #####
> > > This function supports additional flags: Directory.`SKIP_FILE`, Directory.`SKIP_DIRECTORY`, Directory.`SKIP_OTHER`

> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jsio');
  Print( Directory.List('.').join('\n'), '\n' );
```

### Constants ###

> <b><code>SKIP_NONE</code></b>
> > Do not skip any files.

> <b><code>SKIP_DOT</code></b>
> > Skip the directory entry "." representing the current directory.

> <b><code>SKIP_DOT_DOT</code></b>
> > Skip the directory entry ".." representing the parent directory.

> <b><code>SKIP_BOTH</code></b>
> > Skip both "." and ".." ( same as Directory.`SKIP_DOT` | Directory.`SKIP_DOT_DOT` )

> <b><code>SKIP_HIDDEN</code></b>
> > Skip hidden files. On Windows platforms and the Mac OS, this value identifies files with the "hidden" attribute set. On Unix platform, this value identifies files whose names begin with a period (".").

### Example ###
```
var dir = new Directory( 'c:/tmp' );
dir.Open();
for ( var entry; ( entry = dir.Read() ); ) {

   var file = new File(dir.name+'/'+entry);
   Print( entry + ' ('+ file.info.type +')', '\n');
}
```

### Example ###
```
function RecursiveDir(path) {
   var testList = [];
   (function(path) {
      var dir = new Directory(path);
      dir.Open();
      for ( var entry; ( entry = dir.Read(Directory.SKIP_BOTH) ); ) {

         var file = new File(dir.name+'/'+entry);
         switch ( file.info.type ) {
            case File.FILE_DIRECTORY:
               arguments.callee(file.name);
               break;
            case File.FILE_FILE:
               testList.push(file.name);
               break;
         }
      }
      dir.Close();
   })(path);
   return testList;
}

Print( RecursiveDir('jshost').join('\n') );
```


---

## class jsio::File<sup>jsio::Descriptor</sup> ##
- [top](#jsio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsio/file.cpp?r=2577) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( fileName )

### Methods ###

#### <font color='white' size='1'><b>Open</b></font> ####
> <sub>this</sub> <b>Open</b>( flags [[.md](.md), mode] )
> > Open a file for reading, writing, or both.
> > <br />
> > _flags_ is either a combinaison of open mode constants or a string that contains fopen like flags (+, r, w, a).
> > <br />
> > The functions returns the file object itself (this), this allows to write things like: `new File('foo.txt').Open('r').Read();`

#### <font color='white' size='1'><b>Seek</b></font> ####

> <sub>integer</sub> <b>Seek</b>( [[.md](.md) offset = 0 [[.md](.md), whence = File.`SEEK_SET` ] ] )
> > Moves read-write file offset.

#### <font color='white' size='1'><b>Delete</b></font> ####

> <sub>integer</sub> <b>Delete</b>()
> > Delete a file from the filesystem.
> > The operation may fail if the file is open.

#### <font color='white' size='1'><b>Lock</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Lock</b>( state )
> > Lock or unlock a file for exclusive access.
> > _state_ can be _true_ or _false_.

#### <font color='white' size='1'><b>Move</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Move</b>( directory )
> > Move the file to another directory.

### Properties ###

#### <font color='white' size='1'><b>position</b></font> ####

> <sub>integer</sub> <b>position</b>
> > Get or set the current position of the file pointer. Same as Seek() function used with SEEK\_SET.

#### <font color='white' size='1'><b>content</b></font> ####

> <sub>string</sub> <b>content</b>
> > Get or set the content of the file. If the file does not exist, content is _undefined_.
> > Setting content with _undefined_ deletes the file.

#### <font color='white' size='1'><b>name</b></font> ####

> <sub>string</sub> <b>name</b>
> > Contains the name of the file. Changing this value will rename or move the file.

#### <font color='white' size='1'><b>exist</b></font> ####

> <sub>boolean</sub> <b>exist</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Contains true if the file exists.

#### <font color='white' size='1'><b>info</b></font> ####

> <sub>Object</sub> <b>info</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > This property works with opened and closed files.
> > Contains an object that has the following properties:
      * type : Type of file.
      * size : Size, in bytes, of file's contents.
      * creationTime : Creation time (elapsed milliseconds since 1970.1.1).
      * modifyTime : Last modification time (elapsed milliseconds since 1970.1.1).
> > ##### note: #####
> > > Time is relative to midnight (00:00:00), January 1, 1970 Greenwich Mean Time (GMT).

> > ##### <font color='red'>beware</font>: #####
> > > File time resolution depends of hte underlying filesystem.
> > > <br />
> > > eg. the resolution of create time on FAT is 10 milliseconds, while write time has a resolution of 2 seconds and access time has a resolution of 1 day.

> > ##### example: #####
```
   var f = new File('test.txt');
   f.content = '123';
   Print( new Date( f.info.modifyTime ) );
```

### Static properties ###

#### <font color='white' size='1'><b>standard</b></font> ####

> <sub>File</sub> **stdin**  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is a jsio::File that represents the standard input.


> <sub>File</sub> **stdout**  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is a jsio::File that represents the standard output.


> <sub>File</sub> **stderr**  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is a jsio::File that represents the standard error.

### Constants ###

#### <font color='white' size='1'>???</font> ####

> Open mode
> > <b><code>RDONLY</code></b>


> <b><code>WRONLY</code></b>

> <b><code>RDWR</code></b>

> <b><code>CREATE_FILE</code></b>

> <b><code>APPEND</code></b>

> <b><code>TRUNCATE</code></b>

> <b><code>SYNC</code></b>

> <b><code>EXCL</code></b>

> Seek mode
> > <b><code>SEEK_SET</code></b>


> <b><code>SEEK_CUR</code></b>

> <b><code>SEEK_END</code></b>

> info.type
> > <b><code>FILE_FILE</code></b>


> <b><code>FILE_DIRECTORY</code></b>

> <b><code>FILE_OTHER</code></b>


### Native Interface ###
  * **NIStreamRead**
> > Read the file as a stream.

### Example 1 ###
```
function Copy(fromFilename, toFilename) {

 var fromFile = new File(fromFilename).Open(File.RDONLY);
 var toFile = new File(toFilename).Open(File.WRONLY | File.CREATE_FILE | File.TRUNCATE);
 for ( var buf; buf = fromFile.Read(65536); )
  toFile.Write(buf);
 toFile.Close();
 fromFile.Close();
}
```

### Example 2 ###
```
LoadModule('jsstd');
LoadModule('jsio');

try {

   var file = new File('file_test.txt');
   if ( file.exist ) {
      file.Open( File.RDONLY );
      Print( 'file content:\n' + file.Read() );
      file.Close();
   }

} catch ( ex if ex instanceof IoError ) {

   Print( 'IOError: ' + ex.text, '\n' );
} catch( ex ) {

   throw ex;
}
```


---

## class jsio::MemoryMapped ##
- [top](#jsio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsio/memoryMapped.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( file )
> > Creates a new memory-mapped object using the given opened file descriptor.
> > ##### arguments: #####
      1. <sub>File</sub> _file_: any opened file descriptor.

### Properties ###

#### <font color='white' size='1'><b>file</b></font> ####

> <b>file</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the file descriptor used to construct this object.

### Native Interface ###
  * **NIBufferGet**
> > This object can be used as a buffer source.
> > ##### example: #####
```
  var stream = new Stream( new MemoryMapped(new File('directory.cpp').Open("r")) );
  Print(stream.read(10));
  Print(stream.read(10));
  Print(stream.read(10));
```

### Example ###
```
var f = new File('directory.cpp');
f.Open("r");
var m = new MemoryMapped(f);
Print(m);
```


---

## class jsio::Pipe ##
- [top](#jsio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsio/pipe.cpp?r=2555) -

> You cannot construct this class.<br />
> The aim of this class is to provide a way to access Descriptor properties and methods from an existing pipe.
> ##### exemple: #####
```
 var p = new Process( 'c:\\windows\\System32\\cmd.exe', ['/c', 'dir', 'c:'] );
 p.Wait();
 Print( p.stdout.Read() );
```


---

## class jsio::Process ##
- [top](#jsio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsio/process.cpp?r=2212) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <sub>value</sub> <i><b>constructor</b></i>( path [[.md](.md) , argv ] )
> > This function starts a new process optionaly using the JavaScript Array _argv_ for arguments or _undefined_ for no arguments.
> > ##### note: #####
> > > The new process inherits the environment of the parent process.

> > ##### exemple: #####
```
  var p = new Process( 'c:\\windows\\System32\\cmd.exe', ['/c', 'dir', 'c:'] );
  p.Wait();
  Print( p.stdout.Read() );
```

### Methods ###

#### <font color='white' size='1'><b>Wait</b></font> ####

> <sub>integer</sub> <b>Wait</b>()
> > The function waits the end of the nondetached process and returns its exit code. This function will fail if the process has beed detached.
> > ##### note: #####
> > > In bash, `true;echo $?` prints `0` and `false;echo $?` prints `1`

#### <font color='white' size='1'><b>Detach</b></font> ####

> <sub>integer</sub> <b>Detach</b>()
> > This function detaches the process. A detached process does not need to be and cannot be reaped.

#### <font color='white' size='1'><b>Kill</b></font> ####

> <sub>integer</sub> <b>Kill</b>()
> > Terminates the process.

### Properties ###

#### <font color='white' size='1'><b>stdin</b></font> ####

> <sub>Pipe</sub> <b>stdin</b>()
> > Is the stdin pipe to the running process.

#### <font color='white' size='1'><b>stdout</b></font> ####

> <sub>Pipe</sub> <b>stdout</b>()
> > Is the stdout pipe to the running process.

#### <font color='white' size='1'><b>stderr</b></font> ####

> <sub>Pipe</sub> <b>stderr</b>()
> > Is the stderr pipe to the running process.


---

## class jsio::Semaphore ##
- [top](#jsio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsio/semaphore.cpp?r=2577) -

> This class manages interprocess communication semaphores using a counting semaphore model similar to that which is provided in Unix and Windows platforms.

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( name, [[.md](.md) count = 0 ] [[.md](.md), mode = Semaphore.IRUSR | Semaphore.IWUSR ] )
> > Create or open a named semaphore with the specified name. If the named semaphore doesn't exist, the named semaphore is created.
> > ##### exemple: #####
```
  <b><font color="red">TBD</font></b>
```

### Methods ###

#### <font color='white' size='1'><b>Wait</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Wait</b>()
> > If the value of the semaphore is > 0, decrement the value and return. If the value is 0, sleep until the value becomes > 0, then decrement the value and return. The "test and decrement" operation is performed atomically.

#### <font color='white' size='1'><b>Post</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Post</b>()
> > Increment the value of the named semaphore by 1.

### Constants ###


> <b><code>IRWXU</code></b>
> > read, write, execute/search by owner

> <b><code>IRUSR</code></b>
> > read permission, owner

> <b><code>IWUSR</code></b>
> > write permission, owner

> <b><code>IXUSR</code></b>
> > execute/search permission, owner

> <b><code>IRWXG</code></b>
> > read, write, execute/search by group

> <b><code>IRGRP</code></b>
> > read permission, group

> <b><code>IWGRP</code></b>
> > write permission, group

> <b><code>IXGRP</code></b>
> > execute/search permission, group

> <b><code>IRWXO</code></b>
> > read, write, execute/search by others

> <b><code>IROTH</code></b>
> > read permission, others

> <b><code>IWOTH</code></b>
> > write permission, others

> <b><code>IXOTH</code></b>
> > execute/search permission, others


---

## class jsio::SharedMemory ##
- [top](#jsio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsio/sharedMemory.cpp?r=2557) -

> This class manages shared memory between two or more process.

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( name, size [[.md](.md), mode] )
> > Creates a named shared memory area of _size_ bytes using _mode_ linux-like rights.

### Methods ###

#### <font color='white' size='1'><b>Write</b></font> ####

> <b>Write</b>( data [[.md](.md), offset] )
> > Write _data_ at _offset_ in the shared memory.

#### <font color='white' size='1'><b>Read</b></font> ####

> <sub>string</sub> <b>Read</b>( length [[.md](.md), offset] )
> > Read _length_ bytes from _offset_ in the shared memory.

#### <font color='white' size='1'><b>Clear</b></font> ####

> <b>Clear</b>()
> > Clears the content of the shared memory.

#### <font color='white' size='1'><b>Close</b></font> ####

> <b>Close</b>()
> > Close the shared memory.

### Properties ###

#### <font color='white' size='1'><b>content</b></font> ####

> <sub>string</sub> <b>content</b>
> > Read or write the whole content of the shared memory. Setting _undefined_ as value clears the memory area.

### Native Interface ###
  * **NIBufferGet**
> > This object provide a BufferGet native interface an can be used in any function that support this interface. For example a Stream object.

### Exemple ###
```
LoadModule('jsstd');
LoadModule('jsio');

var mem1 = new SharedMemory( 'mytest', 100 );
mem1.Write('foo');

var mem2 = new SharedMemory( 'mytest', 100 );
Print( mem2.Read(3), '\n' );
```


---

## class jsio::Socket<sup>jsio::Descriptor</sup> ##
- [top](#jsio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsio/socket.cpp?r=2577) -

> Socket class is used to create a non-blocking TCP socket.

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( [[.md](.md)type = Socket.TCP] )
> > Type can be Socket.TCP or Socket.UDP.

### Methods ###

#### <font color='white' size='1'><b>Shutdown</b></font> ####

> <b>Shutdown</b>( [[.md](.md) what ] )
> > Shut down part of a full-duplex connection on a socket.
> > <br />
> > if _what_ is `ALSE`, further receives will be disallowed.
> > <br />
> > if _what_ is _true_, further sends will be disallowed.
> > <br />
> > if _what_ is ommited or _undefined_, further sends and receives will be disallowed

#### <font color='white' size='1'><b>Bind</b></font> ####

> <b>Bind</b>( [[.md](.md)port] [[.md](.md), ip] )
> > When a new socket is created, it has no address bound to it.
> > Bind assigns the specified address (also known as name) to the socket.
> > _ip_ is the address (interface) to which the socket will be bound.
> > If _address_ is ommited, any address is will match.
> > If _port_ is _undefined_, the socket is not bind to any port.
> > ##### return value: #####
> > > `ALSE` if the address is already is in use otherwise _true_.

> > ##### example 1: #####
```
  var server = new Socket();
  server.Bind(8099, '127.0.0.1');
  server.Listen();
```
> > ##### example 2: #####
```
  var client = new Socket();
  client.Bind(0, '192.168.0.1');
  client.Connect('127.0.0.1', 8099);
```

#### <font color='white' size='1'><b>Listen</b></font> ####

> <b>Listen</b>( [[.md](.md) backlogSize = 8 ] )
> > Listen for connections on a socket.
> > _backlogSize_ specifies the maximum length of the queue of pending connections.

#### <font color='white' size='1'><b>Accept</b></font> ####

> <sub>Socket</sub> <b>Accept</b>()
> > Accept a connection on a socket.
> > This function returns a connected jsio::Socket.

#### <font color='white' size='1'><b>Connect</b></font> ####

> <sub>this</sub> <b>Connect</b>( host, port [[.md](.md), timeout] )
> > Initiate a connection on a socket.

#### <font color='white' size='1'><b>SendTo</b></font> ####

> <sub>string</sub> <b>SendTo</b>( host, port, string )
> > Send a specified number of bytes from an unconnected socket.
> > See. Static functions.

#### <font color='white' size='1'><b>RecvFrom</b></font> ####

> <sub>string</sub> <b>RecvFrom</b>()
> > Receive all data from socket which may or may not be connected.
> > See. Static functions.

#### <font color='white' size='1'><b>TransmitFile</b></font> ####

> <b>TransmitFile</b>( fileDescriptor [[.md](.md), close [[.md](.md), headers [[.md](.md), timeout]]] )
> > Sends a complete file pointed by _fileDescriptor_ across a socket.
> > <br />
> > _headers_ is a string that contains the headers to send across the socket prior to sending the file.
> > <br />
> > Optionally, _close_ flag specifies that transmitfile should close the socket after sending the data.
> > <br />
> > _timeout_ is the time limit for completion of the transmit operation.
> > ##### note: #####
> > > This function only works with blocking sockets.

### Properties ###

#### <font color='white' size='1'><b>connectContinue</b></font> ####

> <b>connectContinue</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Test if a nonblocking connect has completed.
> > Is _true_ if the socket is connected.
> > <br />
> > Is _undefined_ if the operation is still in progress.
> > <br />
> > Is `ALSE` if the connection is refused.

#### <font color='white' size='1'><b>connectionClosed</b></font> ####

> <b>connectionClosed</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Check if the socket connection is closed.

#### <font color='white' size='1'>linger</font> ####

> <sub>integer</sub> **linger**
> > The time in milliseconds to linger on close if data present.
> > A value of zero means no linger.

#### <font color='white' size='1'>noDelay</font> ####

> <sub>boolean</sub> **noDelay**
> > Don't delay send to coalesce packets.

#### <font color='white' size='1'>reuseAddr</font> ####

> <sub>boolean</sub> **reuseAddr**
> > Allow local address reuse.

#### <font color='white' size='1'>keepAlive</font> ####

> <sub>boolean</sub> **keepAlive**
> > Keep connections alive.

#### <font color='white' size='1'>recvBufferSize</font> ####

> <sub>integer</sub> **recvBufferSize**
> > Receive buffer size.

#### <font color='white' size='1'>sendBufferSize</font> ####

> <sub>integer</sub> **sendBufferSize**
> > Send buffer size.

#### <font color='white' size='1'>maxSegment</font> ####

> <sub>integer</sub> **maxSegment**
> > Maximum segment size.

#### <font color='white' size='1'>nonblocking</font> ####

> <sub>boolean</sub> **nonblocking**
> > Non-blocking (network) I/O.

#### <font color='white' size='1'>broadcast</font> ####

> <sub>boolean</sub> **broadcast**
> > Enable broadcast.

#### <font color='white' size='1'>multicastLoopback</font> ####

> <sub>boolean</sub> **multicastLoopback**
> > IP multicast loopback.

#### <font color='white' size='1'><b>peerName</b></font> ####

> <sub>string</sub> <b>peerName</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Get name of the connected peer.
> > Return the network address for the connected peer socket.

#### <font color='white' size='1'><b>peerPort</b></font> ####

> <sub>integer</sub> <b>peerPort</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Get port of the connected peer.
> > Return the port for the connected peer socket.

#### <font color='white' size='1'><b>sockName</b></font> ####

> <sub>string</sub> <b>sockName</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Get socket name.
> > Return the network address for this socket.

#### <font color='white' size='1'><b>sockPort</b></font> ####

> <sub>integer</sub> <b>sockPort</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Get socket port.
> > Return the port for this socket.

### Static functions ###

#### <font color='white' size='1'><b>GetHostsByName</b></font> ####

> <sub>Array</sub> <b>GetHostsByName</b>( hostName )
> > Lookup a host by name and returns the results in a javascript array.

#### <font color='white' size='1'>SendTo</font> ####

> SendTo
> > see Socket::SendTo

#### <font color='white' size='1'>RecvFrom</font> ####

> RecvFrom
> > see Socket::RecvFrom

### Constants ###


> <b><code>TCP</code></b>

> <b><code>UDP</code></b>


### Native Interface ###
  * **NIStreamRead**

### Intresting lecture ###
  1. [Five pitfalls of Linux sockets programming](http://www.ibm.com/developerworks/linux/library/l-sockpit/)



---

## class jsio::IoError ##
- [top](#jsio_module.md) -

> You cannot construct this class.<br />
> Its aim is to throw as an exception on any NSPR runtime error.

### Properties ###


---

- [top](#jsio_module.md) - [main](JSLibs.md) -
