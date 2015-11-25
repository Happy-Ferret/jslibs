<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jswinshell/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jswinshell/qa.js) -
# jswinshell module #





---

## jswinshell static members ##
- [top](#jswinshell_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jswinshell/global.cpp?r=2555) -

### Static functions ###

#### <font color='white' size='1'><b>ExtractIcon</b></font> ####
> <sub>Icon</sub> <b>ExtractIcon</b>( fileName [[.md](.md), iconIndex ] )
> > Retrieves an icon from the specified executable file, DLL, or icon file.<br />
> > The function returns _undefined_ if no icon is found.
> > ##### <font color='red'>beware</font>: #####
> > > This function is not supported for icons in 16-bit executables and DLLs.

#### <font color='white' size='1'><b>MessageBox</b></font> ####

> <sub>integer</sub> <b>MessageBox</b>( content [[.md](.md), caption [[.md](.md), style ] ] )
> > Displays a modal dialog box that contains a system icon, a set of buttons, and a brief application-specific message, such as status or error information.
> > The message box returns an integer value that indicates which button the user clicked.
> > ##### arguments: #####
      1. <sub>string</sub> _content_: the message to be displayed.
      1. <sub>string</sub> _caption_: the dialog box title.
      1. <sub>integer</sub> _style_: specifies the contents and behavior of the dialog box. This parameter can be a combination of flags from the following groups of flags:
        * MB\_ABORTRETRYIGNORE
        * MB\_CANCELTRYCONTINUE
        * MB\_HELP
        * MB\_OK
        * MB\_OKCANCEL
        * MB\_RETRYCANCEL
        * MB\_YESNO
        * MB\_YESNOCANCEL
        * MB\_ICONEXCLAMATION
        * MB\_ICONWARNING
        * MB\_ICONINFORMATION
        * MB\_ICONASTERISK
        * MB\_ICONQUESTION
        * MB\_ICONSTOP
        * MB\_ICONERROR
        * MB\_ICONHAND
        * MB\_DEFBUTTON1
        * MB\_DEFBUTTON2
        * MB\_DEFBUTTON3
        * MB\_DEFBUTTON4
        * MB\_APPLMODAL
        * MB\_SYSTEMMODAL
        * MB\_TASKMODAL
        * MB\_DEFAULT\_DESKTOP\_ONLY
        * MB\_RIGHT
        * MB\_RTLREADING
        * MB\_SETFOREGROUND
        * MB\_TOPMOST
> > ##### return value: #####
      * IDABORT: Abort button was selected.
      * IDCANCEL: Cancel button was selected.
      * IDCONTINUE: Continue button was selected.
      * IDIGNORE: Ignore button was selected.
      * IDNO: No button was selected.
      * IDOK: OK button was selected.
      * IDRETRY: Retry button was selected.
      * IDTRYAGAIN: Try Again button was selected.
      * IDYES: Yes button was selected.
      * 32000: timeout <b><font color='red'>???</font></b>

#### <font color='white' size='1'><b>CreateProcess</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>CreateProcess</b>( applicationPath , [[.md](.md) commandLine ], [[.md](.md) environment ], [[.md](.md) currentDirectory ] )
> > Creates a new process.

> ##### arguments: #####
    1. <sub>string</sub> _applicationPath_
    1. <sub>string</sub> _commandLine_
    1. <sub>string</sub> _environment_
    1. <sub>string</sub> _currentDirectory_
> ##### example: #####
```
 CreateProcess( 'C:\\WINDOWS\\system32\\calc.exe', undefined, undefined, 'c:\\' );
```

#### <font color='white' size='1'><b>FileOpenDialog</b></font> ####
> <sub>string</sub> | _undefined_ <b>FileOpenDialog</b>( [[.md](.md) filters ] [[.md](.md), defaultFileName ] )
> > Creates an Open dialog box that lets the user specify the drive, directory, and the name of a file. The function returns _undefined_ if the dialog is canceled.

> ##### example: #####
```
 FileOpenDialog( 'executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*' );
```

#### <font color='white' size='1'><b>ExpandEnvironmentStrings</b></font> ####
> <sub>string</sub> <b>ExpandEnvironmentStrings</b>( sourceString )
> > Expands environment-variable strings and replaces them with the values defined for the current user.

#### <font color='white' size='1'><b>Sleep</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Sleep</b>( milliseconds )
> > Suspends the execution of the current process until the time-out interval elapses.

#### <font color='white' size='1'><b>MessageBeep</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>MessageBeep</b>( type )
> > Plays a waveform sound. The waveform sound for each sound type is identified by an entry in the registry.
> > ##### arguments: #####
      1. <sub>integer</sub> _type_:
        * -1 : Simple beep. If the sound card is not available, the sound is generated using the speaker.
        * 0 : MB\_OK SystemDefault
        * ICONASTERISK: SystemAsterisk
        * ICONEXCLAMATION: SystemExclamation
        * ICONHAND: SystemHand
        * ICONQUESTION: SystemQuestion

#### <font color='white' size='1'><b>Beep</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Beep</b>( hertzFrequency, millisecondsDuration )
> > Generates simple tones on the speaker.
> > ##### note: #####
> > > The function is synchronous, it does not return control to its caller until the sound finishes.

#### <font color='white' size='1'><b>RegistryGet</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>RegistryGet</b>( path )
> > Get a value of the system registry

### Static properties ###

#### <font color='white' size='1'><b>clipboard</b></font> ####

> <sub>string</sub> <b>clipboard</b>
> > Places or retrieves text data from the clipboard.


---

## class jswinshell::Console ##
- [top](#jswinshell_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jswinshell/console.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>()
> > Creates a new Console object.
> > ##### <font color='red'>beware</font>: #####
> > > Only one console per process is allowed. The construction fails if the calling process already has a console.

### Methods ###

#### <font color='white' size='1'><b>Close</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Close</b>()
> > Detach the current process from its console.

#### <font color='white' size='1'><b>Write</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Write</b>( text )
> > Write text to the console.
> > ##### arguments: #####
      1. <sub>string</sub> _text_

#### <font color='white' size='1'><b>Read</b></font> ####

> <sub>string</sub> <b>Read</b>( amount )
> > Read _amount_ bytes of text from the console.
> > ##### arguments: #####
      1. <sub>integer</sub> _amount_

### Properties ###

#### <font color='white' size='1'><b>title</b></font> ####

> <sub>string</sub> <b>title</b>
> > Get or set the title of the console window.

### Examples ###
```
var cons = new Console();
cons.title = 'My console';
cons.Write('Hello world');
```


---

## class jswinshell::WinError ##
- [top](#jswinshell_module.md) -



---

## class jswinshell::Icon ##
- [top](#jswinshell_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jswinshell/icon.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( image | integer )
> > Icon constructor accepts an [Image](Image.md) object or a integer.
> > <br />
> > The integer value can be one of:
      * `0`: IDI\_APPLICATION
      * `1`: IDI\_QUESTION
      * `2`: IDI\_INFORMATION
      * `3`: IDI\_WARNING
      * `4`: IDI\_ERROR


---

## class jswinshell::Systray ##
- [top](#jswinshell_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jswinshell/systray.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>()
> > Creates a Systray object.

### Methods ###

#### <font color='white' size='1'><b>Close</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Close</b>()
> > Close the Systray.

#### <font color='white' size='1'><b>ProcessEvents</b></font> ####

> <sub>boolean</sub> <b>ProcessEvents</b>()
> > Precess all pending events of the systray.
> > The function returns true if at least one of the event function ( see Remarks below ) returns true.

#### <font color='white' size='1'><b>Focus</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Focus</b>()
> > Puts the systray into the foreground. Keyboard input is directed to the systray.

#### <font color='white' size='1'><b>PopupMenu</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>PopupMenu</b>()
> > Opens the systray menu.

#### <font color='white' size='1'><b>Position</b></font> ####

> <sub>Array</sub> <b>Position</b>( [[.md](.md)reusableArray] )
> > Returns the [x,y] position pointed by the mouse pointer in the systray icon.
> > ##### note: #####
> > > If you provide a _reusableArray_, the function will use it to store the values.

> > ##### example: #####
```
  systray.onmousemove = function( x, y ) {

   var pos = systray.Position();
   Print( x-pos[0], ', ', y-pos[1], '\n' );
  }
```

#### <font color='white' size='1'><b>Rect</b></font> ####

> <sub>Array</sub> <b>Rect</b>( [[.md](.md)reusableArray] )
> > Returns the dimensions [left, top, width, height] of the systray rectangle.
> > ##### note: #####
> > > If you provide a _reusableArray_, the function will use it to store the values.

### Properties ###

#### <font color='white' size='1'><b>icon</b></font> ####

> <sub>Icon</sub> | <sub>null</sub> <b>icon</b> <font color='red'><sub>write-only</sub></font>
> > This is the Icon to be used as systray icon.

#### <font color='white' size='1'><b>visible</b></font> ####

> <sub>boolean</sub> <b>visible</b> <font color='red'><sub>write-only</sub></font>
> > Show or hide the systray icon.
> > ##### <font color='red'>beware</font>: #####
> > > you cannot use this property to get the current visibility of the icon.

#### <font color='white' size='1'><b>text</b></font> ####

> <sub>string</sub> <b>text</b>
> > Get of set the tooltip text of the systray icon.

#### <font color='white' size='1'><b>menu</b></font> ####

> <sub>Object</sub> <b>menu</b>
> > The object is a key:value map of all available commands in the menu.
> > each command has an single keyName and a description object that hole the following properties:
      * <sub>string | function</sub> **text**
      * <sub>boolean | function</sub> **checked**
      * <sub>boolean | function</sub> **grayed**
      * <sub>boolean</sub> **separator**
      * <sub>boolean</sub> **default**
      * <sub>[Icon] | function</sub> **icon**
> > > ##### example: #####
```
   tray.menu = {
    my_ena:{ text:"enable", checked:true },
    my_add:{ text:"delete", grayed:true },
    sep1:{ separator:true },
    my_exit:"Exit"
   };
```
> > > <br />
> > > If the value of _text_, _checked_, _grayed_ or _icon_ is a function, it is called and the return value is used.
> > > ##### example: #####
```
   tray.menu = { ena:{ text:"enable", checked:function() { return isChecked } }, ...
```

### Callback functions ###

> The following functions are called when you call ProcessEvents() according the events received by the tray icon.
    * **onfocus**( _true_ )
    * **onblur**( `ALSE` )
    * **onchar**( <sub>string</sub> char )
    * **oncommand**( <sub>string</sub> id, <sub>integer</sub> mouseButton )
    * **onmousemove**( <sub>integer</sub> mouseX, <sub>integer</sub> mouseY )
    * **onmousedown**( <sub>integer</sub> mouseButton, _true_ )
    * **onmouseup**( <sub>integer</sub> mouseButton, `ALSE` )
    * **onmousedblclick**( <sub>integer</sub> mouseButton )
> ##### example: #####
```
 var s = new Systray();
 s.icon = new Icon( 0 );
 s.onmousedown = function( button ) {

  MessageBeep();
  s.PopupMenu();
 }
```

### Examples ###
> ##### example 1: #####
```
 var s = new Systray();

 s.icon = new Icon(new Png(new File('calendar.png').Open(File.RDONLY)).Load());
 s.text = "calendar";
 s.menu = { exit_cmd:"exit" }

 s.onmousedown = function(button) {
   if ( button == 2 )
     s.PopupMenu();
 }

 s.oncommand = function(id) {
   if ( id == 'exit_cmd' )
     exit = true;
 }

 while ( !exit ) {
   s.ProcessEvents();
   Sleep(100);
 }
```

> ##### example 2: #####
```
 LoadModule('jsstd');
 LoadModule('jswinshell');

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
         if ( MessageBox( 'Remove item: ' + id + '? ', 'Question', MB_YESNO) == IDYES )
           delete s.menu[id];
     }
 }

 do { Sleep(100) } while ( !s.ProcessEvents() );
```


---

- [top](#jswinshell_module.md) - [main](JSLibs.md) -
