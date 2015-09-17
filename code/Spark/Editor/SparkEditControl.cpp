#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Spark/CharacterRenderer.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"
#include "Spark/Editor/SparkEditControl.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Application.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

void drawBound(CharacterInstance* character, render::PrimitiveRenderer* primitiveRenderer)
{
	if (!character)
		return;

	Aabb2 bounds = character->getBounds();

	Matrix33 T = character->getFullTransform();

	Vector2 e[] =
	{
		T * bounds.mn,
		T * Vector2(bounds.mx.x, bounds.mn.y),
		T * bounds.mx,
		T * Vector2(bounds.mn.x, bounds.mx.y)
	};

	float cx = T.e13;
	float cy = T.e23;
	float r = 20.0f;

	primitiveRenderer->drawLine(
		Vector4(cx - r, cy, 1.0f, 1.0f),
		Vector4(cx + r, cy, 1.0f, 1.0f),
		Color4ub(0, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(cx, cy - r, 1.0f, 1.0f),
		Vector4(cx, cy + r, 1.0f, 1.0f),
		Color4ub(0, 255, 0, 255)
	);

	primitiveRenderer->drawLine(
		Vector4(e[0].x, e[0].y, 1.0f, 1.0f),
		Vector4(e[1].x, e[1].y, 1.0f, 1.0f),
		Color4ub(255, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(e[1].x, e[1].y, 1.0f, 1.0f),
		Vector4(e[2].x, e[2].y, 1.0f, 1.0f),
		Color4ub(255, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(e[2].x, e[2].y, 1.0f, 1.0f),
		Vector4(e[3].x, e[3].y, 1.0f, 1.0f),
		Color4ub(255, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(e[3].x, e[3].y, 1.0f, 1.0f),
		Vector4(e[0].x, e[0].y, 1.0f, 1.0f),
		Color4ub(255, 255, 0, 255)
	);

	if (SpriteInstance* sprite = dynamic_type_cast< SpriteInstance* >(character))
	{
		RefArray< CharacterInstance > children;
		sprite->getCharacters(children);

		for (RefArray< CharacterInstance >::const_iterator i = children.begin(); i != children.end(); ++i)
			drawBound(*i, primitiveRenderer);
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SparkEditControl", SparkEditControl, ui::Widget)

SparkEditControl::SparkEditControl(editor::IEditor* editor)
:	m_editor(editor)
,	m_viewOffset(0.0f, 0.0f)
,	m_viewScale(1.0f)
{
}

bool SparkEditControl::create(
	ui::Widget* parent,
	int style,
	db::Database* database,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
{
	if (!Widget::create(parent, style))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 8;
	desc.multiSample = 4;
	desc.waitVBlank = false;
	desc.nativeWindowHandle = getIWidget()->getSystemHandle();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(resourceManager, renderSystem))
		return false;

	m_characterRenderer = new CharacterRenderer();
	m_characterRenderer->create(1);

	addEventHandler< ui::MouseButtonDownEvent >(this, &SparkEditControl::eventMouseButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &SparkEditControl::eventMouseButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &SparkEditControl::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &SparkEditControl::eventMouseWheel);
	addEventHandler< ui::SizeEvent >(this, &SparkEditControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &SparkEditControl::eventPaint);

	m_database = database;
	m_resourceManager = resourceManager;

	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &SparkEditControl::eventIdle);
	return true;
}

void SparkEditControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler< ui::IdleEvent >(m_idleEventHandler);
	safeDestroy(m_characterRenderer);
	safeDestroy(m_primitiveRenderer);
	safeClose(m_renderView);
	Widget::destroy();
}

void SparkEditControl::setSprite(const Sprite* sprite)
{
	m_sprite = sprite;
	m_spriteInstance = checked_type_cast< SpriteInstance* >(sprite->createInstance(0, m_resourceManager));
	update();
}

void SparkEditControl::eventMouseButtonDown(ui::MouseButtonDownEvent* event)
{
	m_lastMousePosition = event->getPosition();
	setCapture();
}

void SparkEditControl::eventMouseButtonUp(ui::MouseButtonUpEvent* event)
{
	releaseCapture();
}

void SparkEditControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	ui::Point mousePosition = event->getPosition();

	Vector2 deltaMove(
		-(mousePosition.x - m_lastMousePosition.x),
		-(mousePosition.y - m_lastMousePosition.y)
	);
	m_viewOffset += deltaMove / m_viewScale;

	m_lastMousePosition = mousePosition;
	update();
}

void SparkEditControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	m_viewScale += event->getRotation() * 0.1f;
	m_viewScale = clamp(m_viewScale, 0.1f, 1000.0f);
	update();
}

void SparkEditControl::eventSize(ui::SizeEvent* event)
{
	ui::Size sz = event->getSize();
	if (m_renderView)
	{
		m_renderView->reset(sz.cx, sz.cy);
		m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
	}
	update();
}

void SparkEditControl::eventPaint(ui::PaintEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = getInnerRect().getSize();

	if (m_renderView->begin(render::EtCyclop))
	{
		const PropertyGroup* settings = m_editor->getSettings();
		T_ASSERT (settings);

		Color4ub clearColor = settings->getProperty< PropertyColor >(L"Editor.Colors/Background");

		float c[4];
		clearColor.getRGBA32F(c);
		const Color4f cc(c);

		m_renderView->clear(
			render::CfColor | render::CfDepth | render::CfStencil,
			&cc,
			1.0f,
			0
		);

		const int32_t stageWidth = 1080;
		const int32_t stageHeight = 1920;

		float viewWidth = sz.cx / m_viewScale;
		float viewHeight = sz.cy / m_viewScale;

		float viewX = m_viewOffset.x / viewWidth;
		float viewY = m_viewOffset.y / viewHeight;

		Matrix44 projection(
			2.0f / viewWidth, 0.0f, 0.0f, -viewX,
			0.0f, -2.0f / viewHeight, 0.0f, viewY,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);

		if (m_primitiveRenderer->begin(m_renderView, projection))
		{
			m_primitiveRenderer->pushDepthState(false, false, false);

			for (int32_t x = 0; x < stageWidth; x += 40)
			{
				m_primitiveRenderer->drawLine(
					Vector4(x, 0.0f, 1.0f, 1.0f),
					Vector4(x, stageHeight, 1.0f, 1.0f),
					Color4ub(0, 0, 0, 40)
				);
			}
			for (int32_t y = 0; y < stageHeight; y += 40)
			{
				m_primitiveRenderer->drawLine(
					Vector4(0.0f, y, 1.0f, 1.0f),
					Vector4(stageWidth, y, 1.0f, 1.0f),
					Color4ub(0, 0, 0, 40)
				);
			}

			m_primitiveRenderer->drawLine(Vector4(0.0f, 0.0f, 1.0f, 1.0f), Vector4(stageWidth, 0.0f, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));
			m_primitiveRenderer->drawLine(Vector4(stageWidth, 0.0f, 1.0f, 1.0f), Vector4(stageWidth, stageHeight, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));
			m_primitiveRenderer->drawLine(Vector4(stageWidth, stageHeight, 1.0f, 1.0f), Vector4(0.0f, stageHeight, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));
			m_primitiveRenderer->drawLine(Vector4(0.0f, stageHeight, 1.0f, 1.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));

			m_primitiveRenderer->popDepthState();
			m_primitiveRenderer->end();
		}

		if (m_spriteInstance)
		{
			m_spriteInstance->update();

			m_characterRenderer->build(m_spriteInstance, 0);
			m_characterRenderer->render(
				m_renderView,
				projection,
				0
			);

			if (m_primitiveRenderer->begin(m_renderView, projection))
			{
				m_primitiveRenderer->pushDepthState(false, false, false);

				drawBound(m_spriteInstance, m_primitiveRenderer);

				m_primitiveRenderer->popDepthState();
				m_primitiveRenderer->end();
			}
		}

		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

void SparkEditControl::eventIdle(ui::IdleEvent* event)
{
	if (isVisible(true))
	{
		update();
		event->requestMore();
	}
}

	}
}
