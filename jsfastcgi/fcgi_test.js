// decode the query string
var queryData = {};
for each ( var item in GetParam('QUERY_STRING').split('&') ) {

	var [key,value] = item.split('=');
	queryData[key] = decodeURIComponent(value);
}

function CGIVariableList() {
	
	var fcgiParams = GetParam();
	var list = <ul/>;
	for ( var k in fcgiParams )
		list += <li><b>{k}</b><pre>{fcgiParams[k]}</pre></li>;
	return list;
}

Write( "Content-type: text/html; charset=iso-8859-1\r\n\r\n" ); // cgi response: http://www.ietf.org/rfc/rfc3875
Write( '<?xml version="1.0" encoding="iso-8859-1"?>' );
Write( '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">' );
Write(
<html>
	<head>
		<title>teswt</title>
		<style>
		body &#x7B;
			background-color: {queryData.background_color};
		&#x7D;
		</style>
	</head>
	<body>
		<form action={GetParam('SCRIPT_NAME')}>
			<textarea name="text">{queryData.text} </textarea>
			<hr/>
			<input type="submit"/>
			<input type="submit" name="background_color" value="red"/>
			<input type="submit" name="background_color" value="green"/>
			<input type="submit" name="background_color" value="blue"/>
		</form>
		<H1>Expanded query string</H1>
		{queryData.toSource()}
		<H1>CGI/1.1 variables</H1>
		{CGIVariableList()}
	</body>
</html>
);
