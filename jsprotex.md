<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jsprotex/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jsprotex/qa.js) -
# jsprotex module #

> jsprotex is a procedural texture generation module to let you create
> high resolution textures that fit in few lines of source code.
> The texture generator is separated into small operators with each their set of parameters.
> These operators can be connected togethers to produce the final result.




---

## class jsprotex::Texture ##
- [top](#jsprotex_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsprotex/texture.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( width, height, channels )
> <i><b>constructor</b></i>( sourceTexture )
> <i><b>constructor</b></i>( image )
> > Creates a new Texture object.
> > ##### arguments: #####
      1. <sub>integer</sub> _height_: height of texture.
      1. <sub>integer</sub> _width_: width of texture.
      1. <sub>integer</sub> _channels_: number of channels of the texture (current limit is 4). Channel has a meaning only in a few part of the API like ToHLS(), ToRGB(), ...
      1. <sub>Texture</sub> _sourceTexture_: an existing Texture object (acs like a copy constructor).
      1. <sub>ImageObject</sub> _image_: an existing Image object (From a jpeg image for example)
> > ##### note: #####
> > > jsprotex uses single precision values per channel. The visibles values are in range [0,1 ](.md).
> > > The darker value is 0.0 and the brighter value is 1.0.

> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jsimage');

  var image = DecodePngImage( new File('picture.png').Open("r") );
  var tex = new Texture(image);
  tex.NormalizeLevels();
```

### Methods ###

#### <font color='white' size='1'><b>Free</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Free</b>()
> > Free the memory allocated by the current texture.
> > This operation is not mendatory but can be usefull to free memory.

#### <font color='white' size='1'><b>Swap</b></font> ####

> <sub>this</sub> <b>Swap</b>( otherTexture )
> > Swaps the content of two textures the current one and _otherTexture_.
> > ##### arguments: #####
      1. <sub>Texture</sub> _otherTexture_: texture object against witch the exchange is done.
> > ##### example: #####
```
  function AddAlphaChannel( tex ) {

    if ( tex.channels == 1 )
      new Texture(tex.width, tex.height, 2).SetChannel(0, tex, 0).Swap(tex);
    else if ( tex.channels == 3 )
      new Texture(tex.width, tex.height, 4).SetChannel(0, tex, 0).SetChannel(1, tex, 1).SetChannel(2, tex, 2).Swap(tex);
  }
```

#### <font color='white' size='1'><b>ClearChannel</b></font> ####

> <sub>this</sub> <b>ClearChannel</b>( [[.md](.md) channel ] )
> > Clears (set to 0) the given _channel_ or all channels if the method is called without argument.

#### <font color='white' size='1'><b>SetChannel</b></font> ####

> <sub>this</sub> <b>SetChannel</b>( destinationChannel, otherTexture, sourceChannel )
> > Replace the _destinationChannel_ channel of the current texture with the _sourceChannel_ channel of the _otherTexture_.
> > ##### arguments: #####
      1. <sub>integer</sub> _destinationChannel_: a channel of the current texture.
      1. <sub>Texture</sub> _otherTexture_: the texture from witch a channel will be imported.
      1. <sub>integer</sub> _destinationChannel_: the channel of the _otherTexture_ to be imported.
> > ##### example: #####
```
  function NoiseChannel( tex, channel ) {

    var tmp = new Texture(tex.width, tex.height, 1);
    tmp.AddNoise();
    tex.SetChannel(channel, tmp, 0);
    tmp.Free();
  }
```

#### <font color='white' size='1'><b>ToHLS</b></font> ####

> <sub>this</sub> <b>ToHLS</b>()
> > Does a conversion from RGB (Red, Green, Blue) to HSV (Hue, Saturation, Value) colorspace.

#### <font color='white' size='1'><b>ToRGB</b></font> ####

> <sub>this</sub> <b>ToRGB</b>()
> > Does a conversion from HSV (Hue, Saturation, Value) to RGB (Red, Green, Blue) colorspace.

#### <font color='white' size='1'><b>Aliasing</b></font> ####

> <sub>this</sub> <b>Aliasing</b>( count [[.md](.md) , curve] )
> > Reduce the number of values used for each channel.
> > ##### arguments: #####
      1. <sub>integer</sub> _count_: the number of different pixel component intensity value in the resulting texture.
      1. <sub>curveInfo</sub> _curve_: the transformation curve used for each value. For further information about <sub>curveInfo</sub> , see below.
> > ##### note: #####
> > > If _curveInfo_ is not provided, each channel is processed in a linear manner using the following formula:
> > > > floor( count `*` colorValue ) / count

> > ##### note: #####
> > > Each channel is processed independently.

> > ##### example 1: #####
```
  var t = Cloud(size, 0.5);
  t.Aliasing(2);
```
> > ##### example 2: #####
```
  const curveLinear = function(v) { return v }
  var t = Cloud(size, 0.5);
  t.Aliasing(8, curveLinear);
  t.BoxBlur(3, 3)
```

#### <font color='white' size='1'><b>Colorize</b></font> ####

> <sub>this</sub> <b>Colorize</b>( fromColorInfo, toColorInfo [[.md](.md) , power = 1 ] )
> > ##### arguments: #####
      1. <sub>colorInfo</sub> _fromColorInfo_: The color to be changed.
      1. <sub>colorInfo</sub> _toColorInfo_: The substitute color. For further information about <sub>colorInfo</sub> see below.
      1. <sub>real</sub> _power_: strength of color replacement.
> > ##### example: #####
```
  const BLUE = [ 0,0,1,1 ];
  const WHITE = [ 1,1,1,1 ];

  var texture = new Texture( 100, 100, 3 );
  ...
  texture.Colorize( WHITE, BLUE, 0 );
```

#### <font color='white' size='1'><b>ExtractColor</b></font> ####

> <sub>this</sub> <b>ExtractColor</b>( sourceTexture, sourceColorInfo [[.md](.md) , strength = 1] )
> > Fill the current texture with a given color from _sourceTexture_.
> > ##### arguments: #####
      1. <sub>Texture</sub> _sourceTexture_: the texture from witch the color will be extracted.
      1. <sub>colorInfo</sub> _sourceColorInfo_: The color to extract.
      1. <sub>real</sub> _strength_: The strength of the exraction.
> > ##### note: #####
> > > The current texture must have only one channel because the method only extracts one color.

> > ##### example: #####
```
  t1.ExtractColor(t, RED, 10);
```

#### <font color='white' size='1'><b>NormalizeLevels</b></font> ####

> <sub>this</sub> <b>NormalizeLevels</b>()
> > Changes the range of each pixel component intensity value of the texture. The resulting range of each pixel component intensity value will be [0,1 ](.md)
> > ##### note: #####
> > > Normalization is sometimes called contrast stretching.

#### <font color='white' size='1'><b>ClampLevels</b></font> ####

> <sub>this</sub> <b>ClampLevels</b>( min, max )
> > All pixel component intensity value that are out of the [min,max ](.md) range are forced to [min,max ](.md) range.
> > ##### arguments: #####
      1. <sub>real</sub> _min_: low value
      1. <sub>real</sub> _max_: high value

#### <font color='white' size='1'><b>CutLevels</b></font> ####

> <sub>this</sub> <b>CutLevels</b>( min, max )
> > All pixel component intensity value that are out of the [min,max ](.md) range are forced to [0,1 ](.md) range.
> > ##### arguments: #####
      1. <sub>real</sub> _min_: low value
      1. <sub>real</sub> _max_: high value

#### <font color='white' size='1'><b>InvertLevels</b></font> ####

> <sub>this</sub> <b>InvertLevels</b>()
> > Each pixel component intensity value is mathematically inverted ( v = 1 / v ).

#### <font color='white' size='1'><b>OppositeLevels</b></font> ####

> <sub>this</sub> <b>OppositeLevels</b>()
> > Each pixel component intensity value is set to its mathematical opposite ( v = -v ).

#### <font color='white' size='1'><b>PowLevels</b></font> ####

> <sub>this</sub> <b>PowLevels</b>( power )
> > Each pixel component intensity value is powered by _power_ ( v = v ^ _power_ ).

#### <font color='white' size='1'><b>MirrorLevels</b></font> ####

> <sub>this</sub> <b>MirrorLevels</b>( threshold, mirrorFromTop )
> > Each pixel component intensity value is mirrored toward the top or the bottom.
> > ##### arguments: #####
      1. <sub>real</sub> _threshold_: the point from the values are reflected.
      1. <sub>boolean</sub> _mirrorFromTop_: if true, the values over _threshold_ are reflected toward the bottom, else values under _threshold_ are reflected toward the top.

#### <font color='white' size='1'><b>WrapLevels</b></font> ####

> <sub>this</sub> <b>WrapLevels</b>( modulo )
> > Each pixel component intensity value is moduloed by _modulo_ ( v = v % _modulo_ ).
> > ##### arguments: #####
> > > <sub>real</sub> modulo: the non-integer modulo

#### <font color='white' size='1'><b>AddNoise</b></font> ####

> <sub>this</sub> <b>AddNoise</b>( [[.md](.md) color ] )
> > Adds a random noise to the current texture.
> > ##### arguments: #####
      1. <sub>colorInfo</sub> _color_: noise color.

#### <font color='white' size='1'><b>Desaturate</b></font> ####

> <sub>this</sub> <b>Desaturate</b>( sourceTexture [[.md](.md) , mode] )
> > Desaturates _sourceTexture_ texture an put the result in the current texture.
> > ##### arguments: #####
      1. <sub>Texture</sub> _sourceTexture_: texture from witch the desaturation will be done.
      1. <sub>integer</sub> _mode_: is the type of desaturation, either Texture.desaturateLightness, Texture.desaturateSum or Texture.desaturateAverage

#### <font color='white' size='1'><b>Set</b></font> ####

> <sub>this</sub> <b>Set</b>( otherTexture )
> <sub>this</sub> <b>Set</b>( color )
> > ##### arguments: #####
      1. <sub>Texture</sub> _otherTexture_:
      1. <sub>colorInfo</sub> _color_:
> > Set a texture with another texture or a given _color_.

#### <font color='white' size='1'><b>Add</b></font> ####

> <sub>this</sub> <b>Add</b>( textureObject )
> <sub>this</sub> <b>Add</b>( colorInfo )
> > Mathematically adds a texture (_textureObject_) or a given color (_colorInfo_) to the current texture.

#### <font color='white' size='1'><b>Mult</b></font> ####

> <sub>this</sub> <b>Mult</b>( textureObject )
> <sub>this</sub> <b>Mult</b>( colorInfo )
> > Mathematically multiply a texture (_textureObject_) or a given color (_colorInfo_) to the current texture.

#### <font color='white' size='1'><b>Blend</b></font> ####

> <sub>this</sub> <b>Blend</b>( otherTexture, blendTexture )
> <sub>this</sub> <b>Blend</b>( otherTexture, color )
> > Mathematically blends a texture (_textureObject_) or a given color (_colorInfo_) to the current texture.
> > ##### arguments: #####
      1. <sub>Texture</sub> _otherTexture_: the texture to blend with the current one.
      1. <sub>Texture</sub> _blendTexture_: the texture that contains the blending coefficients for each pixel.
      1. <sub>colorInfo</sub> _color_: the color (ratio) ised to blend the current texture with _otherTexture_
> > ##### note: #####
> > > The blend fornula is: this pixel = blend `*` _textureObject1_ + (1-blend) `*` _textureObject2_ pixel or _colorInfo_

> > ##### example: #####
```
  var tmp = new Texture(size, size, 1);
  tmp.ClearChannel();
  tmp.SetRectangle(10,10,size-10,size-10,1);
  t.Blend(tmp, 0.7);
```

#### <font color='white' size='1'><b>SetPixel</b></font> ####

> <sub>this</sub> **SetPixel**( x, y, colorInfo )
> > Sets the color of the given pixel.
> > ##### note: #####
> > > If x and y are wrapped to the image width and height.

#### <font color='white' size='1'><b>SetRectangle</b></font> ####

> <sub>this</sub> <b>SetRectangle</b>( x0, y0, x1, y1, color )
> > Draws a rectangle of the _colorInfo_ color over the current texture.
> > ##### arguments: #####
      1. <sub>integer</sub> _x0_:
      1. <sub>integer</sub> _y0_:
      1. <sub>integer</sub> _x1_:
      1. <sub>integer</sub> _y1_:
      1. <sub>colorInfo</sub> _color_:

#### <font color='white' size='1'><b>Rotate90</b></font> ####

> <sub>this</sub> <b>Rotate90</b>( count )
> > Make _count_ 90 degres rotations.
> > ##### arguments: #####
      1. <sub>integer</sub> _count_: the number of integer rotation to perform with the current texture. _count_ may be negative.
> > ##### note: #####
> > > For non-integer rotations, see RotoZoom() function.

#### <font color='white' size='1'><b>Flip</b></font> ####

> <sub>this</sub> <b>Flip</b>( horizontally, vertically )
> > Flips the current texture horizontally, vertically or both.
> > ##### arguments: #####
      1. <sub>boolean</sub> _horizontally_: flips the texture horizontally (against x axis).
      1. <sub>boolean</sub> _vertically_: flips the texture vertically (against y axis).

#### <font color='white' size='1'><b>RotoZoom</b></font> ####

> <sub>this</sub> <b>RotoZoom</b>( centerX, centerY, zoomX, zoomY, rotations )
> > Make a zoom and/or a rotation of the current texture.
> > ##### arguments: #####
      1. <sub>real</sub> _centerX_: coordinate of the center of the zoom or rotation.
      1. <sub>real</sub> _centerY_:
      1. <sub>real</sub> _zoomX_: the zoom factor (use 1 for none).
      1. <sub>real</sub> _zoomY_:
      1. <sub>real</sub> _rotations_: the number of totations to perform. 0.25 is 90 degres (use 0 for none).

#### <font color='white' size='1'><b>Resize</b></font> ####

> <sub>this</sub> <b>Resize</b>( newWidth, newHeight, [[.md](.md) interpolate = false [[.md](.md), borderMode = Texture.borderWrap ]] )
> > Resize the current texture.
> > ##### arguments: #####
      1. <sub>integer</sub> _newWidth_:
      1. <sub>integer</sub> _newHeight_:
      1. <sub>boolean</sub> _interpolate_: uses a linear interpolation.
      1. <sub>enum</sub> _borderMode_: how to manage the border. either Texture.borderClamp, Texture.borderWrap, Texture.borderMirror or Texture.borderValue.

#### <font color='white' size='1'><b>Convolution</b></font> ####

> <sub>this</sub> <b>Convolution</b>( kernel, [[.md](.md) borderMode = Texture.borderWrap ] )
> > Apply a convolution to the current texture using _kernel_ factors.
> > ##### arguments: #####
      1. <sub>Array</sub> _kernel_: kernel is a square matrix
      1. <sub>enum</sub> _borderMode_: how to manage the border. either Texture.borderClamp, Texture.borderWrap, Texture.borderMirror or Texture.borderValue.
> > ##### note: #####
> > > The convolution is a complex transformation that could be very slow with big kernels.

> > ##### note: #####
> > > Because convolution apples a factor to each point of the texture,
> > > it is recomanded to normalize the levels of the texture using Mult() or NormalizeLevels()

> > ##### example: #####
```
  const kernelGaussian = [0,3,10,3,0, 3,16,26,16,3, 10,26,26,26,10, 3,16,26,16,3, 0,3,10,3,0 ];
  const kernelGaussian2 = [2,4,5,4,2, 4,9,12,9,4, 5,12,15,12,5, 4,9,12,9,4, 2,4,5,4,2]; // G(r) = pow(E,-r*r/(2*o*o))/sqrt(2*PI*o);
  const kernelEmboss = [-1,0,0, 0,0,0 ,0,0,1];
  const kernelLaplacian = [-1,-1,-1, -1,8,-1, -1,-1,-1];
  const kernelLaplacian4 = [0,-1,0, -1,4,-1 ,0,-1,0];
  const kernelShift = [0,0,0, 0,0,0 ,0,0,1];
  const kernelCrystals = [0,-1,0, -1,5,-1, 0,-1,0];
  ...
  texture.Convolution(kernelGaussian);
  texture.NormalizeLevels();
```

#### <font color='white' size='1'><b>ForEachPixels</b></font> ####

> <sub>this</sub> <b>ForEachPixels</b>( function )
> > Call _function_ for each pixel of the texture. the function is called with the arguments (x, y, pixel) where pixel is an array of levels.
> > ##### note: #####
> > > Because a JS function is called for each pixel, the processing could be very slow.

> > ##### example: #####
> > > draws a line from (0,0) to (100,100)
```
  var texture = new Texture(100, 100, 3); // RGB texture
  texture.Set(0); // clears the texture
  texture.ForEachPixels(function(x, y, pixel) {
   if ( x == y ) {
    pixel[0] = 1; // Red
    pixel[1] = 1; // Green
    pixel[2] = 1; // Blue
    return pixel;
   }
  });
```

#### <font color='white' size='1'><b>BoxBlur</b></font> ####

> <sub>this</sub> <b>BoxBlur</b>( blurWidth, blurHeight )
> > Apply a box blur of the given width and height.
> > ##### note: #####
> > > BoxBlur is very fast but the result is not very smooth.

> > ##### example: #####
```
  function AddPixels(t, count) {
   while ( count-- > 0 )
    t.SetPixel(Texture.RandInt(), Texture.RandInt(), 1);
  }

  var texture = new Texture(128, 128, 3);
  texture.Set(0);
  AddPixels(texture, 100);
  texture.BoxBlur(20,20);
  texture.NormalizeLevels();
```

#### <font color='white' size='1'><b>NormalizeVectors</b></font> ####

> <sub>this</sub> <b>NormalizeVectors</b>()
> > Converts each pixel into a vector, normalize this vector, then store the vector as a pixel.

#### <font color='white' size='1'><b>Normals</b></font> ####

> <sub>this</sub> <b>Normals</b>( [[.md](.md) amplify = 1 ] )
> > Converts the texture to a normals map using the Sobel filter.

#### <font color='white' size='1'><b>Light</b></font> ####

> <sub>this</sub> <b>Light</b>( normalsTexture, lightPosition, ambiantColor, diffuseColor, specularColor, bumpPower, specularPower )
> > Floodlight the current texture using the _normalsTexture_ as bump map.
> > ##### arguments: #####
      1. <sub>Texture</sub> _normalsTexture_: the bump map where each pixel is a 3D vector.
      1. <sub>Array</sub> _lightPosition_: is the position of the light in a 3D space ( [x, y, z ](.md) )
      1. <sub>colorInfo</sub> _ambiantColor_:
      1. <sub>colorInfo</sub> _diffuseColor_:
      1. <sub>colorInfo</sub> _specularColor_:
      1. <sub>real</sub> _bumpPower_:
      1. <sub>real</sub> _specularPower_:
> > ##### example: #####
```
  var bump = new Texture(size, size, 3).Cells(8, 0).Add( new Texture(size, size, 3).Cells(8, 1).OppositeLevels() ); // broken floor
  bump.Normals();
  var texture = new Texture(size, size, 3);
  texture.Set(1);
  texture.Light( bump, [-1, -1, 1], 0, [0.1, 0.3, 0.4], 0.2, 0.5, 10 );
```

#### <font color='white' size='1'><b>Trim</b></font> ####

> <sub>this</sub> <b>Trim</b>( x0, y0, x1, y1 )
> > Remove the part of the texture that is outside the rectangle (x1,y1)-(x2,y2).

#### <font color='white' size='1'><b>Copy</b></font> ####

> <sub>this</sub> <b>Copy</b>( sourceTexture, x, y [[.md](.md) , borderMode = Texture.borderClamp] )
> > Copy _sourceTexture_ in the current texture at the position (_x_, _y_).
> > ##### arguments: #####
      1. <sub>Texture</sub> _sourceTexture_:
      1. <sub>integer</sub> _x_:
      1. <sub>integer</sub> _y_:
      1. <sub>enum</sub> _borderMode_: one of Texture.borderWrap or Texture.borderClamp.

#### <font color='white' size='1'><b>Paste</b></font> ####

> <sub>this</sub> <b>Paste</b>( texture, x, y, borderMode )
> > Paste _sourceTexture_ in the current texture at the position (_x_, _y_).
> > ##### arguments: #####
      1. <sub>Texture</sub> _sourceTexture_:
      1. <sub>integer</sub> _x_:
      1. <sub>integer</sub> _y_:
      1. <sub>enum</sub> _borderMode_: one of Texture.borderWrap or Texture.borderClamp.

#### <font color='white' size='1'><b>Export</b></font> ####

> <sub>ImageObject</sub> <b>Export</b>( [[.md](.md) x, y, width, height] )
> > Creates an image object from the whole or a part of current texture.
> > ##### arguments: #####
      1. <sub>integer</sub> _x_:
      1. <sub>integer</sub> _y_:
      1. <sub>integer</sub> _width_:
      1. <sub>integer</sub> _height_:
> > ##### return value: #####
> > > An image object.

> > ##### example: #####
```
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

#### <font color='white' size='1'><b>Import</b></font> ####

> <sub>this</sub> <b>Import</b>( sourceImage, x, y [[.md](.md) , borderMode] )
> > Draws the _image_ over the current texture at position (_x_, _y_).
> > ##### arguments: #####
      1. <sub>ImageObject</sub> _sourceImage_:
      1. <sub>integer</sub> _x_: X position of the image in the current texture.
      1. <sub>integer</sub> _y_: Y position of the image in the current texture.
      1. <sub>enum</sub> _borderMode_: one of Texture.borderWrap, Texture.borderClamp.
> > ##### example: #####
```
  var file = new File('myImage.png').Open('r'); // note: Open() returns the file object.
  var image = DecodePngImage( file );
  file.Close();
  texture.Import( image, 0, 0 );

  Ogl.MatrixMode(MODELVIEW);
  Ogl.DefineTextureImage(TEXTURE_2D, undefined, texture);
  Ogl.LoadIdentity();
  ...
```

#### <font color='white' size='1'><b>Shift</b></font> ####

> <sub>this</sub> <b>Shift</b>( offsetX, offsetY [[.md](.md) , borderMode] )
> > Shift the current image.
> > ##### arguments: #####
      1. <sub>integer</sub> _offsetX_:
      1. <sub>integer</sub> _offsetY_:
      1. <sub>enum</sub> _borderMode_: one of Texture.borderWrap, Texture.borderClamp, Texture.borderMirror or Texture.borderValue.

#### <font color='white' size='1'><b>Displace</b></font> ####

> <sub>this</sub> <b>Displace</b>( displaceTexture, factor [[.md](.md) , borderMode] )
> > Move each pixel of the texture according to the
> > ##### arguments: #####
      1. <sub>Texture</sub> _displaceTexture_: is a texture that contains displacement vectors.
      1. <sub>real</sub> _factor_: displacement factor of each pixel.
      1. <sub>enum</sub> _borderMode_: one of Texture.borderWrap, Texture.borderClamp, Texture.borderMirror or Texture.borderValue.

#### <font color='white' size='1'><b>Cells</b></font> ####

> <sub>this</sub> <b>Cells</b>( density, regularity )
> > Draws cells in the current texture.
> > ##### arguments: #####
      1. <sub>integer</sub> _density_:
      1. <sub>real</sub> _regularity_:

#### <font color='white' size='1'><b>AddGradiantQuad</b></font> ####

> <sub>this</sub> <b>AddGradiantQuad</b>( topLeft, topRight, bottomLeft, bottomRight )
> > Add a quad radiant to the current texture
> > ##### arguments: #####
      1. <sub>colorInfo</sub> _topLeft_: color of the top-left corner.
      1. <sub>colorInfo</sub> _topRight_: color of the top-right corner.
      1. <sub>colorInfo</sub> _bottomLeft_: color of the bottom-left corner.
      1. <sub>colorInfo</sub> _bottomRight_: color of the bottom-right corner.
> > ##### example: #####
```
  const RED = [1,0,0,1];
  const BLUE = [0,0,1,1];
  const BLACK = [0,0,0,1];

  texture.Set(0); // clears the texture
  texture.AddGradiantQuad(BLACK, RED, BLUE, BLACK);
```

#### <font color='white' size='1'><b>AddGradiantLinear</b></font> ####

> <sub>this</sub> <b>AddGradiantLinear</b>( curveInfoX, curveInfoY )
> > Add a linear radiant using a curve for X and Y. Each point of the curve is the light intensity of a pixel.
> > ##### arguments: #####
      1. <sub>curveInfo</sub> _curveInfoX_:
      1. <sub>curveInfo</sub> _curveInfoY_:
> > ##### example 1: #####
```
  const curveHalfSine = function(v) Math.cos(v*Math.PI/2);
  const curveOne = function() 1;

  texture.Set(0); // clears the texture
  texture.AddGradiantLinear(curveHalfSine, curveOne);
```
> > ##### example 2: #####
```
  texture.AddGradiantLinear([0,1,0], [0,1,0]);
```

#### <font color='white' size='1'><b>AddGradiantRadial</b></font> ####

> <sub>this</sub> <b>AddGradiantRadial</b>( curveInfo [[.md](.md) , drawToCorner = false] )
> > Add a radial radiant using a curve from the center to the outside. Each point of the curve is the light intensity of a pixel.
> > ##### arguments: #####
      1. <sub>curveInfo</sub> _curveInfo_:
      1. <sub>boolean</sub> _drawToCorner_: If true, the curve goes from the center to the corner, else from the center to the edge.
> > ##### example 1: #####
```
  const curveHalfSine = function(v) Math.cos(v*Math.PI/2);

  texture.AddGradiantRadial(curveHalfSine);
```
> > ##### example 2: #####
```
  texture.AddGradiantRadial([0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1], true);
```

#### <font color='white' size='1'><b>AddCracks</b></font> ####

> <sub>this</sub> <b>AddCracks</b>( count, crackLength, wayVariation [[.md](.md) , color = 1] [[.md](.md) , curve = 1] )
> > Adds cracks to the current texture.
> > ##### arguments: #####
      1. <sub>integer</sub> _count_: number of cracks to draw.
      1. <sub>integer</sub> _crackLength_: length of each crack.
      1. <sub>real</sub> _wayVariation_: the variation of each crack (eg. 0 is straight and 1 is randomly curved).
      1. <sub>colorInfo</sub> _color_: the color of the crack.
      1. <sub>curveInfo</sub> _curve_: the curve that defines the intensity of each point of the crack.
> > ##### note: #####
> > > The curve is computed before each crack is drawn.

> > ##### <font color='red'>beware</font>: #####
> > > Adding cracks on a white texture does nothing.

> > ##### examples: #####
```
  texture.AddCracks( 1000, 10, 0, RED, 1 );

  texture.AddCracks( 100, 100, 0, 1, [1,0,1,0,1,0,1,0,1] );

  texture.AddCracks( 10, 10000, 0.1, 1, curveLinear );

  texture.AddCracks( 10, 10000, 0.1, 1, function(v) Texture.RandReal() );
```

#### <font color='white' size='1'><b>PixelAt</b></font> ####

> <sub>Array</sub> <b>PixelAt</b>( x, y )
> > Read the value of a pixel in the current texture.
> > ##### arguments: #####
      1. <sub>integer</sub> _x_
      1. <sub>integer</sub> _y_
> > ##### return value: #####
> > > returns the pixel value at position (x, y) in the current texture. If the texture is RGB, an array of 3 values is returned.

> > ##### example: #####
```
  var texture = new Texture(20,20,3);
  texture.Set([0.1, 0.2, 0.3]);
  var pixel = texture.PixelAt(10,10);
  Print( 'Red: '+pixel[0], 'Green: '+pixel[1], 'Blue: '+pixel[2] );
```

#### <font color='white' size='1'><b>LevelRange</b></font> ####

> <sub>this</sub> <b>LevelRange</b>()
> > Returns the [lowest,highest ](.md) level value of the texture.

### Properties ###

#### <font color='white' size='1'><b>vmax</b></font> ####

> <sub>real</sub> <b>vmax</b>
> > Is the higher pixel component intensity value. Higher values are not brighter.
> > <br />
> > See Normalize() function.

#### <font color='white' size='1'><b>width</b></font> ####

> <sub>integer</sub> <b>width</b>
> > Width of the texture in pixel.

#### <font color='white' size='1'><b>height</b></font> ####

> <sub>integer</sub> <b>height</b>
> > Height of the texture in pixel.

#### <font color='white' size='1'><b>channels</b></font> ####

> <sub>integer</sub> <b>channels</b>
> > Number of channels of the texture.

### Static functions ###

#### <font color='white' size='1'><b>RandSeed</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>RandSeed</b>( seed )
> > Resets a random sequence of the Mersenne Twister random number generator.
> > ##### example: #####
```
  Texture.RandSeed(1234);
  Print( Texture.RandReal(), '\n' );
  Print( Texture.RandReal(), '\n' );
  Print( Texture.RandReal(), '\n' );
```
> > will always prints:
```
  0.19151945020806033
  0.4976636663772314
  0.6221087664882906
```

#### <font color='white' size='1'><b>RandInt</b></font> ####

> <sub>integer</sub> <b>RandInt</b>
> > Generates a random number on `[ 0 , 0x7fffffff ]` interval.

#### <font color='white' size='1'><b>RandReal</b></font> ####

> <sub>real</sub> <b>RandReal</b>
> > Generates a random number on `[ 0 , 1 ]` real interval.

### Note ###

> Nearly all methods returns _this_ object. This allows to easily chain texture operations.
> ##### example: #####
```
 var texture = new Texture(64,64,3);
 texture.Set(0).AddNoise().AddCracks(10, 100, 0, BLUE, 1);
```

### Definitions ###
  * **pixel component intensity value**
> > The pixel component intensity value is the value of a channel of a pixel.
> > In a RGB texture, each pixel has 3 components (Red, Green and Blue) and each one represents a light intensity.

### Special values and data types ###

  * **borderMode**
    * Texture.borderWrap: get the pixel from the the other side (opposite edge).
    * Texture.borderClamp: do not take any other pixel.
    * Texture.borderMirror: take the pixel from this edge but in a reverse way (like a mirror).
    * Texture.borderValue: use the pixel of the current border.

  * **colorInfo**
> > a _colorInfo_ can be one of the following type:
    * _real_: in this case, the same value is used for each channel (eg. 0.5 gor gray).
    * _Array_: that contains the pixel component intensity value of each channel (eg. `[ 1, 1, 1 ]` is white in RGB mode).
    * _string_: the string represents an HTML color like ```#1100AA or ```#8800AAFF (depending the number of channels)
> > ##### examples: #####
```
  const RED = [1,0,0,1];
  const GREEN = [0,1,0,1];
  const BLUE = [0,0,1,1];
  const MAGENTA = [1,0,1,1];
  const CYAN = [0,1,1,1];
  const YELLOW = [1,1,0,1];
  const GRAY = [.5,.5,.5,1];
  const BLACK = [0,0,0,1];
  const WHITE = [1,1,1,1];
```

  * **curveInfo**
> > _curveInfo_ describes a curve and can be one of the following type:
      * _real_: this describes a constant curve.
      * _function( <sub>real</sub> posX, INT indexX )_: the function is called and must returns values for each curve point.
      * _Array_: an Array that describes the curve (no interpolation is done between values).
      * _buffer_: a Blob or a string that contains the curve data.
> > ##### examples: #####
```
  const curveLinear = function(v) { return v }
  const curveHalfSine = function(v) { return Math.cos(v*Math.PI/2) }
  const curveSine = function(v) { return Math.sin(v*Math.PI) }
  const curveInverse = function(v) { return 1/v }
  const curveSquare = function(v) { return v*v }
  const curveDot = function(v,i) { return i%2 }
  const curveZero = function(v,i) { return 0 }
  const curveOne = function(v,i) { return 1 }
  function GaussianCurveGenerator(c) { return function(x) { return Math.exp( -(x*x)/(2*c*c) ) } }

  texture.AddGradiantRadial( GaussianCurveGenerator( 0.5 ) );
```

  * **ImageObject**
> > An image object is nothing else that a buffer of data with a width, a height and a channels properties.

### Examples ###
##### example 1: #####

> Some utility functions.
```
function Cloud( size, amp ) {

   var octaves = Math.log(size) / Math.LN2;
   var a = 1, s = 1;
   var cloud = new Texture(s, s, 1);
   cloud.ClearChannel();
   while ( octaves-- > 0 ) {

      cloud.AddNoise(a);
      a *= amp;
      s *= 2;
      cloud.Resize(s, s, false);
      cloud.BoxBlur(5, 5);
   }
   cloud.NormalizeLevels();
   return cloud;
}

function DesaturateLuminosity( tex ) {

   tex.Mult([0.2126, 0.7152, 0.0722]);
   var tmp = new Texture(tex.width, tex.height, 1).Desaturate(tex, Texture.desaturateSum);
   tex.Swap(tmp);
   tmp.Free();
}

function AddAlphaChannel( tex ) {

   if ( tex.channels == 1 )
      new Texture(tex.width, tex.height, 2).SetChannel(0, tex, 0).Swap(tex);
   else if ( tex.channels == 3 )
      new Texture(tex.width, tex.height, 4).SetChannel(0, tex, 0).SetChannel(1, tex, 1).SetChannel(2, tex, 2).Swap(tex);
}
```

##### example 2: #####
> This example shows how to save a texture to the disk.
```
LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsprotex');
LoadModule('jsimage');

var bacteria = new Texture(256,256,3);
bacteria.Set(0);
bacteria.AddCracks(100, 100, 2, undefined, function(v) v);
bacteria.BoxBlur(5,5);
bacteria.MirrorLevels(0.5, false);
bacteria.BoxBlur(2,2);

new File('test.png').content = EncodePngImage(bacteria.Export());
```

##### example 3: #####
> This is a complete example that displays a texture in real-time in a OpenGL environment.
```
LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsgraphics');
LoadModule('jsprotex');
LoadModule('jsimage');

with (Ogl) {

   var texture = new Texture(128, 128, 3);
   texture.Set(0);
   // play here for static textures

   function UpdateTexture(imageIndex) {

      // play here for dynamic textures
      texture.Set(0);
      texture.AddNoise(1);
   }

   var win = new Window();
   win.Open();
   win.CreateOpenGLContext();
   win.rect = [200,200,800,800];

   function ResizeWindow(w, h) {

      Ogl.Viewport(0,0,w,h);
      Ogl.MatrixMode(PROJECTION);
      Ogl.LoadIdentity();
      Ogl.Ortho(0,0,10,10, -1, 1);
      Render();
   }

   ShadeModel(FLAT);
   FrontFace(CCW);
   ClearColor(0, 0, 0, 0);
   Enable(TEXTURE_2D);
   tid = GenTexture();
   BindTexture(TEXTURE_2D, tid);
   TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST); // GL_LINEAR
   TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
   Clear( COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT );

   function Render(imageIndex) {

      UpdateTexture();
      DefineTextureImage(TEXTURE_2D, undefined, texture);
      MatrixMode(MODELVIEW);
      LoadIdentity();
      Scale(1, -1, 1);
      Color(1,1,1);
      Begin(QUADS);
      TexCoord( 0, 0 );
      Vertex( -1, -1, 0 );
      TexCoord( 1, 0, 0 );
      Vertex( 1, -1 );
      TexCoord( 1, 1 );
      Vertex( 1, 1 );
      TexCoord( 0, 1 );
      Vertex( -1, 1 );
      End();
      win.SwapBuffers();
      MaybeCollectGarbage();
   }

   win.onsize = ResizeWindow;
   var end = false;
   win.onkeydown = function( key, l ) { end = ( key == 0x1B ) }
   while (!end) {

      win.ProcessEvents();
      Render();
   }
   win.Close();
}
```


---

- [top](#jsprotex_module.md) - [main](JSLibs.md) -
