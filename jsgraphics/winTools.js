function MouseMotion( win ) {
	
	var _infiniteMode = false; // by default, it is naturaly false
	var _savedMousePosition;
	var prevMouseX, prevMouseY;
	var cx, cy;
	var _this = this;
	
	var prev_onmousemove = win.onmousemove;
	win.onmousemove = function( x,y, b1,b2,b3 ) { // mouse X, mouse Y, left, right, middle
		
		if ( !_infiniteMode ) {
			_this.move && _this.move( x,y, b1,b2,b3 );
			return;
		}

		var r = win.rect;
		cx = Math.round((r[2]-r[0])/2);
		cy = Math.round((r[3]-r[1])/2);
		
		if ( (x != cx || y != cy) && prevMouseX != undefined && prevMouseY != undefined ) {

			_this.delta && _this.delta( x - prevMouseX, y - prevMouseY, b1,b2,b3 );
			win.cursorPosition = [cx,cy];
		}
		prevMouseX = x;
		prevMouseY = y;
	}
	
	function InfiniteModeGetter() {

		return _infiniteMode;
	}

	function InfiniteModeSetter(polarity) {
	
		if ( _infiniteMode == polarity ) // no changes has been done
			return

		if ( polarity ) {
			_savedMousePosition = win.cursorPosition;
		} else {
			prevMouseX = undefined;
			prevMouseY = undefined;
			win.cursorPosition = _savedMousePosition;
		}
		Window.showCursor = !polarity;
		win.captureMouse = polarity;
		_infiniteMode = polarity;
	}
	
	this.__defineGetter__("infiniteMode", InfiniteModeGetter );
	this.__defineSetter__("infiniteMode", InfiniteModeSetter );
}
