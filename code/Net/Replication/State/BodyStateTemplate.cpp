#include <algorithm>
#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Half.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Net/Replication/Pack.h"
#include "Net/Replication/State/BodyStateValue.h"
#include "Net/Replication/State/BodyStateTemplate.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

float errorV4(const Vector4& Vl, const Vector4& Vr)
{
	return (Vl - Vr).length();
}

float safeDeltaTime(float v)
{
	float av = std::abs(v);
	if (av < 1.0f/60.0f)
		return 1.0f/60.0f * sign(v);
	else if (av > 1.0f)
		return 1.0f * sign(v);
	else
		return v;
}

physics::BodyState interpolate(const physics::BodyState& bs0, float T0, const physics::BodyState& bs1, float T1, float T)
{
	Scalar dT_0_1 = Scalar(safeDeltaTime(T1 - T0));

#if 0

	physics::BodyState state;

	state.setTransform(lerp(bs0.getTransform(), bs1.getTransform(), Scalar(T - T0) / dT_0_1));
	state.setLinearVelocity(
		(bs1.getTransform().translation() - bs0.getTransform().translation()) / dT_0_1
	);

	Quaternion Qv0 = bs0.getTransform().rotation();
	Quaternion Qv1 = Qv0.nearest(bs1.getTransform().rotation());
	Quaternion Qdiff = Qv1 * Qv0.inverse();

	Vector4 Vdiff = Qdiff.toAxisAngle();
	Scalar angleDiff = Vdiff.length();

	if (angleDiff > FUZZY_EPSILON)
		state.setAngularVelocity(Vdiff / dT_0_1);
	else
		state.setAngularVelocity(Vector4::zero());

	return state;

#else

	return bs0.interpolate(bs1, Scalar(T - T0) / dT_0_1);

#endif
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.BodyStateTemplate", BodyStateTemplate, IValueTemplate)

BodyStateTemplate::BodyStateTemplate(float linearError, float angularError)
:	m_linearError(linearError)
,	m_angularError(angularError)
{
}

const TypeInfo& BodyStateTemplate::getValueType() const
{
	return type_of< BodyStateValue >();
}

void BodyStateTemplate::pack(BitWriter& writer, const IValue* V) const
{
	physics::BodyState v = *checked_type_cast< const BodyStateValue* >(V);
	const Transform& T = v.getTransform();

	float e[4];
	uint8_t u[3];

	// 3 * 32
	T.translation().storeUnaligned(e);
	for (uint32_t i = 0; i < 3; ++i)
		writer.writeUnsigned(32, *(uint32_t*)&e[i]);

	// 3 * 8 + 16
	Vector4 R = T.rotation().toAxisAngle();
	float a = R.length();
	if (abs(a) > FUZZY_EPSILON)
		R /= Scalar(a);

	packUnit(R, u);
	writer.writeUnsigned(8, u[0]);
	writer.writeUnsigned(8, u[1]);
	writer.writeUnsigned(8, u[2]);
	writer.writeUnsigned(16, floatToHalf(a));

	{
		Vector4 linearVelocity = v.getLinearVelocity().xyz0();
		Scalar ln = linearVelocity.length();

		if (ln > FUZZY_EPSILON)
			linearVelocity /= ln;

		// 3 * 8
		packUnit(linearVelocity, u);
		writer.writeUnsigned(8, u[0]);
		writer.writeUnsigned(8, u[1]);
		writer.writeUnsigned(8, u[2]);

		// 16
		half_t lnf16 = floatToHalf(ln);
		writer.writeUnsigned(16, lnf16);
	}

	{
		Vector4 angularVelocity = v.getAngularVelocity().xyz0();
		Scalar ln = angularVelocity.length();

		if (ln > FUZZY_EPSILON)
			angularVelocity /= ln;

		// 3 * 8
		packUnit(angularVelocity, u);
		writer.writeUnsigned(8, u[0]);
		writer.writeUnsigned(8, u[1]);
		writer.writeUnsigned(8, u[2]);

		// 16
		half_t lnf16 = floatToHalf(ln);
		writer.writeUnsigned(16, lnf16);
	}
}

Ref< const IValue > BodyStateTemplate::unpack(BitReader& reader) const
{
	Vector4 linearVelocity, angularVelocity;
	Transform T;

	float f[4];
	uint8_t u[3];

	for (uint32_t i = 0; i < 3; ++i)
	{
		uint32_t uf = reader.readUnsigned(32);
		f[i] = *(float*)&uf;
		T_FATAL_ASSERT(!isNanOrInfinite(f[i]));
	}

	u[0] = reader.readUnsigned(8);
	u[1] = reader.readUnsigned(8);
	u[2] = reader.readUnsigned(8);
	Vector4 R = unpackUnit(u);
	float Ra = halfToFloat(reader.readUnsigned(16));

	T = Transform(
		Vector4(f[0], f[1], f[2], 1.0f),
		(abs(Ra) > FUZZY_EPSILON && R.length() > FUZZY_EPSILON) ? 
			Quaternion::fromAxisAngle(R, Ra).normalized() :
			Quaternion::identity()
	);

	u[0] = reader.readUnsigned(8);
	u[1] = reader.readUnsigned(8);
	u[2] = reader.readUnsigned(8);
	linearVelocity = unpackUnit(u);

	half_t lnvf16 = reader.readUnsigned(16);
	linearVelocity *= Scalar(halfToFloat(lnvf16));

	u[0] = reader.readUnsigned(8);
	u[1] = reader.readUnsigned(8);
	u[2] = reader.readUnsigned(8);
	angularVelocity = unpackUnit(u);

	half_t lnaf16 = reader.readUnsigned(16);
	angularVelocity *= Scalar(halfToFloat(lnaf16));

	physics::BodyState S;
	S.setTransform(T);
	S.setLinearVelocity(linearVelocity);
	S.setAngularVelocity(angularVelocity);

	return new BodyStateValue(S);
}

float BodyStateTemplate::error(const IValue* Vl, const IValue* Vr) const
{
	const physics::BodyState& Sl = *checked_type_cast< const BodyStateValue* >(Vl);
	const physics::BodyState& Sr = *checked_type_cast< const BodyStateValue* >(Vr);

	float errors[] =
	{
		errorV4(Sl.getTransform().translation(), Sr.getTransform().translation()) * m_linearError,
		errorV4(Sl.getTransform().rotation().e, Sr.getTransform().rotation().e) * m_angularError,
		errorV4(Sl.getLinearVelocity(), Sr.getLinearVelocity()) * m_linearError,
		errorV4(Sl.getAngularVelocity(), Sr.getAngularVelocity()) * m_angularError
	};

	return *std::max_element(&errors[0], &errors[3]);
}

Ref< const IValue > BodyStateTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const
{
	const physics::BodyState& Sn2 = *checked_type_cast< const BodyStateValue* >(Vn2);
	const physics::BodyState& Sn1 = *checked_type_cast< const BodyStateValue* >(Vn1);
	const physics::BodyState& S0 = *checked_type_cast< const BodyStateValue* >(V0);

	Scalar dT_0(safeDeltaTime(T - T0));
	Scalar dT_n1_0(safeDeltaTime(T0 - Tn1));
	Scalar dT_n2_n1(safeDeltaTime(Tn1 - Tn2));

	if (T <= Tn2)
		return Vn2;

	if (T <= Tn1)
	{
		return new BodyStateValue(
			interpolate(Sn2, Tn2, Sn1, Tn1, T)
		);
	}

	if (T <= T0)
	{
		return new BodyStateValue(
			interpolate(Sn1, Tn1, S0, T0, T)
		);
	}

	Vector4 Vl = S0.getLinearVelocity().xyz0();
	Vector4 Va = S0.getAngularVelocity();

	Vector4 P = S0.getTransform().translation().xyz1();
	Quaternion R = S0.getTransform().rotation();

	P = P + (Vl * dT_0);
	R = Quaternion::fromAxisAngle(S0.getAngularVelocity() * dT_0) * R;

	physics::BodyState Sf;
	Sf.setTransform(Transform(P, R.normalized()));
	Sf.setLinearVelocity(Vl);
	Sf.setAngularVelocity(Va);

	return new BodyStateValue(Sf);
}

	}
}
