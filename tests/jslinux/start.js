"use strict";

LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jswinshell');

var noop = function() {}

// Term
function Term(fa, ga, ha) {

	this.ha = ha;

	Console.cursorSize = 100;
	Console.width = fa;
	Console.height = ga;
	this.defaultTextAttribute = Console.textAttribute;
	
	this.SendKeys = function(ta) {
	
		this.ha(ta);
	}

	Console.onMouseDown = function(x, y, button, alt, ctrl, shift) {
		
		if ( button == Console.RIGHTMOST_BUTTON_PRESSED ) {

			var clipboardData = clipboard;
			if ( clipboardData )
				ha(clipboardData);
		}
	}
	
	Console.onKeyDown = function(char, keycode, alt, ctrl, shift) {
	
		var ta = '';
		switch (keycode) {
		case 8:
		case 9:
		case 13:
		case 27:
			ta = String.fromCharCode(keycode);
			break;
		case 37:
			ta = "\x1b[D";
			break;
		case 39:
			ta = "\x1b[C";
			break;
		case 38:
			ta = "\x1b[A";
			break;
		case 40:
			ta = "\x1b[B";
			break;
		case 46:
			ta = "\x1b[3~";
			break;
		case 45:
			ta = "\x1b[2~";
			break;
		case 36:
			ta = "\x1bOH";
			break;
		case 35:
			ta = "\x1bOF";
			break;
		case 33:
			ta = "\x1b[5~";
			break;
		case 34:
			ta = "\x1b[6~";
			break;
		default:
			if (ctrl) {
				if (keycode >= 65 && keycode <= 90) {
					ta = String.fromCharCode(keycode - 64);
				} else if (keycode == 32) {
					ta = String.fromCharCode(0);
				}
			} else if (alt) {
				if (keycode >= 65 && keycode <= 90) {
					ta = "\x1b" + String.fromCharCode(keycode + 32);
				}
			} else if (char != '\x00') {
				ta = char;
			}
			break;
		}
		if (ta)
			ha(ta);
	}
	
	this.state = 0;
	this.color = [
		0, 
		Console.FOREGROUND_RED | Console.FOREGROUND_INTENSITY,
		Console.FOREGROUND_GREEN | Console.FOREGROUND_INTENSITY, 
		Console.FOREGROUND_RED | Console.FOREGROUND_GREEN | Console.FOREGROUND_INTENSITY, 
		Console.FOREGROUND_BLUE | Console.FOREGROUND_INTENSITY, 
		Console.FOREGROUND_BLUE | Console.FOREGROUND_RED | Console.FOREGROUND_INTENSITY, 
		Console.FOREGROUND_GREEN | Console.FOREGROUND_BLUE | Console.FOREGROUND_INTENSITY, 
		Console.FOREGROUND_RED | Console.FOREGROUND_GREEN | Console.FOREGROUND_BLUE | Console.FOREGROUND_INTENSITY
	];
}

Term.prototype.open = noop;

Term.prototype.writeln = function (ta) {
	this.write(ta+'\n');
}
Term.prototype.write = function (ta) {
	
    var ya = 0;
    var za = 1;
    var Aa = 2;
    var i, c, ka, la, l, n, j, x, y;
	
    for (i = 0; i < ta.length; i++) {
        
        c = ta.charCodeAt(i);
        switch (this.state) {
        case ya:
            switch (c) {
				case 27: // <ESC>
					this.state = za;
					break;
            default:
                if (c < 32)
	                break;
            case 8:
            case 9:
            case 10:
            case 13:
					Console.Write(ta.charAt(i));
            }
            break;
        case za:
				if (c == 91) { // [
				
					this.esc_params = [];
					this.cur_param = 0;
					this.state = Aa;
				} else {
				
					this.state = ya;
				}
				break;
        case Aa:
            if (c >= 48 && c <= 57) { // 0-9
					
					//esc_params
					this.cur_param = this.cur_param * 10 + c - 48;
            } else {

					this.esc_params[this.esc_params.length] = this.cur_param;
					this.cur_param = 0;
					if (c == 59) // ;
						break;
					this.state = ya;
					switch (c) {
						case 65: // Cursor Up		<ESC>[{COUNT}A
							Console.cursorPositionY -= this.esc_params[0];
							break;
						case 66: // Cursor Down		<ESC>[{COUNT}B
							Console.cursorPositionY += this.esc_params[0];
							break;
						case 67: // Cursor Forward		<ESC>[{COUNT}C
							Console.cursorPositionX += this.esc_params[0];
							break;
						case 68: // Cursor Backward		<ESC>[{COUNT}D
							Console.cursorPositionX -= this.esc_params[0];
							break;
						case 102: // Force Cursor Position	<ESC>[{ROW};{COLUMN}f
						case 72: // Cursor Home 		<ESC>[{ROW};{COLUMN}H
							
							Console.cursorPositionX = (this.esc_params[1] || 1) - 1;
							Console.cursorPositionY = (this.esc_params[0] || 1) - 1;
							break;
						case 74: // Erase Down		<ESC>[J
							Console.FillConsoleOutput(0, Console.cursorPositionY+1, Console.width, Console.height - Console.cursorPositionY-1, ' ', Console.textAttribute);
						case 75: // Erase End of Line	<ESC>[K
							Console.FillConsoleOutput(Console.cursorPositionX, Console.cursorPositionY, Console.width - Console.cursorPositionX, 1, ' ', Console.textAttribute);
							break;
						case 109: // Set Attribute Mode	<ESC>[{attr1};...;{attrn}m
							for ( var i = 0; i < this.esc_params.length; ++i ) {

								n = this.esc_params[i];
								if (n >= 30 && n <= 37) {
									Console.textAttribute = (Console.textAttribute & ~15) | (this.color[n - 30]);
								} else if (n >= 40 && n <= 47) {
									Console.textAttribute = (Console.textAttribute & ~(15 << 4)) | (this.color[n - 40] << 4);
								} else if (n == 0) {
									Console.textAttribute = this.defaultTextAttribute; // default
								} else {
								}
							}
							break;
						case 110: // n
							this.ha("\x1b[" + (Console.cursorPositionY + 1) + ";" + (Console.cursorPositionX + 1) + "R");
							break;
					}
				}
				break;
        }
    }
}


// window

var window = global;

window.console = new function() {
	
	this.log = function() {
		
		Print( Array.prototype.slice.apply(arguments).join(',') + '\n' );
	}
}

window.XMLHttpRequest = function() {
	
	this.status = 200;
	this.overrideMimeType = noop;
	this.send = noop;
	this.open = function(method, url, async) {

		this.responseText = new File(url).content;
	}
}

var timeoutList = [];

window.setTimeout = function(fct, delay) {

	timeoutList.push(arguments);
}

window.document = new function() {

	this.getElementById = function(id) {

		if ( id == "text_clipboard" ) {
			
			return {
				get value() {
			    
					return new File('clipboard').content || '';
				},
				set value(val) {
			    
					new File('clipboard').content = val || undefined;
				}
    		}
		}
	}
}

// main

Exec('cpux86-ta.js');
Exec('jslinux.js');
start();

function endHandler() {

	if ( endSignal == 1 ) {

		endSignal = 0;
		term.SendKeys('\x03');
	}
}

while ( endSignal != 2 ) {

	var to = timeoutList.shift();
	if ( !to )
		break;
	var mask = ProcessEvents(Console.Events(), EndSignalEvents(endHandler), TimeoutEvents(to[1]/10));
	to[0]();
}
