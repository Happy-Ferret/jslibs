LoadModule('jsstd');
LoadModule('jsdebug');
var i = 0;
Trap(LineToPC(17), 'Print(i,"\\n")');
i++;
i++;
i++;

function inner() {

	i = 100;
	i++;
	i++;
}

inner();

i++;
i++;
i++;






