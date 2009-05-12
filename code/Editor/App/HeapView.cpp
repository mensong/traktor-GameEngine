#include "Editor/App/HeapView.h"
#include "Ui/FloodLayout.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Custom/ProfileControl.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.HeapView", HeapView, ui::Container)

bool HeapView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, gc_new< ui::FloodLayout >()))
		return false;

	m_profileControl = gc_new< ui::custom::ProfileControl >();
	m_profileControl->create(this, 100, 0, 100, ui::WsDoubleBuffer);
	m_profileControl->addDoubleClickEventHandler(ui::createMethodHandler(this, &HeapView::eventDoubleClick));

	return true;
}

void HeapView::destroy()
{
	m_profileControl->destroy();
	ui::Container::destroy();
}

void HeapView::eventDoubleClick(ui::Event* event)
{
	if (checked_type_cast< ui::MouseEvent* >(event)->getButton() == ui::MouseEvent::BtLeft)
		Heap::getInstance().collect();
	else
		Heap::getInstance().dump(log::info);
}

	}
}
