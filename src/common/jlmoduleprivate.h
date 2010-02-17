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

#ifndef _JLMODULEPRIVATE_H_
#define _JLMODULEPRIVATE_H_

#include "jlplatform.h"

extern volatile long _modulePrivateCount;
extern JLTLSKey _modulePrivateKey;

#define DEFINE_MODULE_PRIVATE \
	volatile long _modulePrivateCount = 0; \
	JLTLSKey _modulePrivateKey = JLTLSInvalidKey;

ALWAYS_INLINE void* ModulePrivateAlloc(size_t size) {

	if ( JLAtomicIncrement(&_modulePrivateCount) == 1 )
		_modulePrivateKey = JLTLSAllocKey();
	while ( _modulePrivateKey == JLTLSInvalidKey ) // (TBD) replace this UGLY hack with a mutex (however this case is extremely rare).
		SleepMilliseconds(1);
	JL_ASSERT( _modulePrivateKey != JLTLSInvalidKey );
	JL_ASSERT( JLTLSGet(_modulePrivateKey) == NULL ); // already allocated
	void *modulePrivate = malloc(size);
	JL_ASSERT( modulePrivate ); // not enough memory
	JLTLSSet(_modulePrivateKey, modulePrivate);
	return modulePrivate;
}

ALWAYS_INLINE void ModulePrivateFree() {

	void *modulePrivate = JLTLSGet(_modulePrivateKey);
	JL_ASSERT( modulePrivate != NULL ); // already disallocated
	JLTLSSet(_modulePrivateKey, NULL);
	free(modulePrivate);
	if ( JLAtomicDecrement(&_modulePrivateCount) == 0 )
		JLTLSFreeKey(_modulePrivateKey);
}

ALWAYS_INLINE void* ModulePrivateGet() {

	void *modulePrivate = JLTLSGet(_modulePrivateKey);
	JL_ASSERT( modulePrivate != NULL ); // already disallocated
	return modulePrivate;
}

#endif // _JLMODULEPRIVATE_H_