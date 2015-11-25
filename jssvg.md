<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jssvg/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jssvg/qa.js) -
# jssvg module #

> 


---

## class jssvg::SVG ##
- [top](#jssvg_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jssvg/svg.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>()
> > Constructs a new SVG object.

### Methods ###

#### <font color='white' size='1'><b>Write</b></font> ####

> <sub>this</sub> <b>Write</b>( xmlString )
> > Adds XML data to the current SVG context.
> > ##### note: #####
> > > calls to callback function 'onImage' may occur during this call.

> > ##### example: #####
```
  svg.Write(<svg><circle cx="50" cy="50" r="25" fill="red"/></svg>);
  // or
  svg.Write('<svg><circle cx="50" cy="50" r="25" fill="red"/></svg>');
```

#### <font color='white' size='1'><b>RenderImage</b></font> ####

> <sub>ImageObject</sub> <b>RenderImage</b>( [[.md](.md) imageWidth , imageHeight ] [[.md](.md) , channels ] [[.md](.md) , fit ] [[.md](.md) , elementId ] )
> > Draws the SVG to an image object.
> > ##### arguments: #####
      1. <sub>integer</sub> _imageWidth_: override default SVG's  width.
      1. <sub>integer</sub> _imageHeight_: override default SVG's  width.
      1. <sub>integer</sub> _channels_: 1 (Alpha only), 3 (RGB) or 4 (RGBA).
      1. <sub>boolean</sub> _fit_: fit the SVG dimensions to [imageWidth, imageHeight].
      1. <sub>string</sub> _elementId_: draws a subset of a SVG starting from an element's id. For example, if you have a layer called "layer1" that you wish to render, pass "#layer1" as the id.
> > ##### example: #####
```
  var svg = new SVG();
  svg.Write(<svg><circle cx="50" cy="50" r="25" fill="red"/></svg>);
  svg.Rotate(Math.PI/4); // +45 deg
  svgimage = svg.RenderImage(100, 100, true);
  new File('test.png').content = EncodePngImage( svgimage )
```

#### <font color='white' size='1'><b>SetVisible</b></font> ####

> <sub>boolean</sub> <b>SetVisible</b>( elementId, polarity )
> > ##### arguments: #####
      1. <sub>string</sub> _elementId_: the id of the element with '#' prefix (eg. '#circle1').
      1. <sub>boolean</sub> _polarity_: false to hide, true to show.
> > ##### return value: #####
> > > true if the element visibility has been set, otherwise false.

#### <font color='white' size='1'><b>Scale</b></font> ####

> <sub>this</sub> <b>Scale</b>( sx, sy )
> > Applies scaling by _sx_, _sy_ to the current transformation.
> > The effect of the new transformation is to first scale the coordinates by _sx_ and _sy_,
> > then apply the original transformation to the coordinates.

#### <font color='white' size='1'><b>Rotate</b></font> ####

> <sub>this</sub> <b>Rotate</b>( radians )
> > Applies rotation by _radians_ to the current transformation.
> > The effect of the new transformation is to first rotate the coordinates by _radians_,
> > then apply the original transformation to the coordinates.

#### <font color='white' size='1'><b>Translate</b></font> ####

> <sub>this</sub> <b>Translate</b>( tx, ty )
> > Applies a translation by _tx_, _ty_ to the current transformation.
> > The effect of the new transformation is to first translate the coordinates by _tx_ and _ty_,
> > then apply the original transformation to the coordinates.

### Properties ###

#### <font color='white' size='1'><b>dpi</b></font> ####

> <sub>integer</sub> | <sub>Array</sub> <b>dpi</b> <font color='red'><sub>write-only</sub></font>
> > Sets the dpi of the resulting image. If the argument is an Array (like [dpiX, dpiY ](.md)) X and Y dpi can be set aside.

#### <font color='white' size='1'><b>width</b></font> ####

> <b>width</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the default width of the SVG.

#### <font color='white' size='1'><b>height</b></font> ####

> <b>height</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the default height of the SVG.

#### <font color='white' size='1'><b>title</b></font> ####

> <b>title</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the title of the SVG.

#### <font color='white' size='1'><b>metadata</b></font> ####

> <b>metadata</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the metadata string of the SVG.

#### <font color='white' size='1'><b>description</b></font> ####

> <b>description</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the description string of the SVG.

#### <font color='white' size='1'><b>images</b></font> ####

> <b>images</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)

### callback functions ###
  * <sub>ImageObject</sub> **onImage**( uri )
> > Called when the SVG renderer need to draw an image element. _uri_ is the name of the requested image. The function must return an image Object.
> > ##### example: #####
```
  var svg = new SVG();
  svg.Write(<svg><image x="0" y="0" path="img.png"/></svg>);
  svg.onImage = function(href) {
   return DecodePngImage( new File('myImage.png').Open('r') );
  }
```


---

- [top](#jssvg_module.md) - [main](JSLibs.md) -
