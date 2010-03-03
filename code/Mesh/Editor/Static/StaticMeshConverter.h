#ifndef traktor_mesh_StaticMeshConverter_H
#define traktor_mesh_StaticMeshConverter_H

#include "Mesh/Editor/IMeshConverter.h"

namespace traktor
{
	namespace mesh
	{

class StaticMeshConverter : public IMeshConverter
{
public:
	virtual Ref< IMeshResource > createResource() const;

	virtual bool convert(
		const RefArray< model::Model >& models,
		const std::map< std::wstring, MaterialInfo >& materialInfo,
		const std::vector< render::VertexElement >& vertexElements,
		IMeshResource* meshResource,
		IStream* meshResourceStream
	) const;
};

	}
}

#endif	// traktor_mesh_StaticMeshConverter_H
