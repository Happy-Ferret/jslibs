LoadModule('jsstd');
LoadModule('jsio');

var f = new File('E:\\jslibs\\src\\jsstd\\static.cpp');

var expr = /\/\*\*doc(:.*\r?\n)?((.*\r?\n)*?)\*\*\//g;

var code = f.content;

var res;
while( res = expr(code) ) {

    Print('attr:'+res[1], '\n');
    Print('text:'+res[2], '\n');
}
