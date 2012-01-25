#include <ctime>
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Spray/Effect.h"
#include "Spray/EffectLayer.h"
#include "Spray/EffectInstance.h"
#include "Spray/Emitter.h"
#include "Spray/PointRenderer.h"
#include "Spray/Editor/BoxSourceRenderer.h"
#include "Spray/Editor/ConeSourceRenderer.h"
#include "Spray/Editor/DiscSourceRenderer.h"
#include "Spray/Editor/EffectPreviewControl.h"
#include "Spray/Editor/PointSourceRenderer.h"
#include "Spray/Editor/PointSetSourceRenderer.h"
#include "Spray/Editor/QuadSourceRenderer.h"
#include "Spray/Editor/SphereSourceRenderer.h"
#include "Spray/Editor/SplineSourceRenderer.h"
#include "Spray/Sources/BoxSource.h"
#include "Spray/Sources/ConeSource.h"
#include "Spray/Sources/DiscSource.h"
#include "Spray/Sources/PointSource.h"
#include "Spray/Sources/PointSetSource.h"
#include "Spray/Sources/QuadSource.h"
#include "Spray/Sources/SphereSource.h"
#include "Spray/Sources/SplineSource.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Application.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/IdleEvent.h"
#include "World/WorldRenderView.h"
#include "World/Forward/WorldRenderPassForward.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

const uint32_t c_initialRandomSeed = 5489UL;
const int c_updateInterval = 30;
const float c_deltaMoveScale = 0.025f;
const float c_deltaScaleHead = 0.015f;
const float c_deltaScalePitch = 0.005f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectPreviewControl", EffectPreviewControl, ui::Widget)

EffectPreviewControl::EffectPreviewControl()
:	m_randomSeed(c_initialRandomSeed)
,	m_effectPosition(0.0f, -2.0f, 7.0f, 1.0f)
,	m_angleHead(0.0f)
,	m_anglePitch(0.0f)
,	m_timeScale(1.0f)
,	m_lastDeltaTime(1.0f / c_updateInterval)
,	m_guideVisible(true)
,	m_velocityVisible(false)
,	m_moveEmitter(false)
{
	m_context.deltaTime = 0.0f;
	m_context.random = RandomGeometry(c_initialRandomSeed);
	m_context.soundSystem = 0;

	m_sourceRenderers[&type_of< BoxSource >()] = new BoxSourceRenderer();
	m_sourceRenderers[&type_of< ConeSource >()] = new ConeSourceRenderer();
	m_sourceRenderers[&type_of< DiscSource >()] = new DiscSourceRenderer();
	m_sourceRenderers[&type_of< PointSource >()] = new PointSourceRenderer();
	m_sourceRenderers[&type_of< PointSetSource >()] = new PointSetSourceRenderer();
	m_sourceRenderers[&type_of< QuadSource >()] = new QuadSourceRenderer();
	m_sourceRenderers[&type_of< SphereSource >()] = new SphereSourceRenderer();
	m_sourceRenderers[&type_of< SplineSource >()] = new SplineSourceRenderer();
}

bool EffectPreviewControl::create(
	ui::Widget* parent,
	int style,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	sound::SoundSystem* soundSystem
)
{
	if (!Widget::create(parent, style))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = 4;
	desc.waitVBlank = false;
	desc.nativeWindowHandle = getIWidget()->getSystemHandle();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(resourceManager, renderSystem))
		return false;

	m_renderContext = new render::RenderContext();
	m_soundSystem = soundSystem;

	m_pointRenderer = new PointRenderer(renderSystem);

	addButtonDownEventHandler(ui::createMethodHandler(this, &EffectPreviewControl::eventButtonDown));
	addButtonUpEventHandler(ui::createMethodHandler(this, &EffectPreviewControl::eventButtonUp));
	addMouseMoveEventHandler(ui::createMethodHandler(this, &EffectPreviewControl::eventMouseMove));
	addSizeEventHandler(ui::createMethodHandler(this, &EffectPreviewControl::eventSize));
	addPaintEventHandler(ui::createMethodHandler(this, &EffectPreviewControl::eventPaint));

	m_timer.start();

	// Register our event handler in case of message idle.
	m_idleHandler = ui::createMethodHandler(this, &EffectPreviewControl::eventIdle);
	ui::Application::getInstance()->addEventHandler(ui::EiIdle, m_idleHandler);

	return true;
}

