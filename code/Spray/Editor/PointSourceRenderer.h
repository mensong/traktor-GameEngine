#ifndef traktor_spray_PointSourceRenderer_H
#define traktor_spray_PointSourceRenderer_H

#include "Spray/Editor/SourceRenderer.h"

namespace traktor
{
	namespace spray
	{

class PointSourceRenderer : public SourceRenderer
{
	T_RTTI_CLASS;

public:
	virtual void render(render::PrimitiveRenderer* primitiveRenderer, Source* source) const;
};

	}
}

#endif	// traktor_spray_PointSourceRenderer_H
