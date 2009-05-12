#ifndef traktor_render_ShaderGraphEditorPage_H
#define traktor_render_ShaderGraphEditorPage_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Editor/EditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
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

	namespace ui
	{

class Event;
class PopupMenu;
class Point;

		namespace custom
		{

class ToolBar;
class GraphControl;
class Node;

		}
	}

	namespace render
	{

class ShaderGraph;
class Node;
class NodeFacade;
class Edge;

class T_DLLEXPORT ShaderGraphEditorPage : public editor::EditorPage
{
	T_RTTI_CLASS(ShaderGraphEditorPage)

public:
	ShaderGraphEditorPage(editor::Editor* editor);

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
	Ref< editor::UndoStack > m_undoStack;
	Ref< ShaderGraph > m_shaderGraph;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::GraphControl > m_editorGraph;
	Ref< ui::PopupMenu > m_menuPopup;
	std::map< const Type*, Ref< NodeFacade > > m_nodeFacades;
	Guid m_fragmentGuid;
	bool m_lastValidationResult;

	void createEditorNodes(const RefArray< Node >& shaderNodes, const RefArray< Edge >& shaderEdges);

	ui::custom::Node* createEditorNode(Node* shaderNode);

	void createNode(const Type* nodeType, const ui::Point& at);

	void updateGraph();

	void checkUpdatedFragments();

	void eventToolClick(ui::Event* event);

	void eventPropertyChange(ui::Event* event);

	void eventButtonDown(ui::Event* event);

	void eventSelect(ui::Event* event);

	void eventNodeMoved(ui::Event* event);

	void eventNodeDoubleClick(ui::Event* event);

	void eventEdgeConnect(ui::Event* event);

	void eventEdgeDisconnect(ui::Event* event);
};

	}
}

#endif	// traktor_render_ShaderGraphEditorPage_H
