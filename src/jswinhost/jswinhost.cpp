/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */

#include "stdafx.h"

#include "Shellapi.h"

#define HOST_STACK_SIZE 4194304 // = 4 * 1024 * 1024

// set stack to 4MB:
#pragma comment (linker, JL_TOSTRING(/STACK:HOST_STACK_SIZE))

#define USE_NEDMALLOC


static uint8_t embeddedBootstrapScript[] =
	#include "embeddedBootstrapScript.js.xdr.cres"
	"";


#ifdef DEBUG

class hostIO : public jl::StdIO {
public:
	int
	output( jl::BufString &buf ) {
			
		OutputDebugString( buf );
		return buf.length();
	}

	int
	error( jl::BufString &buf ) {

		OutputDebugString( TEXT( "STDERR: " ) );
		OutputDebugString( buf );
		return 8 + buf.length();
	}
} hostIO;

#else

jl::StdIO hostIO;

#endif


// to be used in the main() function only
#define HOST_MAIN_ASSERT( condition, errorMessage )	\
	JL_MACRO_BEGIN									\
		if ( !(condition) ) {						\
			jl::BufString buf(errorMessage);		\
			hostIO.error(buf);						\
			goto bad;								\
		}											\
	JL_MACRO_END									\


#ifdef USE_NEDMALLOC

#define NO_NED_NAMESPACE
#define NO_MALLINFO 1
#include "../../libs/nedmalloc/nedmalloc.h"


class NedAllocators : public jl::Allocators {
	static volatile bool _skipCleanup;

	static NOALIAS void
	nedfree_handlenull(void *mem) NOTHROW {

		if ( !_skipCleanup && mem != NULL )
			nedfree(mem);
	}

	static NOALIAS size_t
	nedblksize_msize(void *mem) NOTHROW {

		return nedblksize(0, mem);
	}

public:
	NedAllocators()
	: Allocators(nedmalloc, nedcalloc, nedmemalign, nedrealloc, nedblksize_msize, nedfree_handlenull) {
	}

	void
	setSkipCleanup(bool skipCleanup) {

		_skipCleanup = skipCleanup;
	}
};

volatile bool NedAllocators::_skipCleanup = false;

#endif // USE_NEDMALLOC


using namespace jl;

