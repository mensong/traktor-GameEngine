#ifndef traktor_CastAny_H
#define traktor_CastAny_H

#include "Core/Class/Any.h"
#include "Core/Io/OutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Math/Scalar.h"
#include "Core/Meta/Traits.h"
#include "Core/Misc/TString.h"

#define T_CAST_ASSERT(x) T_FATAL_ASSERT((x))

namespace traktor
{

/*! \name Any cast templates */
/*! \ingroup Core */
/*! \{ */

template < typename Type, bool IsTypePtr = IsPointer< Type >::value >
struct CastAny
{
};

template < >
struct CastAny < Any, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"Any";
	}
	static bool accept(const Any& value) {
		return true;
	}
	static Any set(const Any& value) {
		return value;
	}
	static const Any& get(const Any& value) {
		return value;
	}
};

template < >
struct CastAny < const Any&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const Any&";
	}
	static bool accept(const Any& value) {
		return true;
	}
	static Any set(const Any& value) {
		return value;
	}
	static const Any& get(const Any& value) {
		return value;
	}
};

template < >
struct CastAny < bool, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"bool";
	}
	static bool accept(const Any& value) {
		return value.isBoolean();
	}
	static Any set(bool value) {
		return Any::fromBoolean(value);
	}
	static bool get(const Any& value) {
		return value.getBoolean();
	}
};

template < >
struct CastAny < int8_t, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"int8_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(int8_t value) {
		return Any::fromInt32(value);
	}
	static int8_t get(const Any& value) {
		return value.getInt32();
	}
};

template < >
struct CastAny < uint8_t, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"uint8_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(uint8_t value) {
		return Any::fromInt32(uint8_t(value));
	}
	static uint8_t get(const Any& value) {
		return (uint8_t)value.getInt32();
	}
};

template < >
struct CastAny < int16_t, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"int16_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(int16_t value) {
		return Any::fromInt32(value);
	}
	static int16_t get(const Any& value) {
		return value.getInt32();
	}
};

template < >
struct CastAny < uint16_t, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"uint16_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(uint16_t value) {
		return Any::fromInt32(uint16_t(value));
	}
	static uint16_t get(const Any& value) {
		return (uint16_t)value.getInt32();
	}
};

template < >
struct CastAny < int32_t, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"int32_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(int32_t value) {
		return Any::fromInt32(value);
	}
	static int32_t get(const Any& value) {
		return value.getInt32();
	}
};

template < >
struct CastAny < uint32_t, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"uint32_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(uint32_t value) {
		return Any::fromInt32(int32_t(value));
	}
	static uint32_t get(const Any& value) {
		return (uint32_t)value.getInt32();
	}
};

template < >
struct CastAny < int64_t, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"int64_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(int64_t value) {
		return Any::fromInt64(value);
	}
	static int64_t get(const Any& value) {
		return value.getInt64();
	}
};

template < >
struct CastAny < uint64_t, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"uint64_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(uint64_t value) {
		return Any::fromInt64(int64_t(value));
	}
	static uint64_t get(const Any& value) {
		return (uint64_t)value.getInt64();
	}
};

template < >
struct CastAny < float, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"float";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(float value) {
		return Any::fromFloat(value);
	}
	static float get(const Any& value) {
		return value.getFloat();
	}
};

template < >
struct CastAny < double, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"double";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(double value) {
		return Any::fromFloat(float(value));
	}
	static double get(const Any& value) {
		return double(value.getFloat());
	}
};

template < >
struct CastAny < Scalar, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"Scalar";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(const Scalar& value) {
		return Any::fromFloat(float(value));
	}
	static Scalar get(const Any& value) {
		return Scalar(value.getFloat());
	}
};

template < >
struct CastAny < const Scalar&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const Scalar&";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(const Scalar& value) {
		return Any::fromFloat(float(value));
	}
	static Scalar get(const Any& value) {
		return Scalar(value.getFloat());
	}
};

