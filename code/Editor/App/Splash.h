#ifndef traktor_editor_Splash_H
#define traktor_editor_Splash_H

#include "Ui/ToolForm.h"

namespace traktor
{
	namespace editor
	{

class Splash : public ui::ToolForm
{
	T_RTTI_CLASS(Splash)

public:
	bool create();

private:
	void eventTimer(ui::Event* event);
};

	}
}

#endif	// traktor_editor_Splash_H
