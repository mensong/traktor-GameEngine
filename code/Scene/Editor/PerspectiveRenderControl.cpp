#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Format.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/Settings.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererAdapter.h"
#include "Scene/Editor/FrameEvent.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/ISceneControllerEditor.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/PerspectiveRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/SelectEvent.h"
#include "Ui/Command.h"
#include "Ui/MethodHandler.h"
#include "Ui/Widget.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Itf/IWidget.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderer.h"
#include "World/WorldRenderView.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/Entity.h"
#include "World/PostProcess/PostProcess.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const float c_defaultFieldOfView = 45.0f;
const float c_minFieldOfView = 4.0f;
const float c_maxFieldOfView = 90.0f;
const float c_cameraTranslateDeltaScale = 0.025f;
const float c_cameraRotateDeltaScale = 0.01f;
const float c_deltaAdjust = 0.05f;
const float c_deltaAdjustSmall = 0.01f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.PerspectiveRenderControl", PerspectiveRenderControl, ISceneRenderControl)

PerspectiveRenderControl::PerspectiveRenderControl()
:	m_gridEnable(true)
,	m_guideEnable(true)
,	m_postProcessEnable(true)
,	m_fieldOfView(c_defaultFieldOfView)
,	m_mousePosition(0, 0)
,	m_mouseButton(0)
,	m_modifyCamera(false)
,	m_modifyAlternative(false)
,	m_modifyBegun(false)
,	m_dirtySize(0, 0)
{
}

bool PerspectiveRenderControl::create(ui::Widget* parent, SceneEditorContext* context, int32_t index)
{
	m_context = context;
	T_ASSERT (m_context);

	m_renderWidget = new ui::Widget();
	if (!m_renderWidget->create(parent))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 24;
	desc.stencilBits = 0;
	desc.multiSample = 4;
	desc.waitVBlank = false;
	desc.nativeWindowHandle = m_renderWidget->getIWidget()->getSystemHandle();

	m_renderView = m_context->getRenderSystem()->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem()
	))
		return false;

	m_renderWidget->addButtonDownEventHandler(ui::createMethodHandler(this, &PerspectiveRenderControl::eventButtonDown));
	m_renderWidget->addButtonUpEventHandler(ui::createMethodHandler(this, &PerspectiveRenderControl::eventButtonUp));
	m_renderWidget->addDoubleClickEventHandler(ui::createMethodHandler(this, &PerspectiveRenderControl::eventDoubleClick));
	m_renderWidget->addMouseMoveEventHandler(ui::createMethodHandler(this, &PerspectiveRenderControl::eventMouseMove));
	m_renderWidget->addMouseWheelEventHandler(ui::createMethodHandler(this, &PerspectiveRenderControl::eventMouseWheel));
	m_renderWidget->addSizeEventHandler(ui::createMethodHandler(this, &PerspectiveRenderControl::eventSize));
	m_renderWidget->addPaintEventHandler(ui::createMethodHandler(this, &PerspectiveRenderControl::eventPaint));

	updateSettings();
	updateWorldRenderer();

	m_camera = m_context->getCamera(index);
	m_camera->setEnable(true);
	m_timer.start();

	return true;
}

