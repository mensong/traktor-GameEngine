#ifndef traktor_animation_AnimationEditorPage_H
#define traktor_animation_AnimationEditorPage_H

#include "Core/Heap/Ref.h"
#include "Core/Math/Color.h"
#include "Core/Math/Vector4.h"
#include "Editor/EditorPage.h"
#include "Ui/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class Editor;
class UndoStack;

	}

	namespace db
	{

class Instance;

	}

	namespace ui
	{

class Widget;
class TreeView;
class TreeViewItem;
class PopupMenu;
class Event;

		namespace custom
		{

class ToolBar;
class SequencerControl;

		}
	}

	namespace resource
	{

class ResourceCache;
class ResourceLoader;

	}

	namespace render
	{

class PrimitiveRenderer;

	}

	namespace animation
	{

class Animation;
class Skeleton;
class Pose;

class T_DLLCLASS AnimationEditorPage : public editor::EditorPage
{
	T_RTTI_CLASS(AnimationEditorPage)

public:
	AnimationEditorPage(editor::Editor* editor);

	virtual bool create(ui::Container* parent);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual	bool setDataObject(db::Instance* instance, Object* data);

	virtual Object* getDataObject();

	virtual void propertiesChanged();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(const Guid& eventId);

private:
	editor::Editor* m_editor;
	Ref< Animation > m_animation;
	Ref< Skeleton > m_skeleton;
	Ref< ui::Widget > m_renderWidgets[4];
	Ref< ui::custom::ToolBar > m_toolBarPlay;
	Ref< ui::Container > m_sequencerPanel;
	Ref< ui::custom::SequencerControl > m_sequencer;
	Ref< ui::PopupMenu > m_menuPopup;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< resource::ResourceCache > m_resourceCache;
	Ref< resource::ResourceLoader > m_resourceLoader;
	Ref< editor::UndoStack > m_undoStack;
	ui::Point m_lastMousePosition;
	int m_selectedBone;
	bool m_showGhostTrail;
	bool m_twistLock;
	bool m_ikEnabled;
	float m_relativeTwist;
	bool m_haveRelativeTwist;
	bool m_editMode;
	bool m_previewAnimation;
	Vector4 m_cameraOffsetScale;
	float m_cameraSizeScale;

	void setSkeleton(Skeleton* skeleton);

	bool getSelectedPoseId(int& outPoseId) const;

	void updateRenderWidgets();

	void updateSequencer();

	bool calculateRelativeTwist(int poseIndex, int boneIndex, float& outRelativeTwist) const;

	void drawSkeleton(float time, const Color& defaultColor, const Color& selectedColor, bool drawAxis) const;

	void eventRenderButtonDown(ui::Event* event);

	void eventRenderButtonUp(ui::Event* event);

	void eventRenderMouseMove(ui::Event* event);

	void eventRenderMouseWheel(ui::Event* event);

	void eventRenderSize(ui::Event* event);

	void eventRenderPaint(ui::Event* event);

	void eventSequencerButtonDown(ui::Event* event);

	void eventSequencerCursorMove(ui::Event* event);

	void eventSequencerTimer(ui::Event* event);

	void eventToolClick(ui::Event* event);
};

	}
}

#endif	// traktor_animation_AnimationEditorPage_H
