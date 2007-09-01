LoadModule('jsstd');
LoadModule('jsio');


try {

	var f = new File('log.txt');
	f.Open('w');

	var ms = new Socket( Socket.UDP );

	Print('connecting to master.tremulous.net...\n');
	ms.Connect( 'master.tremulous.net', 30710 );

	ms.Write('\xFF\xFF\xFF\xFFgetservers 69');
	var buffer = new Buffer(ms.Read(8192));
	ms.Close();
	
	var responseHdr = '\xFF\xFF\xFF\xFFgetserversResponse\\';
	if ( buffer.Read(responseHdr.length) != responseHdr )
		throw 'Invalid server response';

	Print('Server list received \n');
	
	var socketPool = [];
	
	function OnSocketWritable(s)  {

		Print('sending query to '+s.peerName +' ...\n');
		s.Write('\xFF\xFF\xFF\xFFgetinfo');
		delete s.writable; // no more 'write' notifications
	}

	function OnSocketReadable(s)  {

		var buf = new Buffer(s.Read());
		
		if ( !buf.Match('ÿÿÿÿinfoResponse') )
			throw 'Invalid server response';
		
		
		
		f.Write( '\n\n\n\n response from '+s.peerName+':\n' );
		f.Write( buf.Read() );
		
		s.Close();
		socketPool.splice(socketPool.indexOf(s), 1);
		delete s.readable; // no more 'read' notifications
	}

	var p = new Pack(buffer);
	while ( buffer.length >= 7 ) {
		
		var ip = p.ReadInt(1, false) + '.' + p.ReadInt(1, false) + '.' + p.ReadInt(1, false) + '.' + p.ReadInt(1, false);
		var port = p.ReadInt(2, false, true);
		if ( !buffer.Match('\\') )
			throw 'Protocol error';
		var s = new Socket(Socket.UDP);
		socketPool.push(s);
		Print( 'connecting to '+ ip +':'+ port, '\n' );
		s.Connect(ip, port);
		s.writable = OnSocketWritable;
		s.readable = OnSocketReadable;
	}
	
	for ( var t = 0; t < 100 && socketPool.length; t++ ) {
		
		if ( t == 40 || t == 60 || t == 80 ) { // retry
			
			Print('*** retrying ' + socketPool.length + ' servers' ,'\n');
			for each( var s in socketPool )
				s.writable = OnSocketWritable;
		}
		Print('poll: ' + t ,'\n');
		Poll(socketPool, 10);
	}
	
	Print( socketPool.length + ' servers did not reply' );
	
	f.Close();
	
} catch (ex if ex instanceof IoError) {

	Print( 'Error:'+ ex.text + ' ('+ex.os+')' );
}

/*

OS error 10049
Cannot assign requested address.
    The requested address is not valid in its context. This normally results from an attempt to bind to an address that is not valid for the local computer. This can also result from connect, sendto, WSAConnect, WSAJoinLeaf, or WSASendTo when the remote address or port is not valid for a remote computer (for example, address or port 0). 
    
misc:
  CL_ServersResponsePacket
  
// getinfo avant un getstatus  
  
*/