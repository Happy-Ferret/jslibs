LoadModule('jsstd');
LoadModule('jsio');

LoadModule('jsimage');

LoadModule('jssvg');

var svg = new SVG();


var s = <svg
   xmlns="http://www.w3.org/2000/svg" 
   xmlns:xlink="http://www.w3.org/1999/xlink"
   xmlns:ev="http://www.w3.org/2001/xml-events"
   version="1.1"
   baseProfile="full"
   x="0"
   y="0"
   width="300"
   height="200"
   id="svg2">
  <title>Rectangles</title>
  <defs
     id="defs4" />
  <g
     id="layer1">
    <rect
       width="300"
       height="120"
       x="0"
       y="20"
       fill="green"
       id="rect1306" />
    <rect
       width="80"
       height="150"
       x="20"
       y="30"
       fill="red"
       id="rect1308" />
    <rect
       width="140"
       height="80"
       x="50"
       y="50"
       fill="blue"
       id="rect1310" />
		<text x = "10" y = "25" fill = "navy" font-size = "15" font-family = "arial">foo</text>       
  </g>
</svg>
var image = svg.Write('<?xml version="1.0" encoding="utf-8"?>'+s);

//var s = new File('../../logo.svg').content;

var svgimage = svg.Write(s);

new File('test.png').content = EncodePngImage( svgimage );