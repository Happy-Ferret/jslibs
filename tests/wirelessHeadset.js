
var webPage = 
`<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="initial-scale=1">
<title>WirelessHeadset</title>
<style>
	#output p {
		padding: 0px;
		margin: 0px;
	}
	input[type=button] {
		font-size: 20pt;
		padding: 0.5em;
	}
</style>
</head>
<body>
<input type="button" value="stop" onclick="socket.close()"></div>
<input type="button" value="reload" onclick="location.reload(true)"></div>
<div id="output"></div>
</body>
<script>

	var logindex = 0;
	function log(msg) {

		var output = document.getElementById("output");
		var pre = document.createElement("p");
		pre.innerHTML = '' + (logindex++) + ':' + msg;
		output.insertBefore(pre, output.firstElementChild);
		if ( logindex > 1600 )
			output.removeChild(output.lastElementChild);
	}

	window.AudioContext = window.AudioContext||window.webkitAudioContext;
	var audioCtx = new AudioContext();

	var chunkList = [];
	var chunkByteOffset = 0;

	var outputBufferSampleLength = 512;
	var audioNode = audioCtx.createScriptProcessor(outputBufferSampleLength, 0, 2);

	audioNode.onaudioprocess = function(audioProcessingEvent) {

		if ( chunkList.length < 1 )
			return;

		if ( chunkList.length > 10 ) {

			log('chunkList overrun');
			chunkList.length = 0;
			chunkByteOffset = 0;
		}

		var outputBuffer = audioProcessingEvent.outputBuffer;
		var l = outputBuffer.getChannelData(0);
		var r = outputBuffer.getChannelData(1);

		var chunk = chunkList[0];
		var chunkByteLength = chunk.byteLength;
				
		for ( var i = 0; i < outputBufferSampleLength; ++i ) {

			if ( chunkByteOffset >= chunkByteLength ) {

				chunkList.shift(); // exhausted
				
				if ( chunkList.length == 0 ) {
					
					log('chunkList underrun');
					return;
				}
				chunk = chunkList[0];
				chunkByteLength = chunk.byteLength;
				chunkByteOffset = 0;
			}

			l[i] = chunk.getInt16(chunkByteOffset, true) / 32768;
			r[i] = chunk.getInt16(chunkByteOffset + 2, true) / 32768;
			chunkByteOffset += 4;
		}
	}

	audioNode.connect(audioCtx.destination);

	var socket;
	function connect() {

		socket = new WebSocket('ws://'+location.host+'/');
		socket.binaryType = 'arraybuffer';

		socket.onopen = function(ev) { log('OPEN') }
		socket.onclose = function(ev) {
			
			log('RECONNECT: '+ev.code)
			socket.close();
			connect();
		}

		socket.onerror = function(ev) { log('ERROR: ' + ev.data) }

		socket.onmessage = function(ev) {

			if ( typeof(ev.data) == 'string' ) {
			
				var msg = JSON.parse(ev.data);
				log(ev.data);
				return;
			}

			chunkList.push(new DataView(ev.data));

/*
			audioCtx.decodeAudioData(ev.data, function(buffer) {
				
				log('decodeAudioData ok');
				
				chunkList.push(new DataView(buffer));
			}, function(err) {
				
				log('decodeAudioData err: ', err)
			});
*/
			
		}
	}

	connect();

</script>
</html>
`;

var loadModule = host.loadModule;
loadModule('jsstd');
loadModule('jsio');
loadModule('jsz');
loadModule('jscrypt');
loadModule('jswinshell');
loadModule('jssound');


const CRLF = '\r\n';

host.interruptInterval = 500;
host.onInterrupt = () => host.collectGarbage(true, 10);


function log(msg) {

	print( stringify(msg).replace(/\n/g, '\n   '), '\n' );
}


function Buffer() {

	var chunkList = [];
	var length = 0;

	Object.defineProperty(this, 'length', {
		__proto__: null,
		get: function() { return length; }
	});

	this.write = function(data) {
			
		if ( typeof(data) == 'string' )
			data = [ c.charCodeAt() for (c of data) ];

		var tmp = new Uint8Array(data);
		chunkList.push(tmp);
		length += tmp.length;
	}
	
	this.unread = function(data) {

		if ( typeof(data) == 'string' )
			data = [ c.charCodeAt() for (c of data) ];

		var tmp = new Uint8Array(data);
		chunkList.unshift(tmp);
		length += tmp.length;
	}

	this.read = function(amount) {

		amount = Math.min(amount || length, length);
		var dst = new ArrayBuffer(amount);
		var dstOffset = 0;
		for ( var remain ; remain = amount - dstOffset ; ) {

			var chunk = chunkList.shift();
			if ( remain >= chunk.length ) {

				new Uint8Array(dst, dstOffset).set(new Uint8Array(chunk));
				dstOffset += chunk.length;
				length -= chunk.length;
			} else {

				new Uint8Array(dst, dstOffset).set(chunk.subarray(0, remain));
				chunkList.unshift(chunk.subarray(remain));
				length -= remain;
				break;
			}
		}
		return new Uint8Array(dst);
	}


	this.indexOf = function(subRaw) {

		var sub = new Uint8Array(subRaw);
		var sMax = sub.length;
		if ( length >= sMax && sMax > 0 ) {

			var buf = chunkList[0];

			var bMax = length - sMax + 1;
			for ( var i = 0; i < bMax; ++i ) {

				if ( i == buf.length - sMax + 1 ) {

					buf = this.read(); // flatten the buffer
					this.unread(buf);
				}

				if ( buf[i] == sub[0] ) {

					var j = i, k = 0;
					do {
						if ( k == sMax )
							return i;
					} while ( buf[j++] == sub[k++] )
				}
			}
		}
		return -1;
	}
}