void EffectPreviewControl::destroy()
{
	if (m_idleHandler)
	{
		ui::Application::getInstance()->removeEventHandler(ui::EiIdle, m_idleHandler);
		m_idleHandler = 0;
	}

	if (m_primitiveRenderer)
	{
		m_primitiveRenderer->destroy();
		m_primitiveRenderer = 0;
	}

	if (m_renderView)
	{
		m_renderView->close();
		m_renderView = 0;
	}

	Widget::destroy();
}

void EffectPreviewControl::setEffect(Effect* effect)
{
	m_effect = effect;
	m_effectInstance = m_effect->createInstance();
}

void EffectPreviewControl::setTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}

void EffectPreviewControl::setTotalTime(float totalTime)
{
	if (m_effectInstance)
		m_effectInstance->setTime(totalTime);
}

void EffectPreviewControl::showGuide(bool guideVisible)
{
	m_guideVisible = guideVisible;
}

void EffectPreviewControl::showVelocity(bool velocityVisible)
{
	m_velocityVisible = velocityVisible;
}

void EffectPreviewControl::setMoveEmitter(bool moveEmitter)
{
	m_moveEmitter = moveEmitter;
}

void EffectPreviewControl::randomizeSeed()
{
	m_randomSeed = clock();
	m_context.random = RandomGeometry(m_randomSeed);
}

void EffectPreviewControl::syncEffect()
{
	Context syncContext;
	syncContext.deltaTime = 0.0f;
	syncContext.random = RandomGeometry(m_randomSeed);
	syncContext.soundSystem = 0;

	float currentTime = m_effectInstance->getTime();

	// Re-create instance.
	m_effectInstance = m_effect->createInstance();

	// Run emitter until total time is reached.
	const float c_deltaTime = 1.0f / 30.0f;
	for (float T = 0.0f; T < currentTime; T += c_deltaTime)
	{
		float deltaTime = min(c_deltaTime, currentTime - T);
		syncContext.deltaTime = deltaTime;

		m_effectInstance->update(syncContext, Transform::identity(), true);
		m_effectInstance->synchronize();
	}

	m_context = syncContext;
}

void EffectPreviewControl::eventButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	m_lastMousePosition = mouseEvent->getPosition();
	setCapture();
}

void EffectPreviewControl::eventButtonUp(ui::Event* event)
{
	releaseCapture();
}

void EffectPreviewControl::eventMouseMove(ui::Event* event)
{
	if (!hasCapture())
		return;

	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	
	if (mouseEvent->getButton() == ui::MouseEvent::BtLeft)
	{
		if ((mouseEvent->getKeyState() & ui::KsControl) == 0)
		{
			// Move X/Y direction.
			float dx = -float(m_lastMousePosition.x - mouseEvent->getPosition().x) * c_deltaMoveScale;
			float dy = -float(m_lastMousePosition.y - mouseEvent->getPosition().y) * c_deltaMoveScale;
			m_effectPosition += Vector4(dx, dy, 0.0f, 0.0f);
		}
		else
		{
			// Move X/Z direction.
			float dx = -float(m_lastMousePosition.x - mouseEvent->getPosition().x) * c_deltaMoveScale;
			float dz = -float(m_lastMousePosition.y - mouseEvent->getPosition().y) * c_deltaMoveScale;
			m_effectPosition += Vector4(dx, 0.0f, dz, 0.0f);
		}
	}
	else
	{
		m_angleHead += float(m_lastMousePosition.x - mouseEvent->getPosition().x) * c_deltaScaleHead;
		m_anglePitch += float(m_lastMousePosition.y - mouseEvent->getPosition().y) * c_deltaScalePitch;
	}

	m_lastMousePosition = mouseEvent->getPosition();

	update();
}

void EffectPreviewControl::eventSize(ui::Event* event)
{
	if (!m_renderView)
		return;

	ui::SizeEvent* s = static_cast< ui::SizeEvent* >(event);
	ui::Size sz = s->getSize();

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
}

