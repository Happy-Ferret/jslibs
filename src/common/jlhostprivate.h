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


// JL_HOST_PRIVATE_VERSION is supposed to change each time the structure is changed.
#define JL_HOST_PRIVATE_VERSION (JL_SvnRevToInt("$Revision$"))

#define MAX_CLASS_PROTO_CACHE_BIT 9

struct ClassProtoCache {
	JSClass *clasp;
	JSObject *proto;
};

#define DEF(RET, NAME, ARGS) RET(*NAME)ARGS;
struct JLApi {
#include "jlapi.tbl"
};
#undef DEF

struct HostPrivate {

	bool unsafeMode;
	unsigned int hostPrivateVersion; // used to ensure compatibility between host and modules. see JL_HOST_PRIVATE_VERSION macro.
	void *privateData;
	volatile unsigned int maybeGCInterval;
	JLSemaphoreHandler watchDogSemEnd;
	JLThreadHandler watchDogThread;
	int (*hostStdIn)( void *privateData, char *buffer, size_t bufferSize );
	int (*hostStdOut)( void *privateData, const char *buffer, size_t length );
	int (*hostStdErr)( void *privateData, const char *buffer, size_t length );
	JSErrorCallback errorCallback;
	JSLocaleCallbacks localeCallbacks;
	struct ModulePrivate {
		uint32_t moduleId;
		void *privateData;
	} modulePrivate[1<<8]; // does not support more than 256 modules.
	jl::Queue moduleList;
	jl::Queue registredNativeClasses;
	JSClass *stringObjectClass;
	jl_allocators_t alloc;
	bool canSkipCleanup; // allows modules to skip the memory cleanup phase.
	int camelCase;
	jsid ids[LAST_JSID];
	ClassProtoCache classProtoCache[1<<MAX_CLASS_PROTO_CACHE_BIT]; // does not support more than (1<<MAX_CLASS_PROTO_CACHE_BIT)-1 proto.
	JLApi *jlapi;
};

JL_STATIC_ASSERT( offsetof(HostPrivate, unsafeMode) == 0 ); // check this because JL_S_ASSERT macro must be usable before hostPrivateVersion is tested.