void PerspectiveRenderControl::destroy()
{
	if (m_camera)
	{
		m_camera->setEnable(false);
		m_camera = 0;
	}

	if (m_worldRenderer)
	{
		m_worldRenderer->destroy();
		m_worldRenderer = 0;
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

	if (m_renderWidget)
	{
		m_renderWidget->destroy();
		m_renderWidget = 0;
	}
}

void PerspectiveRenderControl::updateWorldRenderer()
{
	if (m_worldRenderer)
	{
		m_worldRenderer->destroy();
		m_worldRenderer = 0;
	}

	if (m_postProcess)
	{
		m_postProcess->destroy();
		m_postProcess = 0;
	}

	if (m_renderTarget)
	{
		m_renderTarget->destroy();
		m_renderTarget = 0;
	}

	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance)
		return;

	ui::Size sz = m_renderWidget->getInnerRect().getSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return;

	Ref< const world::WorldRenderSettings > worldRenderSettings = sceneInstance->getWorldRenderSettings();
	Ref< const world::PostProcessSettings > postProcessSettings = sceneInstance->getPostProcessSettings();

	// Create post processing.
	if (postProcessSettings)
	{
		m_postProcess = new world::PostProcess();
		if (!m_postProcess->create(
			postProcessSettings,
			m_context->getResourceManager(),
			m_context->getRenderSystem(),
			sz.cx,
			sz.cy
		))
			m_postProcess = 0;
	}

	// Create entity renderers.
	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	for (RefArray< ISceneEditorProfile >::const_iterator i = m_context->getEditorProfiles().begin(); i != m_context->getEditorProfiles().end(); ++i)
	{
		RefArray< world::IEntityRenderer > entityRenderers;
		(*i)->createEntityRenderers(m_context, m_renderView, m_primitiveRenderer, entityRenderers);
		for (RefArray< world::IEntityRenderer >::iterator j = entityRenderers.begin(); j != entityRenderers.end(); ++j)
		{
			Ref< EntityRendererAdapter > entityRenderer = new EntityRendererAdapter(m_context, *j);
			worldEntityRenderers->add(entityRenderer);
		}
	}

	// Create world renderer.
	m_worldRenderer = new world::WorldRenderer();
	if (m_worldRenderer->create(
		worldRenderSettings,
		worldEntityRenderers,
		m_context->getResourceManager(),
		m_context->getRenderSystem(),
		m_renderView,
		4,
		1
	))
	{
		updateWorldRenderView();

		// Create render target used for post processing.
		if (m_postProcess)
		{
			render::RenderTargetSetCreateDesc desc;
			desc.count = 1;
			desc.width = sz.cx;
			desc.height = sz.cy;
			desc.multiSample = 4;
			desc.depthStencil = false;
			desc.targets[0].format = m_postProcess->requireHighRange() ? render::TfR16G16B16A16F : render::TfR8G8B8A8;
			m_renderTarget =  m_context->getRenderSystem()->createRenderTargetSet(desc);
		}

		// Expose shadow mask to debug view.
		Ref< render::RenderTargetSet > debugTarget = m_worldRenderer->getShadowMaskTargetSet();
		if (debugTarget)
			m_context->setDebugTexture(debugTarget->getColorTexture(0));
		else
			m_context->setDebugTexture(0);
	}
	else
		m_worldRenderer = 0;
}

bool PerspectiveRenderControl::handleCommand(const ui::Command& command)
{
	bool result = false;

	if (command == L"Editor.SettingsChanged")
		updateSettings();
	else if (command == L"Scene.Editor.EnableGrid")
		m_gridEnable = true;
	else if (command == L"Scene.Editor.DisableGrid")
		m_gridEnable = false;
	else if (command == L"Scene.Editor.EnableGuide")
		m_guideEnable = true;
	else if (command == L"Scene.Editor.DisableGuide")
		m_guideEnable = false;
	else if (command == L"Scene.Editor.EnablePostProcess")
		m_postProcessEnable = true;
	else if (command == L"Scene.Editor.DisablePostProcess")
		m_postProcessEnable = false;

	return result;
}

void PerspectiveRenderControl::update()
{
	m_renderWidget->update();
}

void PerspectiveRenderControl::updateSettings()
{
	Ref< PropertyGroup > colors = m_context->getEditor()->getSettings()->getProperty< PropertyGroup >(L"Editor.Colors");
	m_colorClear = colors->getProperty< PropertyColor >(L"Background");
	m_colorGrid = colors->getProperty< PropertyColor >(L"Grid");
	m_colorRef = colors->getProperty< PropertyColor >(L"ReferenceEdge");
}

void PerspectiveRenderControl::updateWorldRenderView()
{
	ui::Size sz = m_renderWidget->getInnerRect().getSize();

	world::WorldViewPerspective worldView;
	worldView.width = sz.cx;
	worldView.height = sz.cy;
	worldView.aspect = float(sz.cx) / sz.cy;
	worldView.fov = deg2rad(m_fieldOfView);
	m_worldRenderer->createRenderView(worldView, m_worldRenderView);
}

