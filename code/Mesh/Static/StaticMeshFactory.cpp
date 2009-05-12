#include "Mesh/Static/StaticMeshFactory.h"
#include "Mesh/Static/StaticMeshResource.h"
#include "Mesh/Static/StaticMesh.h"
#include "Render/Mesh/RenderMeshFactory.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Texture.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Io/Stream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMeshFactory", StaticMeshFactory, resource::ResourceFactory)

StaticMeshFactory::StaticMeshFactory(db::Database* database, render::RenderSystem* renderSystem, render::MeshFactory* meshFactory)
:	m_database(database)
,	m_renderSystem(renderSystem)
,	m_meshFactory(meshFactory)
{
	if (!m_meshFactory)
		m_meshFactory = gc_new< render::RenderMeshFactory >(m_renderSystem);
}

const TypeSet StaticMeshFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< StaticMesh >());
	return typeSet;
}

Object* StaticMeshFactory::create(const Type& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
	{
		log::error << L"Static mesh factory failed; no such instance" << Endl;
		return 0;
	}

	Ref< StaticMeshResource > resource = instance->checkout< StaticMeshResource >(db::CfReadOnly);
	if (!resource)
	{
		log::error << L"Static mesh factory failed; unable to checkout resource" << Endl;
		return 0;
	}

	Ref< Stream > dataStream = instance->readData(L"Data");
	if (!dataStream)
	{
		log::error << L"Static mesh factory failed; unable to open data stream" << Endl;
		return 0;
	}

	Ref< render::Mesh > mesh = render::MeshReader(m_meshFactory).read(dataStream);
	if (!mesh)
	{
		log::error << L"Static mesh factory failed; unable to read mesh" << Endl;
		return 0;
	}
	
	dataStream->close();

	const std::vector< StaticMeshResource::Part >& parts = resource->getParts();
	T_ASSERT (parts.size() == mesh->getParts().size());

	Ref< StaticMesh > staticMesh = gc_new< StaticMesh >();
	staticMesh->m_mesh = mesh;
	staticMesh->m_parts.resize(parts.size());

	for (size_t i = 0; i < parts.size(); ++i)
		staticMesh->m_parts[i].material = parts[i].material;

	return staticMesh;
}

	}
}
