#ifndef traktor_terrain_AverageBrush_H
#define traktor_terrain_AverageBrush_H

#include "Resource/Proxy.h"
#include "Terrain/Editor/IBrush.h"

namespace traktor
{
	namespace hf
	{

class Heightfield;

	}

	namespace terrain
	{

class AverageBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	AverageBrush(const resource::Proxy< hf::Heightfield >& heightfield);

	virtual uint32_t begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material) T_OVERRIDE T_FINAL;

	virtual void apply(int32_t x, int32_t y) T_OVERRIDE T_FINAL;

	virtual void end(int32_t x, int32_t y) T_OVERRIDE T_FINAL;

	virtual Ref< IBrush > clone() const T_OVERRIDE T_FINAL;

	virtual bool contained() const  T_OVERRIDE T_FINAL { return true; }

private:
	resource::Proxy< hf::Heightfield > m_heightfield;
	int32_t m_radius;
	const IFallOff* m_fallOff;
	float m_strength;
};

	}
}

#endif	// traktor_terrain_AverageBrush_H
