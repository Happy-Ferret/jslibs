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

#define XP_WIN
#include <jsapi.h>

#include <stdio.h>

#define CONFIGURATION_OBJECT_NAME "configuration"

extern JSClass configuration_class;

JSBool GetConfigurationObject(JSContext *cx, JSObject **configurationObject );
JSBool GetConfigurationValue( JSContext *cx, const char *name, jsval *value );


/*
struct Configuration {

	JSVersion jsVersion;
	bool reportWarnings;
	bool unsafeMode;
	int (*stdOut)(const char *data, int length);
	int (*stdErr)(const char *data, int length);
};


Configuration *GetConfiguration( JSContext *cx );
*/
