// #include <cstring>
// #include <limits>
// #include "Core/Log/Log.h"
// #include "Core/Math/Half.h"
// #include "Core/Misc/String.h"
// #include "Mesh/Editor/IndexRange.h"
// #include "Mesh/Editor/MeshVertexWriter.h"
#include "Mesh/Editor/Proc/ProcMeshConverter.h"
#include "Mesh/Proc/ProcMeshResource.h"
// #include "Model/Model.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/FlattenDoubleSided.h"
#include "Model/Operations/SortCacheCoherency.h"
#include "Model/Operations/SortProjectedArea.h"
#include "Model/Operations/Triangulate.h"

namespace traktor
{
	namespace mesh
	{

Ref< IMeshResource > ProcMeshConverter::createResource() const
{
	return new ProcMeshResource();
}

bool ProcMeshConverter::getOperations(const MeshAsset* meshAsset, RefArray< const model::IModelOperation >& outOperations) const
{
	outOperations.reserve(5);
	outOperations.push_back(new model::Triangulate());
	outOperations.push_back(new model::SortCacheCoherency());
	outOperations.push_back(new model::CalculateTangents(false));
	outOperations.push_back(new model::SortProjectedArea(false));
	outOperations.push_back(new model::FlattenDoubleSided());
	return true;
}

bool ProcMeshConverter::convert(
	const MeshAsset* meshAsset,
	const RefArray< model::Model >& models,
	const Guid& materialGuid,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	const AlignedVector< render::VertexElement >& vertexElements,
	int32_t maxInstanceCount,
	IMeshResource* meshResource,
	IStream* meshResourceStream
) const
{
	const model::Model* model = models[0];
	T_FATAL_ASSERT(model != nullptr);

	return true;
}

	}
}
