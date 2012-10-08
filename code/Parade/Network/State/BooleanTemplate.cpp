#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Parade/Network/State/BooleanValue.h"
#include "Parade/Network/State/BooleanTemplate.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.BooleanTemplate", BooleanTemplate, IValueTemplate)

BooleanTemplate::BooleanTemplate(float threshold)
:	m_threshold(threshold)
{
}

void BooleanTemplate::pack(BitWriter& writer, const IValue* V) const
{
	bool f = *checked_type_cast< const BooleanValue* >(V);
	writer.writeBit(f);
}

Ref< const IValue > BooleanTemplate::unpack(BitReader& reader) const
{
	bool f = reader.readBit();
	return new BooleanValue(f);
}

Ref< const IValue > BooleanTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, const IValue* V, float T) const
{
	bool fn1 = *checked_type_cast< const BooleanValue* >(Vn1);
	bool f0 = *checked_type_cast< const BooleanValue* >(V0);

	if (fn1 == f0)
		return new BooleanValue(f0);

	float k = (T - Tn1) / (T0 - Tn1);
	return new BooleanValue(k >= m_threshold ? f0 : fn1);
}

	}
}
