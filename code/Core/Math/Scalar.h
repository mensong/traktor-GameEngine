#ifndef traktor_Scalar_H
#define traktor_Scalar_H

#include "Core/Config.h"
#include "Core/Math/MathConfig.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Scalar.
 * \ingroup Core
 */
T_MATH_ALIGN16 class T_DLLCLASS Scalar
{
public:
#if defined(T_MATH_USE_SSE2)
	__m128 m_data;
#else
	float m_data;
#endif

	T_MATH_INLINE Scalar();

	T_MATH_INLINE Scalar(const Scalar& value);

	explicit T_MATH_INLINE Scalar(float value);

#if defined(T_MATH_USE_SSE2)
	explicit T_MATH_INLINE Scalar(__m128 value);
#endif

	T_MATH_INLINE Scalar operator - () const;

	T_MATH_INLINE Scalar& operator += (const Scalar& v);

	T_MATH_INLINE Scalar& operator -= (const Scalar& v);

	T_MATH_INLINE Scalar& operator *= (const Scalar& v);

	T_MATH_INLINE Scalar& operator /= (const Scalar& v);

	T_MATH_INLINE bool operator < (const Scalar& r);

	T_MATH_INLINE bool operator <= (const Scalar& r);

	T_MATH_INLINE bool operator > (const Scalar& r);

	T_MATH_INLINE bool operator >= (const Scalar& r);

	T_MATH_INLINE bool operator == (const Scalar& r);

	T_MATH_INLINE bool operator != (const Scalar& r);

	T_MATH_INLINE operator float () const;

	friend T_MATH_INLINE T_DLLCLASS Scalar operator + (const Scalar& l, const Scalar& r);

	friend T_MATH_INLINE T_DLLCLASS Scalar operator - (const Scalar& l, const Scalar& r);

	friend T_MATH_INLINE T_DLLCLASS Scalar operator * (const Scalar& l, const Scalar& r);

	friend T_MATH_INLINE T_DLLCLASS Scalar operator / (const Scalar& l, const Scalar& r);
};

T_MATH_INLINE T_DLLCLASS Scalar abs(const Scalar& s);

T_MATH_INLINE T_DLLCLASS Scalar squareRoot(const Scalar& s);

}

#if defined(T_MATH_USE_INLINE)
#	if defined(T_MATH_USE_SSE2)
#		include "Core/Math/Sse2/Scalar.inl"
#	else
#		include "Core/Math/Std/Scalar.inl"
#	endif
#endif

#endif	// traktor_Scalar_H
