#ifndef traktor_mesh_MeshPipeline_H
#define traktor_mesh_MeshPipeline_H

#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace mesh
	{

class T_DLLCLASS MeshPipeline : public editor::IPipeline
{
	T_RTTI_CLASS;

public:
	MeshPipeline();

	virtual bool create(const editor::IPipelineSettings* settings);

	virtual void destroy();

	virtual TypeInfoSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const;

	virtual bool buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const editor::IPipelineDependencySet* dependencySet,
		const editor::PipelineDependency* dependency,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams,
		uint32_t reason
	) const;

	virtual Ref< ISerializable > buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset
	) const;

private:
	std::wstring m_assetPath;
	bool m_promoteHalf;
	bool m_enableCustomShaders;
	bool m_enableCustomTemplates;
	bool m_enableBakeOcclusion;
	std::set< std::wstring > m_includeOnlyTechniques;
};

	}
}

#endif	// traktor_mesh_MeshPipeline_H
