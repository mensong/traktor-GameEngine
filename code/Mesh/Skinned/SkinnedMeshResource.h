#ifndef traktor_mesh_SkinnedMeshResource_H
#define traktor_mesh_SkinnedMeshResource_H

#include <map>
#include "Mesh/MeshResource.h"
#include "Core/Guid.h"

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

class T_DLLCLASS SkinnedMeshResource : public MeshResource
{
	T_RTTI_CLASS(SkinnedMeshResource)

public:
	struct Part
	{
		std::wstring name;
		Guid material;

		bool serialize(Serializer& s);
	};

	void setParts(const std::vector< Part >& parts);

	const std::vector< Part >& getParts() const;

	void setBone(const std::wstring& boneName, int boneIndex);

	const std::map< std::wstring, int >& getBoneMap() const;

	virtual bool serialize(Serializer& s);

private:
	std::vector< Part > m_parts;
	std::map< std::wstring, int > m_boneMap;
};

	}
}

#endif	// traktor_mesh_SkinnedMeshResource_H
