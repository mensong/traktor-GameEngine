#ifndef ImportProject_H
#define ImportProject_H

#include "CustomTool.h"

class ImportProject : public CustomTool
{
	T_RTTI_CLASS;

public:
	bool execute(traktor::ui::Widget* parent, Solution* solution);
};

#endif	// ImportProject_H
