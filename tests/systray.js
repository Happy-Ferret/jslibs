LoadModule('jsstd');
LoadModule('jswinshell');

// MessageBox() Flags
const MB = {
	OK                  :0x000000,
	OKCANCEL            :0x000001,
	ABORTRETRYIGNORE    :0x000002,
	YESNOCANCEL         :0x000003,
	YESNO               :0x000004,
	RETRYCANCEL         :0x000005,
	CANCELTRYCONTINUE   :0x000006,

	ICONHAND            :0x000010,
	ICONQUESTION        :0x000020,
	ICONEXCLAMATION     :0x000030,
	ICONASTERISK        :0x000040,
	USERICON            :0x000080,
	ICONWARNING         :0x000030,
	ICONERROR           :0x000010,
	ICONINFORMATION     :0x000040,
	ICONSTOP            :0x000010,

	DEFBUTTON1          :0x000000,
	DEFBUTTON2          :0x000100,
	DEFBUTTON3          :0x000200,
	DEFBUTTON4          :0x000300,

	APPLMODAL           :0x000000,
	SYSTEMMODAL         :0x001000,
	TASKMODAL           :0x002000,
	HELP                :0x004000,
	NOFOCUS             :0x008000,

	SETFOREGROUND       :0x010000,
	DEFAULT_DESKTOP_ONLY:0x020000,
	TOPMOST             :0x040000,
	RIGHT               :0x080000,

	RTLREADING          :0x100000
};


// Dialog Box Command IDs
const ID = {
	OK      :1,
	CANCEL  :2,
	ABORT   :3,
	RETRY   :4,
	IGNORE  :5,
	YES     :6,
	NO      :7,
	CLOSE   :8,
	HELP    :9,
	TRYAGAIN:10,
	CONTINUE:11,

	TIMEOUT :32000
};

var s = new Systray();
s.icon = new Icon( 0 );
s.menu = { add:'Add', exit:'Exit', s1:{ separator:true } };
s.onmousedown = function( button ) { 

	s.PopupMenu();
}

s.oncommand = function( id, button ) {

	switch ( id ) {
		case 'exit':
			return true;
		case 'add':
			var fileName = FileOpenDialog( 'executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*' );
			if ( !fileName )
				return;
			var icon = ExtractIcon( fileName );
			var text = fileName.substr( fileName.lastIndexOf( '\\' ) + 1 );
			s.menu[fileName] = { icon:icon, text:text };
			break;
		default:
			if ( button == 1 )
				CreateProcess( id );
			else
				if ( MessageBox( 'Remove item: ' + id + '? ', 'Question', MB.YESNO) == ID.YES )
					delete s.menu[id];
		}
}

do { Sleep(100) } while ( !s.ProcessEvents() );
