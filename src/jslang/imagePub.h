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
 
#pragma once

#include "stdafx.h"

#include "blobPub.h"


///////////////////////////////////////////////////////////////////////////////
// Generic Image object

enum ImageDataType {
    TYPE_INT8    = js::ArrayBufferView::TYPE_INT8,
    TYPE_UINT8   = js::ArrayBufferView::TYPE_UINT8,
    TYPE_INT16   = js::ArrayBufferView::TYPE_INT16,
    TYPE_UINT16  = js::ArrayBufferView::TYPE_UINT16,
    TYPE_INT32   = js::ArrayBufferView::TYPE_INT32,
    TYPE_UINT32  = js::ArrayBufferView::TYPE_UINT32,
    TYPE_FLOAT32 = js::ArrayBufferView::TYPE_FLOAT32,
    TYPE_FLOAT64 = js::ArrayBufferView::TYPE_FLOAT64,
};

template <class T, class U>
ALWAYS_INLINE uint8_t* FASTCALL
JL_NewImageObject( IN JSContext *cx, IN T width, IN T height, IN U channels, IN ImageDataType dataType, OUT JS::MutableHandleValue vp ) {

	ASSERT( width >= 0 && height >= 0 && channels > 0 );

	//uint8_t *data;
	jl::AutoBuffer buffer;
	JS::RootedValue dataVal(cx);
	JS::RootedObject imageObj(cx, JL_NewObj(cx));

	JL_CHK( imageObj );
	vp.setObject(*imageObj);
	//data = JL_NewBuffer(cx, width * height* channels, dataVal);
	buffer.alloc(width * height * channels);
	JL_ASSERT_ALLOC(buffer);
	uint8_t *data = BlobCreate(cx, buffer, &dataVal);
	JL_CHK( data );
	JL_CHK( jl::setProperty(cx, imageObj, JLID(cx, data), &dataVal) );
	JL_CHK( jl::setProperty(cx, imageObj, JLID(cx, width), width) );
	JL_CHK( jl::setProperty(cx, imageObj, JLID(cx, height), height) );
	JL_CHK( jl::setProperty(cx, imageObj, JLID(cx, channels), channels) );
	JL_CHK( jl::setProperty(cx, imageObj, JLID(cx, type), dataType) );
	return data;
bad:
	return NULL;
}

template <class T, class U>
ALWAYS_INLINE bool FASTCALL
JL_NewImageObjectOwner( IN JSContext *cx, IN uint8_t* buffer, IN T width, IN T height, IN U channels, IN ImageDataType dataType, OUT JS::MutableHandleValue vp ) {

	ASSERT_IF( buffer == NULL, width * height * channels == 0 );
	ASSERT_IF( buffer != NULL, width > 0 && height > 0 && channels > 0 );
	ASSERT_IF( buffer != NULL, jl_msize(buffer) >= (size_t)(width * height * channels) );

	JS::RootedValue dataVal(cx);
	JS::RootedObject imageObj(cx, JL_NewObj(cx));

	JL_CHK( imageObj );
	vp.setObject(*imageObj);
	JL_CHK( JL_NewBufferGetOwnership(cx, buffer, width * height * channels, &dataVal) );
	JL_CHK( JS_SetPropertyById(cx, imageObj, JLID(cx, data), &dataVal) );
	JL_CHK( jl::setProperty(cx, imageObj, JLID(cx, width), width) );
	JL_CHK( jl::setProperty(cx, imageObj, JLID(cx, height), height) );
	JL_CHK( jl::setProperty(cx, imageObj, JLID(cx, channels), channels) );
	JL_CHK( jl::setProperty(cx, imageObj, JLID(cx, type), dataType) );
	return true;
	JL_BAD;
}


template <class T, class U>
ALWAYS_INLINE JLData FASTCALL
JL_GetImageObject( IN JSContext *cx, IN JS::HandleValue val, OUT T *width, OUT T *height, OUT U *channels, OUT ImageDataType *dataType ) {

	JLData data;
	JL_CHK( val.isObject() );

	{

	JS::RootedObject imageObj(cx, &val.toObject());
	JL_CHK( jl::getProperty(cx, imageObj, JLID(cx, data), &data) );
	JL_CHK( jl::getProperty(cx, imageObj, JLID(cx, width), width) );
	JL_CHK( jl::getProperty(cx, imageObj, JLID(cx, height), height) );
	JL_CHK( jl::getProperty(cx, imageObj, JLID(cx, channels), channels) );
	int tmp;
	JL_CHK( jl::getProperty(cx, imageObj, JLID(cx, type), &tmp) );
	S_ASSERT(sizeof(ImageDataType) == sizeof(int));
	*dataType = (ImageDataType)tmp;

	int dataTypeSize;
	switch ( *dataType ) {
		case TYPE_INT8:
		case TYPE_UINT8:
			dataTypeSize = 1;
			break;
		case TYPE_INT16:
		case TYPE_UINT16:
			dataTypeSize = 2;
			break;
		case TYPE_INT32:
		case TYPE_UINT32:
		case TYPE_FLOAT32:
			dataTypeSize = 4;
			break;
		case TYPE_FLOAT64:
			dataTypeSize = 8;
			break;
		default:
			JL_CHK(false);
	}

	JL_CHK( *width >= 0 && *height >= 0 && *channels > 0 );
	JL_CHK( data.IsSet() && jl::isInBounds<int>(data.Length()) && (int)data.Length() == (int)(*width * *height * *channels * dataTypeSize) );
//	JL_ASSERT( width >= 0 && height >= 0 && channels > 0, E_STR("image"), E_FORMAT );
//	JL_ASSERT( data.IsSet() && jl::SafeCast<int>(data.Length()) == (int)(*width * *height * *channels * 1), E_DATASIZE, E_INVALID );

	}

	return data;
bad:
	return JLData();
}
