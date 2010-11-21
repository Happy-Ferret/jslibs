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

#ifdef WIN32
 #define DLL_EXT ".dll"
#else
 #define DLL_EXT ".so"
#endif

#include <stdlib.h>
#include <ffi.h>

//#pragma warning( disable : 4100 ) // warning C4100: 'xxx' : unreferenced formal parameter


// symbol _Py_FatalError referenced in function _ffi_prep_args
extern "C" void Py_FatalError(char *msg) {

// (TBD) need to manage a pool of context ?
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//        JavaScript Native Interface
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mini documentation
/*

Overview:
--------

  function Alert( text, caption ) {

	  var ret = new NativeData().PU32.Alloc();
	  new NativeModule('C:\\WINDOWS\\SYSTEM32\\User32').Proc('MessageBoxA')( ret, DWORD( 0 ), SZ(text), SZ( caption || "Alert" ), DWORD( 1 ) );
	  return ret[0];
  }


Detailed examples:
-----------------

  note: some casts are missing in the c-style code representation

  example 1:
  ---------

  goal: create an array of 2 strings

  // creates the root of the native data structure
  // void *nd;
  var nd = new NativeData();

  // casts the NativeData object to a pointer, and allocate the space to store 2 pointer ( .Alloc(1) is equivalent to .Alloc() )
  // nd = new void*[2];
  nd.PP.Alloc(2);

  // access to the first pointer
  // nd[0] = new char[10];
  nd.PP[0].PS8.Alloc(10);

  // fills the first string with "test" ( for a more simple syntax, see example 2 )
  // ((char*)nd[0])[0] = "t";
  // ((char*)nd[0])[1] = "e";
  // ((char*)nd[0])[2] = "s";
  // ((char*)nd[0])[3] = "t";
  // ((char*)nd[0])[4] = 0;
  nd.PP[0].PS8[0] = 't';
  nd.PP[0].PS8[1] = 'e';
  nd.PP[0].PS8[2] = 's';
  nd.PP[0].PS8[3] = 't';
  nd.PP[0].PS8[4] = '\x00';

  nd.PP[1].PS8[0] = '\x00';


  ...call( ret.VOID, nd.PP );


  example 2:
  ---------

  goal: same as example 1 but with a more advanced syntax


  var ndpp = new NativeData().PP;
  nd.PP.Alloc(3);
  nd.PP[0].String = "test";
  nd.PP[1].String = ...




Tools:
-----

  These "tools" allows a simpler handling of usuals native types:

  function NULL() {

	  var nat = new NativeData().PP;
	  nat.Alloc()[0] = 0;
	  return nat;
  }

  function DWORD( value ) {

	  var nat = new NativeData().PU32;
	  nat.Alloc()[0] = value;
	  return nat;
  }

  function BYTE( value ) {

	  var nat = new NativeData().PU8;
	  nat.Alloc()[0] = value;
	  return nat;
  }

  function SZ( value ) {

	  var nat = new NativeData;
	  nat.String = value;
	  return nat.PP;
  }

  function VOID( value ) {

	  return new NativeData().VOID;
  }

  function PDATA( size, initByte ) {

	  var nat = new NativeData().PP;
	  nat.Alloc()[0].PS8.Alloc( size, initByte );
	  return nat;
  }


Call a native function in a dynamic library (.dll/.so):
------------------------------------------------------

  function CreateProcess( fileName, commandLine ) {

	  ret = new NativeData()
	  ret.PU32.Alloc();
	  new NativeModule('C:\\WINDOWS\\SYSTEM32\\kernel32').Proc('CreateProcessA')( ret.PU32, SZ( fileName ), SZ( commandLine || "" ), NULL(), NULL(), DWORD( 0 ), DWORD( 0x20 ), NULL(), NULL(), PDATA(68, 0), PDATA(16) );
	  return ret.PU32[0] == 1;
  }

  function Sleep( time ) {

	  if ( !Sleep.proc ) { // cache NativeData objects
		  Sleep.proc = new NativeModule('C:\\WINDOWS\\SYSTEM32\\kernel32').Proc('Sleep');
		  Sleep.arg1 = new NativeData()
		  Sleep.arg1.PU32.Alloc()[0] = time
		  Sleep.ret = new NativeData().VOID;
	  }
	  Sleep.proc( Sleep.ret, Sleep.arg1.PU32 );
  }


Quick reference:
---------------

  NativeData object:
    .VOID   : getter that cast the NativeData object to void
    .PI     : getter that cast the NativeData object to a int* ( system dependant )
    .PU8    : getter that cast the NativeData object to a unsigned char*
    .PS8    : getter that cast the NativeData object to a char*
    .PU16   : getter that cast the NativeData object to a unsigned short*
    .PS16   : getter that cast the NativeData object to a short*
    .PU32   : getter that cast the NativeData object to a unsigned long*
    .PS32   : getter that cast the NativeData object to a long*
    .PU64   : getter that cast the NativeData object to a ...
    .PS64   : getter that cast the NativeData object to a ...
    .PP     : getter that cast the NativeData object to a void**
    .String : getter/setter that accept string with terminal '\0' ( C string )

  NativeType object:
    .Alloc()             : allocates one element of the type specified by it's slot[1]
    .Alloc( size )       : allocates <size> element of the type specified by it's slot[1]
    .Alloc( size, init ) : allocates <size> element of the type specified by it's slot[1], and initialize the memory with <init> value
    .Free()              : force an allocation to be freed
    [index]              : getter/setter that allow to access the <index>th element of the array.
                           there is a special treatment for .PP[], getter/setter get and return a NativeData object ( see advances examples ) or get a pointer value ( see NULL() )

  NativeModule object:
    new NativeModule( fileName, closeType ) : constructor that allow to open a handler to a dynamic linked library ( filename without .dll, it is automaticaly added )
                                              closeType: false/undefined = close the library on exit. true = close the library when the object is GC'ed
    .Close()                                : release the dynamic linked library handler.
    .Proc(name/ordinal)                     : once opened, you can acces a symbol in the dll using it's name (String) or ordinal number (Number)

  NativeProc objet:
    ( returnValue, arg1, arg2, ... ) : the call operator calls the symbol


Advanced examples:
-----------------

  example 1:
  ---------

    C function:
    ----------

      __declspec(dllexport) void AddOne( int *i ) {

        *i = *i + 1;
      }


    JS code:
    -------

	    var i = new NativeData();
      i.PI.Alloc();
      i[0] = 123;

      ...
		  var pi = new NativeData().PP.Alloc();
		  pi[0] = i;

      ...
      lib.Proc('AddOne')( VOID(), pi );

      Print( i[0] ); => 124;


Advanced informations:
---------------------

  NativeData internal:
    private : pointer to [ the pointer to the data ]
    slot[0] : root NativeData Object ( this is used when we have to store pointers that have to be freed )

  NativeType internal:
    private : &ffi_type_...
    slot[0] : reference to a NativeData object, only in the case PP[0] = nativeData object...
	 slot[1] : it's NativeData ( nd.PP[0] ==> nd.PP.0 ==> NativeData.NativeType.NativeData )


  NativeProc internal:
    private : <not used>
    slot[0] : name of the proc to call
	 slot[1] : reference to it's NativeModule object

  NativeModule internal:
    private : pointer / handle to the loaded library


Lib dependances:
---------------

  - spidermonkey
  - libffi / libffi_msvc


Notes:
-----

  the memory allocated with .Alloc is automaticaly freed when the root NativeData object is released / garbage collected

  char  	1  	character or integer 8 bits length.  	signed: -128 to 127 unsigned: 0 to 255
  short 	2 	integer 16 bits length. 	signed: -32768 to 32767 unsigned: 0 to 65535
  long 	4 	integer 32 bits length. 	signed:-2147483648 to 2147483647 unsigned: 0 to 4294967295
  int 	* 	Integer. Its length traditionally depends on the length of the system's Word type, thus in MSDOS it is 16 bits long, whereas in 32 bit systems (like Windows 9x/2000/NT and systems that work under protected mode in x86 systems) it is 32 bits long (4 bytes). 	See short, long
  float 	4 	floating point number. 	3.4e + / - 38 (7 digits)
  double 	8 	double precision floating point number. 	1.7e + / - 308 (15 digits)
  long double 	10 	long double precision floating point number. 	1.2e + / - 4932 (19 digits)
  bool 	1 	Boolean value. It can take one of two values: true or false NOTE: this is a type recently added by the ANSI-C++ standard. Not all compilers support it. Consult section bool type for compatibility information. 	true or false
  wchar_t 	2 	Wide character. It is designed as a type to store international characters of a two-byte character set. NOTE: this is a type recently added by the ANSI-C++ standard. Not all compilers support it. 	wide characters


Some links:
----------

  jsni
    home: http://soubok.googlepages.com/jsni

  spidermonkey
    home:
      http://www.mozilla.org/js/spidermonkey/
    developer mozilla:
      http://developer.mozilla.org/en/docs/JSAPI_Reference
    updated API reference:
      http://www.sterlingbates.com/jsref/sparse-frameset.html
    search in files:
      http://lxr.mozilla.org/mozilla/

  libffi
    home:
      http://sources.redhat.com/libffi/
    libffi_msvc:
      http://sourceforge.net/projects/ctypes/ ( old )
			http://svn.python.org/view/ctypes/trunk/ctypes/source/libffi_msvc/ ( new )
				checkout: svn checkout http://svn.python.org/projects/ctypes/trunk/ctypes/source/libffi_msvc
    ffi for php5:
      http://pecl.php.net/package-info.php?package=ffi
    other x86 port:
      http://pnet.nu6.org/scvs/module_libffi_src_x86.html


Spidermonkey API reminder:
-------------------------

  JL_GetStringBytes translates a specified JS string, str, into a C character array. If successful, JL_GetStringBytes returns a pointer to the array. The array is automatically freed when str is finalized by the JavaScript garbage collection mechanism.
  JS_GetStringChars provides a Unicode-enabled pointer to a JS string, str.



My TODO list:
------------

  - add offset capability to data ( or extend .Data() to support an offset in the array )

  - allow to test if a pointer is null without using casting

  - add a method to read not zero-ended strings  ( like .Data(100) )

  - check and apply GT tips ( http://www.mozilla.org/js/spidermonkey/gctips.html )

  - port all of this on linux
    => ok, no problem

  - manages natives structures
    => really needed ? libffi seems to not implement the access to a native structure data

  - var nd = new NativeData( [ [ "test" ], [""] ] );
    => the same result can be done in javascript

  - NativeModule.onUse
    => allow the dll to be load later ( lazy ... )

  - create a fast mode, where no checks are done ( datatype, ... )

My notes:
--------

   From the admittedly unclear API doc at
http://www.mozilla.org/js/spid ermonkey/apidoc/gen/api-JS_Add NamedRoot...
"|rp| is a pointer to a pointer to a JS double, string, or object."  You
have to pass the address of the pointer (or jsval) that should be added
to the GC's root set.  You can't pass the pointer (or jsval) itself.
 Obviously, that pointer has to remain valid as long as the root is in
the set (until you call JS_RemoveRoot).

/be

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/


void *_loadedLibraries = NULL; // TODO: explain why we must do this

void *_libObject = NULL; // TODO: explain why we must do this


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// tools

// very simple stack functions
void StackPush( void **stack, void *ptr ) {

  void **newItem = (void**)jl_malloc( sizeof( void* ) * 2 ); // create a new item for the list ( pointer, pointer )
  newItem[0] = *stack; // chain the list
  newItem[1] = ptr; // store the address of the new allocated memory
  *stack = newItem; // store the new start of the list in *list
}

void* StackPop( void **stack ) {

  void **item, *ptr;
  item = (void**)*stack;
  ptr = item[1];
  *stack = item[0];
  jl_free( item );
  return ptr;
}

void StackRemove( void **stack, void *ptr ) {

  while ( *stack != NULL ) {

    if ( (*(void***)stack)[1] == ptr ) {

      void* item = *stack;
      *stack = **(void***)stack;
      jl_free( item );
      return;
    }
    stack = (void**)*stack;
  }
}

bool StackHas( void **stack, void *ptr ) {

  while ( *stack != NULL ) {

    if ( (*(void***)stack)[1] == ptr )
      return true;
    stack = (void**)*stack;
  }
  return false;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NativeData_Finalize(JSContext *cx, JSObject *obj);

JSClass NativeData = {
  "NativeData", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1), // slot[0]: ref. to the root object
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NativeData_Finalize
};

////

JSBool NativeType_Construct(JSContext *cx, uintN argc, jsval *vp) {

  if ( !JS_IsConstructing(cx, vp) )
	  return JS_FALSE;

  return JS_TRUE;
}

// .Alloc( sizeToAlloc, initialValue )
JSBool NativeType_Alloc(JSContext *cx, uintN argc, jsval *vp) {

	JSObject *obj = JS_THIS_OBJECT(cx, vp);
  ffi_type *type = (ffi_type*) JL_GetPrivate( cx, obj );

jsval val;
JL_GetReservedSlot(cx, obj, 1, &val); // ..., JSVAL_TO_OBJECT(val)

void ** pptr = (void**)JL_GetPrivate( cx, JSVAL_TO_OBJECT(val) );

//  void ** x_pptr = (void**)JL_GetPrivate( cx, JSVAL_TO_OBJECT(JS_ARGV(cx, vp)[-1]) );


  size_t size = type->size * ( argc >= 1 ? JSVAL_TO_INT( JS_ARGV(cx, vp)[0] ) : 1 ); // JS_ARGV(cx, vp)[0]: number of elements in the array

  *pptr = jl_malloc(size);

  jsval rootObj;
  JL_GetReservedSlot( cx, JSVAL_TO_OBJECT(val), 0, &rootObj ); // get the slot[0] of the slot[1] object (NativeData)
  void** ppList = &((void**)JL_GetPrivate( cx, JSVAL_TO_OBJECT( rootObj ) ))[1]; // get the pointer to the list

  StackPush( ppList, *pptr );

  if ( argc >= 2 )
    memset( *pptr, JSVAL_TO_INT( JS_ARGV(cx, vp)[1] ), size ); // initialize the memory // INFO: calloc Allocate storage for array, initializing every byte in allocated block to 0

  JS_RVAL(cx, vp) = OBJECT_TO_JSVAL(obj); // allows .Alloc(100)[99] = 'd';
  return JS_TRUE;
}


JSBool NativeType_Free(JSContext *cx, uintN argc, jsval *vp) {

	JSObject *obj = JS_THIS_OBJECT(cx, vp);

jsval val;
JL_GetReservedSlot(cx, obj, 1, &val); // ..., JSVAL_TO_OBJECT(val)

  jsval rootObj;
  JL_GetReservedSlot( cx, JSVAL_TO_OBJECT(val), 0, &rootObj ); // get the slot [0] of the slot[1] object (NativeData)
  void** ppList = &((void**)JL_GetPrivate( cx, JSVAL_TO_OBJECT( rootObj ) ))[1]; // get the pointer to the list
  void *ptr = *(void**)JL_GetPrivate( cx, JSVAL_TO_OBJECT(val) );
  StackRemove( ppList, ptr ); // remove the ptr from the stack to avoid it to be freed on Finalize
  jl_free(ptr);
  return JS_TRUE;
}


/*
// .Init( data1, data2, ... ) || .Vector( "this is a string\x00" ) || .Vector( offset, JSArray ) // all of this can be done with [ xxx, xxx, ... ]
JSBool NativeType_Init(JSContext *cx, uintN argc, jsval *vp) {

  ffi_type *type = (ffi_type*) JL_GetPrivate( cx, obj );
  void ** pptr = (void**)JL_GetPrivate( cx, JS_GetParent( cx, obj ) ); CHANGE GetParent INTO slot[1]

  for ( int i = 0; i < argc; i++ ) {
  }


  JL_GetStringBytes

  return JS_TRUE;
}
*/

JSFunctionSpec NativeType_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "Alloc" , NativeType_Alloc , 1, 0 },
 { "Free"  , NativeType_Free  , 0, 0 },
