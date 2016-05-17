#ifndef traktor_terrain_EmissiveBrush_H
#define traktor_terrain_EmissiveBrush_H

#include "Terrain/Editor/IBrush.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace terrain
	{

class EmissiveBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	EmissiveBrush(drawing::Image* colorImage);

	virtual uint32_t begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material) T_OVERRIDE T_FINAL;

	virtual void apply(int32_t x, int32_t y) T_OVERRIDE T_FINAL;

	virtual void end(int32_t x, int32_t y) T_OVERRIDE T_FINAL;

	virtual Ref< IBrush > clone() const T_OVERRIDE T_FINAL;

	virtual bool contained() const T_OVERRIDE T_FINAL { return true; }

private:
	Ref< drawing::Image > m_colorImage;
	int32_t m_radius;
	const IFallOff* m_fallOff;
	float m_strength;
};

	}
}

#endif	// traktor_terrain_EmissiveBrush_H
