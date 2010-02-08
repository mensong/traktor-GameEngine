#include "Scene/Editor/DefaultEditorProfile.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Core/Serialization/ISerializable.h"
#include "Ui/Command.h"

// Resource factories
#include "Render/TextureFactory.h"
#include "Render/ShaderFactory.h"
#include "World/PostProcess/PostProcessFactory.h"
#include "Mesh/Blend/BlendMeshFactory.h"
#include "Mesh/Indoor/IndoorMeshFactory.h"
#include "Mesh/Instance/InstanceMeshFactory.h"
#include "Mesh/Skinned/SkinnedMeshFactory.h"
#include "Mesh/Static/StaticMeshFactory.h"
#include "Mesh/Stream/StreamMeshFactory.h"
#include "Weather/Clouds/CloudMaskFactory.h"

// Entity factories
#include "Mesh/MeshEntityFactory.h"
#include "Weather/WeatherEntityFactory.h"
#include "World/Entity/WorldEntityFactory.h"

// Entity renderers
#include "World/Entity/LightEntityRenderer.h"
#include "World/Entity/GroupEntityRenderer.h"
#include "Weather/WeatherEntityRenderer.h"
#include "Mesh/MeshEntityRenderer.h"
#include "Mesh/Instance/InstanceMeshEntityRenderer.h"

// Entity editor factories
#include "Scene/Editor/DefaultEntityEditorFactory.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.DefaultEditorProfile", 0, DefaultEditorProfile, ISceneEditorProfile)

void DefaultEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void DefaultEditorProfile::createEditorPlugins(
	SceneEditorContext* context,
	RefArray< ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void DefaultEditorProfile::createResourceFactories(
	SceneEditorContext* context,
	RefArray< resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new render::TextureFactory(context->getResourceDatabase(), context->getRenderSystem(), 0));
	outResourceFactories.push_back(new render::ShaderFactory(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(new world::PostProcessFactory(context->getResourceDatabase()));
	outResourceFactories.push_back(new mesh::BlendMeshFactory(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(new mesh::IndoorMeshFactory(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(new mesh::InstanceMeshFactory(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(new mesh::SkinnedMeshFactory(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(new mesh::StaticMeshFactory(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(new mesh::StreamMeshFactory(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(new weather::CloudMaskFactory(context->getResourceDatabase()));
}

void DefaultEditorProfile::createEntityFactories(
	SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new world::WorldEntityFactory(context->getSourceDatabase()));
	outEntityFactories.push_back(new mesh::MeshEntityFactory(context->getResourceManager()));
	outEntityFactories.push_back(new weather::WeatherEntityFactory(context->getResourceManager(), context->getRenderSystem()));
}

void DefaultEditorProfile::createEntityRenderers(
	SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new world::LightEntityRenderer());
	outEntityRenderers.push_back(new world::GroupEntityRenderer());
	outEntityRenderers.push_back(new weather::WeatherEntityRenderer(primitiveRenderer));
	outEntityRenderers.push_back(new mesh::MeshEntityRenderer());
	outEntityRenderers.push_back(new mesh::InstanceMeshEntityRenderer());
}

void DefaultEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void DefaultEditorProfile::createEntityEditorFactories(
	SceneEditorContext* context,
	RefArray< IEntityEditorFactory >& outEntityEditorFactories
) const
{
	outEntityEditorFactories.push_back(new DefaultEntityEditorFactory());
}

	}
}
