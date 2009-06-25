#ifndef traktor_db_IProviderBus_H
#define traktor_db_IProviderBus_H

#include "Core/Object.h"
#include "Core/Guid.h"
#include "Database/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief Provider event bus interface.
 * \ingroup Database
 *
 * The bus is used to communicate changes made to other
 * peers which is connected to the same database.
 *
 * The high level database layer puts events when
 * for example an instance has been commits.
 */
class T_DLLCLASS IProviderBus : public Object
{
	T_RTTI_CLASS(IProviderBus)

public:
	virtual bool putEvent(ProviderEvent event, const Guid& eventId) = 0;

	/*! \brief Get event from bus.
	 *
	 * \param outEvent Event type.
	 * \param outEventId Event id, currently guid of affected instance.
	 * \param outRemote True if event originates from another connection; ie. another process.
	 * \return True if event was read from bus, false if no events are available.
	 */
	virtual bool getEvent(ProviderEvent& outEvent, Guid& outEventId, bool& outRemote) = 0;
};

	}
}

#endif	// traktor_db_IProviderBus_H
