#include <algorithm>
#include <limits>
#include "Weather/Clouds/CloudEntity.h"
#include "Weather/Clouds/CloudMask.h"
#include "World/WorldRenderer.h"
#include "World/WorldRenderView.h"
#include "World/Entity/EntityUpdate.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/RenderTargetSet.h"
#include "Render/VertexElement.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Context/RenderContext.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Matrix44.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace weather
	{
		namespace
		{

template < typename Comparator >
struct ParticlePredicate
{
	const Vector4& m_center;

	ParticlePredicate(const Vector4& center)
	:	m_center(center)
	{
	}

	bool operator () (const CloudParticle& p1, const CloudParticle& p2) const
	{
		float d1 = (p1.position - m_center).length2();
		float d2 = (p2.position - m_center).length2();
		return Comparator()(d1, d2);
	}
};

struct Vertex
{
	float pos[2];
	float index;
};

const uint32_t c_instanceCount = 16;

void calculateVSQuad(
	const Vector4 viewExtents[8],
	const Frustum& frustum,
	float distance,
	float outMin[2],
	float outMax[2]
)
{
	Scalar nearZ = frustum.getNearZ();

	const Vector4& c1 = frustum.corners[4];
	float d1 = distance / c1.z();
	float clipMinX = c1.x() * d1;
	float clipMaxY = c1.y() * d1;

	const Vector4& c2 = frustum.corners[6];
	float d2 = distance / c2.z();
	float clipMaxX = c2.x() * d2;
	float clipMinY = c2.y() * d2;

	if (clipMinX > clipMaxX)
		std::swap(clipMinX, clipMaxX);
	if (clipMinY > clipMaxY)
		std::swap(clipMinY, clipMaxY);

	outMin[0] = 1e9f; outMax[0] = -1e9f;
	outMin[1] = 1e9f; outMax[1] = -1e9f;

	if (distance <= nearZ)
	{
		outMin[0] = clipMinX;
		outMin[1] = clipMinY;
		outMax[0] = clipMaxX;
		outMax[1] = clipMaxY;
	}
	else
	{
		// Determine if any extent is behind view plane.
		bool infront[8];
		for (int i = 0; i < 8; ++i)
			infront[i] = viewExtents[i].z() >= nearZ;

		// Clip edges with view plane.
		const int* edges = Aabb::getEdges();
		for (int i = 0; i < 12; ++i)
		{
			int i1 = edges[i * 2 + 0];
			int i2 = edges[i * 2 + 1];

			if (infront[i1] != infront[i2])
			{
				const Vector4& e1 = viewExtents[i1];
				const Vector4& e2 = viewExtents[i2];

				Vector4 clip = e1 + (e2 - e1) * (nearZ - e1.z()) / (e2.z() - e1.z());

				float d = distance / clip.z();
				float x = clip.x() * d;
				float y = clip.y() * d;
				outMin[0] = min< float >(outMin[0], x);
				outMin[1] = min< float >(outMin[1], y);
				outMax[0] = max< float >(outMax[0], x);
				outMax[1] = max< float >(outMax[1], y);
			}
		}

		for (int i = 0; i < 8; ++i)
		{
			if (!infront[i])
				continue;

			float d = distance / viewExtents[i].z();
			float x = viewExtents[i].x() * d;
			float y = viewExtents[i].y() * d;
			outMin[0] = min< float >(outMin[0], x);
			outMin[1] = min< float >(outMin[1], y);
			outMax[0] = max< float >(outMax[0], x);
			outMax[1] = max< float >(outMax[1], y);
		}

		if (outMin[0] < clipMinX)
			outMin[0] = clipMinX;
		if (outMin[1] < clipMinY)
			outMin[1] = clipMinY;
		if (outMax[0] > clipMaxX)
			outMax[0] = clipMaxX;
		if (outMax[1] > clipMaxY)
			outMax[1] = clipMaxY;
	}
}

Vector4 colorAsVector4(const Color& color)
{
	return Vector4(
		color.r / 255.0f,
		color.g / 255.0f,
		color.b / 255.0f,
		color.a / 255.0f
	);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.CloudEntity", CloudEntity, world::SpatialEntity)

CloudEntity::CloudEntity()
:	m_handleBillboardView(render::getParameterHandle(L"BillboardView"))
,	m_handleImpostorTarget(render::getParameterHandle(L"ImpostorTarget"))
,	m_timeUntilUpdate(0.0f)
,	m_updateCount(0)
{
}

bool CloudEntity::create(
	render::IRenderSystem* renderSystem,
	const resource::Proxy< render::Shader >& particleShader,
	const resource::Proxy< render::ITexture >& particleTexture,
	const resource::Proxy< render::Shader >& impostorShader,
	const resource::Proxy< CloudMask >& mask,
	uint32_t impostorTargetResolution,
	uint32_t impostorSliceCount,
	uint32_t updateFrequency,
	float updatePositionThreshold,
	float updateDirectionThreshold,
	const CloudParticleData& particleData
)
{
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, pos)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat1, offsetof(Vertex, index)));

	m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, 4 * sizeof(Vertex) * c_instanceCount, false);
	if (!m_vertexBuffer)
		return false;

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	T_ASSERT_M (vertex, L"Unable to lock vertex buffer");

	for (uint32_t i = 0; i < c_instanceCount; ++i)
	{
		vertex->pos[0] = -1.0f; vertex->pos[1] =  1.0f; vertex->index = float(i); vertex++;
		vertex->pos[0] = -1.0f; vertex->pos[1] = -1.0f; vertex->index = float(i); vertex++;
		vertex->pos[0] =  1.0f; vertex->pos[1] =  1.0f; vertex->index = float(i); vertex++;
		vertex->pos[0] =  1.0f; vertex->pos[1] = -1.0f; vertex->index = float(i); vertex++;
	}

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createIndexBuffer(render::ItUInt16, c_instanceCount * 6 * sizeof(uint16_t), false);
	if (!m_indexBuffer)
		return false;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	T_ASSERT_M (index, L"Unable to lock index buffer");

	for (uint32_t i = 0; i < c_instanceCount; ++i)
	{
		index[0] = i * 4 + 0;
		index[1] = i * 4 + 2;
		index[2] = i * 4 + 3;
		index[3] = i * 4 + 0;
		index[4] = i * 4 + 3;
		index[5] = i * 4 + 1;
		index += 6;
	}

	m_indexBuffer->unlock();

	m_impostorTargets.resize(impostorSliceCount);
	for (uint32_t i = 0; i < impostorSliceCount; ++i)
	{
		render::RenderTargetSetCreateDesc targetDesc;

		targetDesc.count = 1;
		targetDesc.width = impostorTargetResolution;
		targetDesc.height = impostorTargetResolution;
		targetDesc.multiSample = 0;
		targetDesc.depthStencil = false;
		targetDesc.targets[0].format = render::TfR8G8B8A8;

		m_impostorTargets[i] = renderSystem->createRenderTargetSet(targetDesc);
		if (!m_impostorTargets[i])
			return false;
	}

	m_impostorSliceCount = impostorSliceCount;
	m_updateFrequency = updateFrequency;
	m_updatePositionThreshold = updatePositionThreshold;
	m_updateDirectionThreshold = updateDirectionThreshold;
	m_particleShader = particleShader;
	m_particleTexture = particleTexture;
	m_impostorShader = impostorShader;
	m_mask = mask;

	if (!m_cluster.create(particleData))
		return false;

	m_particleData = particleData;

	m_transform = Transform::identity();
	m_lastCameraPosition.set(0.0f, 0.0f, 0.0f, 0.0f);

	return true;
}

