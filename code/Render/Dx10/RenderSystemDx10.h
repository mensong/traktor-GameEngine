#ifndef traktor_render_RenderSystemDx10_H
#define traktor_render_RenderSystemDx10_H

#include "Render/Dx10/Platform.h"
#include "Render/RenderSystem.h"
#include "Core/Misc/ComRef.h"
#include "Core/Misc/AutoPtr.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX10_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx10;

/*! \brief DirectX 10 render system.
 * \ingroup DX10
 *
 * OpenGL render system implementation.
 */
class T_DLLCLASS RenderSystemDx10 : public RenderSystem
{
	T_RTTI_CLASS(RenderSystemDx10)

public:
	virtual bool create();

	virtual void destroy();

	virtual int getDisplayModeCount() const;
	
	virtual DisplayMode* getDisplayMode(int index);
	
	virtual DisplayMode* getCurrentDisplayMode();

	virtual bool handleMessages();

	virtual RenderView* createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc);

	virtual RenderView* createRenderView(void* windowHandle, const RenderViewCreateDesc& desc);

	virtual VertexBuffer* createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual IndexBuffer* createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual SimpleTexture* createSimpleTexture(const SimpleTextureCreateDesc& desc);

	virtual CubeTexture* createCubeTexture(const CubeTextureCreateDesc& desc);
	
	virtual VolumeTexture* createVolumeTexture(const VolumeTextureCreateDesc& desc);

	virtual RenderTargetSet* createRenderTargetSet(const RenderTargetSetCreateDesc& desc);

	virtual ProgramResource* compileProgram(const ShaderGraph* shaderGraph, int optimize, bool validate);

	virtual Program* createProgram(const ProgramResource* programResource);

private:
	Ref< ContextDx10 > m_context;
	ComRef< ID3D10Device > m_d3dDevice;
	ComRef< IDXGIFactory > m_dxgiFactory;
	AutoArrayPtr< DXGI_MODE_DESC > m_dxgiDisplayModes;
	RefArray< DisplayMode > m_displayModes;
	HWND m_hWnd;

	static LRESULT wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

	}
}

#endif	// traktor_render_RenderSystemDx10_H
