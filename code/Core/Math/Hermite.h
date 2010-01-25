#ifndef traktor_Hermite_H
#define traktor_Hermite_H

#include "Core/Config.h"
#include "Core/Math/Const.h"
#include "Core/Math/SplineControl.h"

namespace traktor
{

/*! \brief Default element accessor.
 * \ingroup Core
 */
template <
	typename Key,
	typename Time,
	typename Value
>
struct HermiteAccessor
{
	static inline Time time(const Key& key)
	{
		return Time(key.T);
	}

	static inline Value value(const Key& key)
	{
		return Value(key.value);
	}

	static inline Value combine(
		const Value& v0, const Time& w0,
		const Value& v1, const Time& w1,
		const Value& v2, const Time& w2,
		const Value& v3, const Time& w3
	)
	{
		return Value(v0 * w0 + v1 * w1 + v2 * w2 + v3 * w3);
	}
};

/*! \brief Hermite spline evaluator.
 * \ingroup Core
 */
template <
	typename Key,
	typename Time,
	typename Value,
	typename Accessor = HermiteAccessor< Key, Time, Value >,
	typename TimeControl = ClampTime< Time >
>
struct Hermite
{
	static Value evaluate(const Key* keys, size_t nkeys, const Time& Tat, const Time& Tend = Time(-1.0f), const Time& stiffness = Time(0.5f))
	{
		T_ASSERT (nkeys >= 2);

		Time Tfirst(Accessor::time(keys[0]));
		Time Tlast(Accessor::time(keys[nkeys - 1]));
		Time Tcurr = TimeControl::t(Tat, Tfirst, Tlast, Tend > Time(0.0f) ? Tend : Tlast);

		int index = 0;
		while (index < int(nkeys - 1) && Tcurr >= Accessor::time(keys[index + 1]))
			++index;

		int index_n1 = TimeControl::index(index - 1, int(nkeys));
		int index_1 = TimeControl::index(index + 1, int(nkeys));
		int index_2 = TimeControl::index(index + 2, int(nkeys));
		
		const Key& cp0 = keys[index];
		const Key& cp1 = keys[index_1];

		Value v0 = Accessor::value(cp0);
		Value v1 = Accessor::value(cp1);

		Time t0(Accessor::time(cp0));
		Time t1(Accessor::time(cp1));

		if (t0 >= t1)
			t1 = Tend;

		const Key& cpp = keys[index_n1];
		const Key& cpn = keys[index_2];

		Value vp = Accessor::value(cpp);
		Value vn = Accessor::value(cpn);

		Time t = (Tcurr - t0) / (t1 - t0);
		Time t2 = t * t;
		Time t3 = t2 * t;

		Time h2 = Time(3.0f) * t2 - t3 - t3;
		Time h1 = Time(1.0f) - h2;
		Time h4 = t3 - t2;
		Time h3 = h4 - t2 + t;

		h3 *= stiffness;
		h4 *= stiffness;

		return Accessor::combine(
			v0, h1 - h4,
			v1, h2 + h3,
			vp, -h3,
			vn, h4
		);
	}
};

}

#endif	// traktor_Hermite_H
