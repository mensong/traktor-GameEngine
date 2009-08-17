#ifndef traktor_scene_ReferencesRenderControl_H
#define traktor_scene_ReferencesRenderControl_H

#include "Core/Heap/Ref.h"
#include "Scene/Editor/ISceneRenderControl.h"

namespace traktor
{
	namespace ui
	{

class Widget;
class Container;

		namespace custom
		{

class GraphControl;

		}
	}

	namespace scene
	{

class SceneEditorContext;

class ReferencesRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS(ReferencesRenderControl)

public:
	ReferencesRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context);

	void destroy();

	virtual void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	virtual bool handleCommand(const ui::Command& command);

	virtual void update();

private:
	Ref< ui::Container > m_container;
	Ref< ui::custom::GraphControl > m_referenceGraph;
};

	}
}

#endif	// traktor_scene_ReferencesRenderControl_H
