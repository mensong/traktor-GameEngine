#ifndef traktor_db_DbmGetRootGroup_H
#define traktor_db_DbmGetRootGroup_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Get database root group.
 * \ingroup Database
 */
class DbmGetRootGroup : public IMessage
{
	T_RTTI_CLASS;

public:
	virtual bool serialize(ISerializer& s);
};

	}
}

#endif	// traktor_db_DbmGetRootGroup_H
