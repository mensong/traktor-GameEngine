#ifndef traktor_theater_TheaterEditorProfile_H
#define traktor_theater_TheaterEditorProfile_H

#include "Scene/Editor/ISceneEditorProfile.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace theater
	{

/*! \brief
 */
class T_DLLCLASS TheaterEditorProfile : public scene::ISceneEditorProfile
{
	T_RTTI_CLASS;

public:
	virtual void getCommands(
		std::list< ui::Command >& outCommands
	) const;

	virtual void createEditorPlugins(
		scene::SceneEditorContext* context,
		RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
	) const;

	virtual void createResourceFactories(
		scene::SceneEditorContext* context,
		RefArray< const resource::IResourceFactory >& outResourceFactories
	) const;

	virtual void createEntityFactories(
		scene::SceneEditorContext* context,
		RefArray< const world::IEntityFactory >& outEntityFactories
	) const;

	virtual void createEntityRenderers(
		scene::SceneEditorContext* context,
		render::IRenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const;

	virtual void createControllerEditorFactories(
		scene::SceneEditorContext* context,
		RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
	) const;

	virtual void createEntityEditorFactories(
		scene::SceneEditorContext* context,
		RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
	) const;

	virtual Ref< world::EntityData > createEntityData(
		scene::SceneEditorContext* context,
		db::Instance* instance
	) const;
};

	}
}

#endif	// traktor_theater_TheaterEditorProfile_H
