<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jssound/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jssound/qa.js) -
# jssound module #

> Support wav, aiff, au, voc, sd2, flac, ... sound format using libsndfile
> and ogg vorbis using libogg and libvorbis.




---

## jssound static members ##
- [top](#jssound_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jssound/static.cpp?r=2555) -

#### <font color='white' size='1'><b>DecodeOggVorbis</b></font> ####
> <sub>soundObject</sub> <b>DecodeOggVorbis</b>( stream )  ![http://jslibs.googlecode.com/svn/wiki/deprecated.png](http://jslibs.googlecode.com/svn/wiki/deprecated.png)
> > Decodes a ogg vorbis sample to a sound object.
> > ##### arguments: #####
      1. <sub>streamObject</sub> _stream_: any object that has a Read function or that supports the NIStreamRead Native Interface ( file, socket, new Stream(buffer), ... ). For further details about stream objects, see jslang::Stream object and NativeInterface mechanism.
> > ##### return value: #####
> > > A sound object in a 16-bit per sample format.

> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jsio');
  LoadModule('jssound');

  var file = new File('41_30secOgg-q0.ogg');
  var buffer = file.content;
  var stream = new Stream(buffer);
  var mySoundObject = DecodeOggVorbis(stream);
  Print('sample length: ', mySoundObject.length, '\n');
```

#### <font color='white' size='1'><b>DecodeSound</b></font> ####

> <sub>soundObject</sub> <b>DecodeSound</b>( stream )  ![http://jslibs.googlecode.com/svn/wiki/deprecated.png](http://jslibs.googlecode.com/svn/wiki/deprecated.png)
> > Decodes a sample from any supported sound format to a sound object.
> > ##### arguments: #####
      1. <sub>streamObject</sub> _stream_: any object that has a Read function or that supports the NIStreamRead Native Interface ( file, socket, new Stream(buffer), ... ). For further details about stream objects, see jslang::Stream object and native interface mechanism.
> > ##### return value: #####
> > > A sound object in a 16-bit per sample format.

#### <font color='white' size='1'><b>SplitChannels</b></font> ####

> <sub>Array</sub> <b>SplitChannels</b>( sound )
> > Split channels of the _sound_ into an Array of monaural sound object.
> > ##### arguments: #####
      1. <sub>soundObject</sub> _sound_
> > ##### return value: #####
> > > An array that contains each individual channel of the sound.

### Note ###

> SoundObject concatenation can be achieved using the concat() method. This works becuase a sound object is a Blob with some extra properties.

### Examples ###
> ##### example 1: #####
```
 LoadModule('jsstd');
 LoadModule('jsio');
 LoadModule('jssound');

 var file = new File('41_30secOgg-q0.wav');
 file.Open('r');
 var pcm = DecodeSound(file);
 file.Close();
 Print('sample length: '+pcm.length, '\n');
```

> ##### example 2: #####
```
 LoadModule('jsstd');
 LoadModule('jsio');
 LoadModule('jssound');

 var file = new File('41_30secOgg-q0.wav');
 var buffer = file.content;
 var stream = new Stream(buffer);
 var pcm = DecodeSound(stream);

 Print('sample length: '+pcm.length, '\n');
```


---

## class jssound::OggVorbisDecoder ##
- [top](#jssound_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jssound/oggvorbisdecoder.cpp?r=2557) -
> The OggVorbisDecoder support ogg vorbis data format decoding.

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( stream )
> > Creates a new OggVorbisDecoder object. Seekable and non-seekable streams are supported.
> > ##### arguments: #####
      1. <sub>streamObject</sub> _stream_: is the data stream from where encoded data are read from.
> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jsio');
  LoadModule('jssound');
  var file = new File('41_30secOgg-q0.ogg'); // file: http://xiph.org/vorbis/listen.html
  file.Open('r');
  var dec = new OggVorbisDecoder( file );
  Print( dec.bits, '\n' );
  Print( dec.channels, '\n' );
  Print( dec.rate, '\n' );
  do {
   var block = dec.Read(10000);
   Print( 'frames: '+block.frames, '\n' );
  } while(block);
```

### Methods ###

#### <font color='white' size='1'><b>Read</b></font> ####

> <sub>soundObject</sub> <b>Read</b>( [[.md](.md)frames] )
> > Decodes a piece of audio data. If _frames_ argument is omited, the whole stream is decoded.
> > ##### arguments: #####
      1. <sub>integer</sub> _frames_: the number of frames to decode. A frame is a sample of sound.
> > ##### return value: #####
> > > A Blob object that has the following properties set: bits, rate, channels, frames

> > ##### <font color='red'>beware</font>: #####
> > > If all data has been decoded and the Read function is called again, the return expression is evaluated to `ALSE`.

> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jsio');
  LoadModule('jssound');
  var file = new File('41_30secOgg-q0.ogg'); // file: http://xiph.org/vorbis/listen.html
  file.Open('r');
  var dec = new OggVorbisDecoder( file );
  var block = dec.Read(10000);
  Print( 'rezolution: '+block.bits+' per channel', '\n' );
  Print( block.channels == 2 ? 'stereo' : 'mono', '\n' );
  Print( block.rate+' frames/seconds', '\n' );
  Print( 'time: '+(block.frames/block.rate)+' seconds', '\n' );
```

### Properties ###

#### <font color='white' size='1'><b>inputStream</b></font> ####

> object <b>inputStream</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the stream object where encoded audio data are read from.

#### <font color='white' size='1'><b>bits</b></font> ####

> <sub>integer</sub> <b>bits</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the number of bits per frame and per channel.

#### <font color='white' size='1'><b>rate</b></font> ####

> <sub>integer</sub> <b>rate</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the number of frames per seconds of the sound.

#### <font color='white' size='1'><b>channels</b></font> ####

> <sub>integer</sub> <b>channels</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the number of channels of the sound. 1 is mono, 2 is stereo.

#### <font color='white' size='1'><b>frames</b></font> ####

> <sub>integer</sub> <b>frames</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the length (in frames) of the sound.
> > To compute the duration of the sound, use (frames/rate)


---

## class jssound::SoundFileDecoder ##
- [top](#jssound_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jssound/soundfiledecoder.cpp?r=2557) -

> The SoundFileDecoder support various data format decoding.
> Main supported formats are: wav, aiff, au, voc, sd2, flac, ...
> For more information about supported formats, see http://www.mega-nerd.com/libsndfile/

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( stream )
> > Creates a new SoundFileDecoder object. Only seekable streams are supported.
> > ##### arguments: #####
      1. <sub>streamObject</sub> _stream_: is the data stream from where encoded data are read from.
> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jsio');
  LoadModule('jssound');
  var file = new File('41_30secOgg-q0.wav'); // file: http://xiph.org/vorbis/listen.html
  file.Open('r');
  var dec = new SoundFileDecoder( file );
  Print( dec.bits, '\n' );
  Print( dec.channels, '\n' );
  Print( dec.rate, '\n' );
  do {
   var block = dec.Read(10000);
   Print( 'frames: '+block.frames, '\n' );
  } while(block);
```

### Methods ###

#### <font color='white' size='1'><b>Read</b></font> ####

> <sub>soundObject</sub> <b>Read</b>( [[.md](.md)frames] )
> > Decodes a piece of audio data. If _frames_ argument is omited, the whole stream is decoded.
> > ##### arguments: #####
      1. <sub>integer</sub> _frames_: the number of frames to decode. A frame is a sample of sound.
> > ##### return value: #####
> > > A Blob object that has the following properties set: bits, rate, channels, frames

> > ##### <font color='red'>beware</font>: #####
> > > If all data has been decoded and the Read function is called again, the return expression is evaluated to `ALSE`.

> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jsio');
  LoadModule('jssound');
  var file = new File('41_30secOgg-q0.wav'); // file: http://xiph.org/vorbis/listen.html
  file.Open('r');
  var dec = new SoundFileDecoder( file );
  var block = dec.Read(10000);
  Print( 'rezolution: '+block.bits+' per channel', '\n' );
  Print( block.channels == 2 ? 'stereo' : 'mono', '\n' );
  Print( block.rate+' frames/seconds', '\n' );
  Print( 'time: '+(block.frames/block.rate)+' seconds', '\n' );
```

### Properties ###

#### <font color='white' size='1'><b>inputStream</b></font> ####

> object <b>inputStream</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the stream object where encoded audio data are read from.

#### <font color='white' size='1'><b>bits</b></font> ####

> <sub>integer</sub> <b>bits</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the number of bits per frame and per channel.

#### <font color='white' size='1'><b>rate</b></font> ####

> <sub>integer</sub> <b>rate</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the number of frames per seconds of the sound.

#### <font color='white' size='1'><b>channels</b></font> ####

> <sub>integer</sub> <b>channels</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the number of channels of the sound. 1 is mono, 2 is stereo.

#### <font color='white' size='1'><b>frames</b></font> ####

> <sub>integer</sub> <b>frames</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the length (in frames) of the sound.
> > To compute the duration of the sound, use (frames/rate)


---

- [top](#jssound_module.md) - [main](JSLibs.md) -
