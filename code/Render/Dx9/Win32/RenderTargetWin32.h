#ifndef traktor_render_RenderTargetWin32_H
#define traktor_render_RenderTargetWin32_H

#include "Render/Texture.h"
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/TextureBaseDx9.h"
#include "Core/Heap/Ref.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX9_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx9;
struct RenderTargetSetCreateDesc;
struct RenderTargetCreateDesc;

class T_DLLCLASS RenderTargetWin32
:	public Texture
,	public TextureBaseDx9
{
	T_RTTI_CLASS(RenderTargetWin32)

public:
	RenderTargetWin32(ContextDx9* context);

	bool create(
		IDirect3DDevice9* d3dDevice,
		const RenderTargetSetCreateDesc& setDesc,
		const RenderTargetCreateDesc& targetDesc
	);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual IDirect3DBaseTexture9* getD3DBaseTexture() const;

	bool resolve(IDirect3DDevice9* d3dDevice);

	void clear(IDirect3DDevice9* d3dDevice, DWORD flags, const float color[4], float z, DWORD stencil);

	inline IDirect3DSurface9* getD3DColorSurface() const {
		return m_d3dTargetSurface;
	}

private:
	Ref< ContextDx9 > m_context;
	int m_width;
	int m_height;
	ComRef< ID3DXEffect > m_d3dClearEffect;
	D3DXHANDLE m_d3dClearTechnique;
	ComRef< IDirect3DTexture9 > m_d3dTargetTexture;
	ComRef< IDirect3DSurface9 > m_d3dTargetSurface;
	ComRef< IDirect3DSurface9 > m_d3dResolveTargetSurface;
};

	}
}

#endif	// traktor_render_RenderTargetWin32_H
