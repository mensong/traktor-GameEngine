#ifndef traktor_DeepClone_H
#define traktor_DeepClone_H

#include <vector>
#include "Core/Object.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Serializable;

/*! \brief Clone object.
 * \ingroup Core
 *
 * Creates a clone of an object through
 * serialization.
 */
class T_DLLCLASS DeepClone : public Object
{
	T_RTTI_CLASS(DeepClone)

public:
	DeepClone(const Serializable* source);

	/*! \brief Create new instance of source object. */
	Ref< Serializable > create();

	/*! \brief Create new instance of source object. */
	template < typename T >
	Ref< T > create()
	{
		return dynamic_type_cast< T* >(create());
	}

private:
	std::vector< uint8_t > m_copy;
};

}

#endif	// traktor_DeepClone_H
