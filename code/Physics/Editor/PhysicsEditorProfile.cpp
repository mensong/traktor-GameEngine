#include "Core/Serialization/ISerializable.h"
#include "Physics/MeshFactory.h"
#include "Physics/Editor/PhysicsEditorProfile.h"
#include "Physics/Editor/PhysicsEntityEditorFactory.h"
#include "Physics/World/EntityFactory.h"
#include "Physics/World/EntityRenderer.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsEditorProfile", 0, PhysicsEditorProfile, scene::ISceneEditorProfile)

void PhysicsEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Physics.ToggleMeshTriangles"));
}

void PhysicsEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void PhysicsEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new MeshFactory(context->getResourceDatabase()));
}

void PhysicsEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new EntityFactory(context->getEntityEventManager(), context->getResourceManager(), context->getPhysicsManager()));
}

void PhysicsEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new EntityRenderer());
}

void PhysicsEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void PhysicsEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
	outEntityEditorFactories.push_back(new PhysicsEntityEditorFactory());
}

Ref< world::EntityData > PhysicsEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return 0;
}

	}
}
