#include "Ui/Command.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Command", Command, Object)

Command::Command()
:	m_flags(CfNone)
,	m_id(0)
{
}

Command::Command(uint32_t id)
:	m_flags(CfId)
,	m_id(id)
{
}

Command::Command(const std::wstring& name)
:	m_flags(CfName)
,	m_id(0)
,	m_name(name)
{
}

Command::Command(uint32_t id, const std::wstring& name)
:	m_flags(CfId | CfName)
,	m_id(id)
,	m_name(name)
{
}

uint32_t Command::getFlags() const
{
	return m_flags;
}

uint32_t Command::getId() const
{
	return m_id;
}

const std::wstring& Command::getName() const
{
	return m_name;
}

bool Command::operator == (const Command& command) const
{
	if (m_flags != command.m_flags)
		return false;

	if ((m_flags & CfId) == CfId && m_id != command.m_id)
		return false;

	if ((m_flags & CfName) == CfName && m_name != command.m_name)
		return false;

	return true;
}

bool Command::operator == (uint32_t id) const
{
	return (m_flags & CfId) == CfId && m_id == id;
}

bool Command::operator == (const std::wstring& name) const
{
	return (m_flags & CfName) == CfName && m_name == name;
}

	}
}
