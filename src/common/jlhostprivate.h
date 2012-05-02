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


// note: JL_HOSTPRIVATE_KEY is supposed to change each time the structure is modified.
#define JL_HOSTPRIVATE_KEY ( (jl::SvnRevToInt("$Revision: 3524 $") << 16) | ((sizeof(HostPrivate) ^ offsetof(HostPrivate, ids) ^ offsetof(HostPrivate, modulePrivate)) & 0xFFFF) )
#define JL_HOSTPRIVATE_MAX_CLASS_PROTO_CACHE_BIT (9)


struct ClassProtoCache {
	JSClass *clasp;
	JSObject *proto;
};

struct HostPrivate {
	uint32_t versionKey; // used to ensure compatibility between host and modules. see JL_HOSTPRIVATE_KEY macro.
	bool unsafeMode; // used to spread the unsafe status across modules.
	JSObject *hostObject;
	bool isEnding;
	bool canSkipCleanup; // allows modules to skip the memory cleanup phase.
	void *privateData;
	uint32_t maybeGCInterval;
	JLSemaphoreHandler watchDogSemEnd;
	JLThreadHandler watchDogThread;
	int (*hostStdIn)( void *privateData, char *buffer, size_t bufferSize );
	int (*hostStdOut)( void *privateData, const char *buffer, size_t length );
	int (*hostStdErr)( void *privateData, const char *buffer, size_t length );
	JSBool (*report)( JSContext *cx, bool isWarning, ... );
	struct ModulePrivate {
		uint32_t moduleId;
		void *privateData;
	} modulePrivate[1<<8]; // does not support more than 256 modules.
	jl::Queue moduleList;
	JSClass *objectClass;
	JSObject *objectProto;
	jl_allocators_t alloc;
	ClassProtoCache classProtoCache[1<<JL_HOSTPRIVATE_MAX_CLASS_PROTO_CACHE_BIT]; // does not support more than (1<<MAX_CLASS_PROTO_CACHE_BIT)-1 proto.
	jsid ids[LAST_JSID];
	
// experimental:
	//	jl::StaticAlloc<char[16], 128> p16;
#ifdef DEBUG
	uint32_t tmp_count;
#endif
};

S_ASSERT( offsetof(HostPrivate, versionKey) == 0 ); // versionKey must be reachable before any other member.
S_ASSERT( offsetof(HostPrivate, unsafeMode) == 4 ); // then unsafeMode.


ALWAYS_INLINE HostPrivate *
ConstructHostPrivate() {

	return ::new(jl_calloc(1, sizeof(HostPrivate))) HostPrivate; // beware: don't realloc later because WatchDogThreadProc points on it.
}

ALWAYS_INLINE void
DestructHostPrivate(HostPrivate *hpv) {

	hpv->HostPrivate::~HostPrivate();
	jl_free(hpv);
}
