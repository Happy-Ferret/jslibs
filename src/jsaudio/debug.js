// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();


loadModule('jsstd');
loadModule('jsio');
loadModule('jsaudio');
loadModule('jssound');

Oal.maxAuxiliarySends;
halt();


Oal.open();

//var dec = new SoundFileDecoder( new File('41_30secOgg-q0.wav').Open('r') );
//var dec = new OggVorbisDecoder( new File('41_30secOgg-q0.ogg').Open('r') );
var dec = new SoundFileDecoder( new File('break3.wav').open('r') );

var block = dec.read(100000);

//var [left, right] = SplitChannels(block);


print( block.bits, '\n' );
print( block.channels, '\n' );
print( block.rate, '\n' );
print( block.frames, '\n' );

var b = Oal.Buffer(block);
var src = Oal.genSource();
Oal.sourceQueueBuffers(src, b);
Oal.playSource(src);

sleep(2000);

throw 0; ///////////



loadModule('jsio');
loadModule('jsstd');
loadModule('jssound');
loadModule('jsaudio');
loadModule('jsdebug');

Oal.open('Generic Software');

var dec = new OggVorbisDecoder( new File('41_30secOgg-q0.ogg').open('r') );
var [left, right] = splitChannels(dec.read(200000));
var b = new OalBuffer(right);
left.free();
right.free();

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
src.position(-1,0,0);

OalListener.position = [10,0,0];

src.play();

//effectSlot.effect = effect2;

sleep(3000);

//effectSlot.effect = effect;



halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////


function hex(int) '0x'+int.toString(16).toUpperCase();

//var dec = new SoundFileDecoder( new File('41_30secOgg-q0.wav').Open('r') );
var dec = new OggVorbisDecoder( new File('41_30secOgg-q0.ogg').open('r') );
//var dec = new SoundFileDecoder( new File('break3.wav').Open('r') );

print( dec.bits, '\n' );
print( dec.channels, '\n' );
print( dec.rate, '\n' );
print( dec.frames, '\n' );

print( '------------------------\n' );

var block = dec.read(100000);
var [left, right] = splitChannels(block);
block = left;

print( 'Decoded sound blob:', '\n' );
print( ' bits: '+block.bits, '\n' );
print( ' channels: '+block.channels, '\n' );
print( ' rate: '+block.rate, '\n' );
print( ' frames: '+block.frames, '\n' );

print( '\n' );




Oal.open('Generic Software');
print( 'has EFX: '+Oal.hasEfx, '\n' );
print( 'maxAuxiliarySends: '+Oal.maxAuxiliarySends, '\n' );

print( '\n' );
print( 'OpenAL buffer:', '\n' );
var b = new OalBuffer(block);
block.free(); // no more needed

print( ' rate: '+b.frequency, '\n' );
print( ' size: '+b.size, '\n' );
print( ' bits: '+b.bits, '\n' );
print( ' channels: '+b.channels, '\n' );

print( '\n' );
print( 'OpenAL listener:', '\n' );
print( ' position: '+OalListener.position, '\n' );

print( '\n' );
print( 'OpenAL source:', '\n' );

var src = new OalSource();
src.buffer = b;
src.looping = false;
src.position(1,0,0);


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


print( ' effect: '+effect, '\n' );

/*
var filter = new OalFilter();
filter.type = Oal.FILTER_LOWPASS;
filter.lowpassGain = 0.5;
filter.lowpassGainHF = 0.1;
src.directFilter = filter;
*/

print( ' position: '+src.position, '\n' );


src.play();

for ( var i=0; i < 100; i++) {

//	src.position = [Math.sin(i/10)*5,Math.cos(i/10)*5, 0];

//	print( ' state: '+Hex(src.state), '\n' );
//	print( ' offset: '+src.secOffset.toFixed(3), '\n' );
//	print( '\n' );
	sleep(40);
}
src.stop();


Oal.close();




halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////


var filename = '41_30secOgg-q0.ogg';

print('playing '+filename, '\npress ctrl-c to exit\n' );

var decoder = new OggVorbisDecoder(new File(filename).open(File.RDONLY));

Oal.open();
var src = Oal.genSource();

var pcm, decodeSize = 512;
while ( (pcm = decoder.read(decodeSize)) && !host.endSignal ) {

	Oal.sourceQueueBuffers(src, Oal.Buffer(pcm));
	if ( Oal.getSourceInteger(src, Oal.SOURCE_STATE) == Oal.INITIAL )
		Oal.playSource(src);
	decodeSize = pcm.frames * 2;
}

var totalTime = decoder.frames/decoder.rate;
var currentTimeOffset = Oal.getSourceReal(src, Oal.SEC_OFFSET);
for ( i = 0; !host.endSignal && i < totalTime - currentTimeOffset; i++ )
	sleep(1000);

Oal.close();

halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////

//Oal.Listener(Oal.POSITION, [0,0,0]);
//Oal.Listener(Oal.VELOCITY, [0,0,0]);
//Oal.Listener(Oal.ORIENTATION, [0,0,-1, 0,1,0]);

var src = Oal.genSource();

//Oal.SourceQueueBuffers(src, b);
Oal.source(src, Oal.BUFFER, b);
//Oal.Source(src, Oal.PITCH, 1);
//Oal.Source(src, Oal.GAIN, 1);
//Oal.Source(src, Oal.POSITION, [1,1,1]);
//Oal.Source(src, Oal.VELOCITY, [0,0,0]);

Oal.playSource(src);

print( 'error: '+Oal.error.toString(16).toUpperCase(), '\n' );

sleep( 2000 );
halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////

/*
var decoder = new OggVorbisDecoder(new File('41_30secOgg-q0.ogg').open( File.RDONLY ));
var block = decoder.read(40000);
var [left, right] = splitChannels(block);
var src = Oal.genSource();
//Oal.Source( src, Oal.POSITION, [10,10,0] );
Oal.sourceQueueBuffers(src, Oal.Buffer(left));
Oal.playSource(src);
sleep( 1000 );
halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////
*/

var src = Oal.genSource();

var pcm;
while ( pcm = decoder.read(10000) ) {
	
	var b = Oal.Buffer(pcm);
	Oal.sourceQueueBuffers(src, b);
	if ( Oal.getSourceInteger(src, Oal.SOURCE_STATE) == Oal.INITIAL )
		Oal.playSource(src);
};

var totalTime = decoder.frames/decoder.rate;
var currentTimeOffset = Oal.getSourceReal(src, Oal.SEC_OFFSET);

sleep( 1000 * (totalTime - currentTimeOffset) );

