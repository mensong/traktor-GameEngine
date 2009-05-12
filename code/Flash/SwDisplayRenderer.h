#ifndef traktor_flash_SwDisplayRenderer_H
#define traktor_flash_SwDisplayRenderer_H

#include "Flash/DisplayRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief Software display renderer.
 * \ingroup Flash
 */
class T_DLLCLASS SwDisplayRenderer : public DisplayRenderer
{
	T_RTTI_CLASS(SwDisplayRenderer)

public:
	SwDisplayRenderer();

	void setTransform(const Matrix33& transform);

	void setRasterTarget(void* bits, uint32_t width, uint32_t height, uint32_t pitch);

	virtual void begin(const FlashMovie& movie, const SwfColor& backgroundColor);

	virtual void beginMask(bool increment);

	virtual void endMask();

	virtual void renderShape(const FlashMovie& movie, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform);

	virtual void renderMorphShape(const FlashMovie& movie, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform);

	virtual void renderGlyph(const FlashMovie& movie, const Matrix33& transform, const FlashShape& glyphShape, const SwfColor& color, const SwfCxTransform& cxform);

	virtual void end();

private:
	struct Span
	{
		float x;
		float dx;
		uint16_t fillStyle;
	};

	typedef std::vector< Span > spanline_t;

	Matrix33 m_transform;

	void* m_bits;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_pitch;
	std::vector< spanline_t > m_spanlines;

	void insertSpan(spanline_t& spanline, float x, float dx, uint16_t fillStyle);
};

	}
}

#endif	// traktor_flash_SwDisplayRenderer_H
