LoadModule('jsstd');
LoadModule('jsio');

LoadModule('jsimage');

LoadModule('jssvg');

var svg = new SVG();


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


svg.Write('<?xml version="1.0" encoding="utf-8"?>'+s);

var svgimage = svg.GetImage();

//var svgText = new File('Image_Tectonic_plates.svg').content
//Print( svgText.length );
//var svgimage = svg.Write(svgText);

new File('test.png').content = EncodePngImage( svgimage );
