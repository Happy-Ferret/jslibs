function Alert( text, caption ) {

	ret = new NativeData()
	ret.PU32.Alloc();
	new NativeModule('User32', true ).Proc('MessageBoxA')( ret, NULL(), SZ(text), SZ( caption || 'Alert' ), INT( 0 ) ); // using ordinal value of  MessageBoxA
	return ret[0];
}

function Sleep( time ) {

	if ( !Sleep.proc ) {
		Sleep.proc = new NativeModule('kernel32', true ).Proc('Sleep');
		Sleep.arg1 = new NativeData();
		Sleep.arg1.PU32.Alloc()[0] = time
		Sleep.ret = new NativeData().VOID;
	}
	Sleep.proc( Sleep.ret, Sleep.arg1.PU32 );
}

/*
CreateProcessA(
    IN LPCSTR lpApplicationName,
    IN LPSTR lpCommandLine,
    IN LPSECURITY_ATTRIBUTES lpProcessAttributes,
    IN LPSECURITY_ATTRIBUTES lpThreadAttributes,
    IN BOOL bInheritHandles,
    IN DWORD dwCreationFlags,
    IN LPVOID lpEnvironment,
    IN LPCSTR lpCurrentDirectory,
    IN LPSTARTUPINFOA lpStartupInfo,
    OUT LPPROCESS_INFORMATION lpProcessInformation
    );
*/
function CreateProcess( fileName, commandLine ) {

	ret = new NativeData()
	ret.PU32.Alloc();
	new NativeModule('kernel32', true ).Proc('CreateProcessA')( ret.PU32, SZ( fileName ), SZ( commandLine || "" ), NULL(), NULL(), DWORD( 0 ), DWORD( 0x20 ), NULL(), NULL(), PDATA(68, 0), PDATA(16) );
	return ret.PU32[0] == 1;
}
