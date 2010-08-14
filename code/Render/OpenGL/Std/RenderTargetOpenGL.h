#ifndef traktor_render_RenderTargetOpenGL_H
#define traktor_render_RenderTargetOpenGL_H

#include "Render/ITexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/Std/ITextureBinding.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IContext;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS RenderTargetOpenGL
:	public ITexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	RenderTargetOpenGL(IContext* resourceContext);

	virtual ~RenderTargetOpenGL();

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, GLuint depthBuffer);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;
	
	virtual void bind(GLuint unit, const SamplerState& samplerState, GLint locationTexture, GLint locationOffset);

	void bind(bool keepDepthStencil);

	void enter(bool keepDepthStencil);
	
	void resolve();

	void blit();
	
	bool read(void* buffer) const;
	
	GLuint clearMask() const;

private:
	Ref< IContext > m_resourceContext;
	int m_width;
	int m_height;
	GLenum m_textureTarget;
	GLuint m_targetFBO;
	GLuint m_resolveFBO;
	GLuint m_targetColorBuffer;
	GLuint m_colorTexture;
	Vector4 m_originAndScale;
	bool m_haveDepth;
	SamplerState m_shadowState;
};

	}
}

#endif	// traktor_render_RenderTargetOpenGL_H
