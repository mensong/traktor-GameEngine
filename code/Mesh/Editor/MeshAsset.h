#ifndef traktor_mesh_MeshAsset_H
#define traktor_mesh_MeshAsset_H

#include <map>
#include "Core/Guid.h"
#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class T_DLLCLASS MeshAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	enum MeshType
	{
		MtInvalid,
		MtBlend,
		MtIndoor,
		MtInstance,
		MtLod,
		MtPartition,
		MtSkinned,
		MtStatic,
		MtStream
	};

	MeshAsset();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	void setMeshType(MeshType meshType) { m_meshType = meshType; }

	MeshType getMeshType() const { return m_meshType; }

	void setMaterialTemplates(const std::map< std::wstring, Guid >& materialTemplates) { m_materialTemplates = materialTemplates; }

	const std::map< std::wstring, Guid >& getMaterialTemplates() const { return m_materialTemplates; }

	void setMaterialShaders(const std::map< std::wstring, Guid >& materialShaders) { m_materialShaders = materialShaders; }

	const std::map< std::wstring, Guid >& getMaterialShaders() const { return m_materialShaders; }

	void setMaterialTextures(const std::map< std::wstring, Guid >& materialTextures) { m_materialTextures = materialTextures; }

	const std::map< std::wstring, Guid >& getMaterialTextures() const { return m_materialTextures; }

	void setScaleFactor(float scaleFactor) { m_scaleFactor = scaleFactor; }

	float getScaleFactor() const { return m_scaleFactor; }
	
	void setBakeOcclusion(bool bakeOcclusion) { m_bakeOcclusion = bakeOcclusion; }
	
	bool getBakeOcclusion() const { return m_bakeOcclusion; }

	void setCullDistantFaces(bool cullDistantFaces) { m_cullDistantFaces = cullDistantFaces; }

	bool getCullDistantFaces() const { return m_cullDistantFaces; }

	void setLodSteps(int32_t lodSteps) { m_lodSteps = lodSteps; }

	int32_t getLodSteps() const { return m_lodSteps; }

	void setLodMaxDistance(float lodMaxDistance) { m_lodMaxDistance = lodMaxDistance; }

	float getLodMaxDistance() const { return m_lodMaxDistance; }

	void setLodCullDistance(float lodCullDistance) { m_lodCullDistance = lodCullDistance; }

	float getLodCullDistance() const { return m_lodCullDistance; }

private:
	MeshType m_meshType;
	std::map< std::wstring, Guid > m_materialTemplates;
	std::map< std::wstring, Guid > m_materialShaders;
	std::map< std::wstring, Guid > m_materialTextures;
	float m_scaleFactor;
	bool m_bakeOcclusion;
	bool m_cullDistantFaces;
	int32_t m_lodSteps;
	float m_lodMaxDistance;
	float m_lodCullDistance;
};

	}
}

#endif	// traktor_mesh_MeshAsset_H
