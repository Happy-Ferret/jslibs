var queryData = {};
for each ( var item in getParam('QUERY_STRING').split('&') ) {

	var pos = item.indexOf('=');
	if ( pos != -1 )
		queryData[item.substr(0,pos)] = URLDecode(item.substr(++pos));
}



function CGIVariableList() {
	
	var fcgiParams = getParam();
	var list = <ul/>;
	for ( var k in fcgiParams )
		list += <li><b>{k}</b><pre>{fcgiParams[k]}</pre></li>;
	return list;
}


write( 'Content-type: text/html; charset=iso-8859-1\r\n\r\n' );
write( '<?xml version="1.0" encoding="iso-8859-1"?>' );
write( '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">' );
write(
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
		<form action={getParam('SCRIPT_NAME')}>
			<textarea name="text">{queryData.text||''} </textarea>
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
