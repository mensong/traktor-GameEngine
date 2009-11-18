#ifndef traktor_db_DbmCreateGroup_H
#define traktor_db_DbmCreateGroup_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Create group.
 * \ingroup Database
 */
class DbmCreateGroup : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmCreateGroup(uint32_t handle = 0, const std::wstring& name = L"");

	uint32_t getHandle() const { return m_handle; }

	const std::wstring& getName() const { return m_name; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_handle;
	std::wstring m_name;
};

	}
}

#endif	// traktor_db_DbmCreateGroup_H
