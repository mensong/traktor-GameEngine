#include "World/WorldRenderView.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderView", WorldRenderView, Object)

WorldRenderView::WorldRenderView()
:	m_index(0)
,	m_projection(Matrix44::identity())
,	m_lastView(Matrix44::identity())
,	m_view(Matrix44::identity())
,	m_viewSize(0.0f, 0.0f)
,	m_time(0.0f)
,	m_deltaTime(0.0f)
,	m_interval(0.0f)
{
}

void WorldRenderView::setIndex(int32_t index)
{
	m_index = index;
}

void WorldRenderView::setOrthogonal(float width, float height, float nearZ, float farZ)
{
	Frustum viewFrustum;
	viewFrustum.buildOrtho(width, height, -farZ, farZ);

	setViewSize(Vector2(width, height));
	setViewFrustum(viewFrustum);
	setCullFrustum(viewFrustum);
	setProjection(orthoLh(width, height, -farZ, farZ));
}

void WorldRenderView::setPerspective(float width, float height, float aspect, float fov, float nearZ, float farZ)
{
	Frustum viewFrustum;
	viewFrustum.buildPerspective(fov, aspect, nearZ, farZ);

	setViewSize(Vector2(width, height));
	setViewFrustum(viewFrustum);
	setCullFrustum(viewFrustum);
	setProjection(perspectiveLh(fov, aspect, nearZ, farZ));
}

void WorldRenderView::setViewFrustum(const Frustum& viewFrustum)
{
	m_viewFrustum = viewFrustum;
}

void WorldRenderView::setCullFrustum(const Frustum& cullFrustum)
{
	m_cullFrustum = cullFrustum;
}

void WorldRenderView::setProjection(const Matrix44& projection)
{
	m_projection = projection;
}

void WorldRenderView::setView(const Matrix44& lastView, const Matrix44& view)
{
	m_lastView = lastView;
	m_view = view;

	// Calculate eye position and direction in world space.
	Matrix44 viewInverse = view.inverse();
	m_eyePosition = viewInverse.translation().xyz1();
	m_eyeDirection = viewInverse.axisZ().xyz0();
}

void WorldRenderView::setViewSize(const Vector2& viewSize)
{
	m_viewSize = viewSize;
}

void WorldRenderView::setTimes(float time, float deltaTime, float interval)
{
	m_time = time;
	m_deltaTime = deltaTime;
	m_interval = interval;
}

void WorldRenderView::addLight(const Light& light)
{
	if (light.castShadow)
		m_lights.insert(m_lights.begin(), light);
	else
		m_lights.push_back(light);
}

void WorldRenderView::resetLights()
{
	m_lights.resize(0);
}

	}
}
