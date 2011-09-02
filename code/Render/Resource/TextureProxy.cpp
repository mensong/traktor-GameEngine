#include "Render/Resource/TextureProxy.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureProxy", TextureProxy, ITexture)

TextureProxy::TextureProxy(const resource::Proxy< ITexture >& texture)
:	m_texture(texture)
{
}

void TextureProxy::destroy()
{
	m_texture.clear();
}

ITexture* TextureProxy::resolve()
{
	return m_texture.validate() ? m_texture->resolve() : 0;
}

	}
}
