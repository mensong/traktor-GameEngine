#ifndef traktor_editor_LogView_H
#define traktor_editor_LogView_H

#include "Core/Heap/Ref.h"
#include "Ui/Container.h"

namespace traktor
{

class LogTarget;

	namespace ui
	{

class PopupMenu;

		namespace custom
		{

class ToolBarButton;
class ToolBar;
class LogList;

		}
	}

	namespace editor
	{

class LogView : public ui::Container
{
	T_RTTI_CLASS(LogView)

public:
	bool create(ui::Widget* parent);

	void destroy();

private:
	Ref< ui::custom::ToolBarButton > m_toolToggleInfo;
	Ref< ui::custom::ToolBarButton > m_toolToggleWarning;
	Ref< ui::custom::ToolBarButton > m_toolToggleError;
	Ref< ui::custom::ToolBar > m_toolFilter;
	Ref< ui::custom::LogList > m_log;
	Ref< ui::PopupMenu > m_popup;
	LogTarget* m_originalTargets[3];
	LogTarget* m_newTargets[3];

	void eventToolClick(ui::Event* event);

	void eventButtonDown(ui::Event* event);
};

	}
}

#endif	// traktor_editor_LogView_H
