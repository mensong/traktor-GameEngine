#ifndef traktor_render_BlitHelper_H
#define traktor_render_BlitHelper_H

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{
	
class ContextOpenGLES2;

class BlitHelper : public Object
{
	T_RTTI_CLASS;
	
public:
	bool create(ContextOpenGLES2* resourceContext);
	
	void destroy();

	void blit(GLint sourceTextureHandle);
	
private:
	Ref< ContextOpenGLES2 > m_resourceContext;
	GLuint m_programObject;
	GLint m_attribPosition;
	GLint m_attribTexCoord;
	GLint m_samplerIndex;
	GLuint m_vertexBuffer;
	RenderState m_renderState;
};
	
	}
}

#endif	// traktor_render_BlitHelper_H
