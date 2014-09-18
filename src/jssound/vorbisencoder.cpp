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

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <vorbis/vorbisenc.h>

#include <ctime>

/**doc fileIndex:top
$CLASS_HEADER
$SVN_REVISION $Revision: 3782 $
 The OggVorbisDecoder support ogg vorbis data format decoding.
**/
BEGIN_CLASS( VorbisEncoder )


struct Private {

	ogg_page page;
	vorbis_info vi;
	vorbis_dsp_state vds;
	vorbis_block vb;

	ogg_stream_state oss;
	int index;
};


DEFINE_FINALIZE() {

	if ( jl::HostRuntime::getJLRuntime( fop->runtime() ).skipCleanup() )
		return;

	Private *pv = static_cast<Private*>(JL_GetPrivateFromFinalize(obj));
	if ( pv != NULL ) {

		vorbis_dsp_clear(&pv->vds);
		vorbis_info_clear(&pv->vi);
		vorbis_block_clear(&pv->vb);
		
		ogg_stream_clear(&pv->oss);

		JL_freeop(fop, pv);
		//js_free(pv);
	}
}

DEFINE_CONSTRUCTOR() {

	Private *pv = NULL;

	JL_DEFINE_ARGS;
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC_RANGE(0,3);
	
	pv = static_cast<Private*>(jl_malloc(sizeof(Private)));
	JL_SetPrivate(JL_OBJ, pv);

	long channels = jl::getValueDefault(cx, JL_SARG(1), 2L);
	long rate = jl::getValueDefault(cx, JL_SARG(2), 44100L);
	float quality = jl::getValueDefault(cx, JL_SARG(3), 0.f);


//	JL_ASSERT_RANGE(quality, -0.1f, 1.0f, "quality");

	vorbis_info_init(&pv->vi);
	int st;
	
	//st = vorbis_encode_setup_vbr(&pv->vi, 2, 44100, 0.);
	// st = vorbis_encode_setup_init(&pv->vi);
	//vorbis_encode_ctl(&pv->vi, 


	vorbis_encode_ctl(&pv->vi, OV_ECTL_RATEMANAGE_SET, NULL);

	// VBR
	st = vorbis_encode_init_vbr(&pv->vi, channels, rate, quality);
	ASSERT( st == 0 );


/*
	// CBR/ABR
	st = vorbis_encode_init(&pv->vi, channels, rate, 64 *1024, 96 *1024, 128 *1024);
	ASSERT( st == 0 );
*/

//	st = vorbis_encode_ctl(&pv->vi, OV_ECTL_RATEMANAGE2_SET, NULL);
//	ASSERT( st == 0 );


	st = vorbis_analysis_init(&pv->vds, &pv->vi);
	ASSERT( st == 0 );


	st = vorbis_block_init(&pv->vds, &pv->vb);
	ASSERT( st == 0 );


	// ogg stream

	srand((unsigned)time(NULL));
	st = ogg_stream_init(&pv->oss, rand());
	ASSERT( st == 0 );

	ogg_packet header;
	ogg_packet header_comm;
	ogg_packet header_code;

	vorbis_comment vc;
	vorbis_comment_init(&vc);
	//vorbis_comment_add_tag(&vc, "ENCODER", "jslibs");

	st = vorbis_analysis_headerout(&pv->vds, &vc, &header, &header_comm, &header_code);
	ASSERT( st == 0 );

	st = ogg_stream_packetin(&pv->oss, &header);
	ASSERT( st == 0 );

	st = ogg_stream_packetin(&pv->oss, &header_comm);
	ASSERT( st == 0 );

	st = ogg_stream_packetin(&pv->oss, &header_code);
	ASSERT( st == 0 );

/*
	for (;;) {
		
		int flushStatus = ogg_stream_flush(&pv->oss, &pv->op);
		if( flushStatus == 0 )
			break;
	}

	dstBuf.Write(pv->op.header, pv->op.header_len);
	dstBuf.Write(pv->op.body, pv->op.body_len);
*/





	return true;
bad:
	jl_free(pv);
	return false;
}

