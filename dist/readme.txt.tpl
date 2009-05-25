                               jslibs
                               $(type) $(version) r$(revision)


DESCRIPTION:
  jslibs is a standalone JavaScript development runtime environment for using JavaScript as a general-purpose scripting language.
  Check the project website http://jslibs.googlecode.com for more details.


LICENSE:
  This software is under the GNU GENERAL PUBLIC LICENSE Version 2
  Refer to the file gpl-2.0.txt for license agreement.


IMPORTANT:
  jslibs binaries are compiled with the dynamic version of the C Runtime Library (msvcr80.dll).
  This mean you need the "Visual C++ 2005 SP1 Redistributable Package (x86)" to be installed on your system.
  To test if the package is already installed, just run ./bin/jshost.exe and if you get an error message, the package is not installed.
  The Microsoft® C Runtime Library installer is provided with this jslibs package (vcredist_x86.exe).
  The Microsoft® C Runtime Library is also available here:
    http://www.microsoft.com/downloads/details.aspx?familyid=200B2FD9-AE1A-4A14-984D-389C36F85647


QUICK TEST:
  Launch "run examples.cmd" and type:
    jshost helloworld.js


DOCUMENTATION:
  The on-line jslibs API documentation is available here:
    http://code.google.com/p/jslibs/wiki/JSLibs


SOURCE CODE:
  The source code of jslibs is available here:
    http://code.google.com/p/jslibs/source/checkout


BUGS:
  You can report bugs or issues here:
    http://code.google.com/p/jslibs/issues/list


CONTACT:
  soubok+jslibs@gmail.com


CHANGES:
	http://code.google.com/p/jslibs/wiki/ReleaseNotes

$(changes)
