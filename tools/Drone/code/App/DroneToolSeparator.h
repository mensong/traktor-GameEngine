#ifndef traktor_drone_DroneToolSeparator_H
#define traktor_drone_DroneToolSeparator_H

#include "App/DroneTool.h"

namespace traktor
{
	namespace drone
	{

class DroneToolSeparator : public DroneTool
{
	T_RTTI_CLASS(DroneToolSeparator)

public:
	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems);

	virtual bool execute(ui::Widget* parent, ui::MenuItem* menuItem);

	virtual bool serialize(Serializer& s);
};

	}
}

#endif	// traktor_drone_DroneToolSeparator_H
