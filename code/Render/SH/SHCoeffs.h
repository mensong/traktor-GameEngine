#ifndef traktor_render_SHCoeffs_H
#define traktor_render_SHCoeffs_H

#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class SHMatrix;

/*! \brief Spherical harmonics coefficients.
 * \ingroup Render
 */
class T_DLLCLASS SHCoeffs : public Serializable
{
	T_RTTI_CLASS(SHCoeffs)

public:
	void resize(uint32_t coefficientCount);

	bool empty() const;

	SHCoeffs transform(const SHMatrix& matrix) const;

	float operator * (const SHCoeffs& coeffs) const;

	float& operator [] (uint32_t index) { return m_coefficients[index]; }

	float operator [] (uint32_t index) const { return m_coefficients[index]; }

	const std::vector< float >& get() const { return m_coefficients; }

	virtual bool serialize(Serializer& s);

private:
	std::vector< float > m_coefficients;
};

	}
}

#endif	// traktor_render_SHCoeffs_H
