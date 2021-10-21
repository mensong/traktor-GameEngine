#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class ISimpleTexture;

	}

	namespace spark
	{

class AccBitmapRect;
class FillStyle;

/*! Gradient cache for accelerated rendering.
 * \ingroup Spark
 */
class AccGradientCache : public Object
{
public:
	AccGradientCache(render::IRenderSystem* renderSystem);

	virtual ~AccGradientCache();

	void destroy();

	void clear();

	Ref< AccBitmapRect > getGradientTexture(const FillStyle& style);

	void synchronize();

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::ISimpleTexture > m_gradientsTexture;
	SmallMap< uint64_t, Ref< AccBitmapRect > > m_cache;
	AutoArrayPtr< uint8_t > m_gradientsData;
	uint32_t m_currentGradientColumn = 0;
	uint32_t m_nextGradient = 0;
	bool m_dirty = false;
};

	}
}

