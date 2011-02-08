// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "jlhelper.h"
#include "jlclass.h"
#include "../common/jsvalserializer.h"

#include <jstypedarray.h>

#include "pool.h"


#define _USE_MATH_DEFINES
#include "math.h"

#include "matrix44.h"
#include "vector3.h"


#include "jstransformation.h"




#ifdef _MACOSX // MacosX platform specific
	#include <AGL/agl.h>
	#include <OpenGL/gl.h>
#endif

//#define GL_GLEXT_PROTOTYPES

#include <gl/gl.h>
#include "glext.h" // download at http://www.opengl.org/registry/api/glext.h (http://www.opengl.org/registry/#headers)

#include "oglError.h"

