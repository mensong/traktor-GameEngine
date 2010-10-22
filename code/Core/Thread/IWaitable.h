#ifndef traktor_IWaitable_H
#define traktor_IWaitable_H

#include "Core/Config.h"

namespace traktor
{

/*! \brief Waitable object interface.
 * \ingroup Core
 */
class IWaitable
{
public:
	virtual ~IWaitable() {}

	virtual bool wait(int32_t timeout = -1) = 0;
};

}

#endif	// traktor_IWaitable_H
