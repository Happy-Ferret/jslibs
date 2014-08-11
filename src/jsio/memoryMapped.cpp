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


DECLARE_CLASS( File )


#define MEMORYMAPPED_SLOT_FILE 0

struct MemoryMappedPrivate {
	PRInt32 size;
	PRFileMap *fmap;
	size_t offset;
	void *addr;
};

bool MemoryMappedBufferGet( JSContext *cx, JS::HandleObject obj, jl::BufString *str ) {

	JL_IGNORE( cx );

	MemoryMappedPrivate *pv = (MemoryMappedPrivate*)JL_GetPrivate(obj);
	//*str = JLData(((const char*)pv->addr) + pv->offset, false, pv->size);
	str->get(((const char*)pv->addr) + pv->offset, pv->size, false);
	return true;
}

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( MemoryMapped )

DEFINE_FINALIZE() {

	MemoryMappedPrivate *pv = (MemoryMappedPrivate*)JL_GetPrivateFromFinalize(obj);
	if ( !pv )
		return;

	PR_MemUnmap(pv->addr, pv->size);
	PR_CloseFileMap(pv->fmap);
	JS_freeop(fop, pv);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( file )
  Creates a new memory-mapped object using the given opened file descriptor.
  $H arguments
   $ARG File file: any opened file descriptor.
**/
DEFINE_CONSTRUCTOR() {

	MemoryMappedPrivate *pv = NULL;

	JL_DEFINE_ARGS;
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_OBJECT(1);
	
	{
	JS::RootedObject fdObj(cx, &JL_ARG(1).toObject());
	JL_ASSERT_INSTANCE( fdObj, JL_CLASS(File) );
	JL_CHK( JL_SetReservedSlot(JL_OBJ, MEMORYMAPPED_SLOT_FILE, JL_ARG(1)) ); // avoid the file to be GCed while being used by MemoryMapped

	PRFileDesc *fd;
	fd = (PRFileDesc*)JL_GetPrivate(fdObj);
	JL_ASSERT_OBJECT_STATE( fd, JL_CLASS_NAME(File) );

	pv = (MemoryMappedPrivate*)JS_malloc(cx, sizeof(MemoryMappedPrivate));
	JL_CHK( pv );
	pv->addr = NULL;
	pv->fmap = NULL;

	pv->offset = 0;
	pv->size = PR_Available(fd);

	pv->fmap = PR_CreateFileMap(fd, pv->size, PR_PROT_READONLY);
	if ( pv->fmap == NULL )
		return ThrowIoError(cx);

/* 
	// Doc. The offset must be aligned to whole pages. !!!
	PROffset64 offset;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &offset) );
	else
		offset = 0;

	if ( offset < 0 ) {
		offset = pv->size - offset;
		pv->size -= offset;
	}
*/


	// Doc. Length of the section of the file to be mapped. The length must be a multiple of whole pages.
/*
	PRInt32 pageSize = PR_GetPageSize();
	pv->size = (pv->size / pageSize + 1) * pageSize;
*/

	pv->addr = PR_MemMap(pv->fmap, 0, pv->size);
	if ( pv->addr == NULL )
		return ThrowIoError(cx);

	JL_CHK( jl::setBufferGetInterface(cx, JL_OBJ, MemoryMappedBufferGet) );

	JL_SetPrivate(JL_OBJ, pv);

	}

	return true;

bad:
	if ( pv ) {

		if ( pv->addr )
			PR_MemUnmap(pv->addr, pv->size);
		if ( pv->fmap )
			PR_CloseFileMap(pv->fmap);
		JS_free(cx, pv);
	}
	return false;
}

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  is the file descriptor used to construct this object.
**/
DEFINE_PROPERTY_GETTER( file ) {

	JL_IGNORE(id, cx);

	JL_CHK( JL_GetReservedSlot( obj, MEMORYMAPPED_SLOT_FILE, vp) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  offset from wich the buffer starts.
**/
DEFINE_PROPERTY_SETTER( offset ) {

	JL_IGNORE( strict, id );

	MemoryMappedPrivate *pv = (MemoryMappedPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_CHK( jl::getValue(cx, vp, &pv->offset) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( offset ) {

	JL_DEFINE_PROP_ARGS;

	MemoryMappedPrivate *pv = (MemoryMappedPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_CHK( jl::setValue(cx, JL_RVAL, pv->offset) );
	return true;
	JL_BAD;
}


/**doc
=== Native Interface ===
 * *NIBufferGet*
  This object can be used as a buffer source.
  $H example
  {{{
  var stream = new Stream( new MemoryMapped(new File('directory.cpp').Open("r")) );
  print(stream.read(10));
  print(stream.read(10));
  print(stream.read(10));
  }}}
**/

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1) // MEMORYMAPPED_SLOT_FILE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( file )
		PROPERTY( offset )
	END_PROPERTY_SPEC

END_CLASS

/**doc
=== Example ===
{{{
var f = new File('directory.cpp');
f.open("r");
var m = new MemoryMapped(f);
print(m);
}}}
**/
