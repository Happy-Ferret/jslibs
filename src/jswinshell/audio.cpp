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
#include "error.h"
#include <mmsystem.h>

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( Audio )



/**doc
$TOC_MEMBER $INAME
 $INAME()
**/
//DEFINE_FUNCTION( open ) {
DEFINE_CONSTRUCTOR() {

	JL_DEFINE_ARGS;

	UINT uDeviceID;

	if ( JL_ARG_ISDEF(1) ) {

		JLData deviceName;

		JL_CHK( jl::getValue(cx, JL_ARG(1), &deviceName) );

		MMRESULT mmResult;
		UINT uNumDevs = waveInGetNumDevs();
		for (uDeviceID = 0; uDeviceID < uNumDevs; uDeviceID++) {
		
			// Take a look at the driver capabilities.
			WAVEINCAPS wic;
			mmResult = waveInGetDevCaps(uDeviceID, &wic, sizeof(wic));

			if ( strcmp(wic.szPname, "Rec. Playback (IDT High Definit") == 0 )
				break;
			ASSERT( mmResult == MMSYSERR_NOERROR );
		}
	} else {
		
		uDeviceID = WAVE_MAPPER;
	}

	MMRESULT res;

	WAVEFORMATEX wfx;
	wfx.nSamplesPerSec = 44100;
	wfx.wBitsPerSample = 16;
	wfx.nChannels = 2;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.cbSize = 0;
	wfx.nBlockAlign = (wfx.wBitsPerSample * wfx.nChannels) / 8;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	HWAVEIN hwi;
	res = waveInOpen(&hwi, uDeviceID, &wfx, NULL, NULL, WAVE_FORMAT_DIRECT);




	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
**/
DEFINE_PROPERTY_GETTER( inputDeviceList ) {

	JL_DEFINE_PROP_ARGS;

	JS::RootedObject list(cx, jl::newArray(cx));
	JL_RVAL.setObject(*list);

	UINT uDeviceID;
	MMRESULT mmResult;
	UINT uNumDevs = waveInGetNumDevs();
	for (uDeviceID = 0; uDeviceID < uNumDevs; uDeviceID++) {
		
		// Take a look at the driver capabilities.
		WAVEINCAPS wic;
		mmResult = waveInGetDevCaps(uDeviceID, &wic, sizeof(wic));


		ASSERT( mmResult == MMSYSERR_NOERROR );

		jl::pushElement(cx, list, wic.szPname);
	}

	return true;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))

	HAS_CONSTRUCTOR

	BEGIN_STATIC_FUNCTION_SPEC
		//FUNCTION( open )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( inputDeviceList )
	END_STATIC_PROPERTY_SPEC

END_CLASS

/**doc
=== Examples ===
{{{
}}}
**/
