#ifndef traktor_render_RenderSystemOpenGL_H
#define traktor_render_RenderSystemOpenGL_H

#include "Render/IRenderSystem.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"

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

class BlitHelper;
class Window;

/*! \brief OpenGL render system.
 * \ingroup OGL
 *
 * OpenGL render system implementation.
 */
class T_DLLCLASS RenderSystemOpenGL : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemOpenGL();

	virtual bool create(const RenderSystemCreateDesc& desc);

	virtual void destroy();

	virtual uint32_t getDisplayModeCount() const;

	virtual DisplayMode getDisplayMode(uint32_t index) const;

	virtual DisplayMode getCurrentDisplayMode() const;

	virtual float getDisplayAspectRatio() const;

	virtual Ref< IRenderView > createRenderView(const RenderViewDefaultDesc& desc);

	virtual Ref< IRenderView > createRenderView(const RenderViewEmbeddedDesc& desc);

	virtual Ref< VertexBuffer > createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc);

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc);

	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc);

	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc);

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource);

	virtual Ref< IProgramCompiler > createProgramCompiler() const;

	virtual void getStatistics(RenderSystemStatistics& outStatistics) const;

private:
	friend class RenderViewOpenGL;

#if defined(_WIN32)

	Ref< Window > m_windowShared;
	Ref< Window > m_window;

#elif defined(__APPLE__)

	void* m_windowHandle;

#endif

	Ref< ContextOpenGL > m_resourceContext;
	Ref< BlitHelper > m_blitHelper;
	GLfloat m_maxAnisotrophy;
};

	}
}

#endif	// traktor_render_RenderSystemOpenGL_H
