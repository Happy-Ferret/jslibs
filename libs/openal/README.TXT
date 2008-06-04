OpenAL wikipedia:
	http://en.wikipedia.org/wiki/Openal

OpenAL tutorial:
	http://www.devmaster.net/articles.php?catID=6

OpenAL website:
	http://www.openal.org/

OpenAL SVN:
	branch: Win1-1-2_Spec1-0 for the last Windows code before the transition towards OpenAL 1.1 began
	svn checkout http://www.openal.org/repos/openal/trunk openal

OpenAL Installer for Windows
	http://developer.creative.com/articles/article.asp?cat=1&sbcat=31&top=38&aid=46=======

OpenAL Installer for Windows:
	http://www.openal.org/downloads.html
	contain OpenAL32.dll

OpenAL 1.1 SDK for Windows
	http://www.openal.org/downloads.html
	contain OpenAL32.lib, al.h

Windows compilation issues:

BUILD REQUIREMENTS:
	Microsoft Platform SDK

	Microsoft DirectX 9+
		- you only need to install header and libraries files.

	In Tools > Options > Projects and Solutions > VC++ Directories > Include files, add the following directories if they are missing:
		$(ProgramFiles)\Microsoft Platform SDK\include
		$(ProgramFiles)\Microsoft Platform SDK\include\atl
		$(ProgramFiles)\Microsoft Platform SDK\include\mfc


	Remove any reference to odbc libraries from link

	add user32.lib and ole32.lib in OpenAL-Windows\OpenAL32\OpenAL32.vcproj


TBD:
  - fix the following error while building in command line:
      .\OpenAL32.rc(10) : fatal error RC1015: cannot open include file 'afxres.h'.
