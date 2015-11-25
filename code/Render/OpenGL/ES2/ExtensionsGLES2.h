#ifndef traktor_render_ExtensionsGLES2_H
#define traktor_render_ExtensionsGLES2_H

#include "Render/OpenGL/Platform.h"

namespace traktor
{
	namespace render
	{

#if defined(GL_OES_vertex_array_object)
#	if defined(__IOS__)
#		define g_glBindVertexArrayOES glBindVertexArrayOES
#		define g_glDeleteVertexArraysOES glDeleteVertexArraysOES
#		define g_glGenVertexArraysOES glGenVertexArraysOES
#	else
extern PFNGLBINDVERTEXARRAYOESPROC g_glBindVertexArrayOES;
extern PFNGLDELETEVERTEXARRAYSOESPROC g_glDeleteVertexArraysOES;
extern PFNGLGENVERTEXARRAYSOESPROC g_glGenVertexArraysOES;
#	endif
#endif

void initializeExtensions();

	}
}

#endif	// traktor_render_ExtensionsGLES2_H
