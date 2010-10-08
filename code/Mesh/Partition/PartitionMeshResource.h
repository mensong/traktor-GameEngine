#ifndef traktor_mesh_PartitionMeshResource_H
#define traktor_mesh_PartitionMeshResource_H

#include "Core/Guid.h"
#include "Mesh/IMeshResource.h"

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

class IPartitionData;

class T_DLLCLASS PartitionMeshResource : public IMeshResource
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Part
	{
		std::wstring shaderTechnique;
		uint32_t meshPart;
		bool opaque;

		Part();

		bool serialize(ISerializer& s);
	};

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const;

	virtual bool serialize(ISerializer& s);

private:
	friend class PartitionMeshConverter;

	Guid m_shader;
	std::list< Part > m_parts;
	Ref< IPartitionData > m_partitionData;
};

	}
}

#endif	// traktor_mesh_PartitionMeshResource_H