Ref< EntityAdapter > PerspectiveRenderControl::pickEntity(const ui::Point& position) const
{
	Frustum viewFrustum = m_worldRenderView.getViewFrustum();
	ui::Rect innerRect = m_renderWidget->getInnerRect();

	Scalar fx(float(position.x) / innerRect.getWidth());
	Scalar fy(float(position.y) / innerRect.getHeight());

	// Interpolate frustum edges to find view pick-ray.
	const Vector4& viewEdgeTopLeft = viewFrustum.corners[4];
	const Vector4& viewEdgeTopRight = viewFrustum.corners[5];
	const Vector4& viewEdgeBottomLeft = viewFrustum.corners[7];
	const Vector4& viewEdgeBottomRight = viewFrustum.corners[6];

	Vector4 viewEdgeTop = lerp(viewEdgeTopLeft, viewEdgeTopRight, fx);
	Vector4 viewEdgeBottom = lerp(viewEdgeBottomLeft, viewEdgeBottomRight, fx);
	Vector4 viewRayDirection = lerp(viewEdgeTop, viewEdgeBottom, fy).normalized().xyz0();

	// Transform ray into world space.
	Matrix44 viewInv = m_worldRenderView.getView().inverseOrtho();
	Vector4 worldRayOrigin = viewInv.translation().xyz1();
	Vector4 worldRayDirection = viewInv * viewRayDirection;

	return m_context->queryRay(worldRayOrigin, worldRayDirection);
}

Matrix44 PerspectiveRenderControl::getView() const
{
	Matrix44 view = m_camera->getCurrentView();
	Ref< EntityAdapter > followEntityAdapter = m_context->getFollowEntityAdapter();
	if (followEntityAdapter)
		return followEntityAdapter->getTransform().inverse().toMatrix44();
	else
		return m_camera->getCurrentView();
}

void PerspectiveRenderControl::eventButtonDown(ui::Event* event)
{
	m_mousePosition = checked_type_cast< ui::MouseEvent* >(event)->getPosition();
	m_modifyCamera = (event->getKeyState() & ui::KsControl) == ui::KsControl;
	m_modifyAlternative = (event->getKeyState() & ui::KsMenu) == ui::KsMenu;

	// Are we already captured then we abort.
	if (m_renderWidget->hasCapture())
		return;

	if (m_modifyCamera)
	{
		m_renderWidget->setCapture();
	}
	else
	{
		// Handle entity picking if enabled.
		if (m_context->getPickEnable())
		{
			Ref< EntityAdapter > entityAdapter = pickEntity(m_mousePosition);
			
			// De-select all other if shift isn't held.
			if ((event->getKeyState() & ui::KsShift) == 0)
				m_context->selectAllEntities(false);

			m_context->selectEntity(entityAdapter);
			m_context->raiseSelect(this);
		}

		m_context->setPlaying(false);
		m_context->setPhysicsEnable(false);

		m_modifyBegun = false;
		m_renderWidget->setCapture();
	}

	m_renderWidget->setFocus();
}

void PerspectiveRenderControl::eventButtonUp(ui::Event* event)
{
	// Issue finished modification event.
	if (!m_modifyCamera)
	{
		if (m_modifyBegun)
		{
			m_context->raisePostModify();

			// Leave modify mode in entity editors.
			for (RefArray< EntityAdapter >::iterator i = m_modifyEntities.begin(); i != m_modifyEntities.end(); ++i)
			{
				if ((*i)->getEntityEditor())
					(*i)->getEntityEditor()->endModifier(
						m_context,
						*i
					);
			}
		}
	}

	m_modifyEntities.resize(0);
	m_modifyCamera = false;
	m_modifyAlternative = false;
	m_modifyBegun = false;

	if (m_renderWidget->hasCapture())
		m_renderWidget->releaseCapture();
}

void PerspectiveRenderControl::eventDoubleClick(ui::Event* event)
{
	ui::Point mousePosition = checked_type_cast< ui::MouseEvent* >(event)->getPosition();
	
	Ref< EntityAdapter > entityAdapter = pickEntity(mousePosition);
	if (entityAdapter && entityAdapter->isExternal())
	{
		Guid externalGuid;
		entityAdapter->getExternalGuid(externalGuid);

		Ref< db::Instance > instance = m_context->getEditor()->getSourceDatabase()->getInstance(externalGuid);
		if (instance)
			m_context->getEditor()->openEditor(instance);
	}
}

