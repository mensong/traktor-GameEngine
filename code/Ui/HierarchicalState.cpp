#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Ui/HierarchicalState.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ui.HierarchicalState", 0, HierarchicalState, ISerializable)

void HierarchicalState::addState(const std::wstring& path, bool expanded, bool selected)
{
	if (expanded || selected)
		m_states[path] = std::make_pair(expanded, selected);
	else
	{
		std::map< std::wstring, std::pair< bool, bool > >::iterator i = m_states.find(path);
		if (i != m_states.end())
			m_states.erase(i);
	}
}

bool HierarchicalState::getExpanded(const std::wstring& path) const
{
	std::map< std::wstring, std::pair< bool, bool > >::const_iterator i = m_states.find(path);
	return i != m_states.end() ? i->second.first : false;
}

bool HierarchicalState::getSelected(const std::wstring& path) const
{
	std::map< std::wstring, std::pair< bool, bool > >::const_iterator i = m_states.find(path);
	return i != m_states.end() ? i->second.second : false;
}

Ref< HierarchicalState > HierarchicalState::merge(const HierarchicalState* state) const
{
	Ref< HierarchicalState > merged = new HierarchicalState();

	merged->m_states = m_states;

	for (std::map< std::wstring, std::pair< bool, bool > >::const_iterator i = state->m_states.begin(); i != state->m_states.end(); ++i)
		merged->addState(i->first, i->second.first, i->second.second);

	return merged;
}

bool HierarchicalState::serialize(ISerializer& s)
{
	return s >> MemberStlMap< 
		std::wstring,
		std::pair< bool, bool >,
		MemberStlPair< 
			std::wstring, 
			std::pair< bool, bool >,
			Member< std::wstring >,
			MemberStlPair< bool, bool >
		>
	>(L"states", m_states);
}

	}
}