void EffectPreviewControl::eventPaint(ui::Event* event)
{
	if (!m_renderView)
		return;

	if (!m_renderView->begin(render::EtCyclop))
		return;

	const float clearColor[] = { 0.2f, 0.2f, 0.2f, 0.0f };
	m_renderView->clear(
		render::CfColor | render::CfDepth,
		clearColor,
		1.0f,
		128
	);

	render::Viewport viewport = m_renderView->getViewport();
	float aspect = float(viewport.width) / viewport.height;

	Matrix44 viewTransform = translate(m_effectPosition) * rotateX(m_anglePitch) * rotateY(m_angleHead);
	Matrix44 projectionTransform = perspectiveLh(
		65.0f * PI / 180.0f,
		aspect,
		0.01f,
		1000.0f
	);

	Matrix44 viewInverse = viewTransform.inverseOrtho();
	Plane cameraPlane(
		viewInverse.axisZ(),
		viewInverse.translation()
	);

	if (m_primitiveRenderer->begin(m_renderView))
	{
		m_primitiveRenderer->pushProjection(projectionTransform);
		m_primitiveRenderer->pushView(viewTransform);

		for (int x = -10; x <= 10; ++x)
		{
			m_primitiveRenderer->drawLine(
				Vector4(float(x), 0.0f, -10.0f, 1.0f),
				Vector4(float(x), 0.0f, 10.0f, 1.0f),
				Color4ub(90, 90, 90)
			);
			m_primitiveRenderer->drawLine(
				Vector4(-10.0f, 0.0f, float(x), 1.0f),
				Vector4(10.0f, 0.0f, float(x), 1.0f),
				Color4ub(90, 90, 90)
			);
		}

		// Draw emitter sources.
		if (m_effect && m_guideVisible)
		{
			const RefArray< EffectLayer >& layers = m_effect->getLayers();
			for (RefArray< EffectLayer >::const_iterator i = layers.begin(); i != layers.end(); ++i)
			{
				Ref< Emitter > emitter = (*i)->getEmitter();
				if (!emitter)
					continue;

				Ref< Source > source = emitter->getSource();
				if (!source)
					continue;

				std::map< const TypeInfo*, Ref< SourceRenderer > >::const_iterator j = m_sourceRenderers.find(&type_of(source));
				if (j != m_sourceRenderers.end())
					j->second->render(m_primitiveRenderer, source);
			}
		}

		m_primitiveRenderer->end(m_renderView);
	}

	if (m_effectInstance)
	{
		float time = float(m_timer.getElapsedTime());
		float deltaTime = float(m_timer.getDeltaTime() * 0.9f + m_lastDeltaTime * 0.1f);

		m_context.deltaTime = deltaTime * m_timeScale;
		m_context.soundSystem = m_soundSystem;

		Transform effectTransform = Transform::identity();
		if (m_moveEmitter)
		{
			Vector4 effectPosition(
				std::sin(time) * 8.0f,
				0.0f,
				std::cos(time) * 8.0f,
				1.0f
			);
			effectTransform = Transform(effectPosition);
		}

		m_effectInstance->update(m_context, effectTransform, true);
		m_effectInstance->synchronize();
		m_effectInstance->render(m_pointRenderer, cameraPlane);

		Frustum viewFrustum;
		viewFrustum.buildPerspective(80.0f * PI / 180.0f, aspect, 0.1f, 2000.0f);

		world::WorldRenderView worldRenderView;
		worldRenderView.setProjection(projectionTransform);
		worldRenderView.setView(viewTransform);
		worldRenderView.setViewSize(Vector2(float(viewport.width), float(viewport.height)));
		worldRenderView.setCullFrustum(viewFrustum);
		worldRenderView.setViewFrustum(viewFrustum);
		worldRenderView.setTimes(time, deltaTime, 0.0f);

		world::Light globalLight;
		globalLight.type = world::LtDirectional;
		globalLight.position = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		globalLight.direction = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
		globalLight.sunColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		globalLight.baseColor = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
		globalLight.shadowColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		globalLight.range = Scalar(0.0f);
		worldRenderView.addLight(globalLight);

		world::WorldRenderPassForward defaultPass(
			render::getParameterHandle(L"World_ForwardColor"),
			worldRenderView,
			0.0f,
			0
		);

		m_pointRenderer->flush(
			m_renderContext,
			defaultPass
		);

		m_renderContext->render(m_renderView, render::RfAll, 0);
		m_renderContext->flush();

		m_lastDeltaTime = deltaTime;
	}

	m_renderView->end();
	m_renderView->present();

	event->consume();
}

void EffectPreviewControl::eventIdle(ui::Event* event)
{
	ui::IdleEvent* idleEvent = checked_type_cast< ui::IdleEvent* >(event);
	if (isVisible(true))
	{
		update();
		idleEvent->requestMore();
	}
}

	}
}
