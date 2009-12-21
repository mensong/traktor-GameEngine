#ifndef traktor_SafeDestroy_H
#define traktor_SafeDestroy_H

namespace traktor
{

/*! \brief Safe destroy call.
 *
 * Helper function to reduce code bloat
 * in common pattern.
 */
template < typename T >
void safeDestroy(T& tv)
{
	if (tv)
	{
		tv->destroy();
		tv = 0;
	}
}

}

#endif	// traktor_SafeDestroy_H
