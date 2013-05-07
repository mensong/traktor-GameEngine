#ifndef traktor_amalgam_AiClasses_H
#define traktor_amalgam_AiClasses_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class IScriptManager;

	}

	namespace amalgam
	{

void T_DLLCLASS registerAiClasses(script::IScriptManager* scriptManager);

	}
}

#endif	// traktor_amalgam_AiClasses_H
