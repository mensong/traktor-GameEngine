#ifndef traktor_ui_xtrme_ImageCache_H
#define traktor_ui_xtrme_ImageCache_H

#include <map>
#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class ITexture;

	}

	namespace ui
	{

class IBitmap;

		namespace xtrme
		{

class ImageCache : public Object
{
	T_RTTI_CLASS;

public:
	ImageCache(render::IRenderSystem* renderSystem);

	virtual ~ImageCache();
	
	Ref< render::ITexture > getTexture(IBitmap* bitmap);

	void flush();
	
private:
	struct CachedTexture
	{
		uint32_t touched;
		Ref< render::ITexture > texture;
	};

	Ref< render::IRenderSystem > m_renderSystem;
	std::map< IBitmap*, CachedTexture > m_cache;
};

		}
	}
}

#endif	// traktor_ui_xtrme_ImageCache_H