// { "Init", NativeType_Init, 0, 0 },
 { 0 }
};

// eg. var val = arg.PU32[0];
JSBool NativeType_getter(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {

  if ( JSID_IS_INT( id ) ) {

    int index = JSID_TO_INT( id );
    ffi_type *ffiType = (ffi_type*) JL_GetPrivate( cx, obj );


jsval val;
JL_GetReservedSlot(cx, obj, 1, &val); // ..., JSVAL_TO_OBJECT(val)


    JSObject *nativeDataObj = JSVAL_TO_OBJECT(val);
    void **pptr = (void **)JL_GetPrivate( cx, nativeDataObj );

    switch ( ffiType->type ) {

    case FFI_TYPE_INT:
      //JS_NewNumberValue
		 JL_NativeToJsval( cx, (jsdouble)((signed int*)*pptr)[index], vp ); // JS_NewNumberValue checks INT_FITS_IN_JSVAL
      break;

    case FFI_TYPE_SINT8: // char => javascript string of 1 char
      *vp = INT_TO_JSVAL( ((signed char*)*pptr)[index] ); // or  *vp = STRING_TO_JSVAL( JS_NewStringCopyN( cx, ((char*)*pptr)+index, 1 ) );
      break;

    case FFI_TYPE_UINT8: // char => javascript int
      *vp = INT_TO_JSVAL( ((unsigned char*)*pptr)[index] );
      break;

    case FFI_TYPE_UINT16:
      *vp = INT_TO_JSVAL( ((unsigned short*)*pptr)[index] );
      break;

    case FFI_TYPE_SINT16:
      *vp = INT_TO_JSVAL( ((signed short*)*pptr)[index] );
      break;

    case FFI_TYPE_UINT32:
      //JS_NewNumberValue
		 JL_NativeToJsval( cx, (jsdouble)((unsigned long*)*pptr)[index], vp );
      break;

    case FFI_TYPE_SINT32:
			//JS_NewNumberValue
		 JL_NativeToJsval( cx, (jsdouble)((signed long*)*pptr)[index], vp );
      break;

    case FFI_TYPE_SINT64:
      {
        char val[21]; // 19digits + minus sign + \0
        _i64toa( ((signed __int64*)*pptr)[index], val, 10 );
        *vp = STRING_TO_JSVAL( JS_NewStringCopyZ( cx, val ) );
      }
      break;

    case FFI_TYPE_UINT64:
      {
        char val[20]; // 19digits + \0
        _ui64toa( ((unsigned __int64*)*pptr)[index], val, 10 );
        *vp = STRING_TO_JSVAL( JS_NewStringCopyZ( cx, val ) );
      }
      break;

    case FFI_TYPE_POINTER:
      {
      JSObject* dataObj = JS_NewObject( cx, &NativeData, NULL, NULL ); // no reference to the parent? slot[1]? is needed here

      jsval rootObj;
      JL_GetReservedSlot( cx, nativeDataObj, 0, &rootObj );

		// (TBD) check the following line. I transformed it from: JS_SetReservedSlot( cx, dataObj, 0, OBJECT_TO_JSVAL( rootObj ) );
		JS_SetReservedSlot( cx, dataObj, 0, rootObj ); // a reference of the root object is stored in the slot[0]

      JL_SetPrivate( cx, dataObj, pptr[index] ); // do not free because JS_NewObject do not call the constructor !!
      *vp = OBJECT_TO_JSVAL( dataObj );
      }
      break;

    default:

      JS_ReportError( cx, "NativeType_getter: no conversion found ( index: %d, ffiType: %d )", index, ffiType->type );
      return JS_FALSE;
    }
  }
  return JS_TRUE;
}

// eg. arg.PU32[0] = 12345;
JSBool NativeType_setter(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {

  // TODO: manage null & undefined

  if ( JSID_IS_INT( id ) ) {

    // remove the property from the object ( JSPROP_SHARED ? ) JS_SetProperty

    int index = JSID_TO_INT( id );
    ffi_type *ffiType = (ffi_type*) JL_GetPrivate( cx, obj );

jsval val;
JL_GetReservedSlot(cx, obj, 1, &val); // ..., JSVAL_TO_OBJECT(val)

    void ** pptr = (void**)JL_GetPrivate( cx, JSVAL_TO_OBJECT(val) );

    switch ( ffiType->type ) {
      // note: JSVAL_IS_INT is faster than any conversion like JS_ValueTo...

    case FFI_TYPE_INT: // PI[0] = system dependant
      {
        signed int *pVal = &((signed int*)*pptr)[index];
        if ( JSVAL_IS_INT( *vp ) )
          *pVal = JSVAL_TO_INT( *vp );
        else {
          int32 val;
          JS_ValueToInt32( cx, *vp, &val ); // TODO: enhance this conversion
          *pVal = (signed int)val;
        }
      }
      break;

    case FFI_TYPE_SINT8: // PS8[0] = String( char[0] ) or Number( -128 to 127 )
      {
        signed char *pVal = &((signed char *)*pptr)[index];
        if ( JSVAL_IS_INT( *vp ) )
          *pVal = (signed char)JSVAL_TO_INT( *vp );
        else
			  if ( JSVAL_IS_STRING( *vp ) ) {

				  size_t length;
				  const jschar *s = JS_GetStringCharsAndLength(JSVAL_TO_STRING(*vp), &length);
				  if ( length < 1 )
					  return JS_ReportError( cx, "this conversion is not implemented yet !" ), JS_FALSE;
            *pVal = (char)s[0];
			  } else {
            if ( JSVAL_IS_DOUBLE( *vp ) ) {
              int32 val;
              JS_ValueToInt32( cx, *vp, &val ); // TODO: enhance this conversion
              *pVal = (signed char)val;
            }
			  }

      }
      break;

    case FFI_TYPE_UINT8: // PU8[0] = Number( 0 to 255 )
      {
        unsigned char *pVal = &((unsigned char*)*pptr)[index];
        if ( JSVAL_IS_INT( *vp ) )
          *pVal = (unsigned char)JSVAL_TO_INT( *vp );
        else {
          uint16 val;
          JS_ValueToUint16( cx, *vp, &val ); // TODO: enhance this conversion
          *pVal = (unsigned char)val;
        }
      }
      break;

    case FFI_TYPE_SINT16: // PS16[0] = Number( -32768 to 32767 )
      {
        signed short *pVal = &((signed short*)*pptr)[index];
        if ( JSVAL_IS_INT( *vp ) )
          *pVal = (signed short)JSVAL_TO_INT( *vp );
        else {
          int32 val;
          JS_ValueToInt32( cx, *vp, &val ); // TODO: enhance this conversion
          *pVal = (signed short)val;
        }
      }
      break;

    case FFI_TYPE_UINT16: // PU16[0] = Number( 0 to 65535 )
      {
        unsigned short *pVal = &((unsigned short*)*pptr)[index];
        if ( JSVAL_IS_INT( *vp ) )
          *pVal = (unsigned short)JSVAL_TO_INT( *vp );
        else
          JS_ValueToUint16( cx, *vp, pVal );
      }
      break;

    case FFI_TYPE_SINT32: // PS32[0] = Number( -2147483648 to 2147483647 )
      {
        int32 *pVal = &((int32*)*pptr)[index];
        if ( JSVAL_IS_INT( *vp ) )
          *pVal = JSVAL_TO_INT( *vp );
        else
          JS_ValueToInt32( cx, *vp, pVal );
      }
      break;

    case FFI_TYPE_UINT32: // PU32[0] = Number( 0 to 4294967295 )
      {
        uint32 *pVal = &((uint32*)*pptr)[index];
        if ( JSVAL_IS_INT( *vp ) )
          *pVal = JSVAL_TO_INT( *vp );
        else
          JS_ValueToECMAUint32( cx, *vp, pVal );
      }
      break;

    case FFI_TYPE_SINT64:
      {
        signed __int64 *pVal = &((signed __int64*)*pptr)[index];
        if ( JSVAL_IS_INT( *vp ) )
          *pVal = JSVAL_TO_INT( *vp );
        else {
          // __int64 _atoi64( char*
          JS_ReportError( cx, "this conversion is not implemented yet !" );
          return JS_FALSE;
        }
      }
      break;

    case FFI_TYPE_UINT64:   //  0 to 9223372036854775808-1  ( 19 digits )
      {
        unsigned __int64 *pVal = &((unsigned __int64*)*pptr)[index];
        if ( JSVAL_IS_INT( *vp ) )
          *pVal = JSVAL_TO_INT( *vp );
        else {
          JS_ReportError( cx, "this conversion is not implemented yet !" );
          return JS_FALSE;
        }
      }
      break;

    case FFI_TYPE_POINTER: // PU32[0] = nativeData ...
      {
        if ( JSVAL_IS_OBJECT( *vp ) && JS_InstanceOf( cx, JSVAL_TO_OBJECT( *vp ), &NativeData, NULL ) ) {

          ((void**)*pptr)[index] = *(void**)JL_GetPrivate( cx, JSVAL_TO_OBJECT( *vp ) );

			 // (TBD) check the following line. I transformed it from: JS_SetReservedSlot( cx, obj, 0, OBJECT_TO_JSVAL( *vp ) );
          JS_SetReservedSlot( cx, obj, 0, *vp ); // it is important to keep a reference to the NativeData *vp to avoid it to be finalised before this one
        } else {

          uint32 *pVal = &((uint32*)*pptr)[index];
          if ( JSVAL_IS_INT( *vp ) )
            *pVal = JSVAL_TO_INT( *vp );
          else
            JS_ValueToECMAUint32( cx, *vp, pVal );
        }
      }
      break;

    default:
      JS_ReportError( cx, "NativeType_setter: no conversion found ( index: %d, ffiType: %d, jsType: %s )", index, ffiType->type, JS_GetTypeName( cx, JS_TypeOfValue( cx, *vp ) )  );
      return JS_FALSE;
    }
  }
  return JS_TRUE;
}


JSBool NativeType_newResolve(JSContext *cx, JSObject *obj, jsval id, uintN flags, JSObject **objp) {

  if ( flags & JSRESOLVE_ASSIGNING && JSVAL_IS_INT( id ) ) {

    JS_DefineProperty( cx, obj, (char*)JSVAL_TO_INT( id ), JSVAL_VOID, NULL, NULL, JSPROP_SHARED | JSPROP_INDEX ); // JSPROP_INDEX: see jsapi.c:DefineProperty(...)
    *objp = obj;
  }
  return JS_TRUE;
}


JSClass NativeType = {
  "NativeType", JSCLASS_HAS_PRIVATE | JSCLASS_NEW_RESOLVE | JSCLASS_HAS_RESERVED_SLOTS(2), // slot[0] is used to store a ref to the NativeData in the case NativeType_setter:FFI_TYPE_POINTER, ...
  JS_PropertyStub, JS_PropertyStub, NativeType_getter, NativeType_setter,
  JS_EnumerateStub, (JSResolveOp)NativeType_newResolve, JS_ConvertStub, JS_FinalizeStub
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// When you call JS_InitClass, it creates a prototype object of your class,
// so that object will be finalized too.  It won't be constructed, though
// -- you have to call the constructor yourself, if you need to.
// http://groups.google.fr/group/netscape.public.mozilla.jseng/browse_thread/thread/404b546c34515543/22b875b4a2e41f59?q=Finalize+construct&rnum=2&hl=fr#22b875b4a2e41f59
void NativeData_Finalize(JSContext *cx, JSObject *obj) {

  // JL_GetPrivate( cx, obj ) != NULL ) { // this appends only once, when the prototype (created on JS_InitClass) is destroyed, so it's not usefull here

  jsval rootObj;
  JL_GetReservedSlot( cx, obj, 0, &rootObj );

  if ( !JSVAL_IS_PRIMITIVE(rootObj) && JSVAL_TO_OBJECT(rootObj) == obj ) { // the root object is being finalized

    void** pv = (void**)JL_GetPrivate( cx, obj ); // get the pointer to the list
    void** ppList = pv+1; // see NativeData_Construct

    // free all the memory allocated by the underlying objects
    while ( *ppList )
      jl_free( StackPop( ppList ) );
    jl_free( pv );
  }
}


JSBool NativeData_Construct(JSContext *cx, uintN argc, jsval *vp) {

  // this is the root of the NativeData/NativeType chain; the only one that is Constructed
  if ( !JS_IsConstructing(cx, vp) ) // JS_IsConstructing(cx): var a = new Toto(123); !JS_IsConstructing(cx): var a = Toto(123);
	  return JS_FALSE;

	JSObject *obj = JS_THIS_OBJECT(cx, vp);

  JS_SetReservedSlot( cx, obj, 0, OBJECT_TO_JSVAL(obj) ); // the slot[0] points to this root object. We use this because all the memory allocations are stored in this object ( (*JL_GetPrivate)[1] )

  void** ppPrivate = (void**)jl_malloc( sizeof(void*) * 2 ); // freed in NativeData_Finalize
  ppPrivate[0] = NULL; // pointer to the start of the native data structure
  ppPrivate[1] = NULL; // chained-list of pointer to free
  JL_SetPrivate( cx, obj, ppPrivate );
  return JS_TRUE;
}


JSBool NativeData_getter_Type(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {

  static ffi_type* ffiTypeList[] = {
    &ffi_type_void, // 0
    &ffi_type_sint,
    &ffi_type_float,
    &ffi_type_double,
    &ffi_type_longdouble,
    &ffi_type_uint8, // 5
    &ffi_type_sint8,
    &ffi_type_uint16,
    &ffi_type_sint16,
    &ffi_type_uint32,
    &ffi_type_sint32,
    &ffi_type_uint64,
    &ffi_type_sint64,
    NULL,
    &ffi_type_pointer // 14
  };

  JSObject* accessObj = JS_ConstructObject( cx, &NativeType, NULL, NULL );

  JS_SetReservedSlot(cx, accessObj, 1, OBJECT_TO_JSVAL( obj ));

  JL_SetPrivate( cx, accessObj, ffiTypeList[ JSID_TO_INT( id ) ] );
  *vp = OBJECT_TO_JSVAL( accessObj );
  return JS_TRUE;
}

// usage: var str = arg.String;
// the string hold by arg MUST be null terminated, else use arg.Data(len)
JSBool NativeData_getter_String(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {

  void** pptr = (void**)JL_GetPrivate( cx, obj );

  if ( *pptr == NULL ) {

    JS_ReportError( cx, "Uninitialized data" );
    return JS_FALSE;
  }

  char* str = *((char**)*pptr);
  *vp = STRING_TO_JSVAL( JS_NewStringCopyZ( cx, str ) ); // the string MUST be ended with \0 ( it is ok, see NativeData_setter_String() )
  return JS_TRUE;
}



// usage: arg.String = 'qwerty';
// the string CAN contain null char inside.
// One more char is allocated for a null terminator BUT the interpretation of this char is NOT amndatory, this ending null char can keep unused.
JSBool NativeData_setter_String(JSContext *cx, JSObject *obj, jsid id, jsval *vp) { // note: *vp is converted into a js string

  if ( JS_TypeOfValue( cx, *vp ) != JSTYPE_STRING )
    *vp = STRING_TO_JSVAL( JS_ValueToString( cx, *vp ) ); // convert any vp type to JS string

//  size_t len = JS_GetStringLength( JSVAL_TO_STRING( *vp ) );
//  const char* str = JL_GetStringBytesZ( cx, JSVAL_TO_STRING( *vp ) ); // this string is terminated with \0 or not? ( only thrust len )
//  if ( str == NULL )
//	  return JS_FALSE;

	JLStr str;
	if ( !JL_JsvalToNative(cx, *vp, str) )
		return JS_FALSE;

  void** pptr = (void**)JL_GetPrivate( cx, obj );
  char** sptr = (char**)jl_malloc( sizeof(char*) );
//  *sptr = (char*)jl_malloc( len+1 ); // len+1 because we will add a '\0' at the end ( don't thrust JL_GetStringBytes about '\0' )
//  memcpy( *sptr, str, len );
//  *((*sptr)+len) = '\0';
  *sptr = str.GetStrZOwnership();
  *pptr = sptr;

  jsval rootObj;
  JL_GetReservedSlot( cx, obj, 0, &rootObj ); // get the slot [0] of the slot[1] object (NativeData)
  void** ppList = &((void**)JL_GetPrivate( cx, JSVAL_TO_OBJECT( rootObj ) ))[1]; // get the pointer to the list

  StackPush( ppList, sptr );
  StackPush( ppList, *sptr );

  return JS_TRUE;
}

// usage: var str = arg.Data(5);
// allows to create a javascript string with binary data ( not based on ending '\0' character )
JSBool NativeData_Data(JSContext *cx, uintN argc, jsval *vp) {

  int32 len;
  if ( JS_TypeOfValue( cx, JS_ARGV(cx, vp)[0] ) != JSTYPE_NUMBER )
    JS_ValueToInt32( cx, JS_ARGV(cx, vp)[0], &len );
  else
    len = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);

  if ( len < 0 ) {

    JS_ReportError( cx, "Invalid amount of data, must be >= 0" );
    return JS_FALSE;
  }

	JSObject *obj = JS_THIS_OBJECT(cx, vp);

  void** pptr = (void**)JL_GetPrivate( cx, obj );

  if ( *pptr == NULL ) {

    JS_ReportError( cx, "Uninitialized data" );
    return JS_FALSE;
  }

  char* data = *((char**)*pptr);

  JS_RVAL(cx, vp) = STRING_TO_JSVAL( JS_NewStringCopyN( cx, data, len ) );
  return JS_TRUE;
}


JSPropertySpec NativeData_PropertySpec[] = { // *name, tinyid, flags, getter, setter
  { "VOID",  0, JSPROP_SHARED|JSPROP_READONLY|JSPROP_PERMANENT, NativeData_getter_Type, NULL },
  { "PI"  ,  1, JSPROP_SHARED|JSPROP_READONLY|JSPROP_PERMANENT, NativeData_getter_Type, NULL },
  { "PU8" ,  5, JSPROP_SHARED|JSPROP_READONLY|JSPROP_PERMANENT, NativeData_getter_Type, NULL },
  { "PS8" ,  6, JSPROP_SHARED|JSPROP_READONLY|JSPROP_PERMANENT, NativeData_getter_Type, NULL },
  { "PU16",  7, JSPROP_SHARED|JSPROP_READONLY|JSPROP_PERMANENT, NativeData_getter_Type, NULL },
  { "PS16",  8, JSPROP_SHARED|JSPROP_READONLY|JSPROP_PERMANENT, NativeData_getter_Type, NULL },
  { "PU32",  9, JSPROP_SHARED|JSPROP_READONLY|JSPROP_PERMANENT, NativeData_getter_Type, NULL },
  { "PS32", 10, JSPROP_SHARED|JSPROP_READONLY|JSPROP_PERMANENT, NativeData_getter_Type, NULL },
  { "PU64", 11, JSPROP_SHARED|JSPROP_READONLY|JSPROP_PERMANENT, NativeData_getter_Type, NULL },
  { "PS64", 12, JSPROP_SHARED|JSPROP_READONLY|JSPROP_PERMANENT, NativeData_getter_Type, NULL },
  { "PP"  , 14, JSPROP_SHARED|JSPROP_READONLY|JSPROP_PERMANENT, NativeData_getter_Type, NULL },
  { "String", -1, JSPROP_SHARED|JSPROP_PERMANENT, NativeData_getter_String, NativeData_setter_String },
  { 0 }
};


JSFunctionSpec NativeData_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "Data" , NativeData_Data , 1, 0 },
 { 0 }
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


JSBool NativeProc_Call(JSContext *cx, uintN argc, jsval *vp) {

  // !! avoid the GC to be called until the end of the call !! with 	JS_BeginRequest( cx ); ??

  // ASSERT( argc < 101 ) // manages a max of 100 arguments ! ( this avoid malloc/free of the arrays )
  ffi_type *ffiArgTypeList[101];
  void *ffiValueList[101];

  for ( unsigned int argIterator = 0; argIterator < argc; ++argIterator ) {

    jsval currentArg = JS_ARGV(cx, vp)[argIterator];
    if ( JSVAL_IS_PRIMITIVE(currentArg) || !JS_InstanceOf( cx, JSVAL_TO_OBJECT( currentArg ), &NativeType, NULL ) ) {

      JS_ReportError( cx, "argument %d must be a NativeType ( current type: %d )", argIterator+1, JS_TypeOfValue( cx, currentArg ) );
      return JS_FALSE;
    }

    ffiArgTypeList[argIterator] = (ffi_type*) JL_GetPrivate( cx, JSVAL_TO_OBJECT( currentArg ) );

jsval val;
JL_GetReservedSlot(cx, JSVAL_TO_OBJECT( currentArg ), 1, &val); // ..., JSVAL_TO_OBJECT(val)


    ffiValueList[argIterator] = *((void**)JL_GetPrivate( cx, JSVAL_TO_OBJECT(val) ) );
  }

  JSObject *thisObj = JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)); // get 'this' object of the current object ... TODO: check JS_InstanceOf( cx, thisObj, &NativeProc, NULL )

  jsval val;
  JL_GetReservedSlot(cx, thisObj, 1, &val );
  HMODULE module = (HMODULE)JL_GetPrivate( cx, JSVAL_TO_OBJECT( val ) ); // slot[1]: ClassModule

  jsval id;
  JL_GetReservedSlot( cx, thisObj, 0, &id ); // slot[0] = name of the proc. to call

  // Pointer to a null-terminated string that specifies the function or variable name, or the function's ordinal value.
  // If this parameter is an ordinal value, it must be in the low-order word; the high-order word must be zero.

//  const char *procName = JL_GetStringBytesZ(cx, JSVAL_TO_STRING(id));
//  if ( procName == NULL )
//	  return JS_FALSE;

  JLStr procName;
  if ( !JL_JsvalToNative(cx, id, procName) )
	  return JS_FALSE;


  FARPROC procAddress = ::GetProcAddress( module, JSVAL_IS_INT( id ) ? (LPCSTR) LOWORD( JSVAL_TO_INT( id ) ) : procName );

  if ( procAddress == NULL ) {

    DWORD err = ::GetLastError();
    JS_ReportError( cx, "GetProcAddress error, symbol not found ? module closed ? (error: 0x%X)", err );
    return JS_FALSE;
  }

  ffi_cif cif;
  if (ffi_prep_cif( &cif, FFI_STDCALL, argc-1, ffiArgTypeList[0], ffiArgTypeList+1 ) != FFI_OK ) { // +1 because [0] is the return type

    JS_ReportError( cx, "ffi_prep_cif error" );
    return JS_FALSE;
  }

  ffi_call( &cif, FFI_FN(procAddress), ffiValueList[0], ffiValueList+1 );

  return JS_TRUE;
}

/*

JSFunctionSpec NativeProc_FunctionSpec[] = {
 { "Call" , NativeProc_Call , 1, 0, 0 },
 { 0 }
};

*/


JSClass NativeProc = {
  "NativeProc", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(2),
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
  0,0, NativeProc_Call
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// argv[2] : close methode // false/undefined = release at the end, true = random released (GC-based)
// lazy... arg[3] : load style : immediate / lazy
JSBool NativeModule_Construct(JSContext *cx, uintN argc, jsval *vp) {

//    char* str = JL_GetStringBytes( JS_ValueToString ( cx, OBJECT_TO_JSVAL(obj) ) );

	if ( !JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]) )
		JS_ARGV(cx, vp)[0] = STRING_TO_JSVAL( JS_ValueToString(cx, JS_ARGV(cx, vp)[0]) );

//  const char *libName = JL_GetStringBytesZ( cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]) ); // JL_GetStringBytes never returns NULL
//  if ( libName == NULL || *libName == '\0' )
//    return JS_FALSE;

	JLStr libName;
	if ( !JL_JsvalToNative(cx, JS_ARGV(cx, vp)[0], libName) )
		return JS_FALSE;


  char libFileName[PATH_MAX];
  strcpy( libFileName, libName );
  strcat( libFileName, DLL_EXT );

// lazy mecanism...  JS_SetReservedSlot( cx, obj, 0, STRING_TO_JSVAL( JS_NewStringCopyZ( cx, libFileName ) ) ); // slot[0]

  HMODULE module = ::LoadLibrary(libFileName);

	if ( module == NULL ) {

		DWORD err = ::GetLastError();

    JS_ReportError( cx, "LoadLibrary error 0x%x while loading %s ", err, libFileName );
    return JS_FALSE;
  }

	JSObject *obj = JS_THIS_OBJECT(cx, vp);
  JL_SetPrivate( cx, obj, (void*)module );

  if ( argc <= 1 || JSVAL_TO_BOOLEAN( JS_ARGV(cx, vp)[1] ) != JS_TRUE ) { // if automatic mode

    JSObject **rt = (JSObject **)jl_malloc( sizeof(JSObject*) );
    *rt = obj;
    //JS_AddRoot( cx, rt );
	 JS_AddObjectRoot(cx, rt);
    StackPush( &_libObject, rt );
    StackPush( &_loadedLibraries, module );
  }

  return JS_TRUE;
}


