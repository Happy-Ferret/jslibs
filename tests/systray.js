LoadModule('jsstd');
LoadModule('jswinshell');

var s = new Systray();
s.icon = new Icon( 0 );
s.menu = { add:'Add', exit:'Exit', s1:{ separator:true } };
s.onmousedown = s.PopupMenu;

s.oncommand = function( id, button ) {

	switch ( id ) {
		case 'exit':
			return true;
		case 'add':
			var fileName = FileOpenDialog( 'executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*' );
			if ( !fileName )
				return;
			s.menu[fileName] = { 
				icon: ExtractIcon( fileName ), 
				text: fileName.substr( fileName.lastIndexOf( '\\' ) + 1 )
			};
			break;
		default:
			if ( button == 1 ) // left-click to run.
				CreateProcess( id );
			else // right-click to remove.
				if ( MessageBox( 'Remove item: ' + id + '? ', 'Question', MB_YESNO) == IDYES )
					delete s.menu[id];
		}
}

do { Sleep(100) } while ( !s.ProcessEvents() );
