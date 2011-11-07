#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/FlashBitmap.h"
#include "Flash/FlashCanvas.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashShape.h"
#include "Flash/Acc/AccDisplayRenderer.h"
#include "Flash/Acc/AccGlyph.h"
#include "Flash/Acc/AccTextureCache.h"
#include "Flash/Acc/AccShape.h"
#include "Flash/Acc/AccShapeResources.h"
#include "Flash/Acc/AccShapeVertexPool.h"
#include "Flash/Acc/AccQuad.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/Context/RenderContext.h"

#define T_DEBUG_GLYPH_CACHE 0
#define T_FLUSH_CACHE 0

namespace traktor
{
	namespace flash
	{
		namespace
		{

#if T_FLUSH_CACHE
const uint32_t c_maxCacheSize = 64;
const uint32_t c_maxUnusedCount = 40;
#endif
#if TARGET_OS_IPHONE
const uint32_t c_cacheGlyphSize = 64;
#else
const uint32_t c_cacheGlyphSize = 128;
#endif
const uint32_t c_cacheGlyphMargin = 7;
const uint32_t c_cacheGlyphCountX = 16;
const uint32_t c_cacheGlyphCountY = 8;
const uint32_t c_cacheGlyphCount = c_cacheGlyphCountX * c_cacheGlyphCountY;
const uint32_t c_cacheGlyphDimX = c_cacheGlyphSize * c_cacheGlyphCountX;
const uint32_t c_cacheGlyphDimY = c_cacheGlyphSize * c_cacheGlyphCountY;

const SwfCxTransform c_cxfZero = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
const SwfCxTransform c_cxfIdentity = { 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };

bool insideFrameBounds(const FlashMovie& movie, const Matrix33& transform, const SwfRect& bounds)
{
	if (bounds.max.x <= bounds.min.x || bounds.max.y <= bounds.min.y)
		return false;

	const SwfRect& frameBounds = movie.getFrameBounds();

	Vector2 emn = transform * bounds.min;
	Vector2 emx = transform * bounds.max;
	Vector2 xmn(min(emn.x, emx.x), min(emn.y, emx.y));
	Vector2 xmx(max(emn.x, emx.x), max(emn.y, emx.y));

	if (xmn.x > frameBounds.max.x || xmn.y > frameBounds.max.y)
		return false;
	if (xmx.x < frameBounds.min.x || xmx.y < frameBounds.min.y)
		return false;

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AccDisplayRenderer", AccDisplayRenderer, IDisplayRenderer)

AccDisplayRenderer::AccDisplayRenderer()
:	m_vertexPool(0)
,	m_nextIndex(0)
,	m_frameSize(0.0f, 0.0f, 0.0f, 0.0f)
,	m_viewSize(0.0f, 0.0f, 0.0f, 0.0f)
,	m_viewOffset(0.0f, 0.0f, 1.0f, 1.0f)
,	m_aspectRatio(1.0f)
,	m_clearBackground(false)
,	m_stereoscopicOffset(0.0f)
,	m_maskWrite(false)
,	m_maskIncrement(false)
,	m_maskReference(0)
,	m_handleScreenOffset(render::getParameterHandle(L"Flash_ScreenOffset"))
{
}

AccDisplayRenderer::~AccDisplayRenderer()
{
	T_EXCEPTION_GUARD_BEGIN

	destroy();

	T_EXCEPTION_GUARD_END
}

bool AccDisplayRenderer::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	float viewWidth,
	float viewHeight,
	float aspectRatio,
	uint32_t frameCount,
	bool clearBackground,
	float stereoscopicOffset
)
{
	m_resourceManager = resourceManager;
	m_renderSystem = renderSystem;
	m_textureCache = new AccTextureCache(m_renderSystem);
	m_viewSize.set(viewWidth, viewHeight, 1.0f / viewWidth, 1.0f / viewHeight);
	m_aspectRatio = aspectRatio;
	m_clearBackground = clearBackground;
	m_stereoscopicOffset = stereoscopicOffset;

	m_shapeResources = new AccShapeResources();
	if (!m_shapeResources->create(resourceManager))
		return false;

	// Create pool of vertices; one for each frame as we need to cycle through
	// them in order to prevent locking "in-use" vertex buffers.
	if (frameCount > 0)
	{
		m_vertexPools.resize(frameCount);
		for (uint32_t i = 0; i < frameCount; ++i)
		{
			m_vertexPools[i] = new AccShapeVertexPool(renderSystem);
			if (!m_vertexPools[i]->create())
				return false;
		}
	}
	else
	{
		m_vertexPools.resize(1);
		m_vertexPools[0] = new AccShapeVertexPool(renderSystem);
		if (!m_vertexPools[0]->create())
			return false;
	}

	m_glyph = new AccGlyph();
	if (!m_glyph->create(resourceManager, renderSystem))
		return false;

	m_quad = new AccQuad();
	if (!m_quad->create(resourceManager, renderSystem))
		return false;

	render::RenderTargetSetCreateDesc rtscd;
	rtscd.count = 1;
	rtscd.width = c_cacheGlyphDimX;
	rtscd.height = c_cacheGlyphDimY;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = false;
	rtscd.usingPrimaryDepthStencil = false;
	rtscd.targets[0].format = render::TfR8;

	m_renderTargetGlyphs = m_renderSystem->createRenderTargetSet(rtscd);
	if (!m_renderTargetGlyphs)
		return false;

	m_renderContexts.resize(frameCount);
	for (uint32_t i = 0; i < frameCount; ++i)
		m_renderContexts[i] = new render::RenderContext(256 * 1024);

	// Allocate "global" parameter context; as it's reset for each render
	// call this can be fairly small.
	m_globalContext = new render::RenderContext(4096);

	return true;
}

void AccDisplayRenderer::destroy()
{
	m_renderSystem = 0;

	safeDestroy(m_glyph);
	safeDestroy(m_quad);
	safeDestroy(m_textureCache);
	safeDestroy(m_renderTargetGlyphs);

	for (std::map< uint64_t, CacheEntry >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
		safeDestroy(i->second.shape);
	m_shapeCache.clear();

	safeDestroy(m_shapeResources);

	for (RefArray< AccShapeVertexPool >::iterator i = m_vertexPools.begin(); i != m_vertexPools.end(); ++i)
		(*i)->destroy();
	m_vertexPools.clear();

	m_renderContexts.clear();
	m_renderContext = 0;

	m_globalContext = 0;
}

void AccDisplayRenderer::build(uint32_t frame)
{
	m_shapeResources->validate();
	m_renderContext = m_renderContexts[frame];
	m_renderContext->flush();
	m_vertexPool = m_vertexPools[frame];
	m_viewOffset.set(0.0f, 0.0f, 1.0f, 1.0f);

	for (std::map< uint64_t, CacheEntry >::const_iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
		i->second.shape->preBuild();
}

void AccDisplayRenderer::build(render::RenderContext* renderContext, uint32_t frame)
{
	m_renderContext = renderContext;
	m_vertexPool = m_vertexPools[frame];

	for (std::map< uint64_t, CacheEntry >::const_iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
		i->second.shape->preBuild();
}

void AccDisplayRenderer::render(render::IRenderView* renderView, uint32_t frame, render::EyeType eye)
{
	render::ProgramParameters programParams;

	programParams.beginParameters(m_globalContext);
	if (eye == render::EtCyclop)
		programParams.setFloatParameter(m_handleScreenOffset, 0.0f);
	else if (eye == render::EtLeft)
		programParams.setFloatParameter(m_handleScreenOffset, -m_stereoscopicOffset);
	else if (eye == render::EtRight)
		programParams.setFloatParameter(m_handleScreenOffset, m_stereoscopicOffset);
	programParams.endParameters(m_globalContext);

	m_renderContexts[frame]->render(renderView, render::RfOverlay, &programParams);

	m_globalContext->flush();
}

void AccDisplayRenderer::setViewSize(float width, float height)
{
	m_viewSize.set(width, height, 1.0f / width, 1.0f / height);
}

void AccDisplayRenderer::setAspectRatio(float aspectRatio)
{
	m_aspectRatio = aspectRatio;
}

void AccDisplayRenderer::preload(const FlashMovie& movie)
{
	/*
	// Upload all bitmaps to texture cache.
	const std::map< uint16_t, Ref< FlashBitmap > >& bitmaps = movie.getBitmaps();
	for (std::map< uint16_t, Ref< FlashBitmap > >::const_iterator i = bitmaps.begin(); i != bitmaps.end(); ++i)
		m_textureCache->getBitmapTexture(*(i->second));

	const std::map< uint16_t, Ref< FlashCharacter > >& characters = movie.getCharacters();
	for (std::map< uint16_t, Ref< FlashCharacter > >::const_iterator i = characters.begin(); i != characters.end(); ++i)
	{
		if (const FlashShape* shape = dynamic_type_cast< const FlashShape* >(i->second))
		{
			Ref< AccShape > accShape = new AccShape();
			if (accShape->create(
				m_resourceManager,
				m_renderSystem,
				*m_textureCache,
				movie,
				*shape
			))
			{
				uint64_t hash = reinterpret_cast< uint64_t >(shape);
				m_shapeCache[hash].unusedCount = 0;
				m_shapeCache[hash].shape = accShape;
			}
		}
	}
	*/
}

void AccDisplayRenderer::begin(const FlashMovie& movie, const SwfColor& backgroundColor)
{
	const SwfRect& bounds = movie.getFrameBounds();

	m_frameSize = Vector4(
		bounds.min.x,
		bounds.min.y,
		bounds.max.x,
		bounds.max.y
	);

	// Calculate horizontal scale factor to match aspect ratios.
	if (m_aspectRatio > FUZZY_EPSILON)
	{
		float frameAspect = (bounds.max.x - bounds.min.x) / (bounds.max.y - bounds.min.y);
		float scaleX = frameAspect / m_aspectRatio;
		m_viewOffset.set(-(scaleX - 1.0f) / 2.0f, 0.0f, scaleX, 1.0f);
	}

	if (m_clearBackground)
	{
		render::TargetClearRenderBlock* renderBlock = m_renderContext->alloc< render::TargetClearRenderBlock >("Flash clear (color+stencil)");
		renderBlock->clearMask = render::CfColor | render::CfStencil;
		renderBlock->clearColor[0] = backgroundColor.red / 255.0f;
		renderBlock->clearColor[1] = backgroundColor.green / 255.0f;
		renderBlock->clearColor[2] = backgroundColor.blue / 255.0f;
		m_renderContext->draw(render::RfOverlay, renderBlock);
	}
	else
	{
		render::TargetClearRenderBlock* renderBlock = m_renderContext->alloc< render::TargetClearRenderBlock >("Flash clear (stencil)");
		renderBlock->clearMask = render::CfStencil;
		m_renderContext->draw(render::RfOverlay, renderBlock);
	}

	// Flush glyph cache is RT has become invalid.
	if (!m_renderTargetGlyphs->isContentValid())
	{
		m_glyphCache.clear();
		m_nextIndex = 0;
	}

	m_maskWrite = false;
	m_maskIncrement = false;
	m_maskReference = 0;
}

void AccDisplayRenderer::beginMask(bool increment)
{
	m_glyph->render(
		m_renderContext,
		m_frameSize,
		m_viewSize,
		m_viewOffset,
		1.0f,
		m_renderTargetGlyphs->getColorTexture(0),
		m_maskReference
	);

	m_maskWrite = true;
	m_maskIncrement = increment;
}

void AccDisplayRenderer::endMask()
{
	m_glyph->render(
		m_renderContext,
		m_frameSize,
		m_viewSize,
		m_viewOffset,
		1.0f,
		m_renderTargetGlyphs->getColorTexture(0),
		m_maskReference
	);

	m_maskWrite = false;
	if (m_maskIncrement)
	{
		T_ASSERT (m_maskReference < 255);
		m_maskReference++;
	}
	else
	{
		T_ASSERT (m_maskReference > 0);
		m_maskReference--;
	}
}

void AccDisplayRenderer::renderShape(const FlashMovie& movie, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform)
{
	uint64_t hash = reinterpret_cast< uint64_t >(&shape);
	Ref< AccShape > accShape;

	std::map< uint64_t, CacheEntry >::iterator it = m_shapeCache.find(hash);
	if (it == m_shapeCache.end())
	{
		accShape = new AccShape(m_shapeResources, m_vertexPool);
		if (!accShape->createTesselation(shape))
			return;

		m_shapeCache[hash].unusedCount = 0;
		m_shapeCache[hash].shape = accShape;
	}
	else
	{
		it->second.unusedCount = 0;
		accShape = it->second.shape;
	}

	if (!accShape->updateRenderable(
		*m_textureCache,
		movie,
		shape.getFillStyles(),
		shape.getLineStyles()
	))
		return;

	if (!insideFrameBounds(movie, transform, accShape->getBounds()))
		return;

	m_glyph->render(
		m_renderContext,
		m_frameSize,
		m_viewSize,
		m_viewOffset,
		1.0f,
		m_renderTargetGlyphs->getColorTexture(0),
		m_maskReference
	);

	accShape->render(
		m_renderContext,
		transform,
		m_frameSize,
		m_viewSize,
		m_viewOffset,
		1.0f,
		cxform,
		m_maskWrite,
		m_maskIncrement,
		m_maskReference
	);
}

void AccDisplayRenderer::renderMorphShape(const FlashMovie& movie, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform)
{
}

void AccDisplayRenderer::renderGlyph(const FlashMovie& movie, const Matrix33& transform, const FlashShape& shape, const SwfColor& color, const SwfCxTransform& cxform)
{
	uint64_t hash = reinterpret_cast< uint64_t >(&shape);
	Ref< AccShape > accShape;
	int32_t index;

	// Get glyph shape; create if not already cached.
	std::map< uint64_t, CacheEntry >::iterator it1 = m_shapeCache.find(hash);
	if (it1 == m_shapeCache.end())
	{
		accShape = new AccShape(m_shapeResources, m_vertexPool);
		if (!accShape->createTesselation(shape))
			return;

		m_shapeCache[hash].unusedCount = 0;
		m_shapeCache[hash].shape = accShape;
	}
	else
	{
		it1->second.unusedCount = 0;
		accShape = it1->second.shape;
	}

	if (!accShape->updateRenderable(
		*m_textureCache,
		movie,
		shape.getFillStyles(),
		shape.getLineStyles()
	))
		return;

	SwfRect bounds = accShape->getBounds();
	if (!insideFrameBounds(movie, transform, bounds))
		return;

	// Keep 1:1 aspect ratio; use maximum bound dimension.
	float gw = bounds.max.x - bounds.min.x;
	float gh = bounds.max.y - bounds.min.y;
	if (gw >= gh)
		bounds.max.y = bounds.min.y + gw;
	else
		bounds.max.x = bounds.min.x + gh;

	float m = c_cacheGlyphMargin * (bounds.max.x - bounds.min.x) / c_cacheGlyphSize;
	bounds.min.x -= m;
	bounds.min.y -= m;
	bounds.max.x += m;
	bounds.max.y += m;

	// Get cached glyph target.
	std::map< uint64_t, int32_t >::iterator it2 = m_glyphCache.find(hash);
	if (it2 != m_glyphCache.end())
	{
		index = it2->second;
	}
	else
	{
		// Glyph not cached; pick index by cycling which means oldest glyph get discarded.
		index = m_nextIndex++ % c_cacheGlyphCount;
		for (std::map< uint64_t, int32_t >::iterator i = m_glyphCache.begin(); i != m_glyphCache.end(); ++i)
		{
			if (i->second == index)
			{
				m_glyphCache.erase(i);
				break;
			}
		}

		int32_t column = index & (c_cacheGlyphCountX - 1);
		int32_t row = index / c_cacheGlyphCountX;

		render::TargetBeginRenderBlock* renderBlockBegin = m_renderContext->alloc< render::TargetBeginRenderBlock >("Flash glyph render begin");
		renderBlockBegin->renderTargetSet = m_renderTargetGlyphs;
		renderBlockBegin->renderTargetIndex = 0;
		m_renderContext->draw(render::RfOverlay, renderBlockBegin);

		Vector4 frameSize(bounds.min.x, bounds.min.y, bounds.max.x, bounds.max.y);
		Vector4 viewSize(0.0f, 0.0f, 0.0f, 0.0f);
		Vector4 viewOffset(
			float(column) / c_cacheGlyphCountX - 2.0f / c_cacheGlyphDimX,
			float(row) / c_cacheGlyphCountY - 2.0f / c_cacheGlyphDimY,
			1.0f / c_cacheGlyphCountX + 4.0f / c_cacheGlyphDimX,
			1.0f / c_cacheGlyphCountY + 4.0f / c_cacheGlyphDimY
		);

		// Clear previous glyph by drawing a solid quad at it's place.
		m_quad->render(
			m_renderContext,
			bounds,
			Matrix33::identity(),
			frameSize,
			viewSize,
			viewOffset,
			0.0f,
			c_cxfZero,
			0,
			Vector4::zero(),
			0
		);

		// Draw new glyph.
		accShape->render(
			m_renderContext,
			Matrix33::identity(),
			frameSize,
			viewSize,
			viewOffset,
			0.0f,
			c_cxfIdentity,
			false,
			false,
			false
		);

		render::TargetEndRenderBlock* renderBlockEnd = m_renderContext->alloc< render::TargetEndRenderBlock >("Flash glyph render end");
		m_renderContext->draw(render::RfOverlay, renderBlockEnd);

		// Place in cache.
		m_glyphCache[hash] = index;
	}

	// Draw glyph quad.
	SwfCxTransform cxf =
	{
		{ (color.red * cxform.red[0]) / 255.0f + cxform.red[1], 0.0f, },
		{ (color.green * cxform.green[0]) / 255.0f + cxform.green[1], 0.0f },
		{ (color.blue * cxform.blue[0]) / 255.0f + cxform.blue[1], 0.0f },
		{ (color.alpha * cxform.alpha[0]) / 255.0f + cxform.alpha[1], 0.0f }
	};

	float cachePixelDx = 1.0f / c_cacheGlyphDimX;
	float cachePixelDy = 1.0f / c_cacheGlyphDimY;
	int32_t column = index & (c_cacheGlyphCountX - 1);
	int32_t row = index / c_cacheGlyphCountX;

	m_glyph->add(
		bounds,
		transform,
		cxf,
		Vector4(
			float(column) / c_cacheGlyphCountX + cachePixelDx * 2.0f,
			float(row) / c_cacheGlyphCountY + cachePixelDy * 2.0f,
			1.0f / c_cacheGlyphCountX - cachePixelDx * 4.0f,
			1.0f / c_cacheGlyphCountY - cachePixelDy * 4.0f
		)
	);
}

void AccDisplayRenderer::renderCanvas(const FlashMovie& movie, const Matrix33& transform, const FlashCanvas& canvas, const SwfCxTransform& cxform)
{
	uint64_t hash = reinterpret_cast< uint64_t >(&canvas);
	Ref< AccShape > accShape;

	std::map< uint64_t, CacheEntry >::iterator it = m_shapeCache.find(hash);
	if (it == m_shapeCache.end() || it->second.tag != canvas.getTag())
	{
		accShape = new AccShape(m_shapeResources, m_vertexPool);
		if (!accShape->createTesselation(canvas))
			return;

		m_shapeCache[hash].unusedCount = 0;
		m_shapeCache[hash].shape = accShape;
	}
	else
	{
		it->second.unusedCount = 0;
		accShape = it->second.shape;
	}

	if (!accShape->updateRenderable(
		*m_textureCache,
		movie,
		canvas.getFillStyles(),
		canvas.getLineStyles()
	))
		return;

	if (!insideFrameBounds(movie, transform, accShape->getBounds()))
		return;

	m_glyph->render(
		m_renderContext,
		m_frameSize,
		m_viewSize,
		m_viewOffset,
		1.0f,
		m_renderTargetGlyphs->getColorTexture(0),
		m_maskReference
	);

	accShape->render(
		m_renderContext,
		transform,
		m_frameSize,
		m_viewSize,
		m_viewOffset,
		1.0f,
		cxform,
		m_maskWrite,
		m_maskIncrement,
		m_maskReference
	);
}

void AccDisplayRenderer::end()
{
#if T_DEBUG_GLYPH_CACHE
	// Overlay glyph cache.
	const SwfRect bounds = { Vector2(0.0f, 0.0f), Vector2(c_cacheGlyphCount * 1024, 1024) };
	m_quad->render(
		m_renderContext,
		bounds,
		Matrix33::identity(),
		m_frameSize,
		m_viewSize,
		m_viewOffset,
		c_cxfIdentity,
		m_renderTargetGlyphs->getColorTexture(0),
		Vector4(0.0f, 0.0f, 1.0f, 1.0f)
	);
#endif

	m_glyph->render(
		m_renderContext,
		m_frameSize,
		m_viewSize,
		m_viewOffset,
		1.0f,
		m_renderTargetGlyphs->getColorTexture(0),
		m_maskReference
	);

#if T_FLUSH_CACHE
	// Don't flush cache if it doesn't contain that many shapes.
	if (m_shapeCache.size() < c_maxCacheSize)
	{
		// Increment "unused" counter still.
		for (std::map< uint64_t, CacheEntry >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
			i->second.unusedCount++;
		return;
	}

	// Nuke cached shapes which hasn't been used for X number of frames.
	for (std::map< uint64_t, CacheEntry >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); )
	{
		if (i->second.unusedCount++ >= c_maxUnusedCount)
		{
			i->second.shape->destroy();
			m_shapeCache.erase(i++);
		}
		else
			++i;
	}
#endif

	m_renderContext = 0;
}

	}
}
