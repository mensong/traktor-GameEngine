#ifndef traktor_mesh_MeshEntityRenderer_H
#define traktor_mesh_MeshEntityRenderer_H

#include <map>
#include "Core/Heap/Ref.h"
#include "World/Entity/EntityRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class MeshEntity;

class T_DLLCLASS MeshEntityRenderer : public world::EntityRenderer
{
	T_RTTI_CLASS(MeshEntityRenderer)

public:
	virtual const TypeSet getEntityTypes() const;

	virtual void render(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView,
		world::Entity* entity
	);

	virtual void flush(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView
	);
};

	}
}

#endif	// traktor_mesh_MeshEntityRenderer_H
