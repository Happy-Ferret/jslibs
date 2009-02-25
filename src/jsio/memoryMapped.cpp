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

#include "file.h"

#include <prio.h>

#define MEMORYMAPPED_SLOT_FILE 0

struct MemoryMappedPrivate {
	PRInt32 size;
	PRFileMap *fmap;
	void *addr;
};

static JSBool BufferGet( JSContext *cx, JSObject *obj, const char **buf, size_t *size ) {

	MemoryMappedPrivate *pv = (MemoryMappedPrivate*)JS_GetPrivate(cx, obj);
	*size = pv->size;
	*buf = (const char*)pv->addr;
	return JS_TRUE;
}

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( MemoryMapped )

DEFINE_FINALIZE() {

	MemoryMappedPrivate *pv = (MemoryMappedPrivate*)JS_GetPrivate(cx, obj);
	if ( pv != NULL ) {

		PR_MemUnmap(pv->addr, pv->size);
		PR_CloseFileMap(pv->fmap);
		free(pv);
	}
}

/**doc
 * $INAME( file )
  Creates a new memory-mapped object using the given opened file descriptor.
  $H arguments
   $ARG File file: any opened file descriptor.
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_OBJECT( J_ARG(1) );

	JSObject *fdObj;
	fdObj = JSVAL_TO_OBJECT( J_ARG(1) );
	J_S_ASSERT_CLASS( fdObj, classFile );
	J_CHK( JS_SetReservedSlot(cx, obj, MEMORYMAPPED_SLOT_FILE, J_ARG(1)) ); // avoid the file to be GCed while being used by MemoryMapped

	PRFileDesc *fd;
	fd = (PRFileDesc*)JS_GetPrivate(cx, fdObj);
	J_S_ASSERT_RESOURCE( fd );

	MemoryMappedPrivate *pv;
	pv = (MemoryMappedPrivate*)malloc(sizeof(MemoryMappedPrivate));

	pv->size = PR_Available(fd);

	pv->fmap = PR_CreateFileMap(fd, pv->size, PR_PROT_READONLY);
	if ( pv->fmap == NULL )
		return ThrowIoError(cx);

/* 
	// Doc. The offset must be aligned to whole pages. !!!
	PROffset64 offset;
	if ( J_ARG_ISDEF(2) )
		J_CHK( JsvalToInt(cx, J_ARG(2), &offset) );
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
	J_CHK( JS_SetPrivate(cx, obj, pv) );

	J_CHK( SetBufferGetInterface(cx, obj, BufferGet) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
 * $INAME $READONLY
  is the file descriptor used to construct this object.
**/
DEFINE_PROPERTY( file ) {

	J_CHK( JS_GetReservedSlot(cx, obj, MEMORYMAPPED_SLOT_FILE, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Native Interface ===
 *NIBufferGet*
**/

CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1) // MEMORYMAPPED_SLOT_FILE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( file )
	END_PROPERTY_SPEC

END_CLASS

/**doc
=== Example ===
{{{
var f = new File('directory.cpp');
f.Open("r");
var m = new MemoryMapped(f);
Print(m);
}}}
**/
