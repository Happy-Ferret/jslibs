<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jsaudio/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jsaudio/qa.js) -
# jsaudio module #

> Support 2D and 3D sound source and listener using OpenAL library.



---

## class jsaudio::OalBuffer ##
- [top](#jsaudio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsaudio/buffer.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( soundBlob )
> > ##### arguments: #####
      1. <sub>Blob</sub> _soundBlob_:

### Methods ###

#### <font color='white' size='1'><b><i>valueOf</i></b></font> ####

> <sub>integer</sub> <b><i>valueOf</i></b>()

### Properties ###

#### <font color='white' size='1'><b>frequency</b></font> ####
> <sub>integer</sub> <b>frequency</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the frquency (in Hz) of the sound hold by the buffer.

#### <font color='white' size='1'><b>size</b></font> ####

> <sub>integer</sub> <b>size</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the size (in bytes) of the sound hold by the buffer.

#### <font color='white' size='1'><b>bits</b></font> ####

> <sub>integer</sub> <b>bits</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the resolution (in bits) of the sound hold by the buffer.

#### <font color='white' size='1'><b>channels</b></font> ####

> <sub>integer</sub> <b>channels</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the number of channels of the sound hold by the buffer.


---

## class jsaudio::OalEffect ##
- [top](#jsaudio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsaudio/effect.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>()
> > Creates a new effect object.

### Methods ###

#### <font color='white' size='1'><b><i>valueOf</i></b></font> ####

> <sub>integer</sub> <b><i>valueOf</i></b>()
> > Returns the internal OpenAL buffer id.

### Properties ###

#### <font color='white' size='1'><b>type</b></font> ####

> <sub>integer</sub> <b>type</b>
> > set the type of effect represented by the Effect object.
    * undefined
    * Oal.EFFECT\_EAXREVERB
    * Oal.EFFECT\_REVERB
    * Oal.EFFECT\_CHORUS
    * Oal.EFFECT\_DISTORTION
    * Oal.EFFECT\_ECHO
    * Oal.EFFECT\_FLANGER
    * Oal.EFFECT\_FREQUENCY\_SHIFTER
    * Oal.EFFECT\_VOCAL\_MORPHER
    * Oal.EFFECT\_PITCH\_SHIFTER
    * Oal.EFFECT\_RING\_MODULATOR
    * Oal.EFFECT\_AUTOWAH
    * Oal.EFFECT\_COMPRESSOR
    * Oal.EFFECT\_EQUALIZER

====<font color='white' size='1'></font>====(many)
  * <sub>real</sub> **reverbDensity**
  * <sub>real</sub> **reverbDiffusion**
  * <sub>real</sub> **reverbGain**
  * <sub>real</sub> **reverbGainHF**
  * <sub>real</sub> **reverbDecayTime**
  * <sub>real</sub> **reverbDecayHFRatio**
  * <sub>real</sub> **reverbReflectionsGain**
  * <sub>real</sub> **reverbReflectionsDelay**
  * <sub>real</sub> **reverbLateReverbGain**
  * <sub>real</sub> **reverbLateReverbDelay**
  * <sub>real</sub> **reverbAirAbsorptionGainHF**
  * <sub>real</sub> **reverbRoomRolloffFactor**
  * <sub>boolean</sub> **reverbDecayHFLimit**

  * <sub>integer</sub> **chorusWaveform**
  * <sub>integer</sub> **chorusPhase**
  * <sub>real</sub> **chorusRate**
  * <sub>real</sub> **chorusDepth**
  * <sub>real</sub> **chorusFeedback**
  * <sub>real</sub> **chorusDelay**

  * <sub>real</sub> **distortionEdge**
  * <sub>real</sub> **distortionGain**
  * <sub>real</sub> **distortionLowpassCutoff**
  * <sub>real</sub> **distortionEqcenter**
  * <sub>real</sub> **distortionEqbandwidth**

  * <sub>real</sub> **echoDelay**
  * <sub>real</sub> **echoLrdelay**
  * <sub>real</sub> **echoDamping**
  * <sub>real</sub> **echoFeedback**
  * <sub>real</sub> **echoSpread**

  * <sub>integer</sub> **flangerWaveform**
  * <sub>real</sub> **flangerPhase**
  * <sub>real</sub> **flangerRate**
  * <sub>real</sub> **flangerDepth**
  * <sub>real</sub> **flangerFeedback**
  * <sub>real</sub> **flangerDelay**

  * <sub>real</sub> **frequencyShifterFrequency**
  * <sub>integer</sub> **frequencyShifterLeftDirection**
  * <sub>integer</sub> **frequencyShifterRightDirection**

  * <sub>integer</sub> **vocalMorpherPhonemea**
  * <sub>integer</sub> **vocalMorpherPhonemeaCoarseTuning**
  * <sub>integer</sub> **vocalMorpherPhonemeb**
  * <sub>integer</sub> **vocalMorpherPhonemebCoarseTuning**
  * <sub>integer</sub> **vocalMorpherWaveform**
  * <sub>real</sub> **vocalMorpherRate**

  * <sub>integer</sub> **pitchShifterCoarseTune**
  * <sub>integer</sub> **pitchShifterFineTune**

  * <sub>real</sub> **ringModulatorFrequency**
  * <sub>real</sub> **ringModulatorHighpassCutoff**
  * <sub>integer</sub> **ringModulatorWaveform**

  * <sub>real</sub> **autowahAttackTime**
  * <sub>real</sub> **autowahReleaseTime**
  * <sub>real</sub> **autowahResonance**
  * <sub>real</sub> **autowahPeakGain**

  * <sub>boolean</sub> **compressorOnoff**

  * <sub>real</sub> **equalizerLowGain**
  * <sub>real</sub> **equalizerLowCutoff**
  * <sub>real</sub> **equalizerMid1Gain**
  * <sub>real</sub> **equalizerMid1Center**
  * <sub>real</sub> **equalizerMid1Width**
  * <sub>real</sub> **equalizerMid2Gain**
  * <sub>real</sub> **equalizerMid2Center**
  * <sub>real</sub> **equalizerMid2Width**
  * <sub>real</sub> **equalizerHighGain**
  * <sub>real</sub> **equalizerHighCutoff**


---

## class jsaudio::OalFilter ##
- [top](#jsaudio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsaudio/filter.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>()
> > Creates a new filter object.

### Methods ###

#### <font color='white' size='1'><b><i>valueOf</i></b></font> ####

> <sub>integer</sub> <b><i>valueOf</i></b>()
> > Returns the internal OpenAL filter id.

### Properties ###

#### <font color='white' size='1'><b>type</b></font> ####

> <sub>integer</sub> <b>type</b>
> > set the type of filter represented by the Filter object.
    * undefined
    * Oal.FILTER\_NULL
    * Oal.FILTER\_LOWPASS
    * Oal.FILTER\_HIGHPASS
    * Oal.FILTER\_BANDPASS

====<font color='white' size='1'></font>====(many)
  * <sub>real</sub> **lowpassGain**
  * <sub>real</sub> **lowpassGainHF**

  * <sub>real</sub> **highpassGain**
  * <sub>real</sub> **highpassGainLF**

  * <sub>real</sub> **bandpassGain**
  * <sub>real</sub> **bandpassGainLF**
  * <sub>real</sub> **bandpassGainHF**


---

## class jsaudio::OalListener ##
- [top](#jsaudio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsaudio/listener.cpp?r=2555) -

### Properties ###

#### <font color='white' size='1'><b>position</b></font> ####

> <sub>Array</sub> <b>position</b>
> > Gets or sets the position of the listener in the 3D environment.

#### <font color='white' size='1'><b>metersPerUnit</b></font> ####

> <sub>Array</sub> <b>metersPerUnit</b>
> > Gets or sets the unit size of the 3D environment.


---

## class jsaudio::Oal ##
- [top](#jsaudio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsaudio/oal.cpp?r=2557) -

### Static functions ###

#### <font color='white' size='1'><b>Open</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Open</b>( [[.md](.md) deviceName ] )
> > Open an audio device.
> > ##### arguments: #####
      1. <sub>string</sub> _deviceName_: "Generic Hardware", "Generic Software", "DirectSound3D" (for legacy), "DirectSound", "MMSYSTEM"
> > > > If no device name is specified, we will attempt to use DS3D.

> > ##### OpenAL API: #####
> > > alcOpenDevice, alcCreateContext, alcMakeContextCurrent

#### <font color='white' size='1'><b>Close</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Close</b>()
> > Close the current audio device.
> > ##### OpenAL API: #####
> > > alcGetCurrentContext, alcMakeContextCurrent, alcGetContextsDevice, alcDestroyContext, alcCloseDevice

#### <font color='white' size='1'><b>hasEfx</b></font> ####

> <sub>boolean</sub> <b>hasEfx</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is true if EFX extension is available.

#### <font color='white' size='1'><b>maxAuxiliarySends</b></font> ####

> <sub>boolean</sub> <b>maxAuxiliarySends</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the number of aux sends per source.

#### <font color='white' size='1'><b>DopplerFactor</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>DopplerFactor</b>( value )
> > Selects the OpenAL Doppler factor value. The default Doppler factor value is 1.0 .
> > ##### arguments: #####
      1. <sub>number</sub> _value_
> > ##### OpenAL API: #####
> > > alDopplerFactor

#### <font color='white' size='1'><b>DopplerVelocity</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>DopplerVelocity</b>( value )
> > Selects the OpenAL Doppler velocity value. The default Doppler velocity value is 343.3 .
> > ##### arguments: #####
      1. <sub>number</sub> _value_
> > ##### OpenAL API: #####
> > > alDopplerVelocity

#### <font color='white' size='1'><b>SpeedOfSound</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>SpeedOfSound</b>( value )
> > Selects the OpenAL Speed of Sound value.
> > ##### arguments: #####
      1. <sub>number</sub> _value_
> > ##### OpenAL API: #####
> > > alSpeedOfSound

#### <font color='white' size='1'><b>DistanceModel</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>DistanceModel</b>( distanceModel )
> > ##### arguments: #####
      1. <sub>integer</sub> _value_
> > ##### OpenAL API: #####
> > > alDistanceModel

#### <font color='white' size='1'><b>Enable</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Enable</b>( cap )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _cap_
> > ##### OpenAL API: #####
> > > alEnable

#### <font color='white' size='1'><b>Disable</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Disable</b>( cap )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _cap_
> > ##### OpenAL API: #####
> > > alDisable

#### <font color='white' size='1'><b>IsEnabled</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>IsEnabled</b>( cap )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _cap_
> > ##### OpenAL API: #####
> > > alIsEnabled

#### <font color='white' size='1'><b>GetString</b></font> ####

> <sub>boolean</sub> <b>GetString</b>( pname )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _pname_
> > ##### return value: #####
> > > value of a selected parameter.

> > ##### OpenAL API: #####
> > > alGetString

#### <font color='white' size='1'><b>GetBoolean</b></font> ####

> <sub>boolean</sub> <b>GetBoolean</b>( pname )
> > ##### arguments: #####
      1. <sub>ALenum</sub> _pname_
> > ##### return value: #####
> > > value of a selected parameter.

> > ##### OpenAL API: #####
> > > alGetBooleanv

#### <font color='white' size='1'><b>GetInteger</b></font> ####

> <sub>integer</sub> | <sub>Array</sub> <b>GetInteger</b>( pname [[.md](.md), count] )
> > ##### arguments: #####
      1. <sub>ALenum</sub> _pname_
      1. <sub>integer</sub> _count_: is the number of expected values. If _count_ is defined, the function will returns an array of values, else it returns a single value.
> > ##### return value: #####
> > > A single value or an array of values of a selected parameter.

> > ##### OpenAL API: #####
> > > alGetIntegerv

#### <font color='white' size='1'><b>GetDouble</b></font> ####

> <sub>real</sub> | <sub>Array</sub> <b>GetDouble</b>( pname [[.md](.md), count] )
> > ##### arguments: #####
      1. <sub>ALenum</sub> _pname_
      1. <sub>integer</sub> _count_: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
> > ##### return value: #####
> > > single value or Array of values of the selected parameter.

> > ##### OpenAL API: #####
> > > alGetDoublev

#### <font color='white' size='1'><b>Listener</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Listener</b>( pname, params )
> > ##### arguments: #####
      1. <sub>ALenum</sub> _pname_:
      1. <sub>Array</sub> _params_:
> > ##### OpenAL API: #####
> > > alListeneri, alListenerf, alListenerfv

#### <font color='white' size='1'><b>GetListenerReal</b></font> ####

> <sub>real</sub> | <sub>Array</sub> <b>GetListenerReal</b>( source, pname [[.md](.md), count] )
> > ##### arguments: #####
      1. <sub>integer</sub> _source_:
      1. <sub>ALenum</sub> _pname_:
      1. <sub>integer</sub> _count_: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
> > ##### return value: #####
> > > single value or Array of values of the selected parameter.

> > ##### OpenAL API: #####
> > > alGetListenerfv

#### <font color='white' size='1'><b>GenSource</b></font> ####

> <sub>integer</sub> <b>GenSource</b>()
> > ##### OpenAL API: #####
> > > alSourcei, alSourcef, alSourcefv

#### <font color='white' size='1'><b>Source</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Source</b>( source, pname, params )
> > ##### arguments: #####
      1. <sub>integer</sub> _source_:
      1. <sub>ALenum</sub> _pname_:
      1. <sub>Array</sub> _params_:
> > ##### OpenAL API: #####
> > > alSourcei, alSourcef, alSourcefv

#### <font color='white' size='1'><b>GetSourceReal</b></font> ####

> <sub>real</sub> | <sub>Array</sub> <b>GetSourceReal</b>( source, pname [[.md](.md), count] )
> > ##### arguments: #####
      1. <sub>integer</sub> _source_:
      1. <sub>ALenum</sub> _pname_:
      1. <sub>integer</sub> _count_: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
> > ##### return value: #####
> > > single value or Array of values of the selected parameter.

> > ##### OpenAL API: #####
> > > alGetSourcef

#### <font color='white' size='1'><b>GetSourceInteger</b></font> ####

> <sub>real</sub> | <sub>Array</sub> <b>GetSourceInteger</b>( source, pname [[.md](.md), count] )

#### <font color='white' size='1'><b>DeleteSource</b></font> ####
> <font color='gray' size='1'><sub>void</sub></font> <b>DeleteSource</b>( source )
> > ##### arguments: #####
      1. <sub>integer</sub> _source_: the source id.
> > ##### OpenAL API: #####
> > > alDeleteBuffers

#### <font color='white' size='1'><b>SourceQueueBuffers</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>SourceQueueBuffers</b>( source, buffer | bufferArray )
> > ##### arguments: #####
      1. <sub>integer</sub> _source_: the source id.
      1. <sub>integer</sub> _buffer_: the buffer id.
      1. <sub>Array</sub> _bufferArray_: an Array of buffer id.
> > ##### OpenAL API: #####
> > > alDeleteBuffers

#### <font color='white' size='1'><b>SourceUnqueueBuffers</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>SourceUnqueueBuffers</b>( source, buffer | bufferArray )
> > ##### arguments: #####
      1. <sub>integer</sub> _buffer_: the buffer id.
      1. <sub>Array</sub> _bufferArray_: an Array of buffer id.
> > ##### OpenAL API: #####
> > > alDeleteBuffers

#### <font color='white' size='1'><b>Buffer</b></font> ####

> <sub>integer</sub> <b>Buffer</b>( soundObject )
> > Creates a new buffer and attach a sound data to it. The data comming from the soundObject is copied into the OpenAL system.
> > undefined
> > > Buffers containing audio data with more than one channel will be played without 3D spatialization features  these formats are normally used for background music.

> > ##### arguments: #####
      1. <sub>soundObject</sub> _sound_: a sound object that contains PCM audio data and the following properties: rate, channels and bits.

#### <font color='white' size='1'><b>GetBufferReal</b></font> ####

> <sub>real</sub> | <sub>Array</sub> <b>GetBufferReal</b>( source, pname [[.md](.md), count] )
> > ##### arguments: #####
      1. <sub>integer</sub> _source_:
      1. <sub>ALenum</sub> _pname_:
      1. <sub>integer</sub> _count_: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
> > ##### return value: #####
> > > single value or Array of values of the selected parameter.

> > ##### OpenAL API: #####
> > > alGetBufferfv

#### <font color='white' size='1'><b>GetBufferInteger</b></font> ####

> <sub>integer</sub> | <sub>Array</sub> <b>GetBufferInteger</b>( source, pname [[.md](.md), count] )
> > ##### arguments: #####
      1. <sub>integer</sub> _source_:
      1. <sub>ALenum</sub> _pname_:
      1. <sub>integer</sub> _count_: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
> > ##### return value: #####
> > > single value or Array of values of the selected parameter.

> > ##### OpenAL API: #####
> > > alGetBufferiv

#### <font color='white' size='1'><b>DeleteBuffer</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>DeleteBuffer</b>( buffer )
> > ##### arguments: #####
      1. <sub>integer</sub> _buffer_: the buffer id.
> > ##### note: #####
> > > Buffers that have been unqueued from all sources are UNUSED. Buffers that are UNUSED can be deleted, or changed by alBufferData commands.

> > ##### OpenAL API: #####
> > > alDeleteBuffers

#### <font color='white' size='1'><b>PlaySource</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>PlaySource</b>( source )
> > Plays the given source.
> > ##### arguments: #####
      1. <sub>integer</sub> _source_: the ID of the source to play.

#### <font color='white' size='1'><b>StopSource</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>StopSource</b>( source )
> > Stop the given source.
> > ##### arguments: #####
      1. <sub>integer</sub> _source_: the ID of the source to play.

#### <font color='white' size='1'><b>PauseSource</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>PauseSource</b>( source )
> > Pause the given source.
> > ##### arguments: #####
      1. <sub>integer</sub> _source_: the ID of the source to play.

#### <font color='white' size='1'><b>RewindSource</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>RewindSource</b>( source )
> > Rewind the given source. set playback postiton to beginning.
> > ##### arguments: #####
      1. <sub>integer</sub> _source_: the ID of the source to play.

#### <font color='white' size='1'><b>GenEffect</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>GenEffect</b>()
> > ##### OpenaL API: #####
> > > alGenEffects

#### <font color='white' size='1'><b>DeleteEffect</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>DeleteEffect</b>()
> > ##### OpenaL API: #####
> > > alGenEffects

#### <font color='white' size='1'><b>PlaySound</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>PlaySound</b>( sound )  ![http://jslibs.googlecode.com/svn/wiki/deprecated.png](http://jslibs.googlecode.com/svn/wiki/deprecated.png)
> > Plays a sound on the default playback device.
> > ##### arguments: #####
      1. <sub>soundObject</sub> _sound_: sound object to play.

### Examples ###
##### example 1: #####

> A simple ogg player
```
 LoadModule('jsio');
 LoadModule('jsstd');
 LoadModule('jssound');
 LoadModule('jsaudio');

 Oal.Open();
 var decoder = new OggVorbisDecoder(new File('41_30secOgg-q0.ogg').Open(File.RDONLY));
 var sourceId = Oal.GenSource();

 var pcm;
 while ( pcm = decoder.Read(10000) ) {

  var bufferId = Oal.Buffer(pcm);
  Oal.SourceQueueBuffers(sourceId, bufferId);
  if ( Oal.GetSourceInteger(sourceId, Oal.SOURCE_STATE) == Oal.INITIAL )
   Oal.PlaySource(sourceId);
 };

 var totalTime = decoder.frames/decoder.rate;
 var currentTimeOffset = Oal.GetSourceReal(sourceId, Oal.SEC_OFFSET);
 Sleep( 1000 * (totalTime - currentTimeOffset) );
```


---

## class jsaudio::OalSource ##
- [top](#jsaudio_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsaudio/source.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>()
> > Creates a new source object.

#### <font color='white' size='1'><b>QueueBuffers</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>QueueBuffers</b>( buffer )
> > ##### arguments: #####
      1. <sub>OalBuffer</sub> | BufferId: a Buffer Object or a buffer Id.
> > ##### OpenAL API: #####
> > > alDeleteBuffers

#### <font color='white' size='1'><b>UnqueueBuffers</b></font> ####

> <sub>OalBuffer</sub> <b>UnqueueBuffers</b>()
> > ##### OpenAL API: #####
> > > alDeleteBuffers

### Properties ###



---

## class jsaudio::OalError ##
- [top](#jsaudio_module.md) -


#### <font color='white' size='1'><b>code</b></font> ####

> <sub>integer</sub> <b>code</b>
> > OpenAL error number.

#### <font color='white' size='1'><b>text</b></font> ####

> <sub>string</sub> <b>text</b>
> > OpenAL error literal.

#### <font color='white' size='1'><b>const</b></font> ####

> <sub>string</sub> <b>const</b>
> > Const name of the OpenAL error.

#### <font color='white' size='1'><b><i>toString</i></b></font> ####

> <sub>string</sub> <b><i>toString</i></b>
> > see Text().


## more information ##
[OpenAL Specification and Reference](http://connect.creativelabs.com/openal/Documentation/oalspecs-specs.pdf)
[OpenAL 1.1 Specification and Reference](http://connect.creativelabs.com/openal/Documentation/OpenAL%201.1%20Specification.pdf)


---

- [top](#jsaudio_module.md) - [main](JSLibs.md) -