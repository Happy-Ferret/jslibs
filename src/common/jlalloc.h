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

//#ifndef _JLALLOC_H_
//#define _JLALLOC_H_

#pragma once

#include <sys/types.h>

typedef void* (*jl_malloc_t)( size_t size );
typedef void* (*jl_calloc_t)( size_t num, size_t size );
typedef void* (*jl_realloc_t)( void *ptr, size_t size );
typedef void (*jl_free_t)( void *ptr );

extern jl_malloc_t jl_malloc;
extern jl_calloc_t jl_calloc;
extern jl_realloc_t jl_realloc;
extern jl_free_t jl_free;

//#endif // _JLALLOC_H_
