#include <Ui/MenuItem.h>
#include <Ui/MessageBox.h>
#include <Core/Io/StringOutputStream.h>
#include "App/DroneToolTimer.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.drone.DroneToolTimer", DroneToolTimer, DroneTool)

DroneToolTimer::DroneToolTimer()
:	m_started(false)
,	m_elapsed(0.0)
{
}

void DroneToolTimer::getMenuItems(RefArray< ui::MenuItem >& outItems)
{
	Ref< ui::MenuItem > toggleItem = gc_new< ui::MenuItem >(ui::Command(L"Drone.Timer.Toggle"), L"Start timer");
	toggleItem->setData(L"TOOL", this);
	outItems.push_back(toggleItem);

	Ref< ui::MenuItem > resetItem = gc_new< ui::MenuItem >(ui::Command(L"Drone.Timer.Reset"), L"Reset timer");
	resetItem->setData(L"TOOL", this);
	outItems.push_back(resetItem);
}

bool DroneToolTimer::execute(ui::Widget* parent, ui::MenuItem* menuItem)
{
	T_ASSERT (menuItem);

	if (menuItem->getCommand() == L"Drone.Timer.Toggle")
	{
		if (!m_started)
		{
			m_timer.start();
			m_started = true;
		}
		else
		{
			m_elapsed += m_timer.getElapsedTime();
			m_timer.stop();
			m_started = false;

			int minutes = int(m_elapsed / 60.0) % 60;
			int hours = int(minutes / 60.0);

			StringOutputStream ss;
			ss << hours << L":" << minutes << Endl;

			ui::MessageBox::show(parent, ss.str(), L"Timer", ui::MbOk | ui::MbIconInformation);
		}
		menuItem->setText(m_started ? L"Stop timer" : L"Start timer");
	}
	else if (menuItem->getCommand() == L"Drone.Timer.Reset")
	{
		m_timer.stop();
		if (m_started)
			m_timer.start();

		m_elapsed = 0.0;
	}

	return true;
}

bool DroneToolTimer::serialize(Serializer& s)
{
	return true;
}

	}
}
