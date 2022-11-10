#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Color4f.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

/*! Palette
 * \ingroup Drawing
 */
class T_DLLCLASS Palette : public Object
{
	T_RTTI_CLASS;

public:
	Palette(int32_t size = 256);

	int32_t getSize() const;

	void set(int32_t index, const Color4f& c);

	const Color4f& get(int32_t index) const;

	int32_t find(const Color4f& c, bool exact = false) const;

private:
	AlignedVector< Color4f > m_colors;
};

}
