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
#include <jsvalserializer.h>

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3321 $
**/
BEGIN_CLASS( Serializer )

DEFINE_FINALIZE() {

	if ( jl::HostRuntime::getJLRuntime( fop->runtime() ).skipCleanup() )
		return;

	jl::Serializer *ser;
	ser = static_cast<jl::Serializer*>(js::GetObjectPrivate(obj));
	if ( !ser )
		return;
	delete ser;
}

DEFINE_CONSTRUCTOR() {

	jl::Serializer *ser = NULL;

	JL_DEFINE_ARGS;
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC(0);

	ser = new jl::Serializer(cx, JL_OBJVAL);
	JL_ASSERT_ALLOC(ser);

	JL_updateMallocCounter(cx, sizeof(jl::Serializer));

	ser->Write(cx, JL_THIS_CLASS_REVISION);

	JL_SetPrivate(JL_OBJ, ser);
	return true;

bad:
	if ( ser )
		delete ser;
	return false;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( value )
**/
DEFINE_FUNCTION( write ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

	JL_RVAL.setObject(*JL_OBJ);
	jl::Serializer *ser;
	ser = static_cast<jl::Serializer*>(JL_GetPrivate(JL_OBJ));
	JL_ASSERT_THIS_OBJECT_STATE(ser);
	JL_CHKM( ser->Write(cx, JL_ARG(1)), E_MODULE, E_INTERNAL ); // "Serializer write error."
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
**/
DEFINE_FUNCTION( done ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(0);

	jl::Serializer* ser;
	ser = (jl::Serializer*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(ser);

	void *data;
	size_t length;
	JL_CHKM( ser->GetBufferOwnership(&data, &length), E_MODULE, E_INTERNAL ); // "Serializer buffer error."
	
	// invalidate the object
	delete ser;
	JL_SetPrivate(JL_OBJ, NULL);
	//JL_updateMallocCounter(cx, length);
	//JL_CHK( JL_NewBufferGetOwnership(cx, data, length, JL_RVAL) );
	JL_CHK( BlobCreate(cx, data, length, JL_RVAL) );
	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3321 $"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC(write, 1)
		FUNCTION_ARGC(done, 0)
	END_FUNCTION_SPEC

END_CLASS





/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3321 $
**/
BEGIN_CLASS( Unserializer )

DEFINE_FINALIZE() {

	if ( jl::HostRuntime::getJLRuntime( fop->runtime() ).skipCleanup() )
		return;

	jl::Unserializer *unser;
	unser = static_cast<jl::Unserializer*>(js::GetObjectPrivate(obj));
	if ( !unser )
		return;
	delete unser;
}

DEFINE_CONSTRUCTOR() {

	jl::Unserializer *unser = NULL;
	jl::BufString str;

	JL_DEFINE_ARGS;
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_STRING(1);

	JL_CHK( jl::getValue(cx, JL_ARG(1), &str) );
	unser = new jl::Unserializer(cx, str.toData<char*>(), str.length(), JL_OBJVAL);
	JL_ASSERT_ALLOC(unser);

	JL_updateMallocCounter(cx, sizeof(jl::Unserializer));

	jl::SourceId_t srcId;
	JL_CHK( unser->Read(cx, srcId) );
	JL_ASSERT( srcId == JL_THIS_CLASS_REVISION, E_ARG, E_NUM(1), E_VERSION, E_COMMENT("serialized data") );

	JL_SetPrivate(JL_OBJ, unser);
	return true;
bad:
	if ( unser )
		delete unser;
	return false;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( value )
**/
DEFINE_FUNCTION( read ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(0);

	jl::Unserializer *unser;
	unser = static_cast<jl::Unserializer*>(JL_GetPrivate(JL_OBJ));
	JL_ASSERT_THIS_OBJECT_STATE(unser);
	JL_CHKM( unser->Read(cx, JL_RVAL), E_MODULE, E_INTERNAL ); // "Unserializer read error."
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( eof ) {
	
	JL_IGNORE( id );

	JL_DEFINE_PROP_ARGS;	

	JL_ASSERT_THIS_INSTANCE();

	jl::Unserializer *unser;
	unser = static_cast<jl::Unserializer*>(JL_GetPrivate(JL_OBJ));
	JL_ASSERT_THIS_OBJECT_STATE(unser);
	vp.setBoolean(!unser->IsEmpty());
	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3321 $"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC(read, 0)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER(eof)
	END_PROPERTY_SPEC

END_CLASS

