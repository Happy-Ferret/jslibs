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

#define SLOT_STREAM_SOURCE 0
#define SLOT_STREAM_POSITION 1

DECLARE_CLASS( Stream );


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( Stream )


ALWAYS_INLINE bool
GetStreamSource(JSContext *cx, JS::HandleObject obj, JS::MutableHandleObject srcObj) {

	JS::RootedValue val(cx);

	JL_CHK( JL_GetReservedSlot(obj, SLOT_STREAM_SOURCE, &val) );
	ASSERT( val.isObject() );
	srcObj.set( &val.toObject() );
	return true;
	JL_BAD;
}

ALWAYS_INLINE bool
SetStreamSource(JSContext *cx, JS::HandleObject obj, JS::HandleValue srcVal) {

	if ( srcVal.isPrimitive() ) {

		JS::RootedObject tmpObj(cx);
		JL_CHK( JS_ValueToObject(cx, srcVal, &tmpObj) );
		JS::RootedValue tmpVal(cx, OBJECT_TO_JSVAL(tmpObj));
		JL_CHK( JL_SetReservedSlot(obj, SLOT_STREAM_SOURCE, tmpVal) );
	} else {

		JL_CHK( JL_SetReservedSlot(obj, SLOT_STREAM_SOURCE, srcVal) );
	}

	return true;
	JL_BAD;
}

ALWAYS_INLINE bool
SetPosition(JSContext *cx, JS::HandleObject obj, size_t position) {

	JL_CHK( jl::setSlot(cx, obj, SLOT_STREAM_POSITION, position) );
	return true;
	JL_BAD;
}

ALWAYS_INLINE bool
GetPosition(JSContext *cx, JS::HandleObject obj, size_t *position) {

	JL_CHK( jl::getSlot(cx, obj, SLOT_STREAM_POSITION, position) );
	return true;
	JL_BAD;
}


ALWAYS_INLINE bool
GetAvailable(JSContext *cx, JS::HandleObject obj, size_t *available) {

	size_t position, length;
	JS::RootedObject srcObj(cx);
	JS::RootedValue val(cx);

	JL_CHK( GetPosition(cx, obj, &position) );
	JL_CHK( GetStreamSource(cx, obj, &srcObj) );

	bool found;
	JL_CHK( JS_HasPropertyById(cx, obj, JLID(cx, length), &found) );


	JL_CHK( JS_GetPropertyById(cx, obj, JLID(cx, length), &val) );

	if ( !val.isUndefined() ) {

		JL_CHK( jl::getValue(cx, val, &length) );
	} else {

		JLData data;
		val = OBJECT_TO_JSVAL(srcObj);
		JL_CHK( jl::getValue(cx, val, &data) );
		JL_CHK( data.IsSet() );
		length = data.Length();
	}

	*available = length - position;
	return true;
	JL_BAD;
}