void PerspectiveRenderControl::eventMouseMove(ui::Event* event)
{
	if (!m_renderWidget->hasCapture())
		return;

	int32_t mouseButton = (static_cast< ui::MouseEvent* >(event)->getButton() == ui::MouseEvent::BtLeft) ? 0 : 1;
	ui::Point mousePosition = checked_type_cast< ui::MouseEvent* >(event)->getPosition();

	Vector4 screenDelta(
		float(m_mousePosition.x - mousePosition.x),
		float(m_mousePosition.y - mousePosition.y),
		0.0f,
		0.0f
	);

	if (!m_modifyCamera)
	{
		if (!m_modifyBegun)
		{
			// Clone selection set.
			if (m_modifyAlternative)
				m_context->cloneSelected();

			// Get selected entities.
			m_context->getEntities(
				m_modifyEntities,
				SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants
			);

			// Enter modify mode in entity editors.
			for (RefArray< EntityAdapter >::iterator i = m_modifyEntities.begin(); i != m_modifyEntities.end(); ++i)
			{
				if ((*i)->getEntityEditor())
					(*i)->getEntityEditor()->beginModifier(
					m_context,
					*i
				);
			}

			// Issue begin modification event.
			m_context->raisePreModify();

			m_modifyBegun = true;
		}

		// Apply modifier on selected entities.
		Ref< IModifier > modifier = m_context->getModifier();
		T_ASSERT (modifier);

		Matrix44 projection = m_worldRenderView.getProjection();
		Matrix44 projectionInverse = projection.inverse();

		Matrix44 view = getView();
		Matrix44 viewInverse = view.inverse();

		ui::Rect innerRect = m_renderWidget->getInnerRect();
		Vector4 clipDelta = projectionInverse * (screenDelta * Vector4(-2.0f / innerRect.getWidth(), 2.0f / innerRect.getHeight(), 0.0f, 0.0f));

		for (RefArray< EntityAdapter >::iterator i = m_modifyEntities.begin(); i != m_modifyEntities.end(); ++i)
		{
			Ref< IEntityEditor > entityEditor = (*i)->getEntityEditor();
			if (entityEditor)
			{
				// Transform screen delta into world delta at entity's position.
				Vector4 viewPosition = view * (*i)->getTransform().translation().xyz1();
				Vector4 viewDelta = clipDelta * viewPosition.z();
				Vector4 worldDelta = viewInverse * viewDelta;

				// Apply modifier through entity editor.
				entityEditor->applyModifier(
					m_context,
					*i,
					view,
					screenDelta,
					viewDelta,
					worldDelta,
					mouseButton
				);
			}
		}
	}
	else
	{
		if (mouseButton == 0)
		{
			screenDelta *= Scalar(c_cameraTranslateDeltaScale);
			if (m_modifyAlternative)
				m_camera->move(screenDelta.shuffle< 0, 1, 2, 3 >());
			else
				m_camera->move(screenDelta.shuffle< 0, 2, 1, 3 >());
		}
		else
		{
			screenDelta *= Scalar(c_cameraRotateDeltaScale);
			m_camera->rotate(screenDelta.y(), screenDelta.x());
		}
	}

	m_mousePosition = mousePosition;
	m_mouseButton = mouseButton;

	m_renderWidget->update();
}

void PerspectiveRenderControl::eventMouseWheel(ui::Event* event)
{
	int rotation = static_cast< ui::MouseEvent* >(event)->getWheelRotation();

	const float delta = 1.0f;

	if (m_context->getEditor()->getSettings()->getProperty(L"SceneEditor.InvertMouseWheel"))
		m_fieldOfView -= rotation * delta;
	else
		m_fieldOfView += rotation * delta;

	m_fieldOfView = max(m_fieldOfView, c_minFieldOfView);
	m_fieldOfView = min(m_fieldOfView, c_maxFieldOfView);

	updateWorldRenderView();
}

void PerspectiveRenderControl::eventSize(ui::Event* event)
{
	if (!m_renderView || !m_renderWidget->isVisible(true))
		return;

	ui::SizeEvent* s = static_cast< ui::SizeEvent* >(event);
	ui::Size sz = s->getSize();

	// Don't update world renderer if, in fact, size hasn't changed.
	if (sz.cx == m_dirtySize.cx && sz.cy == m_dirtySize.cy)
		return;

	m_renderView->resize(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));

	updateWorldRenderer();

	m_dirtySize = sz;
}