////


const WS_TEXT_DATA = 1;
const WS_BINARY_DATA = 2;

function SimpleHTTPServer(port, bind, basicAuthRequest, requestHandler, wsHandler) {

	var pendingRequestList = [], serverSocket = new Socket(), socketList = [serverSocket], deflate = new Z(Z.DEFLATE, Z.BEST_SPEED);

	function closeSocket(s) {

		delete s.writable;
		delete s.readable;
		s.close();
		socketList.splice(socketList.indexOf(s), 1);
		s.state(undefined);
		log('[connection closed]');
	}


	function processRequest(s, b) {
			
		if ( !s )
			return;

		log('[http:'+s.peerPort+']');

		var eoh = b.indexOf([0x0D, 0x0A, 0x0D, 0x0A]); // CRLFCRLF
		if ( eoh == -1 )
			return;

		var headerStr = stringify(b.read(eoh+4));

		log( 'headers: '+headerStr );

		var headerLines = headerStr.split(CRLF);
		var status = headerLines.shift().split(' ');
		var headers = {};
		for ( var h of headerLines ) {

			var line = h.split(/: ?/);
			headers[line[0].toLowerCase()] = line[1];
		}

		var basicAuth = (/^Basic (.*)/.exec(headers['authorization'])||[])[1];
		var contentLength = parseInt(headers['content-length'], 10);

		function processRequestBody() {
	
			if ( b.length < contentLength ) {

				return;
			}

			if ( basicAuthRequest && basicAuth != base64Encode(basicAuthRequest) ) {

				sleep(2000);
				s.bufWrite('HTTP/1.1 401 Authorization Required'+CRLF+'WWW-Authenticate: Basic realm="test"'+CRLF+'Content-length: 0'+CRLF+CRLF);
				return;
			}

			if ( headers['connection'].indexOf('Upgrade') != -1 && headers['upgrade'] == 'websocket' ) {
				
				// https://tools.ietf.org/html/rfc6455

				var head = 'HTTP/1.1 101 Switching Protocols'+CRLF;
				var sha1 = new Hash('sha1');
				sha1.write(headers['sec-websocket-key'] + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11');
				head += 'sec-websocket-accept:'+base64Encode(sha1.done())+CRLF;
				head += 'connection:Upgrade'+CRLF;
				head += 'upgrade:WebSocket'+CRLF;
				s.bufWrite(head+CRLF);


				var wsMessageHandler = wsHandler( function(data, dataType) {
					
					var header = new Uint8Array(10);
					var dataLen = data.length;
					header[0] = 0x80 | (dataType & 0xF) ;
					if ( dataLen < 126 ) {
						
						header[1] = dataLen;
						s.bufWrite(header.subarray(0, 2));
					} else
					if ( dataLen > 65535 ) {
						
						header[1] = 127;
						new DataView(header.buffer).setInt32(6, dataLen);
						s.bufWrite(header.subarray(0, 10));
					} else {

						header[1] = 126;
						new DataView(header.buffer).setInt16(2, dataLen);
						s.bufWrite(header.subarray(0, 4));
					}

//					log('ws> length:'+dataLen);

					s.bufWrite(data);
				});


				var processWebSocket = function(s, b) {

					if ( !s ) {

						wsMessageHandler();
						return;
					}
			
					if ( b.length < 2 )
						return;

					log('[webSocket:'+s.peerPort+']');

					log('buffer length:'+b.length);


					var header = b.read(2);
					var fin = !!(header[0] & 0x80);
					var opcode = header[0] & 0x0F;

					if ( opcode == 0x8 ) { // close

						wsMessageHandler(undefined);
						s.state = processRequest;
						processRequest(s, b);
						return;
					}

					var mask = null;
					var length = header[1] & 0x7F;
					if ( length == 126 ) {
			
						length = new DataView(b.read(2).buffer).getInt16(0);
					} else
					if ( length == 127 ) {

						length = new DataView(b.read(8).buffer).getInt32(4); // only handle 32bit
					}
					if ( header[1] & 0x80 ) // masked
						mask = b.read(4);

					log('ws< fin:'+fin);
					log('ws< length:'+length);
					log('ws< opcode:'+opcode);
					log('ws< mask:'+!!mask);

					function processPayload(s, b) {

						if ( !s || b.length < length )
							return;

						log('got payload: '+b.length);

						var data = b.read(length);
						if ( mask )
							for ( var i = 0; i < length; ++i )
								data[i] = data[i] ^ mask[i%4];

						wsMessageHandler(data);

						s.state = processWebSocket;
						processWebSocket(s, b);
					};

					s.state = processPayload;
					processPayload(s, b);
				}

				s.state = processWebSocket;
				processWebSocket(s, b);
				return;
			}

			requestHandler(status[0], status[1], status[2], b.read(contentLength), function(response, responseHeaders) {

				if ( s.connectionClosed )
					return closeSocket(s);
				if ( response == undefined ) {

					s.bufWrite('HTTP/1.1 204 No Content'+CRLF+CRLF);
				} else {

					var head = 'HTTP/1.1 200 OK';
					head += 'content-type: text/plain'+CRLF;

					//if ( response.length >= 1460 ) {
					//
					//	response = deflate.process(response, true);
					//	head += 'content-encoding: deflate'+CRLF;
					//}

					head += 'content-length: '+response.length + CRLF;
					s.bufWrite(head + responseHeaders + CRLF + response);
				}

				s.state = processRequest;
				processRequest(s, b);
				return undefined;
			});

			return;
		}

		s.state = processRequestBody;
		processRequestBody(s, b);

		return;
	}

	serverSocket.readable = function() {
		
		var clientSocket = serverSocket.accept();
		clientSocket.noDelay = true;

		log('[incomming:'+clientSocket.peerPort+']');
		socketList.push(clientSocket);
		clientSocket.buffer = new Buffer();
		clientSocket.state = processRequest;

		var sendBuffer = new Buffer();

		clientSocket.bufWrite = function(data) {

			//clientSocket.write(data);

			sendBuffer.write(data);

			clientSocket.writable = function(s) {

				var data = sendBuffer.read();
				s.write(data);

				if ( sendBuffer.length == 0 )
					delete s.writable;
			}
		}

		clientSocket.readable = function(s) {

			var buf = s.read();
			if ( buf == undefined )
				return closeSocket(s);
			s.buffer.write(buf.arrayBuffer);
			log(' [socket '+clientSocket.peerPort+' state: '+clientSocket.state.name );
			clientSocket.state(clientSocket, clientSocket.buffer);
			return undefined;
		}
	}

	serverSocket.noDelay = true;
	serverSocket.nonblocking = true;
	serverSocket.bind(port, bind);
	serverSocket.listen();

	this.events = Descriptor.events(socketList);
}

///



var audioFormat = {
	bits:16,
	channels:2,
	rate:44100,
	frames: 1024 //10  * 44100 / 1000
};



function ProcessAudio(framesPerBuffer, audioFormat, audioHandler) {

	var audio = new AudioIn(AudioIn.inputDeviceList[1], audioFormat.rate, audioFormat.bits, audioFormat.channels, framesPerBuffer);

	this.read = function() {
		
		return bufferList.shift();
	}

	this.start = function() {

		audio.start();
	}

	this.stop = function() {

		audio.stop();
	}

	this.events = audio.events(function() {

		var chunkList = [];
		var s;
		while ( (s = this.read()) ) {
		
			chunkList.push(s);
		}

		audioHandler(chunkList);
	});
}


function requestHandler(method, url, status, content, response) {

	if ( url == '/' ) {

		response( webPage, 'content-type: text/html'+CRLF );
		return;
	}
}

var audioListeners = [];

function wsHandler(send) {

	send(JSON.stringify({ init: { format: audioFormat } }), WS_TEXT_DATA);
	
	//var encoder = new VorbisEncoder(audioFormat.channels, audioFormat.rate, -0.1);

	function audioListener(chunkList) {
		
		for ( var chunk of chunkList ) {

			send(Uint8Array(chunk.data), WS_BINARY_DATA);

/*
			var encBinChunk = encoder.encode(chunk);
			log('send '+encBinChunk.arrayBuffer.byteLength)
			
			if ( encBinChunk.arrayBuffer.byteLength > 0 )
				send(Uint8Array(encBinChunk.arrayBuffer), WS_BINARY_DATA);
*/

		}
	}
	
	audioListeners.push(audioListener);

	return function(data) {
		
		if ( data == undefined ) { // ws closed
			
			audioListeners.splice(audioListeners.indexOf(audioListener), 1);
			return
		}
	}
}

var http = new SimpleHTTPServer(81, undefined, '', requestHandler, wsHandler);

var audio = new ProcessAudio(audioFormat.frames, audioFormat, function(chunkList) {
	
	for ( var listener of audioListeners )
		listener(chunkList);
});

try {
	audio.start();
	for (;;) {
		processEvents(host.endSignalEvents(function() { throw 0 }), audio.events, http.events);
	}
	audio.stop();
} catch ( ex ) {
	
	print(ex, '\nstack:\n', ex.stack)
}
