#ifndef traktor_flash_WireDisplayRenderer_H
#define traktor_flash_WireDisplayRenderer_H

#include <stack>
#include "Flash/IDisplayRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class IRenderView;
class PrimitiveRenderer;

	}

	namespace resource
	{
	
class IResourceManager;

	}

	namespace flash
	{

/*! \brief Debug wire display renderer.
 * \ingroup Flash
 */
class T_DLLCLASS WireDisplayRenderer : public IDisplayRenderer
{
	T_RTTI_CLASS;

public:
	WireDisplayRenderer(IDisplayRenderer* displayRenderer);

	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		uint32_t frameCount
	);

	void begin(uint32_t frame);

	void end(uint32_t frame);

	void render(render::IRenderView* renderView, uint32_t frame);

	virtual bool wantDirtyRegion() const T_OVERRIDE T_FINAL;
	
	virtual void begin(
		const FlashDictionary& dictionary,
		const Color4f& backgroundColor,
		const Aabb2& frameBounds,
		const Vector4& frameTransform,
		float viewWidth,
		float viewHeight,
		const Aabb2& dirtyRegion
	) T_OVERRIDE T_FINAL;

	virtual void beginSprite(const FlashSpriteInstance& sprite, const Matrix33& transform) T_OVERRIDE T_FINAL;

	virtual void endSprite(const FlashSpriteInstance& sprite, const Matrix33& transform) T_OVERRIDE T_FINAL;

	virtual void beginMask(bool increment) T_OVERRIDE T_FINAL;

	virtual void endMask() T_OVERRIDE T_FINAL;

	virtual void renderShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashShape& shape, const ColorTransform& cxform, uint8_t blendMode) T_OVERRIDE T_FINAL;

	virtual void renderMorphShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashMorphShape& shape, const ColorTransform& cxform) T_OVERRIDE T_FINAL;

	virtual void renderGlyph(const FlashDictionary& dictionary, const Matrix33& transform, const Vector2& fontMaxDimension, const FlashShape& glyphShape, const Color4f& color, const ColorTransform& cxform, uint8_t filter, const Color4f& filterColor) T_OVERRIDE T_FINAL;

	virtual void renderQuad(const Matrix33& transform, const Aabb2& bounds, const ColorTransform& cxform) T_OVERRIDE T_FINAL;

	virtual void renderCanvas(const Matrix33& transform, const FlashCanvas& canvas, const ColorTransform& cxform, uint8_t blendMode) T_OVERRIDE T_FINAL;

	virtual void end() T_OVERRIDE T_FINAL;

private:
	Ref< IDisplayRenderer > m_displayRenderer;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;

	Aabb2 m_frameBounds;
	Vector4 m_frameTransform;
	//int32_t m_wireEnableCount;

	std::stack< bool > m_wireEnable;
};

	}
}

#endif	// traktor_flash_WireDisplayRenderer_H
