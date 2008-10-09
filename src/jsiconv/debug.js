LoadModule('jsstd');
LoadModule('jsiconv');

Print( Iconv.list.indexOf('ISO-8859-15') );


var conv = new Iconv('UTF-8', 'ISO-8859-15');

Print( conv('testé et approuvé') ,'\n' );




