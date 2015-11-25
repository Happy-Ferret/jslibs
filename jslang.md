<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jslang/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jslang/qa.js) -
# jslang module #

> This module contains all common classes used by other jslibs modules.
> ##### note: #####
> > This module is automatically loaded by jshost and jswinhost. Then LoadModule call is not needed.




---

## jslang static members ##
- [top](#jslang_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jslang/static.cpp?r=2555) -

### Static functions ###

#### <font color='white' size='1'><b>Stringify</b></font> ####

> <sub>string</sub> <b>Stringify</b>( value )
> > This function converts any value of stream into a string.


---

## class jslang::Blob ##
- [top](#jslang_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jslang/blob.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( [[.md](.md)data] )
> > Creates an object that can contain binary data.
> > ##### note: #####
> > When called in a non-constructor context, Object behaves identically.

### Methods ###

#### <font color='white' size='1'><b>Free</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Free</b>( [[.md](.md) wipe = false ] )
> > Frees the memory allocated by the blob and invalidates the blob.
> > ##### arguments: #####
      1. <sub>boolean</sub> _wipe_: clears the buffer before freeing it. This is useful when the blob contains sensitive data.
> > ##### note: #####
> > > Any access to a freed Blob will rise an error.

#### <font color='white' size='1'><b>concat</b></font> ####

> <sub>Blob</sub> <b>concat</b>( data [[.md](.md),data1 [[.md](.md),...]] )
> > Combines the text of two or more strings and returns a new string.
> > ##### details: #####
> > > [Mozilla](http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/concat)

#### <font color='white' size='1'><b>substr</b></font> ####

> <sub>Blob</sub> <b>substr</b>( start [[.md](.md), length ] )
> > Returns the bytes in a string beginning at the specified location through the specified number of characters.
> > ##### arguments: #####
      1. <sub>integer</sub> _start_: location at which to begin extracting characters (an integer between 0 and one less than the length of the string).
      1. <sub>integer</sub> _length_: the number of characters to extract.
> > ##### details: #####
> > > fc. [Mozilla](http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/substr)

#### <font color='white' size='1'><b>substring</b></font> ####

> <sub>Blob</sub> <b>substring</b>( indexA, [[.md](.md) indexB ] )
> > Extracts characters from indexA up to but not including indexB. In particular:
      * If indexA equals indexB, substring returns an empty string.
      * If indexB is omitted, substring extracts characters to the end of the blob.
      * If either argument is less than 0 or is NaN, it is treated as if it were 0.
      * If either argument is greater than stringName.length, it is treated as if it were stringName.length.
> > > If indexA is larger than indexB, then the effect of substring is as if the two arguments were swapped; for example, str.substring(1, 0) == str.substring(0, 1).

> > ##### arguments: #####
      1. <sub>integer</sub> _indexA_: An integer between 0 and one less than the length of the blob.
      1. <sub>integer</sub> _indexB_: (optional) An integer between 0 and the length of the blob.
> > ##### details: #####
> > > fc. [Mozilla](http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/substring)

#### <font color='white' size='1'><b>indexOf</b></font> ####

> <sub>integer</sub> <b>indexOf</b>( searchValue [[.md](.md), fromIndex] )
> > Returns the index within the calling Blob object of the first occurrence of the specified value, starting the search at fromIndex, or -1 if the value is not found.
> > ##### arguments: #####
      1. <sub>string</sub> _searchValue_: A string representing the value to search for.
      1. <sub>integer</sub> _fromIndex_: The location within the calling string to start the search from. It can be any integer between 0 and the length of the string. The default value is 0.
> > ##### details: #####
> > > fc. [Mozilla](http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/indexOf)

#### <font color='white' size='1'><b>lastIndexOf</b></font> ####

> <sub>integer</sub> <b>lastIndexOf</b>( searchValue [[.md](.md), fromIndex] )
> > Returns the index within the calling Blob object of the last occurrence of the specified value, or -1 if not found. The calling string is searched backward, starting at fromIndex.
> > ##### arguments: #####
      1. <sub>string</sub> _searchValue_: A string representing the value to search for.
      1. <sub>integer</sub> _fromIndex_: The location within the calling string to start the search from, indexed from left to right. It can be any integer between 0 and the length of the string. The default value is the length of the string.
> > ##### details: #####
> > > fc. [Mozilla](http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/lastIndexOf)

#### <font color='white' size='1'><b>charAt</b></font> ####

> <sub>string</sub> <b>charAt</b>( index )
> > Returns the specified character from a string.
> > ##### details: #####
> > > fc. [Mozilla](http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/charAt)

#### <font color='white' size='1'><b>charCodeAt</b></font> ####

> <sub>integer</sub> <b>charCodeAt</b>( index )
> > Returns a number indicating the ASCII value of the character at the given index.
> > ##### details: #####
> > > fc. [Mozilla](http://developer.mozilla.org/index.php?title=En/Core_JavaScript_1.5_Reference/Global_Objects/String/charCodeAt)

#### <font color='white' size='1'><b><i>toString</i></b></font> ####

> <sub>string</sub> <b><i>toString</i></b>()
> > Returns a JavaScript string version of the current Blob object.
> > ##### <font color='red'>beware</font>: #####
> > > This function may be called automatically by the JavaScript engine when it needs to convert the Blob object to a JS string.

### Properties ###

#### <font color='white' size='1'><b>length</b></font> ####

> <sub>integer</sub> <b>length</b>
> > is the length of the current Blob.

#### <font color='white' size='1'><i><b><a href='N.md'>N</a> operator</b></i></font> ####

> <sub>char</sub> <i><b><a href='N.md'>N</a> operator</b></i>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Used to access the character in the _N\_th position where_N_is a positive integer between 0 and one less than the value of length._

### Note ###

> Blobs are immutable. This mean that its content cannot be modified after it is created.

### Native Interface ###
  * **NIBufferGet**


---

## class jslang::Stream ##
- [top](#jslang_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jslang/stream.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( bufferObject )
> > Creates an object that transforms any buffer-like objects into a stream.
> > <br />
> > buffer-like objects are: string, Blob, and any objects that implements NIBufferGet native interface.
> > <br />
> > ##### note: #####
> > When called in a non-constructor context, Object behaves identically.

#### <font color='white' size='1'><b>Read</b></font> ####

> <sub>Blob</sub> <b>Read</b>( amount )
> > Read _amount_ of data into the stream.

#### <font color='white' size='1'><b>position</b></font> ####

> <sub>integer</sub> <b>position</b>
> > Get or set the stream pointer position.

#### <font color='white' size='1'><b>available</b></font> ####

> <sub>integer</sub> <b>available</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > The remaining data from the stream pointer position to the end of the stream.

#### <font color='white' size='1'><b>source</b></font> ####

> <sub>Object</sub> <b>source</b>
> > The object used to create the steam.

### note ###

> Basically, a Stream is nothing else that a buffer with a stream pointer position.

### Native Interface ###
  * **NIStreamRead**


---

- [top](#jslang_module.md) - [main](JSLibs.md) -
