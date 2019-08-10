#pragma once

#include "Core/Class/Boxed.h"
#include "Core/Class/CastAny.h"
#include "Core/Math/Quaternion.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BoxedVector4;

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedQuaternion : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedQuaternion();

	explicit BoxedQuaternion(const Quaternion& value);

	explicit BoxedQuaternion(float x, float y, float z, float w);

	explicit BoxedQuaternion(const BoxedVector4* axis, float angle);

	explicit BoxedQuaternion(float head, float pitch, float bank);

	explicit BoxedQuaternion(const BoxedVector4* from, const BoxedVector4* to);

	explicit BoxedQuaternion(const class BoxedMatrix44* m);

	void set_x(float v) { m_value.e.set(0, Scalar(v)); }

	float get_x() const { return m_value.e.x(); }

	void set_y(float v) { m_value.e.set(1, Scalar(v)); }

	float get_y() const { return m_value.e.y(); }

	void set_z(float v) { m_value.e.set(2, Scalar(v)); }

	float get_z() const { return m_value.e.z(); }

	void set_w(float v) { m_value.e.set(3, Scalar(v)); }

	float get_w() const { return m_value.e.w(); }

	Quaternion normalized() const;

	Quaternion inverse() const;

	Quaternion concat(const BoxedQuaternion* q) const;

	Vector4 transform(const BoxedVector4* v) const;

	Vector4 getEulerAngles() const;

	Vector4 getAxisAngle() const;

	static Quaternion fromEulerAngles(float head, float pitch, float bank);

	static Quaternion fromAxisAngle(const BoxedVector4* axisAngle);

	static Quaternion lerp(const BoxedQuaternion* a, const BoxedQuaternion* b, float c) { return traktor::lerp(a->m_value, b->m_value, c); }

	static Quaternion slerp(const BoxedQuaternion* a, const BoxedQuaternion* b, float c) { return traktor::slerp(a->m_value, b->m_value, c); }

	const Quaternion& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Quaternion m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Quaternion, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.Quaternion";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedQuaternion >(value.getObjectUnsafe());
	}
    static Any set(const Quaternion& value) {
        return Any::fromObject(new BoxedQuaternion(value));
    }
    static const Quaternion& get(const Any& value) {
		return static_cast< BoxedQuaternion* >(value.getObject())->unbox();
    }
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Quaternion&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.Quaternion&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedQuaternion >(value.getObjectUnsafe());
	}
    static Any set(const Quaternion& value) {
        return Any::fromObject(new BoxedQuaternion(value));
    }
    static const Quaternion& get(const Any& value) {
		return static_cast< BoxedQuaternion* >(value.getObject())->unbox();
	}
};

}
