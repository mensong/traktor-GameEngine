#ifndef traktor_render_IndexBufferDx11_H
#define traktor_render_IndexBufferDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/IndexBuffer.h"
#include "Render/Dx11/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*!
 * \ingroup DX11
 */
class T_DLLCLASS IndexBufferDx11 : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	virtual void prepare(ID3D11DeviceContext* d3dDeviceContext);

	ID3D11Buffer* getD3D11Buffer() const { return m_d3dBuffer; }

protected:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Buffer > m_d3dBuffer;

	IndexBufferDx11(IndexType indexType, uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_IndexBufferDx11_H
