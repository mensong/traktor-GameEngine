#ifndef AddAggregatesTool_H
#define AddAggregatesTool_H

#include "CustomTool.h"

class AddAggregatesTool : public CustomTool
{
	T_RTTI_CLASS;

public:
	bool execute(traktor::ui::Widget* parent, Solution* solution);
};

#endif	// AddAggregatesTool_H
