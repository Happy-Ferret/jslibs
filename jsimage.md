<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jsimage/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jsimage/qa.js) -
# jsimage module #

> This module manage jpeg and png image decomppression and png image compression.
> Supported output format:
    1. Gray
> > 2:Gray,Alpha
> > 3:Red,Green,Blue
> > 4:Red,Green,Blue,Alpha




---

## jsimage static members ##
- [top](#jsimage_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsimage/static.cpp?r=2555) -

#### <font color='white' size='1'><b>DecodeJpegImage</b></font> ####

> <sub>imageObject</sub> <b>DecodeJpegImage</b>( streamObject )
> > This function returns an image object that represents the decompressed jpeg image given as argument.
> > <br />
> > The streamObject argument is any object that supports the NIStreamRead Native Interface ( file, socket, new Stream(buffer), ... )
> > <br />
> > For further details about stream objects, see jslang::Stream object and NativeInterface mechanism.

#### <font color='white' size='1'><b>DecodePngImage</b></font> ####

> <sub>imageObject</sub> <b>DecodePngImage</b>( streamObject )
> > This function returns an image object that represents the decompressed png image given as argument.
> > <br />
> > The streamObject argument is any object that supports the NIStreamRead Native Interface ( file, socket, new Stream(buffer), ... )
> > <br />
> > For further details about stream objects, see jslang::Stream object and NativeInterface mechanism.


---

- [top](#jsimage_module.md) - [main](JSLibs.md) -