int CALLBACK WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd ) {

	JL_enableLowFragmentationHeap();

//  js engine and jslibs low-level allocators must the same
//	#if defined(USE_NEDMALLOC) && defined(HAS_JL_ALLOCATORS)
//	NedAllocators allocators;
//	#else
	StdAllocators allocators;
//	#endif // USE_NEDMALLOC

	HostRuntime::setJSEngineAllocators(allocators); // need to be done before AutoJSEngineInit ?

	ThreadedAllocator alloc(allocators);

	AutoJSEngineInit ase;

	{

		HostRuntime hostRuntime(allocators, true); // HOST_STACK_SIZE / 2
		JSContext *cx = hostRuntime.context();

		jl::Global global(hostRuntime);
		JL_CHK( global );

		jl::Host host(global, hostIO);
		JL_CHK( host );

		{
			JSAutoCompartment ac(global.hostRuntime().context(), global.globalObject());

			TCHAR moduleName[PATH_MAX];
			TCHAR tmp[PATH_MAX];

			DWORD moduleNameLen = ::GetModuleFileName(hInstance, moduleName, COUNTOF(moduleName));

			HOST_MAIN_ASSERT( moduleNameLen > 0 && moduleNameLen < PATH_MAX && moduleName[moduleNameLen] == '\0', "Invalid module filename." );

			// construct host.path and host.name properties
			jl::memcpy( tmp, moduleName, TSIZE(moduleNameLen + 1) );
			TCHAR *name = jl::strrchr( tmp, TEXT( PATH_SEPARATOR ) );
			JL_CHK( name );
			name += 1;
			JL_CHK( host.setHostName(name) );
			*name = TEXT( '\0' );
			JL_CHK( host.setHostPath(tmp) );

			LPTSTR *szArglist;
			int nArgs;
			szArglist = jl::CommandLineToArgvW(::GetCommandLineW(), &nArgs);
			host.setHostArguments(szArglist, nArgs);
			::free(szArglist);

			{

				JS::RootedObject globalObject(cx, global.globalObject());
				JS::RootedValue rval(cx);

				// embedded bootstrap script
				if ( sizeof(embeddedBootstrapScript)-1 > 0 ) {

					JS::AutoSaveContextOptions asco(cx);
					JS::ContextOptionsRef(cx).setDontReportUncaught(false);

					JS::RootedScript script(cx, JS_DecodeScript(cx, embeddedBootstrapScript, sizeof(embeddedBootstrapScript)-1, NULL) ); // -1 because sizeof("") == 1
					JL_CHK( script );
					JL_CHK( JS_ExecuteScript(cx, globalObject, script, &rval) );
				}

				// construct script name
				jl::memcpy( tmp, moduleName, TSIZE(moduleNameLen + 1) );
				TCHAR *dotPos = jl::strrchr( tmp, TEXT( '.' ) );
				JL_CHK( dotPos );
				jl::strcpy( dotPos + 1, TEXT( "js" ) );

				bool executeStatus;
				executeStatus = jl::executeScriptFileName( cx, globalObject, tmp, EncodingType::ENC_UNKNOWN, false, &rval );

				if ( !executeStatus )
					if ( JL_IsExceptionPending(cx) )
						JS_ReportPendingException(cx); // see JSOPTION_DONT_REPORT_UNCAUGHT option.
			}
		}
	}

	return 0;
bad:
	return -1;
}


	//doc: If you need to detect whether another instance already exists, create a uniquely named mutex using the CreateMutex function. 
	//CreateMutex will succeed even if the mutex already exists, but the function will return ERROR_ALREADY_EXISTS. 
	//This indicates that another instance of your application exists, because it created the mutex first.

	// (TBD) use file index as mutexName. note: If the file is on an NTFS volume, you can get a unique 64 bit identifier for it with GetFileInformationByHandle.  The 64 bit identifier is the "file index". 
	
	// remoce isFirstInstance, use |new Semaphore('aaa').isOwner| instead !
	// normalize the mutex name
	// handle host.isFirstInstance property
	//
	//HANDLE instanceCheckMutex = NULL;
	//jl::memcpy(tmp, moduleName, moduleNameLen+1);
	//bool isFirstInstance;
	//char *pos;
	//while ( (pos = strchr(tmp, PATH_SEPARATOR)) != 0 )
	//	*pos = '/';
	//SetLastError(0);
	//instanceCheckMutex = ::CreateMutex(NULL, TRUE, tmp); // see Global\\ and Local\\ prefixes for mutex name.
	//switch ( GetLastError() ) {
	//	case ERROR_SUCCESS:
	//		isFirstInstance = true;
	//		break;
	//	case ERROR_ALREADY_EXISTS:
	//		isFirstInstance = false;
	//		break;
	//	default: {
	//		char message[1024];
	//		JLLastSysetmErrorMessage(message, sizeof(message));
	//		HOST_MAIN_ASSERT( false,  message );
	//	}
	//}
	// ...
	// ...
	//JL_CHK( jl::setProperty(cx, host.hostObject(), JLID(cx, isFirstInstance), isFirstInstance) );
	// ...
	// ...
	//CloseHandle( instanceCheckMutex ); //ReleaseMutex


/**doc
#summary jswinhost executable
#labels doc

= jswinhost executable =
 [http://code.google.com/p/jslibs/ home] *>* [JSLibs] *>* [jswinhost] - [http://jslibs.googlecode.com/svn/trunk/jswinhost/jswinhost.cpp http://jslibs.googlecode.com/svn/wiki/source.png]

=== Description ===
 jswinhost (javascript windows host) is a small executable file that run javascript programs under a windows environment.
 The main difference with jshost is the jswinhost does not create any console windows.

=== Methods ===

 * status *loadModule*( moduleFileName )
  see [jshost]

=== Properties ===

 * $OBJ *global* $READONLY
  is the global object.

 * $ARRAY *arguments* $READONLY
  is the host path [0] and whole command line [1].

 * $BOOL *isfirstinstance* $READONLY
  is true if the current instance is the first one. This can help to avoid jswinhost to be run twice at the same time.

=== Host object ===
 see [jshost]

=== Remarks ===
 There is no way to specify the script to execute using the command line. You have to create a .js file using the name of the host.
 By default, jswinhost.exe is the name of the host and jswinhost.js is the script the host execute.
 
Because jwinshost do not use a console window, errors and printed messages will not be displayed.

However, you can write your own output system:
{{{
loadModule('jswinshell');
host.stdout = new Console().Write;
host.stderr = MessageBox;
loadModule('jsstd');
print('toto');
hkqjsfhkqsdu_error();
}}}

== Embedding JS scripts in your jswinhost binary ==
 see [jshost]

**/
