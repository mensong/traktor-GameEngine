#ifndef traktor_mesh_IndoorMeshEntity_H
#define traktor_mesh_IndoorMeshEntity_H

#include "Mesh/MeshEntity.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class IndoorMesh;

class T_DLLCLASS IndoorMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	IndoorMeshEntity(const Transform& transform, const resource::Proxy< IndoorMesh >& mesh);

	virtual Aabb getBoundingBox() const;

	virtual void render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance);

private:
	mutable resource::Proxy< IndoorMesh > m_mesh;
};

	}
}

#endif	// traktor_mesh_IndoorMeshEntity_H