template < >
struct CastAny < std::string, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"std::string";
	}
	static bool accept(const Any& value) {
		return value.isString();
	}
	static Any set(const std::string& value) {
		return Any::fromString(value);
	}
	static std::string get(const Any& value) {
		return value.getString();
	}
};

template < >
struct CastAny < const std::string&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const std::string&";
	}
	static bool accept(const Any& value) {
		return value.isString();
	}
	static Any set(const std::string& value) {
		return Any::fromString(value);
	}
	static std::string get(const Any& value) {
		return value.getString();
	}
};

template < >
struct CastAny < const char, true >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const char";
	}
	static bool accept(const Any& value) {
		return value.isString();
	}
	static Any set(const char* value) {
		return Any::fromString(value);
	}
	static const char* get(const Any& value) {
		return value.getString().c_str();
	}
};

template < >
struct CastAny < std::wstring, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"std::wstring";
	}
	static bool accept(const Any& value) {
		return value.isString();
	}
	static Any set(const std::wstring& value) {
		return Any::fromString(value);
	}
	static std::wstring get(const Any& value) {
		return value.getWideString();
	}
};

template < >
struct CastAny < const std::wstring&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const std::wstring&";
	}
	static bool accept(const Any& value) {
		return value.isString();
	}
	static Any set(const std::wstring& value) {
		return Any::fromString(value);
	}
	static std::wstring get(const Any& value) {
		return value.getWideString();
	}
};

template < >
struct CastAny < const wchar_t, true >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const wchar_t";
	}
	static bool accept(const Any& value) {
		return value.isString();
	}
	static Any set(const wchar_t* value) {
		return Any::fromString(value);
	}
	static const wchar_t* get(const Any& value) {
		return value.getWideString().c_str();
	}
};

template < typename Type >
struct CastAny < Ref< Type >, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"Ref< " << type_name< Type >() << L">";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< Type >(value.getObjectUnsafe());
	}
	static Any set(const Ref< Type >& value) {
		return Any::fromObject(const_cast< typename IsConst< Type >::type_t* >(value.ptr()));
	}
	static Ref< Type > get(const Any& value) {
		T_CAST_ASSERT (value.getObject() == 0 || is_a< Type >(value.getObject()));
		return checked_type_cast< Type* >(value.getObject());
	}
};

template < typename Type >
struct CastAny < const Ref< Type >&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const Ref< " << type_name< Type >() << L" >&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< Type >(value.getObjectUnsafe());
	}
	static Any set(const Ref< Type >& value) {
		return Any::fromObject(const_cast< typename IsConst< Type >::type_t* >(value.ptr()));
	}
	static Ref< Type > get(const Any& value) {
		T_CAST_ASSERT (value.getObject() == 0 || is_a< Type >(value.getObject()));
		return checked_type_cast< Type* >(value.getObject());
	}
};

template < typename Type >
struct CastAny < Type, false >
{
	typedef typename IsConst< typename IsReference< Type >::base_t >::type_t type_t;

	static OutputStream& typeName(OutputStream& ss) {
		return ss << type_name< Type >();
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< Type >(value.getObjectUnsafe());
	}
	static Any set(const type_t& value) {
		return Any::fromObject(new type_t(value));
	}
	static Type get(const Any& value) {
		T_CAST_ASSERT (value.getObject() != 0 && is_a< type_t* >(value.getObject()));
		return Type(*checked_type_cast< type_t*, false >(value.getObject()));
	}
};

template < typename Type >
struct CastAny < Type, true >
{
	typedef typename IsConst< Type >::type_t no_const_type_t;

	static OutputStream& typeName(OutputStream& ss) {
		return ss << type_name< Type >();
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< Type >(value.getObjectUnsafe());
	}
	static Any set(Type value) {
		return Any::fromObject((ITypedObject*)(value));
	}
	static Type get(const Any& value) {
		T_CAST_ASSERT (value.getObject() == 0 || is_a< Type >(value.getObject()));
		return checked_type_cast< Type >(value.getObject());
	}
};

/*! \} */

}

#endif	// traktor_CastAny_H
