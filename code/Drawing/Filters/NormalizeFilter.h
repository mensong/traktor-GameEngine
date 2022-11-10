#pragma once

#include "Drawing/IImageFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

/*! Normalize normal map image filter.
 * \ingroup Drawing
 */
class T_DLLCLASS NormalizeFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	NormalizeFilter() = default;

	explicit NormalizeFilter(float scale);

protected:
	virtual void apply(Image* image) const override final;

private:
	float m_scale = 0.0f;
};

}
