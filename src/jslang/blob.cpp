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
#include "blobPub.h"

BEGIN_CLASS( Blob )

ALWAYS_INLINE bool
invalidateBlob(JSContext *cx, JS::HandleObject obj) {
	
	JL_SetPrivate(obj, NULL);
	return jl::setSlot(cx, obj, JL_BLOB_LENGTH, JL_UNDEFINED);
}


DEFINE_FINALIZE() { // see HandleClose()

	void *pv = js::GetObjectPrivate(obj);
	if ( pv && !jl::HostRuntime::getJLRuntime( fop->runtime() ).skipCleanup() )
		jl_free(pv);
}

DEFINE_PROPERTY_GETTER( length ) {

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INSTANCE();
	JL_CHK( jl::getSlot(cx, JL_OBJ, JL_BLOB_LENGTH, JL_RVAL) );

	if ( JL_RVAL.isUndefined() ) {

		JL_WARN( E_OBJ, E_STATE, E_INVALID );
	}

	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( arrayBuffer ) {

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INSTANCE();
	JL_CHK( jl::getSlot(cx, JL_OBJ, JL_BLOB_LENGTH, JL_RVAL) );
	if ( !JL_RVAL.isUndefined() ) {

		ASSERT( JL_RVAL.isInt32() );
		int32_t size = JL_RVAL.toInt32();

		if ( size > 0 ) {

			jl::BufBase( JL_GetPrivate( JL_OBJ ), size ).toArrayBuffer( cx, JL_RVAL );
		} else {

			JL_RVAL.setObjectOrNull( JS_NewArrayBuffer( cx, 0 ) );
			JL_ASSERT_ALLOC( JL_RVAL.isNull() );
		}

		JL_CHK( invalidateBlob(cx, JL_OBJ) );
		JL_CHK( JS_DefinePropertyById(cx, JL_OBJ, id, JL_RVAL, JSPROP_READONLY | JSPROP_PERMANENT) );
	} else {

		JL_WARN( E_OBJ, E_STATE, E_INVALID );
	}
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( string ) {

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INSTANCE();
	JL_CHK( jl::getSlot(cx, JL_OBJ, JL_BLOB_LENGTH, JL_RVAL) );
	if ( !JL_RVAL.isUndefined() ) {
		
		ASSERT( JL_RVAL.isInt32() );
		int32_t size = JL_RVAL.toInt32();
		if ( size > 0 ) {

			// wait for Bug 1045830 - restore JS_NewString function for latin1 strings
			//jl::BufString(static_cast<char*>(JL_GetPrivate(JL_OBJ)), size, false).toString(cx, JL_RVAL);
			JL_RVAL.setString( JL_NewString(cx, static_cast<char*>(JL_GetPrivate(JL_OBJ)), size) );
		} else {

			JL_RVAL.set(JL_GetEmptyStringValue(cx));
		}
		JL_CHK( invalidateBlob(cx, JL_OBJ) );
		JL_CHK( JS_DefinePropertyById(cx, JL_OBJ, id, JL_RVAL, JSPROP_READONLY | JSPROP_PERMANENT) );
	} else {

		JL_WARN( E_OBJ, E_STATE, E_INVALID );
	}
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( ucString ) {

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INSTANCE();
	JL_CHK( jl::getSlot(cx, JL_OBJ, JL_BLOB_LENGTH, JL_RVAL) );
	if ( !JL_RVAL.isUndefined() ) {
		
		ASSERT( JL_RVAL.isInt32() );
		int32_t size = JL_RVAL.toInt32();
		if ( size > 0 ) {

			JL_CHKM( size % 2 == 0, E_DATASIZE, E_INVALID );
			//jl::BufString(static_cast<jschar*>(JL_GetPrivate(JL_OBJ)), size / 2, false).toString(cx, JL_RVAL);
			JL_CHK( JS_NewUCString(cx, static_cast<jschar*>(JL_GetPrivate(JL_OBJ)), size / 2) );
		} else {

			JL_RVAL.set(JL_GetEmptyStringValue(cx));
		}
		JL_CHK( invalidateBlob(cx, JL_OBJ) );
		JL_CHK( JS_DefinePropertyById(cx, JL_OBJ, id, JL_RVAL, JSPROP_READONLY | JSPROP_PERMANENT) );
	} else {

		JL_WARN( E_OBJ, E_STATE, E_INVALID );
	}
	return true;
	JL_BAD;
}


DEFINE_FUNCTION( toString ) {

	JL_DEFINE_ARGS;
	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JLID(cx, string), JL_RVAL) );
	return true;
	JL_BAD;
}


DEFINE_FUNCTION( free ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_THIS_INSTANCE();
	void *pv = JL_GetPrivate(JL_OBJ);
	if ( pv != NULL ) {

		jl_free(pv);
		JL_CHK( invalidateBlob(cx, JL_OBJ) );
	} else {

		JL_WARN( E_OBJ, E_STATE, E_INVALID );
	}
	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3524 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)
	HAS_FINALIZE
	IS_UNCONSTRUCTIBLE

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( length )
		PROPERTY_GETTER( arrayBuffer )
		PROPERTY_GETTER( ucString )
		PROPERTY_GETTER( string )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION( toString )
		FUNCTION( free )
	END_FUNCTION_SPEC

END_CLASS