void NativeModule_Finalize(JSContext *cx, JSObject *obj) {

  void *pv = JL_GetPrivate( cx, obj );
  if ( pv != NULL ) {

    if ( !StackHas( &_loadedLibraries, pv ) ) { // manual mode

      ::FreeLibrary( (HMODULE)pv );
    }

  }
}


JSBool NativeModule_Close(JSContext *cx, uintN argc, jsval *vp) {

	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	void *pv = JL_GetPrivate( cx, obj );
  if ( pv != NULL ) {

    if ( StackHas( &_loadedLibraries, pv ) ) {

      JS_ReportError( cx, "modules opened in safe mode cannot be closed manually ( not yet )" );
      return JS_FALSE;
    }


    jsval onRelease;
    JS_GetProperty( cx, obj, "onRelease", &onRelease );

    if ( !JSVAL_IS_VOID( onRelease ) ) {

		 // no GC protection is needed
      jsval tmp;
      JS_CallFunctionValue( cx, obj, onRelease, 0, NULL, &tmp );
    }

/* TODO:
  - StackRemove( &_loadedLibraries, pv ); // it's not a problem if pv don't exist in the stack
  - remove the rooted lib (obj) from _libObject
*/

    ::FreeLibrary( (HMODULE)pv );
    JL_SetPrivate( cx, obj, NULL ); // lib is freed
  }
  return JS_TRUE;
}



