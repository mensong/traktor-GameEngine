#ifndef traktor_db_StmGetStat_H
#define traktor_db_StmGetStat_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Get stream status.
 * \ingroup Database
 */
class StmGetStat : public IMessage
{
	T_RTTI_CLASS;

public:
	StmGetStat(uint32_t handle = 0);

	uint32_t getHandle() const { return m_handle; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_StmGetStat_H