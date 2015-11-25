<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jsiconv/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jsiconv/qa.js) -
# jsiconv module #

> jsiconv is a wrapper to the GNU iconv library.
> iconv is a library convert between different character encodings.
> 


---

## class jsiconv::Iconv ##
- [top](#jsiconv_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsiconv/iconv.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( toCode, fromCode [[.md](.md) , toUseWide, fromUseWide ] )
> > Constructs a new conversion object that transforms from _fromCode_ into _toCode_.
> > ##### arguments: #####
      1. <sub>string</sub> _toCode_: destination encoding (see Iconv.list property)
      1. <sub>string</sub> _fromCode_: source encoding (see Iconv.list property)
      1. <sub>boolean</sub> _toUseWide_:
      1. <sub>boolean</sub> _fromUseWide_:

#### <font color='white' size='1'><i><b>call operator</b></i></font> ####

> <sub>string</sub> <i><b>call operator</b></i>( [[.md](.md) textData ] )
> > Converts textData. If called without argument, this resets the conversion state to the initial state and returns _undefined_.

#### <font color='white' size='1'><b>list</b></font> ####

> <sub>Object</sub> <b>list</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Lists locale independent encodings.

### Example 1 ###

> Convert and convert back a string.
```
LoadModule('jsstd');
LoadModule('jsiconv');

var conv = new Iconv('UTF-8', 'ISO-8859-1');
var invConv = new Iconv('ISO-8859-1', 'UTF-8');
var converted = conv('�t�');
var result = invConv(converted);
Print( result == '�t�','\n' ); // should be true
```

### Example 2 ###
> Convert and convert back a string char by char.
```
var conv = new Iconv('UTF-8', 'ISO-8859-1');
var invConv = new Iconv('ISO-8859-1', 'UTF-8');
var converted = conv('�t�');
var result = '';
for each ( var c in converted )
 result += invConv(c);
Print( result == '�t�','\n' ); // should be true
```


---

- [top](#jsiconv_module.md) - [main](JSLibs.md) -