void PerspectiveRenderControl::eventPaint(ui::Event* event)
{
	float colorClear[4];
	float deltaTime = float(m_timer.getDeltaTime());
	float scaledTime = m_context->getTime();

	// Convert color to fp.
	m_colorClear.getRGBA32F(colorClear);

	// Update camera.
	m_camera->update(deltaTime);

	if (!m_renderView || !m_primitiveRenderer || !m_worldRenderer)
		return;

	Matrix44 view = getView();

	// Get entities.
	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters);

	// Get root entity.
	Ref< EntityAdapter > rootEntityAdapter = m_context->getRootEntityAdapter();
	Ref< world::Entity > rootEntity = rootEntityAdapter ? rootEntityAdapter->getEntity() : 0;

	// Render world.
	if (m_renderView->begin())
	{
		m_renderView->clear(
			render::CfColor | render::CfDepth,
			colorClear,
			1.0f,
			128
		);

		m_primitiveRenderer->begin(m_renderView);
		m_primitiveRenderer->setClipDistance(m_worldRenderView.getViewFrustum().getNearZ());
		m_primitiveRenderer->pushProjection(m_worldRenderView.getProjection());
		m_primitiveRenderer->pushView(view);

		// Render XZ grid.
		if (m_gridEnable)
		{
			Vector4 viewPosition = view.inverse().translation();
			float vx = floorf(viewPosition.x());
			float vz = floorf(viewPosition.z());

			for (int x = -20; x <= 20; ++x)
			{
				float fx = float(x);
				m_primitiveRenderer->drawLine(
					Vector4(fx + vx, 0.0f, -20.0f + vz, 1.0f),
					Vector4(fx + vx, 0.0f, 20.0f + vz, 1.0f),
					(int(fx + vx) == 0) ? 2.0f : 0.0f,
					m_colorGrid
				);
				m_primitiveRenderer->drawLine(
					Vector4(-20.0f + vx, 0.0f, fx + vz, 1.0f),
					Vector4(20.0f + vx, 0.0f, fx + vz, 1.0f),
					(int(fx + vz) == 0) ? 2.0f : 0.0f,
					m_colorGrid
				);
			}
		}

		// Draw selection marker(s).
		Ref< IModifier > modifier = m_context->getModifier();
		for (RefArray< EntityAdapter >::const_iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
		{
			// Draw modifier and reference arrows; only applicable to spatial entities we must first check if it's a spatial entity.
			if (modifier && (*i)->isSpatial() && (*i)->isSelected() && !(*i)->isChildOfExternal())
			{
				modifier->draw(
					m_context,
					view,
					(*i)->getTransform(),
					m_primitiveRenderer,
					m_mouseButton
				);
			}

			// Draw entity guides.
			if (m_guideEnable)
				m_context->drawGuide(m_primitiveRenderer, *i);
		}

		// Draw controller guides.
		Ref< ISceneControllerEditor > controllerEditor = m_context->getControllerEditor();
		if (controllerEditor)
			controllerEditor->draw(
				m_primitiveRenderer
			);

		// Render entities.
		m_worldRenderView.setTimes(scaledTime, deltaTime, 0.0f);
		m_worldRenderView.setView(view);

		if (rootEntity)
		{
			m_worldRenderer->build(m_worldRenderView, rootEntity, 0);
			m_worldRenderer->render(world::WrfDepthMap | world::WrfVelocityMap | world::WrfShadowMap, 0);

			if (m_postProcessEnable && m_renderTarget)
			{
				m_renderView->begin(m_renderTarget, 0, true);
				m_renderView->clear(
					render::CfColor,
					colorClear,
					1.0f,
					128
				);
			}

			m_worldRenderer->render(world::WrfVisualOpaque | world::WrfVisualAlphaBlend, 0);

			if (m_postProcessEnable && m_renderTarget)
			{
				m_renderView->end();

				world::PostProcessStep::Instance::RenderParams params;

				params.viewFrustum = m_worldRenderView.getViewFrustum();
				params.projection = m_worldRenderView.getProjection();
				params.shadowFarZ = m_worldRenderer->getSettings().shadowFarZ;
				params.shadowMapBias = m_worldRenderer->getSettings().shadowMapBias;
				params.deltaTime = deltaTime;				

				m_postProcess->render(
					m_renderView,
					m_renderTarget,
					m_worldRenderer->getDepthTargetSet(),
					m_worldRenderer->getVelocityTargetSet(),
					m_worldRenderer->getShadowMaskTargetSet(),
					params
				);
			}

			m_worldRenderer->flush(0);
		}

		m_primitiveRenderer->end(m_renderView);

		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

	}
}
