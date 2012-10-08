#include <limits>
#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Math/MathUtils.h"
#include "Parade/Network/State/FloatValue.h"
#include "Parade/Network/State/FloatTemplate.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.FloatTemplate", FloatTemplate, IValueTemplate)

FloatTemplate::FloatTemplate()
:	m_min(std::numeric_limits< float >::max())
,	m_max(-std::numeric_limits< float >::max())
,	m_lowPrecision(false)
{
}

FloatTemplate::FloatTemplate(float min, float max, bool lowPrecision)
:	m_min(min)
,	m_max(max)
,	m_lowPrecision(lowPrecision)
{
}

void FloatTemplate::pack(BitWriter& writer, const IValue* V) const
{
	float f = *checked_type_cast< const FloatValue* >(V);
	if (!m_lowPrecision)
		writer.writeUnsigned(32, *(uint32_t*)&f);
	else
	{
		uint8_t uf = uint8_t(clamp((f - m_min) / (m_max - m_min), 0.0f, 1.0f) * 255);
		writer.writeUnsigned(8, uf);
	}
}

Ref< const IValue > FloatTemplate::unpack(BitReader& reader) const
{
	if (!m_lowPrecision)
	{
		uint32_t u = reader.readUnsigned(32);
		return new FloatValue(*(float*)&u);
	}
	else
	{
		uint8_t uf = reader.readUnsigned(8);
		float f = (uf / 255.0f) * (m_max - m_min) + m_min;
		return new FloatValue(f);
	}
}

Ref< const IValue > FloatTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, const IValue* V, float T) const
{
	float fn1 = *checked_type_cast< const FloatValue* >(Vn1);
	float f0 = *checked_type_cast< const FloatValue* >(V0);
	
	if (Vn2)
	{
		float fn2 = *checked_type_cast< const FloatValue* >(Vn2);

		float v2_1 = (fn1 - fn2) / (Tn1 - Tn2);
		float v1_0 = (f0 - fn1) / (T0 - Tn1);
		float a = clamp((v1_0 - v2_1) / (T0 - Tn1), -1.0f, 1.0f);
		float f = f0 + (f0 - fn1) * (T - T0) + 0.5f * a * (T - T0) * (T - T0);

		if (m_min < m_max)
			f = clamp(f, m_min, m_max);

		return new FloatValue(f);
	}
	else
	{
		float k = (T - Tn1) / (T0 - Tn1);
		float f = f0 + (fn1 - f0) * k;

		if (m_min < m_max)
			f = clamp(f, m_min, m_max);

		return new FloatValue(f);
	}
}

	}
}
