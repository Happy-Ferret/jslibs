// WARNING: rename it into vst.js

LoadModule('jsio');
LoadModule('jswinshell');

	var f = new File('D:\Franck\Mes documents\MyMusic\VSTplugins\jsvst.log');
	f.content = 'test';

var log = [];
function Log(str) log.push(str);

var opcodeName = {};
with ( AudioEffect ) {

	opcodeName[	effOpen ] = 'effOpen';
	opcodeName[	effClose ] = 'effClose';
	opcodeName[	effSetProgram ] = 'effSetProgram';
	opcodeName[	effGetProgram ] = 'effGetProgram';
	opcodeName[	effSetProgramName ] = 'effSetProgramName';
	opcodeName[	effGetProgramName ] = 'effGetProgramName';
	opcodeName[	effGetParamLabel ] = 'effGetParamLabel';
	opcodeName[	effGetParamDisplay ] = 'effGetParamDisplay';
	opcodeName[	effGetParamName ] = 'effGetParamName';
	opcodeName[	effSetSampleRate ] = 'effSetSampleRate';
	opcodeName[	effSetBlockSize ] = 'effSetBlockSize';
	opcodeName[	effMainsChanged ] = 'effMainsChanged';
	opcodeName[	effEditGetRect ] = 'effEditGetRect';
	opcodeName[	effEditOpen ] = 'effEditOpen';
	opcodeName[	effEditClose ] = 'effEditClose';
	opcodeName[	effGetChunk ] = 'effGetChunk';
	opcodeName[	effSetChunk ] = 'effSetChunk';
	opcodeName[	effProcessEvents ] = 'effProcessEvents';
	opcodeName[	effCanBeAutomated ] = 'effCanBeAutomated';
	opcodeName[	effString2Parameter ] = 'effString2Parameter';
	opcodeName[	effGetProgramNameIndexed ] = 'effGetProgramNameIndexed';
	opcodeName[	effGetInputProperties ] = 'effGetInputProperties';
	opcodeName[	effGetOutputProperties ] = 'effGetOutputProperties';
	opcodeName[	effGetPlugCategory ] = 'effGetPlugCategory';
	opcodeName[	effOfflineNotify ] = 'effOfflineNotify';
	opcodeName[	effOfflinePrepare ] = 'effOfflinePrepare';
	opcodeName[	effOfflineRun ] = 'effOfflineRun';
	opcodeName[	effProcessVarIo ] = 'effProcessVarIo';
	opcodeName[	effSetSpeakerArrangement ] = 'effSetSpeakerArrangement';
	opcodeName[	effSetBypass ] = 'effSetBypass';
	opcodeName[	effGetEffectName ] = 'effGetEffectName';
	opcodeName[	effGetVendorString ] = 'effGetVendorString';
	opcodeName[	effGetProductString ] = 'effGetProductString';
	opcodeName[	effGetVendorVersion ] = 'effGetVendorVersion';
	opcodeName[	effVendorSpecific ] = 'effVendorSpecific';
	opcodeName[	effCanDo ] = 'effCanDo';
	opcodeName[	effGetTailSize ] = 'effGetTailSize';
	opcodeName[	effGetParameterProperties ] = 'effGetParameterProperties';
	opcodeName[	effGetVstVersion ] = 'effGetVstVersion';
	opcodeName[	effEditKeyDown ] = 'effEditKeyDown';
	opcodeName[	effEditKeyUp ] = 'effEditKeyUp';
	opcodeName[	effSetEditKnobMode ] = 'effSetEditKnobMode';
	opcodeName[	effGetMidiProgramName ] = 'effGetMidiProgramName';
	opcodeName[	effGetCurrentMidiProgram ] = 'effGetCurrentMidiProgram';
	opcodeName[	effGetMidiProgramCategory ] = 'effGetMidiProgramCategory';
	opcodeName[	effHasMidiProgramsChanged ] = 'effHasMidiProgramsChanged';
	opcodeName[	effGetMidiKeyName ] = 'effGetMidiKeyName';
	opcodeName[	effBeginSetProgram ] = 'effBeginSetProgram';
	opcodeName[	effEndSetProgram ] = 'effEndSetProgram';
	opcodeName[	effGetSpeakerArrangement ] = 'effGetSpeakerArrangement';
	opcodeName[	effShellGetNextPlugin ] = 'effShellGetNextPlugin';
	opcodeName[	effStartProcess ] = 'effStartProcess';
	opcodeName[	effStopProcess ] = 'effStopProcess';
	opcodeName[	effSetTotalSampleToProcess ] = 'effSetTotalSampleToProcess';
	opcodeName[	effSetPanLaw ] = 'effSetPanLaw';
	opcodeName[	effBeginLoadBank ] = 'effBeginLoadBank';
	opcodeName[	effBeginLoadProgram ] = 'effBeginLoadProgram';
	opcodeName[	effSetProcessPrecision ] = 'effSetProcessPrecision';
	opcodeName[	effGetNumMidiInputChannels ] = 'effGetNumMidiInputChannels';
	opcodeName[	effGetNumMidiOutputChannels ] = 'effGetNumMidiOutputChannels';
}


vst.dispatcher = function(opcode, index, value, opt) {
	
	Log( 'opcode: '+opcodeName[opcode] + ' index: '+index + ' value: '+value + ' opt: '+opt );
}


vst.close = function() {

	var c = new Console();
	Log('Press <enter> to continue...');
	c.Write( log.join('\r\n') );
	c.Read();
}

vst.getParameterName = function(index) {

//	Log('getParameterName');
	return index < 10 ? 'test' : undefined;
}

vst.getParameterLabel = function(index) {
	
//	Log('getParameterLabel');
	return index < 10 ? 'test' : undefined;
}