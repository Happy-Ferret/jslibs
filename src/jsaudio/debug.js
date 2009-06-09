LoadModule('jsio');
LoadModule('jsstd');
LoadModule('jssound');
LoadModule('jsaudio');
LoadModule('jsdebug');

Oal.Open('Generic Software');

var dec = new OggVorbisDecoder( new File('41_30secOgg-q0.ogg').Open('r') );
var [left, right] = SplitChannels(dec.Read(200000));
var b = new OalBuffer(right);
left.Free();
right.Free();

var src = new OalSource();

var effect = new OalEffect();
effect.type = Oal.EFFECT_REVERB;
effect.reverbDensity = 0;
effect.reverbDiffusion = 1;
effect.reverbGain = 1;
effect.reverbDecayTime = 20;

var effect2 = new OalEffect();
effect2.type = Oal.EFFECT_REVERB;
effect2.reverbDensity = 0;
effect2.reverbDiffusion = 1;
effect2.reverbGain = 1;
effect2.reverbDecayTime = 2;

var effectSlot = new OalEffectSlot();
src.effectSlot = effectSlot;

src.buffer = b;
src.looping = false;
src.Position(-1,0,0);

OalListener.position = [10,0,0];

src.Play();

//effectSlot.effect = effect2;

Sleep(3000);

//effectSlot.effect = effect;



Halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////


function Hex(int) '0x'+int.toString(16).toUpperCase();

//var dec = new SoundFileDecoder( new File('41_30secOgg-q0.wav').Open('r') );
var dec = new OggVorbisDecoder( new File('41_30secOgg-q0.ogg').Open('r') );
//var dec = new SoundFileDecoder( new File('break3.wav').Open('r') );

Print( dec.bits, '\n' );
Print( dec.channels, '\n' );
Print( dec.rate, '\n' );
Print( dec.frames, '\n' );

Print( '------------------------\n' );

var block = dec.Read(100000);
var [left, right] = SplitChannels(block);
block = left;

Print( 'Decoded sound blob:', '\n' );
Print( ' bits: '+block.bits, '\n' );
Print( ' channels: '+block.channels, '\n' );
Print( ' rate: '+block.rate, '\n' );
Print( ' frames: '+block.frames, '\n' );

Print( '\n' );




Oal.Open('Generic Software');
Print( 'has EFX: '+Oal.hasEfx, '\n' );
Print( 'maxAuxiliarySends: '+Oal.maxAuxiliarySends, '\n' );

Print( '\n' );
Print( 'OpenAL buffer:', '\n' );
var b = new OalBuffer(block);
block.Free(); // no more needed

Print( ' rate: '+b.frequency, '\n' );
Print( ' size: '+b.size, '\n' );
Print( ' bits: '+b.bits, '\n' );
Print( ' channels: '+b.channels, '\n' );

Print( '\n' );
Print( 'OpenAL listener:', '\n' );
Print( ' position: '+OalListener.position, '\n' );

Print( '\n' );
Print( 'OpenAL source:', '\n' );

var src = new OalSource();
src.buffer = b;
src.looping = false;
src.Position(1,0,0);


var effect = new OalEffect();
effect.type = Oal.EFFECT_REVERB;
effect.reverbDensity = 0;
effect.reverbDiffusion = 1;
effect.reverbGain = 1;
effect.reverbDecayTime = 20;

src.effect = effect;



// Changing a parameter value in the Filter Object after it has been attached to a Source will not affect the Source.
// To update the filter(s) used on a Source, an application must update the parameters of a Filter object and then re-attach it to the Source.
src.directFilter = undefined;


Print( ' effect: '+effect, '\n' );

/*
var filter = new OalFilter();
filter.type = Oal.FILTER_LOWPASS;
filter.lowpassGain = 0.5;
filter.lowpassGainHF = 0.1;
src.directFilter = filter;
*/

Print( ' position: '+src.position, '\n' );


src.Play();

for ( var i=0; i < 100; i++) {

//	src.position = [Math.sin(i/10)*5,Math.cos(i/10)*5, 0];

//	Print( ' state: '+Hex(src.state), '\n' );
//	Print( ' offset: '+src.secOffset.toFixed(3), '\n' );
//	Print( '\n' );
	Sleep(40);
}
src.Stop();


Oal.Close();




Halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////


var filename = '41_30secOgg-q0.ogg';

Print('playing '+filename, '\npress Ctrl-c to exit\n' );

var decoder = new OggVorbisDecoder(new File(filename).Open(File.RDONLY));

Oal.Open();
var src = Oal.GenSource();

var pcm, decodeSize = 512;
while ( (pcm = decoder.Read(decodeSize)) && !endSignal ) {

	Oal.SourceQueueBuffers(src, Oal.Buffer(pcm));
	if ( Oal.GetSourceInteger(src, Oal.SOURCE_STATE) == Oal.INITIAL )
		Oal.PlaySource(src);
	decodeSize = pcm.frames * 2;
}

var totalTime = decoder.frames/decoder.rate;
var currentTimeOffset = Oal.GetSourceReal(src, Oal.SEC_OFFSET);
for ( i = 0; !endSignal && i < totalTime - currentTimeOffset; i++ )
	Sleep(1000);

Oal.Close();

Halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////

//Oal.Listener(Oal.POSITION, [0,0,0]);
//Oal.Listener(Oal.VELOCITY, [0,0,0]);
//Oal.Listener(Oal.ORIENTATION, [0,0,-1, 0,1,0]);

var src = Oal.GenSource();

//Oal.SourceQueueBuffers(src, b);
Oal.Source(src, Oal.BUFFER, b);
//Oal.Source(src, Oal.PITCH, 1);
//Oal.Source(src, Oal.GAIN, 1);
//Oal.Source(src, Oal.POSITION, [1,1,1]);
//Oal.Source(src, Oal.VELOCITY, [0,0,0]);

Oal.PlaySource(src);

Print( 'error: '+Oal.error.toString(16).toUpperCase(), '\n' );

Sleep( 2000 );
Halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////

/*
var decoder = new OggVorbisDecoder(new File('41_30secOgg-q0.ogg').Open( File.RDONLY ));
var block = decoder.Read(40000);
var [left, right] = SplitChannels(block);
var src = Oal.GenSource();
//Oal.Source( src, Oal.POSITION, [10,10,0] );
Oal.SourceQueueBuffers(src, Oal.Buffer(left));
Oal.PlaySource(src);
Sleep( 1000 );
Halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////
*/

var src = Oal.GenSource();

var pcm;
while ( pcm = decoder.Read(10000) ) {
	
	var b = Oal.Buffer(pcm);
	Oal.SourceQueueBuffers(src, b);
	if ( Oal.GetSourceInteger(src, Oal.SOURCE_STATE) == Oal.INITIAL )
		Oal.PlaySource(src);
};

var totalTime = decoder.frames/decoder.rate;
var currentTimeOffset = Oal.GetSourceReal(src, Oal.SEC_OFFSET);

Sleep( 1000 * (totalTime - currentTimeOffset) );