void CloudEntity::render(render::RenderContext* renderContext, const world::WorldRenderView* worldRenderView, render::PrimitiveRenderer* primitiveRenderer)
{
	// Ensure all proxies are validated.
	if (!m_particleShader.validate() || !m_particleTexture.validate() || !m_impostorShader.validate())
		return;

	// Validate optional mask.
	m_mask.validate();

	renderCluster(renderContext, worldRenderView, primitiveRenderer, m_cluster);
}

void CloudEntity::update(const world::EntityUpdate* update)
{
	m_cluster.update(m_particleData, update->getDeltaTime());
	m_timeUntilUpdate -= update->getDeltaTime();
}

void CloudEntity::renderCluster(
	render::RenderContext* renderContext,
	const world::WorldRenderView* worldRenderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const CloudParticleCluster& cluster
)
{
	if (worldRenderView->getTechnique() != world::WorldRenderer::getTechniqueDefault())
		return;

	render::IRenderView* renderView = renderContext->getRenderView();
	T_ASSERT (renderView);

	const Frustum& viewFrustum = worldRenderView->getViewFrustum();
	const Matrix44& projection = worldRenderView->getProjection();
	const Matrix44& view = worldRenderView->getView();

	Matrix44 worldView = view * m_transform.toMatrix44();
	Vector4 cameraDirection = worldView.inverse().axisZ();
	Vector4 cameraPosition = worldView.inverse().translation();

	float nearZ = viewFrustum.getNearZ();
	float farZ = viewFrustum.getFarZ();

	// Cluster extents in view space.
	const Aabb& clusterBoundingBox = cluster.getBoundingBox();

	Vector4 extents[8];
	clusterBoundingBox.getExtents(extents);

	for (int i = 0; i < 8; ++i)
		extents[i] = worldView * extents[i];

	float minZ = 1e9f;
	float maxZ = -1e9f;
	for (int i = 0; i < 8; ++i)
	{
		minZ = min< float >(minZ, extents[i].z());
		maxZ = max< float >(maxZ, extents[i].z());
	}
	if (maxZ <= nearZ)
		return;

	float sliceDistance = (maxZ - minZ) / m_impostorSliceCount;

	// Render particle cloud onto impostor.
	static Vector4 instanceData1[c_instanceCount], instanceData2[c_instanceCount];

	// Render impostors.
	float cameraPositionDelta = (m_lastCameraPosition - cameraPosition).length();
	float cameraDirectionDelta = rad2deg(acosf(dot3(m_lastCameraDirection, cameraDirection)));
	if (
		m_timeUntilUpdate <= 0.0f ||
		cameraPositionDelta > m_updatePositionThreshold ||
		abs(cameraDirectionDelta) >= m_updateDirectionThreshold
	)
	{
		// Sort all cloud particles back to front.
		AlignedVector< CloudParticle > particles = cluster.getParticles();
		std::sort(particles.begin(), particles.end(), ParticlePredicate< std::greater< float > >(cameraPosition));

		// Sample opacity from mask.
		if (m_mask.valid())
		{
			for (AlignedVector< CloudParticle >::iterator i = particles.begin(); i != particles.end(); ++i)
			{
				Vector4 position = i->position / m_particleData.getSize();

				int32_t x = int32_t((position.x() * 0.5f + 0.5f) * m_mask->getSize());
				int32_t z = int32_t((position.z() * 0.5f + 0.5f) * m_mask->getSize());

				CloudMask::Sample sample = m_mask->getSample(x, z);
				i->opacity = sample.opacity / 255.0f;
				i->radius = i->maxRadius * sample.size / 255.0f;
			}
		}

		// Update slices with different frequency.
		uint32_t lastUpdateSlice = m_impostorSliceCount;
		if (m_timeUntilUpdate <= 0.0f)
		{
			switch (m_updateCount++ % 3)
			{
			case 1:
				lastUpdateSlice = (m_impostorSliceCount + 1) / 2;
				break;
			case 2:
				lastUpdateSlice = (m_impostorSliceCount + 3) / 4;
				break;
			}
		}

		for (uint32_t slice = 0; slice < lastUpdateSlice; ++slice)
		{
			float sliceNearZ = minZ + sliceDistance * (slice + 0.0f);
			float sliceFarZ = minZ + sliceDistance * (slice + 1.0f);

			float minXY[2], maxXY[2];
			calculateVSQuad(extents, viewFrustum, sliceFarZ, minXY, maxXY);

			// Calculate cluster to impostor transform.
			Matrix44 clusterProjection = perspectiveLh(PI / 2.0f, 1.0f, 1.0f, 1000.0f);

			float scaleX = 2.0f * sliceFarZ / (maxXY[0] - minXY[0]);
			float scaleY = 2.0f * sliceFarZ / (maxXY[1] - minXY[1]);

			float offsetX = (maxXY[0] + minXY[0]) / (2.0f * sliceFarZ);
			float offsetY = (maxXY[1] + minXY[1]) / (2.0f * sliceFarZ);

			clusterProjection =
				scale(scaleX, scaleY, 1.0f) *
				translate(-offsetX, -offsetY, 0.0f) *
				clusterProjection;

			// Gather cloud particles in current slice.
			std::vector< const CloudParticle* > sliceParticles;
			for (AlignedVector< CloudParticle >::const_iterator i = particles.begin(); i != particles.end(); ++i)
			{
				const float c_threshold = 0.1f;
				float z = (worldView * i->position).z();
				if (z >= sliceNearZ - c_threshold && z < sliceFarZ + c_threshold)
					sliceParticles.push_back(&(*i));
			}

			Vector4 haloColor = colorAsVector4(m_particleData.getHaloColor());

			render::TargetRenderBlock* targetRenderBlock = renderContext->alloc< render::TargetRenderBlock >();
			targetRenderBlock->type = render::RbtOpaque;
			targetRenderBlock->distance = -std::numeric_limits< float >::max();
			targetRenderBlock->renderTargetSet = m_impostorTargets[slice];
			targetRenderBlock->renderTargetIndex = 0;
			targetRenderBlock->keepDepthStencil = false;
			targetRenderBlock->clearMask = render::CfColor;
			targetRenderBlock->clearColor[0] = haloColor.x();
			targetRenderBlock->clearColor[1] = haloColor.y();
			targetRenderBlock->clearColor[2] = haloColor.z();
			targetRenderBlock->inner = 0;

			render::ChainRenderBlock* chainRenderBlockTail = 0;

			uint32_t sliceParticleCount = uint32_t(sliceParticles.size());
			for (uint32_t i = 0; i < sliceParticleCount; )
			{
				uint32_t instanceCount = std::min(sliceParticleCount - i, c_instanceCount);

				for (uint32_t j = 0; j < instanceCount; ++j)
				{
					instanceData1[j] = sliceParticles[i + j]->position.xyz0() + Vector4(0.0f, 0.0f, 0.0f, sliceParticles[i + j]->opacity);
					instanceData2[j] = Vector4(
						sliceParticles[i + j]->radius,
						sliceParticles[i + j]->rotation,
						(sliceParticles[i + j]->sprite & 1) / 2.0f,
						(sliceParticles[i + j]->sprite >> 1) / 2.0f
					);
				}

				const wchar_t* c_techniquePasses[] = { L"Opacity", L"Default" };
				for (uint32_t pass = 0; pass < sizeof_array(c_techniquePasses); ++pass)
				{
					render::IndexedRenderBlock* particleRenderBlock = renderContext->alloc< render::IndexedRenderBlock >();
					particleRenderBlock->type = render::RbtAlphaBlend;
					particleRenderBlock->distance = 0.0f;
					particleRenderBlock->shader = m_particleShader;
					particleRenderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
					particleRenderBlock->indexBuffer = m_indexBuffer;
					particleRenderBlock->vertexBuffer = m_vertexBuffer;
					particleRenderBlock->primitive = render::PtTriangles;
					particleRenderBlock->offset = 0;
					particleRenderBlock->count = instanceCount * 2;
					particleRenderBlock->minIndex = 0;
					particleRenderBlock->maxIndex = c_instanceCount * 4;

					particleRenderBlock->shaderParams->beginParameters(renderContext);
					worldRenderView->setShaderParameters(particleRenderBlock->shaderParams, m_transform.toMatrix44(), Matrix44::identity(), clusterBoundingBox);
					particleRenderBlock->shaderParams->setTechnique(c_techniquePasses[pass]);
					particleRenderBlock->shaderParams->setFloatParameter(L"ParticleDensity", m_particleData.getDensity());
					particleRenderBlock->shaderParams->setFloatParameter(L"SunInfluence", m_particleData.getSunInfluence());
					particleRenderBlock->shaderParams->setMatrixParameter(L"Projection", clusterProjection);
					particleRenderBlock->shaderParams->setVectorParameter(L"SkyColor", colorAsVector4(m_particleData.getSkyColor()));
					particleRenderBlock->shaderParams->setVectorParameter(L"GroundColor", colorAsVector4(m_particleData.getGroundColor()));
					particleRenderBlock->shaderParams->setVectorArrayParameter(L"InstanceData1", instanceData1, instanceCount);
					particleRenderBlock->shaderParams->setVectorArrayParameter(L"InstanceData2", instanceData2, instanceCount);
					particleRenderBlock->shaderParams->setTextureParameter(L"ParticleTexture", m_particleTexture);
					particleRenderBlock->shaderParams->endParameters(renderContext);

					render::ChainRenderBlock* chainRenderBlock = renderContext->alloc< render::ChainRenderBlock >();
					chainRenderBlock->inner = particleRenderBlock;
					chainRenderBlock->next = 0;

					if (chainRenderBlockTail)
					{
						chainRenderBlockTail->next = chainRenderBlock;
						chainRenderBlockTail = chainRenderBlock;
					}
					else
					{
						targetRenderBlock->inner = chainRenderBlock;
						chainRenderBlockTail = chainRenderBlock;
					}
				}

				i += instanceCount;
			}

			renderContext->draw(targetRenderBlock);
		}

		m_lastCameraPosition = cameraPosition;
		m_lastCameraDirection = cameraDirection;

		m_timeUntilUpdate = 1.0f / m_updateFrequency;
	}

	// Render impostors.
	for (uint32_t slice = 0; slice < m_impostorSliceCount; ++slice)
	{
		float sliceNearZ = minZ + sliceDistance * slice;
		float sliceFarZ = minZ + sliceDistance * (slice + 1.0f);

		if (sliceFarZ < nearZ)
			continue;

		// Clamp slice distance; otherwise it will be clipped.
		if (sliceNearZ < nearZ + FUZZY_EPSILON)
			sliceNearZ = nearZ + FUZZY_EPSILON;

		render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >();

		renderBlock->type = render::RbtAlphaBlend;
		renderBlock->distance = sliceNearZ;
		renderBlock->shader = m_impostorShader;
		renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
		renderBlock->vertexBuffer = m_vertexBuffer;
		renderBlock->primitive = render::PtTriangleStrip;
		renderBlock->offset = 0;
		renderBlock->count = 2;

		float minXY[2], maxXY[2];
		calculateVSQuad(extents, viewFrustum, sliceNearZ, minXY, maxXY);

		Matrix44 billboardView =
			translate((maxXY[0] + minXY[0]) / 2.0f, (maxXY[1] + minXY[1]) / 2.0f, sliceNearZ) *
			scale((maxXY[0] - minXY[0]) / 2.0f, (maxXY[1] - minXY[1]) / 2.0f, 1.0f);

		renderBlock->shaderParams->beginParameters(renderContext);
		worldRenderView->setShaderParameters(renderBlock->shaderParams, m_transform.toMatrix44(), Matrix44::identity(), clusterBoundingBox);
		renderBlock->shaderParams->setMatrixParameter(L"View", billboardView);
		renderBlock->shaderParams->setFloatParameter(L"SliceDistance", sliceDistance);
		renderBlock->shaderParams->setTextureParameter(m_handleImpostorTarget, m_impostorTargets[slice]->getColorTexture(0));
		renderBlock->shaderParams->endParameters(renderContext);

		renderContext->draw(renderBlock);
	}
}

void CloudEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool CloudEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb CloudEntity::getBoundingBox() const
{
	return m_cluster.getBoundingBox();
}

	}
}
