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
#include "event.h"
#include "nsprError.h"

BEGIN_CLASS( Event )

DEFINE_FINALIZE() {

	PRFileDesc *fd = (PRFileDesc*)PR_NewPollableEvent();

	if ( fd != NULL ) {
		
		PRStatus st = PR_DestroyPollableEvent(fd);
		if ( st != PR_SUCCESS )
			JS_ReportError( cx, "failed to DestroyPollableEvent." );
	}
}

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(_class);

	PRFileDesc *fd = PR_NewPollableEvent();

	if ( fd == NULL )
		return ThrowNSPRError( cx, PR_GetError() );

	RT_CHECK_CALL( JS_SetPrivate(cx, obj, fd) );
	return JS_TRUE;
}

DEFINE_FUNCTION( Set ) {
	
	PRFileDesc *fd = (PRFileDesc*)PR_NewPollableEvent();
	RT_ASSERT_RESOURCE( fd );
	PRStatus st = PR_SetPollableEvent(fd);
	if ( st != PR_SUCCESS )
		return ThrowNSPRError( cx, PR_GetError() );
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION(Set)
	END_FUNCTION_SPEC

	HAS_PRIVATE

END_CLASS
