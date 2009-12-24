// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jssvg');
LoadModule('jsprotex');

function Time( fct ) {

	var t0 = new Date().valueOf();
	fct();
	Print( fct.name + ' time: ', new Date().valueOf() - t0, 'ms\n' );
}



var s = 
<svg width="8cm" height="3cm" viewBox="0 0 800 300" version="1.1" xmlns:xlink="http://www.w3.org/1999/xlink">
     <desc>Example mask01 - blue text masked with gradient against red background </desc>
     <defs>
          <linearGradient id="Gradient" gradientUnits="userSpaceOnUse" x1="0" y1="0" x2="800" y2="0">
               <stop offset="0" stop-color="white" stop-opacity="0"/>
               <stop offset="1" stop-color="white" stop-opacity="1"/>
          </linearGradient>
          <mask id="Mask" maskUnits="userSpaceOnUse" x="0" y="0" width="800" height="300">
               <rect x="0" y="0" width="800" height="300" fill="url(#Gradient)"/>
          </mask>
          <text id="Text" x="400" y="200" font-family="Verdana" font-size="100" text-anchor="middle"> Masked text </text>
     </defs>
     <rect x="0" y="0" width="800" height="300" fill="#FF8080"/>
     <use xlink:href="#Text" fill="blue" mask="url(#Mask)"/>
     <use xlink:href="#Text" fill="none" stroke="black" stroke-width="2"/>
</svg>;

/*
var s =
<svg width="100" height="50" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" >
     <image x="0" y="0" width="85" height="25" xlink:href="img.png"/>
</svg>  
*/

var s = 
<svg width="200" height="200" xmlns="http://www.w3.org/2000/svg" >
     <defs>
          <pattern id="pattern" x="0" y="0" width=".1" height=".1">
               <rect x="2" y="2" width="16" height="16" fill="#3399cc"/>
          </pattern>
     </defs>
     <rect x="0" y="0" width="100" height="100" fill="url(#pattern)" stroke="black"/>
</svg> 



var s = <svg width="100%" height="100%" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" >
<rect id="a" x="0" y="0" width="100" height="120" fill="#FF0000" />
<rect id="b" x="100" y="0" width="100" height="120" fill="#00FF00" />
<rect id="c" x="200" y="0" width="100" height="120" fill="#0000FF" />
<circle cx="150" cy="60" r="40" stroke="black" stroke-width="1" fill="none" shape-rendering="crispEdges"/>
 <defs>
      <path id="MyPath" d="M 100 200 C 200 100 300 0 400 100 C 500 200 600 300 700 200 C 800 100 900 100 900 100"/>
 </defs>
  <use xlink:href="#MyPath" fill="none" stroke="red"/>
  <text>123</text>
</svg>


var s = <svg width="100%" height="100%" version="1.1"
xmlns="http://www.w3.org/2000/svg">
<defs>
<filter id="MyFilter" filterUnits="userSpaceOnUse" x="0" y="0" width="200" height="120">
	<feGaussianBlur in="SourceAlpha" stdDeviation="4" result="blur"/>
	<feOffset in="blur" dx="4" dy="4" result="offsetBlur"/>
	<feSpecularLighting in="blur" surfaceScale="5" specularConstant=".75" specularExponent="20"
	lighting-color="#bbbbbb" result="specOut">
		<fePointLight x="-5000" y="-10000" z="20000"/>
	</feSpecularLighting>
	<feComposite in="specOut" in2="SourceAlpha" operator="in" result="specOut"/>
	<feComposite in="SourceGraphic" in2="specOut" operator="arithmetic" k1="0" k2="1" k3="1" k4="0" result="litPaint"/>
	<feMerge>
        	<feMergeNode in="offsetBlur"/>
	        <feMergeNode in="litPaint"/>
	</feMerge>
</filter>
</defs>
<rect x="1" y="1" width="198" height="118" fill="#cccccc" />
<g filter="url(#MyFilter)">
<path fill="none" stroke="#D90000" stroke-width="10" d="M50,90 C0,90 0,30 50,30 L150,30 C200,30 200,90 150,90 z" />
<text fill="#FFFFFF" stroke="black" font-size="45" font-family="Verdana" x="52" y="76">SVG</text>
</g>
</svg>


var s = <svg width="100%" height="100%" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
	<rect x="0" y="0" width="100" height="100" fill="#FF4422" />
	<circle id="a" cx="50" cy="50" r="25" stroke="black" stroke-width="16" fill="none"/>
	<use xlink:href="#a" stroke="#FF4422" stroke-width="10" x="8" />
	<use xlink:href="#a" stroke="#black" stroke-width="6" x="8" />
	<image x="0" y="0" width="100" height="64" path="img.png"/>
	<rect id="test" x="0" y="0" width="100" height="100" fill="#000000" />
</svg>


var s = <svg width="128" height="128">
	<rect x="0" y="0" width="128" height="128" fill="red" />
	<rect x="0" y="0" width="64" height="64" fill="green" />
	<rect x="0" y="0" width="32" height="32" fill="blue" />
</svg>;


var svg = new SVG();
svg.onImage = function(href) {
//	return DecodePngImage( new File('img.png').Open('r') );
}

Time( function readXmlData()
svg.Write(s)
)

Print( 'width: '+svg.width + ' height: '+svg.height, '\n' );

svg.SetVisible('#test', false);

//svg.dpi = [30, 600];

var svgimage;

//svg.Translate(64,0);
svg.Rotate(Math.PI/4);

Time( function renderImage()
svgimage = svg.RenderImage(64, 64, 3, true)
)


//var svgText = new File('Image_Tectonic_plates.svg').content
//Print( svgText.length );
//var svgimage = svg.Write(svgText);

Time( function encodePNG()
new File('test.png').content = EncodePngImage( svgimage )
)

