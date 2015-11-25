<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jsfont/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jsfont/qa.js) -
# jsfont module #

> Support text rendering (text to image) from the following font format:<br />
> TrueType, Type 1, CID-keyed Type 1, CFF, OpenType TrueType, OpenType CFF, SFNT-based bitmap, X11 PCF, Windows FNT, BDF, PFR, Type 42




---

## class jsfont::Font ##
- [top](#jsfont_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsfont/font.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( filePathName [[.md](.md), faceIndex = 0] )
> > Creates a new Font object and seletc the face to use.
> > ##### arguments: #####
      1. <sub>string</sub> _filePathName_: the path of the font file.
      1. <sub>integer</sub> _faceIndex_: the index of the face to use.

### Methods ###

#### <font color='white' size='1'><b>DrawChar</b></font> ####

> <sub>imageObject</sub> <b>DrawChar</b>( oneChar )
> > Draws one char with the current face.
> > ##### arguments: #####
      1. <sub>string</sub> _oneChar_: string of one char.
> > ##### return value: #####
> > > An image object that contains the char.

#### <font color='white' size='1'><b>DrawString</b></font> ####

> <sub>imageObject</sub> | <sub>integer</sub> <b>DrawString</b>( text [[.md](.md), keepTrailingSpace = false] [[.md](.md), getWidthOnly = false ] )
> > Draws a string with the current face.
> > ##### arguments: #####
      1. <sub>string</sub> _text_: the single-line text to draw.
      1. <sub>boolean</sub> _keepTrailingSpace_: if true, the last letter separator space is keept.
      1. <sub>boolean</sub> _getWidthOnly_: if true, the function will return the length (in pixel) of the _text_.
> > ##### return value: #####
> > > An image object that contains the text or the length of the text in pixel.

### Properties ###

#### <font color='white' size='1'><b>ascender</b></font> ####

> <sub>integer</sub> <b>ascender</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the ascender length (in pixel) of the current face.
> > ##### note: #####
> > > The ascender is the portion of a letter in a Latin-derived alphabet that extends above the mean line of a font. That is, the part of the letter that is taller than the font's x-height.

#### <font color='white' size='1'><b>descender</b></font> ####

> <sub>integer</sub> <b>descender</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the descender length (in pixel) of the current face.
> > ##### note: #####
> > > The descender is the portion of a letter in a Latin alphabet that extends below the baseline of a font. For example, in the letter y, the descender would be the "tail," or that portion of the diagonal line which lies below the v created by the two lines converging.

#### <font color='white' size='1'><b>width</b></font> ####

> <sub>integer</sub> <b>width</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the maximum width (in pixel) of the current face.

#### <font color='white' size='1'><b>size</b></font> ####

> <sub>integer</sub> <b>size</b>
> > is the size (in pixel) of the current face.

#### <font color='white' size='1'><b>encoding</b></font> ####

> <sub>enum</sub> <b>encoding</b>
> > is the current encoding.
> > ##### supported encodings: #####
> > > see constants section below.

#### <font color='white' size='1'><b>poscriptName</b></font> ####

> <sub>integer</sub> <b>poscriptName</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the postscript name of the face.

#### <font color='white' size='1'><b>useKerning</b></font> ####

> <sub>boolean</sub> <b>useKerning</b>
> > enable or disable kerning usage for the current face.
> > ##### note: #####
> > > Kerning is the process of adjusting letter spacing in a proportional font. In a well-kerned font, the two-dimensional blank spaces between each pair of letters all have similar area.

#### <font color='white' size='1'><b>horizontalPadding</b></font> ####

> <sub>integer</sub> <b>horizontalPadding</b>
> > is the size (in pixel) of the horizontal padding of any drawn text i.e. the space before and after the text.

#### <font color='white' size='1'><b>verticalPadding</b></font> ####

> <sub>integer</sub> <b>verticalPadding</b>
> > is the size (in pixel) of the vertical padding of any drawn text i.e. the space above and below the text.

#### <font color='white' size='1'><b>letterSpacing</b></font> ####

> <sub>integer</sub> <b>letterSpacing</b>
> > is the length (in pixel) of the additional space added between each letter in a text.

#### <font color='white' size='1'><b>italic</b></font> ####

> <sub>boolean</sub> <b>italic</b>
> > enable or disable italic.

#### <font color='white' size='1'><b>bold</b></font> ####

> <sub>boolean</sub> <b>bold</b>
> > enable or disable bold.

### Constants ###



> Encoding constants
> > <b><code>NONE</code></b>


> <b><code>MS_SYMBOL</code></b>

> <b><code>UNICODE</code></b>

> <b><code>SJIS</code></b>

> <b><code>GB2312</code></b>

> <b><code>BIG5</code></b>

> <b><code>WANSUNG</code></b>

> <b><code>JOHAB</code></b>

> <b><code>MS_SJIS</code></b>

> <b><code>MS_GB2312</code></b>

> <b><code>MS_BIG5</code></b>

> <b><code>MS_WANSUNG</code></b>

> <b><code>MS_JOHAB</code></b>

> <b><code>ADOBE_STANDARD</code></b>

> <b><code>ADOBE_EXPERT</code></b>

> <b><code>ADOBE_CUSTOM</code></b>

> <b><code>ADOBE_LATIN_1</code></b>

> <b><code>OLD_LATIN_2</code></b>

> <b><code>APPLE_ROMAN</code></b>


### Examples ###
##### example 1: #####
> Write "Hello world" in the file text.png
```
LoadModule('jslang');
LoadModule('jsstd');
LoadModule('jsfont');
LoadModule('jsimage');
LoadModule('jsprotex');
LoadModule('jsio');

var f = new Font('arial.ttf');
f.size = 100;
f.verticalPadding = -16;
var img = f.DrawString('Hello world', true);

var t = new Texture(img);
var t1 = new Texture(t);

t.BoxBlur(10,10);
t1.OppositeLevels();
t.Add(t1);
t.OppositeLevels();
t.Add(1);

new File('text.png').content = EncodePngImage(t.Export());
```


---

- [top](#jsfont_module.md) - [main](JSLibs.md) -
