#pragma once

#include "Render/IRenderSystem.h"
#include "Render/OpenGL/ES2/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_ES2_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextOpenGLES2;

/*! \brief OpenGL ES 2.0 render system.
 * \ingroup OGL
 *
 * OpenGL render system implementation.
 */
class T_DLLCLASS RenderSystemOpenGLES2 : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemOpenGLES2();

	virtual bool create(const RenderSystemDesc& desc) override final;

	virtual void destroy() override final;

	virtual bool reset(const RenderSystemDesc& desc) override final;

	virtual void getInformation(RenderSystemInformation& outInfo) const override final;

	virtual uint32_t getDisplayModeCount() const override final;

	virtual DisplayMode getDisplayMode(uint32_t index) const override final;

	virtual DisplayMode getCurrentDisplayMode() const override final;

	virtual float getDisplayAspectRatio() const override final;

	virtual Ref< IRenderView > createRenderView(const RenderViewDefaultDesc& desc) override final;

	virtual Ref< IRenderView > createRenderView(const RenderViewEmbeddedDesc& desc) override final;

	virtual Ref< VertexBuffer > createVertexBuffer(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic) override final;

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic) override final;

	virtual Ref< StructBuffer > createStructBuffer(const AlignedVector< StructElement >& structElements, uint32_t bufferSize) override final;

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc) override final;

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc) override final;

	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc) override final;

	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc) override final;

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource, const wchar_t* const tag) override final;

	virtual Ref< ITimeQuery > createTimeQuery() const override final;

	virtual void purge() override final;

	virtual void getStatistics(RenderSystemStatistics& outStatistics) const override final;

private:
	SystemApplication m_sysapp;
	Ref< ContextOpenGLES2 > m_context;
};

	}
}

