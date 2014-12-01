#ifndef traktor_input_InputMappingEditor_H
#define traktor_input_InputMappingEditor_H

#include <map>
#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IDocument;
class IEditor;
class IEditorPageSite;

	}

	namespace ui
	{

class MouseButtonDownEvent;
class PopupMenu;
class SelectionChangeEvent;

		namespace custom
		{

class EdgeConnectEvent;
class EdgeDisconnectEvent;
class EditList;
class EditListEditEvent;
class GraphControl;
class NodeMovedEvent;
class SelectEvent;

		}
	}

	namespace input
	{

class InputMappingAsset;
class InputNodeTraits;

class T_DLLCLASS InputMappingEditor : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	InputMappingEditor(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	std::map< const TypeInfo*, Ref< const InputNodeTraits > > m_traits;
	Ref< InputMappingAsset > m_mappingAsset;
	Ref< ui::custom::EditList > m_listValueSources;
	Ref< ui::custom::GraphControl > m_graph;
	Ref< ui::PopupMenu > m_menuPopup;

	void updateGraphView();

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventListValueSourceSelect(ui::SelectionChangeEvent* event);

	void eventListValueEdit(ui::custom::EditListEditEvent* event);

	void eventNodeSelect(ui::custom::SelectEvent* event);

	void eventNodeMoved(ui::custom::NodeMovedEvent* event);

	void eventEdgeConnected(ui::custom::EdgeConnectEvent* event);

	void eventEdgeDisconnected(ui::custom::EdgeDisconnectEvent* event);
};

	}
}

#endif	// traktor_input_InputMappingEditor_H
