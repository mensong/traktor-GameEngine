#ifndef traktor_MemberComposite_H
#define traktor_MemberComposite_H

#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

/*! \brief Composite member.
 * \ingroup Core
 */
template < typename Class >	
class MemberComposite : public MemberComplex
{
public:
	typedef Class value_type;
	
	MemberComposite(const std::wstring& name, value_type& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(Serializer& s) const
	{
		return m_ref.serialize(s);
	}
	
private:
	value_type& m_ref;
};

}

#endif	// traktor_MemberComposite_H
