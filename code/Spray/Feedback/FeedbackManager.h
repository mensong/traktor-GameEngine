#ifndef traktor_spray_FeedbackManager_H
#define traktor_spray_FeedbackManager_H

#include <map>
#include <vector>
#include "Spray/Feedback/IFeedbackManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class T_DLLCLASS FeedbackManager : public IFeedbackManager
{
	T_RTTI_CLASS;

public:
	virtual void addListener(FeedbackType type, IFeedbackListener* listener);

	virtual void removeListener(FeedbackType type, IFeedbackListener* listener);

	virtual void apply(FeedbackType type, const float* values, int32_t count);

private:
	std::map< FeedbackType, std::vector< IFeedbackListener* > > m_listeners;
};

	}
}

#endif	// traktor_spray_FeedbackManager_H
