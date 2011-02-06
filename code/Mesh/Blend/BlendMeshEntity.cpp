#include "Mesh/Blend/BlendMeshEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.BlendMeshEntity", BlendMeshEntity, MeshEntity)

BlendMeshEntity::BlendMeshEntity(const Transform& transform, const resource::Proxy< BlendMesh >& mesh)
:	MeshEntity(transform)
,	m_mesh(mesh)
{
}

void BlendMeshEntity::setBlendWeights(const std::vector< float >& blendWeights)
{
	size_t copy = std::min(m_blendWeights.size(), blendWeights.size());
	for (size_t i = 0; i < copy; ++i)
		m_blendWeights[i] = blendWeights[i];
}

const std::vector< float >& BlendMeshEntity::getBlendWeights() const
{
	return m_blendWeights;
}

Aabb3 BlendMeshEntity::getBoundingBox() const
{
	return validate() ? m_mesh->getBoundingBox() : Aabb3();
}

bool BlendMeshEntity::supportTechnique(render::handle_t technique) const
{
	return validate() ? m_mesh->supportTechnique(technique) : false;
}

void BlendMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	if (!validate())
		return;

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		m_transform,
		m_instance,
		m_blendWeights,
		distance,
		getParameterCallback()
	);

	m_first = false;
}

bool BlendMeshEntity::validate() const
{
	if (m_mesh.valid() && m_instance)
		return true;

	if (!m_mesh.validate())
		return false;

	m_instance = m_mesh->createInstance();
	if (!m_instance)
		return false;

	uint32_t blendTargetCount = m_mesh->getBlendTargetCount();
	if (blendTargetCount != m_blendWeights.size())
		m_blendWeights.resize(blendTargetCount, 0.0f);
	
	return true;
}

	}
}