/*
JSBool NativeModule_UnloadCallback(JSContext *cx, uintN argc, jsval *vp) {

  return JS_TRUE;
}
*/


JSBool NativeModule_Proc(JSContext *cx, uintN argc, jsval *vp) {

	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	JSObject * fo = JS_NewObject( cx, &NativeProc, NULL, NULL );
	JS_SetReservedSlot( cx, fo, 0, JS_ARGV(cx, vp)[0] );
	JS_SetReservedSlot( cx, fo, 1, OBJECT_TO_JSVAL(obj) ); // OBJECT_TO_JSVAL(obj) == JS_ARGV(cx, vp)[-1] ???
	JS_RVAL(cx, vp) = OBJECT_TO_JSVAL( fo );
	return JS_TRUE;
}



JSFunctionSpec NativeModule_FunctionSpec[] = {
 { "Proc",  NativeModule_Proc, 1, 0 },
 { "Close",  NativeModule_Close, 0, 0 },
// { "UnloadCallback",  NativeModule_UnloadCallback, 1, 0, 0 },
 { 0 }
};


JSClass NativeModule = {
  "NativeModule", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub /*NativeModule_Getter*/, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, /*JS_FinalizeStub*/ NativeModule_Finalize
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

JSBool NativeTools_JSContextToPtr(JSContext *cx, uintN argc, jsval *vp) {

  //JS_NewNumberValue
	JL_NativeToJsval( cx, (jsdouble)(unsigned long)cx, &JS_RVAL(cx, vp) );
  return JS_TRUE;
}

JSBool NativeTools_JSObjectToPtr(JSContext *cx, uintN argc, jsval *vp) {

  JSObject *object = JSVAL_TO_OBJECT(JS_ARGV(cx, vp)[0]);
  //JS_NewNumberValue
  JL_NativeToJsval( cx, (jsdouble)(unsigned long)object, &JS_RVAL(cx, vp) );
  return JS_TRUE;
}

/*
JSBool NativeTools_PtrToJsval(JSContext *cx, uintN argc, jsval *vp) {

  unsigned long val;
  JS_ValueToECMAUint32( cx, JS_ARGV(cx, vp)[0], &val );
  JS_RVAL(cx, vp) = (jsval)(void*)val;
  return JS_TRUE;
}
*/


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Init_JSNI( JSContext *cx, JSObject *obj ) {

  JS_InitClass( cx, obj, NULL, &NativeData, NativeData_Construct, 0, NativeData_PropertySpec, NativeData_FunctionSpec, NULL, NULL );
  JS_InitClass( cx, obj, NULL, &NativeType, NativeType_Construct, 0, NULL, NativeType_FunctionSpec, NULL, NULL );
  JS_InitClass( cx, obj, NULL, &NativeModule, NativeModule_Construct, 0, NULL, NativeModule_FunctionSpec, NULL, NULL );

  JS_DefineFunction( cx, obj, "JSContextToPtr", NativeTools_JSContextToPtr, 0, 0 );
  JS_DefineFunction( cx, obj, "JSObjectToPtr", NativeTools_JSObjectToPtr, 1, 0 );
//  JS_DefineFunction( cx, obj, "PtrToJsval", NativeTools_PtrToJsval, 1, 0 );
}


void Release_JSNI( JSContext *cx ) {

  while ( _libObject ) {

    JSObject **rt = (JSObject **)StackPop( &_libObject );

    JSObject *obj = *rt;

    jsval onRelease;
    JS_GetProperty( cx, obj, "onRelease", &onRelease );
    if ( !JSVAL_IS_VOID( onRelease ) ) {

		 // no GC protection is needed
      jsval tmp;
      JS_CallFunctionValue( cx, obj, onRelease, 0, NULL, &tmp );
    }

    //JS_RemoveRoot( cx, rt );
	 JS_RemoveObjectRoot(cx, rt);
    jl_free( rt );
  }
}

void Destroy_JSNI() {

  // close the libraries that have not be closed manualy by .Close() or automaticaly by Finalize
  while ( _loadedLibraries )
    ::FreeLibrary( (HMODULE)StackPop( &_loadedLibraries ) );
}