ALWAYS_INLINE bool
StreamRead( JSContext *cx, JS::HandleObject streamObj, char *buf, size_t *amount ) {

	JLData data;
	size_t position;
	JS::RootedValue source(cx);

	JL_ASSERT_INSTANCE(streamObj, JL_THIS_CLASS);

	JL_CHK( GetPosition(cx, streamObj, &position) );
	JL_CHK( JL_GetReservedSlot(streamObj, SLOT_STREAM_SOURCE, &source) );
	JL_CHK( jl::getValue(cx, source, &data) );

	size_t length = data.Length();
	if ( length - position <= 0 ) { // position >= length

		*amount = 0; // EOF
		return true;
	}

	if ( position + *amount > length )
		*amount = length - position;

	jl::memcpy( buf, data.GetConstStr() + position, *amount ); // (TBD) possible optimization. see JLData::CopyTo() ?
	JL_CHK( SetPosition(cx, streamObj, position + *amount) );
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INAME( bufferObject )
  Creates an object that transforms any buffer-like objects into a stream.
  $LF
  buffer-like objects are: string, Blob, and any objects that implements NIBufferGet native interface.
  $LF
  $H note
  When called in a non-constructor context, Object behaves identically.
**/
DEFINE_CONSTRUCTOR() {

	JL_DEFINE_ARGS;
	
	JL_DEFINE_CONSTRUCTOR_OBJ;
	// JL_ASSERT_CONSTRUCTING(); // supports this form (w/o new operator) : result.param1 = Blob('Hello World');
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	JL_CHK( SetStreamSource(cx, JL_OBJ, JL_ARG(1)) );
	JL_CHK( SetPosition(cx, JL_OBJ, 0) );
	JL_CHK( SetStreamReadInterface(cx, JL_OBJ, StreamRead) );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Blob $INAME( amount )
  Read _amount_ of data into the stream.
**/
DEFINE_FUNCTION( read ) {


	uint8_t *buffer = NULL;
	
	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(0, 1);

	size_t amount, available;

	JL_CHK( GetAvailable(cx, JL_OBJ, &available) );

	if ( JL_ARG_ISDEF(1) ) {

		JL_CHK( jl::getValue(cx, JL_ARG(1), &amount) );
		if ( amount == 0 ) {

			JL_CHK( JL_NewEmptyBuffer(cx, JL_RVAL) );
			return true;
		}
		if ( available < amount )
			amount = available;
	} else {

		amount = available;
	}

	if ( available == 0 ) {

		JL_RVAL.setUndefined();
		return true;
	}

	buffer = JL_NewBuffer(cx, amount, JL_RVAL);
	JL_CHK( buffer );

	size_t readAmount;
	readAmount = amount;

	JL_CHK( StreamRead(cx, JL_OBJ, (char*)buffer, &readAmount) );

	if ( readAmount == 0 ) {

		JL_RVAL.setUndefined();
		return true;
	}

	if ( readAmount != amount )
		JL_ChangeBufferLength(cx, JL_RVAL, readAmount);

	return true;
bad:
	JL_DataBufferFree(cx, buffer);
	return false;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Get or set the stream pointer position.
**/
DEFINE_PROPERTY_GETTER( position ) {

	JL_IGNORE(id);
	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	size_t position;
	JL_CHK( GetPosition(cx, obj, &position) );
	
	JL_CHK( jl::setValue(cx, vp, position) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( position ) {

	JL_IGNORE(id, strict);
	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	size_t position;
	JL_CHK( jl::getValue(cx, vp, &position) );
	JL_CHK( SetPosition(cx, obj, position) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  The remaining data from the stream pointer position to the end of the stream.
**/
/**qa
	QA.ASSERTOP( Stream(stringify('1234ABCDEF', true)), 'has', 'available' );
	QA.ASSERTOP( Stream(stringify('1234ABCDEF', true)).available, '===', 10 );
	QA.ASSERTOP( Stream(Int8Array(stringify("1234ABCDEF", true))).available, '==', 10 );
**/
DEFINE_PROPERTY_GETTER( available ) {

	JL_IGNORE(id);
	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	size_t available;
	JL_CHK( GetAvailable(cx, obj, &available) );
	JL_CHK( jl::setValue(cx, vp, available) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME
  The object used to create the steam.
**/
DEFINE_PROPERTY_GETTER( source ) {

	JL_IGNORE(id);
	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	JL_CHK( JL_GetReservedSlot( obj, SLOT_STREAM_SOURCE, vp) );
	return true;
	JL_BAD;
}


/**doc
=== note ===
 Basically, a Stream is nothing else that a buffer with a stream pointer position.
**/


/**doc
=== Native Interface ===
 * *NIStreamRead*
**/

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_CONSTRUCTOR
	HAS_RESERVED_SLOTS(2)

	BEGIN_FUNCTION_SPEC
		FUNCTION(read)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY(position)
		PROPERTY_GETTER(available)
		PROPERTY_GETTER(source)
	END_PROPERTY_SPEC

END_CLASS

/*
	Adapter pattern:
		http://en.wikipedia.org/wiki/Adapter_pattern

*/
