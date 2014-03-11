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
#include <jslibsModule.cpp>

#include <videoinput.h>


videoInput *vi = NULL;

DECLARE_CLASS( VideoInput )

bool 
ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	JL_CHK( InitJslibsModule(cx, id)  );

	JL_ASSERT( vi == NULL, E_LIB, E_STR("videoinput"), E_INIT ); // "Invalid case: videoInput already initialized'"

	videoInput::setVerbose(false);
	vi = new videoInput();
	JL_ASSERT_ALLOC( vi );

	INIT_CLASS( VideoInput );

	return true;
	JL_BAD;
}


void
ModuleFree() {

	delete vi;
}