DEFINE_FUNCTION( encode ) {

	JL_DEFINE_ARGS;

	int bits;
	int channels;
	int frames;
	int rate;

	int st;


	Private *pv = static_cast<Private*>(JL_GetPrivate(JL_OBJ));

	// int16_t *data;
	jl::StrData data(cx);
	jl::ChunkedBuffer<uint8_t> dstBuf;

	if ( !JL_SARG(1).isUndefined() ) {

		JL_ASSERT( JL_isAudioObject(cx, JL_ARG(1)) );
		JL_CHK( JL_GetByteAudioObject(cx, JL_ARG(1), &bits, &channels, &frames, &rate, data) );
		
		ASSERT( pv->vi.rate == rate );
		ASSERT( pv->vi.channels == channels );
	} else {

		frames = 0;
	}


//	ogg_page_bos(&pv->page);
/*
	if ( pv->index == 0 ) {

		for (;;) {
		
			int flushStatus = ogg_stream_flush(&pv->oss, &pv->page);
			if( flushStatus == 0 )
				break;
		}

		dstBuf.Write(pv->page.header, pv->page.header_len);
		dstBuf.Write(pv->page.body, pv->page.body_len);
	}
*/	
	++pv->index;

	ogg_packet packet;

	int frameOffset = 0;

		
	for (;;) {

		int frameCount = jl::min(frames - frameOffset, 1024);

		float **buffers = vorbis_analysis_buffer(&pv->vds, frameCount);
		ASSERT( buffers );
		JS::AutoCheckCannotGC nogc;

		const int16_t *frameData = reinterpret_cast<const int16_t*>(data.toBytes(nogc)) + frameOffset * 2; // 2 channels

		for ( int i = 0; i < frameCount; ++i ) {

			buffers[0][i] = frameData[i  ] / float(int16_t(~0));
			buffers[1][i] = frameData[i+1] / float(int16_t(~0));
		}

		st = vorbis_analysis_wrote(&pv->vds, frameCount); // doc: A value of zero means all input data has been provided and the compressed stream should be finalized.
		ASSERT( st == 0 );


		for (;;) {

			int blockStatus = vorbis_analysis_blockout(&pv->vds, &pv->vb);
			ASSERT( blockStatus >= 0 );
			if ( blockStatus == 0 ) // until it returns zero (need more data)
				break;

			st = vorbis_analysis(&pv->vb, NULL);
			ASSERT( st == 0 );
			
			st = vorbis_bitrate_addblock(&pv->vb);
			ASSERT( st == 0 );

			for (;;) {
					
				int packetStatus = vorbis_bitrate_flushpacket(&pv->vds, &packet);
				ASSERT( packetStatus >= 0 );
		

				// ogg stream

				st = ogg_stream_packetin(&pv->oss, &packet);
				ASSERT( st == 0 );

//				ogg_packet_clear(&packet);
/*
				if ( frames != 0 ) { // not end of input stream
 
					for (;;) {

						if ( ogg_stream_pageout(&pv->oss, &pv->page) == 0 )
							break;
					
						dstBuf.Write(pv->page.header, pv->page.header_len);
						dstBuf.Write(pv->page.body, pv->page.body_len);

						if ( ogg_page_eos(&pv->page) )
							break;
					}
				} else {
*/
					while ( ogg_stream_flush(&pv->oss, &pv->page) != 0 ) {
		
						dstBuf.Write(pv->page.header, pv->page.header_len);
						dstBuf.Write(pv->page.body, pv->page.body_len);
					}
/*
				}
*/

				//dstBuf.Reserve(op.bytes);
				//jl::memcpy(dstBuf.Ptr(), op.packet, op.bytes);
				//dstBuf.Advance(op.bytes);

				if ( packetStatus == 0 )
					break;
			}
 		}

		frameOffset += frameCount;
		if ( frameOffset >= frames )
			break;

	}

	JL_CHK( BlobCreate(cx, dstBuf.GetDataOwnership(), dstBuf.Length(), JL_RVAL) );

	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3782 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( encode, 1 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

END_CLASS
