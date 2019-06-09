#include <algorithm>
#include <cstring>
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Common/Boolean.h"
#include "Flash/Action/Common/Number.h"
#include "Flash/Action/Common/String.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

#if defined(_DEBUG)
#	define T_VALIDATE(av) \
	T_ASSERT((av).getType() >= ActionValue::AvtUndefined && (av).getType() <= ActionValue::AvtObjectWeak);
#else
#	define T_VALIDATE(av)
#endif

#define T_SAFE_ADD_WEAK_REF(o) \
	{ if ((o) != 0) (o)->addWeakRef(this); }

#define T_SAFE_RELEASE_WEAK_REF(o) \
	{ if ((o) != 0) (o)->releaseWeakRef(this); }

#if defined(_DEBUG)
static int32_t s_stringCount = 0;
#endif

#pragma pack(1)
struct StringType
{
	uint16_t rc;
	int32_t id;
};
#pragma pack()

char* refStringCreate(const char* s, int32_t id)
{
	uint32_t len = uint32_t(strlen(s));
	T_ASSERT(len < 4096);

	void* ptr = getAllocator()->alloc(sizeof(StringType) + (len + 1) * sizeof(char), 4, T_FILE_LINE);
	if (!ptr)
		return nullptr;

	StringType* base = static_cast< StringType* >(ptr);
	base->rc = 1;
	base->id = id;

	char* c = reinterpret_cast< char* >(base + 1);
	if (len > 0)
		std::memcpy(c, s, len * sizeof(char));

	c[len] = '\0';

#if defined(_DEBUG)
	++s_stringCount;
#endif
	return c;
}

char* refStringInc(char* s)
{
	StringType* base = reinterpret_cast< StringType* >(s) - 1;
	base->rc++;
	return s;
}

char* refStringDec(char* s)
{
	StringType* base = reinterpret_cast< StringType* >(s) - 1;
	if (--base->rc == 0)
	{
		getAllocator()->free(base);
#if defined(_DEBUG)
		--s_stringCount;
#endif
		return nullptr;
	}
	return s;
}

int32_t refStringId(char* s)
{
	StringType* base = reinterpret_cast< StringType* >(s) - 1;
	return base->id;
}

		}

ActionValue::ActionValue()
:	m_type(AvtUndefined)
{
	m_value.o = nullptr;
}

ActionValue::ActionValue(const ActionValue& v)
:	m_type(v.m_type)
{
	T_VALIDATE(v);
	m_value.o = 0;
	if (m_type == AvtString)
		m_value.s = refStringInc(v.m_value.s);
	else if (m_type == AvtObject)
	{
		T_SAFE_ADDREF (v.m_value.o);
		m_value.o = v.m_value.o;
	}
	else if (m_type == AvtObjectWeak)
	{
		T_SAFE_ADD_WEAK_REF(v.m_value.o);
		m_value.o = v.m_value.o;
	}
	else
		m_value = v.m_value;
}

#if defined(T_CXX11)
ActionValue::ActionValue(ActionValue&& v)
:	m_type(v.m_type)
,	m_value(v.m_value)
{
	v.m_type = AvtUndefined;
}
#endif

ActionValue::ActionValue(bool b)
:	m_type(AvtBoolean)
{
	m_value.b = b;
}

ActionValue::ActionValue(int32_t i)
:	m_type(AvtInteger)
{
	m_value.i = i;
}

ActionValue::ActionValue(float f)
:	m_type(AvtFloat)
{
	m_value.f = f;
}

ActionValue::ActionValue(const char* s, int32_t id)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(s, id);
}

ActionValue::ActionValue(const std::string& s, int32_t id)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(s.c_str(), id);
}

ActionValue::ActionValue(const wchar_t* s, int32_t id)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(wstombs(Utf8Encoding(), s).c_str(), id);
}

ActionValue::ActionValue(const std::wstring& s, int32_t id)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(wstombs(Utf8Encoding(), s).c_str(), id);
}

ActionValue::ActionValue(ActionObject* o)
:	m_type(AvtObject)
{
	T_SAFE_ADDREF(o);
	m_value.o = o;
}

ActionValue::ActionValue(ActionObject* o, bool weak)
:	m_type(AvtObjectWeak)
{
	T_ASSERT(weak);
	T_SAFE_ADD_WEAK_REF(o);
	m_value.o = o;
}

ActionValue::~ActionValue()
{
	T_EXCEPTION_GUARD_BEGIN

	T_VALIDATE(*this);
	if (m_type == AvtString && m_value.s)
		refStringDec(m_value.s);
	else if (m_type == AvtObject)
		{ T_SAFE_RELEASE (m_value.o); }
	else if (m_type == AvtObjectWeak)
		{ T_SAFE_RELEASE_WEAK_REF(m_value.o); }

	T_EXCEPTION_GUARD_END
}

void ActionValue::clear()
{
	if (m_type == AvtString && m_value.s)
		refStringDec(m_value.s);
	else if (m_type == AvtObject)
		{ T_SAFE_RELEASE (m_value.o); }
	else if (m_type == AvtObjectWeak)
		{ T_SAFE_RELEASE_WEAK_REF(m_value.o); }

	m_type = AvtUndefined;
	m_value.o = nullptr;
}

bool ActionValue::getBoolean() const
{
	T_VALIDATE(*this);
	switch (m_type)
	{
	case AvtBoolean:
		return m_value.b;

	case AvtInteger:
		return bool(m_value.i != 0);

	case AvtFloat:
		return bool(m_value.f != 0.0);

	case AvtString:
		return std::strlen(m_value.s) > 0;

	case AvtObject:
	case AvtObjectWeak:
		return m_value.o ? m_value.o->valueOf().getBoolean() : false;

	default:
		break;
	}
	return false;
}

int32_t ActionValue::getInteger() const
{
	T_VALIDATE(*this);
	switch (m_type)
	{
	case AvtBoolean:
		return m_value.b ? 1 : 0;

	case AvtInteger:
		return m_value.i;

	case AvtFloat:
		return int32_t(m_value.f);

	case AvtObject:
	case AvtObjectWeak:
		return m_value.o ? m_value.o->valueOf().getInteger() : 0;

	default:
		break;
	}
	return 0;
}

float ActionValue::getFloat() const
{
	T_VALIDATE(*this);
	switch (m_type)
	{
	case AvtBoolean:
		return m_value.b ? 1.0f : 0.0f;

	case AvtInteger:
		return float(m_value.i);

	case AvtFloat:
		return m_value.f;

	case AvtObject:
	case AvtObjectWeak:
		return m_value.o ? m_value.o->valueOf().getFloat() : 0.0f;

	default:
		break;
	}
	return 0.0f;
}

std::string ActionValue::getString() const
{
	T_VALIDATE(*this);
	switch (m_type)
	{
	case AvtBoolean:
		return m_value.b ? "true" : "false";

	case AvtInteger:
		return wstombs(traktor::toString(m_value.i));

	case AvtFloat:
		return wstombs(traktor::toString(m_value.f));

	case AvtString:
		return m_value.s;

	case AvtObject:
	case AvtObjectWeak:
		return m_value.o ? m_value.o->toString().getString() : "null";

	default:
		break;
	}
	return "undefined";
}

std::wstring ActionValue::getWideString() const
{
	return mbstows(Utf8Encoding(), getString());
}

Ref< ActionObject > ActionValue::getObjectAlways(ActionContext* context) const
{
	T_VALIDATE(*this);
	switch (m_type)
	{
	case AvtBoolean:
		return (new Boolean(m_value.b))->getAsObject(context);

	case AvtInteger:
		return (new Number(float(m_value.i)))->getAsObject(context);

	case AvtFloat:
		return (new Number(m_value.f))->getAsObject(context);

	case AvtString:
		return (new String(m_value.s))->getAsObject(context);

	case AvtObject:
	case AvtObjectWeak:
		return m_value.o ? m_value.o : new ActionObject(context);

	default:
		break;
	}
	return new ActionObject(context);
}

int32_t ActionValue::getStringId() const
{
	T_ASSERT(m_type == AvtString);
	return refStringId(m_value.s);
}

void ActionValue::serialize(ISerializer& s)
{
	const MemberEnum< Type >::Key kType[] =
	{
		{ L"AvtUndefined", AvtUndefined },
		{ L"AvtBoolean", AvtBoolean },
		{ L"AvtInteger", AvtInteger },
		{ L"AvtFloat", AvtFloat },
		{ L"AvtString", AvtString },
		{ L"AvtObject", AvtObject },
		{ L"AvtObjectWeak", AvtObjectWeak },
		{ 0 }
	};

	s >> MemberEnum< Type >(L"type", m_type, kType);
	switch (m_type)
	{
	case AvtUndefined:
		break;

	case AvtBoolean:
		s >> Member< bool >(L"value", m_value.b);
		break;

	case AvtInteger:
		s >> Member< int32_t >(L"value", m_value.i);
		break;

	case AvtFloat:
		s >> Member< float >(L"value", m_value.f);
		break;

	case AvtString:
		{
			if (s.getDirection() == ISerializer::SdRead)
			{
				std::string str;
				s >> Member< std::string >(L"value", str);
				m_value.s = refStringCreate(str.c_str(), -1);
			}
			else
			{
				std::string str = m_value.s;
				s >> Member< std::string >(L"value", str);
			}
		}
		break;

    default:
        break;
	}
}

ActionValue& ActionValue::operator = (const ActionValue& v)
{
	T_VALIDATE(*this);
	T_VALIDATE(v);

	if (v.m_type == AvtString)
		refStringInc(v.m_value.s);
	else if (v.m_type == AvtObject)
		{ T_SAFE_ADDREF(v.m_value.o); }
	else if (v.m_type == AvtObjectWeak)
		{ T_SAFE_ADD_WEAK_REF(v.m_value.o); }

	if (m_type == AvtString)
		refStringDec(m_value.s);
	else if (m_type == AvtObject)
		{ T_SAFE_RELEASE(m_value.o); }
	else if (m_type == AvtObjectWeak)
		{ T_SAFE_RELEASE_WEAK_REF(m_value.o); }

	m_type = v.m_type;
	m_value = v.m_value;

	return *this;
}

#if defined(T_CXX11)
ActionValue& ActionValue::operator = (ActionValue&& v)
{
	T_VALIDATE(*this);
	T_VALIDATE(v);

	if (m_type == AvtString)
		refStringDec(m_value.s);
	else if (m_type == AvtObject)
		{ T_SAFE_RELEASE(m_value.o); }
	else if (m_type == AvtObjectWeak)
		{ T_SAFE_RELEASE_WEAK_REF(m_value.o); }

	m_type = v.m_type;
	m_value = v.m_value;

	v.m_type = AvtUndefined;
	return *this;
}
#endif

ActionValue ActionValue::operator + (const ActionValue& r) const
{
	if (r.isString() || isString())
	{
		ActionValue string2 = r.toString();
		ActionValue string1 = toString();
		if (string2.isString() && string1.isString())
		{
			std::string str2 = string2.getString();
			std::string str1 = string1.getString();
			return ActionValue(str1 + str2);
		}
		else
			return ActionValue();
	}
	else if (r.isInteger() && isInteger())
	{
		int32_t n2 = r.getInteger();
		int32_t n1 = getInteger();
		return ActionValue(n1 + n2);
	}
	else if (isNumeric() && r.isNumeric())
	{
		float n2 = r.getFloat();
		float n1 = getFloat();
		return ActionValue(n1 + n2);
	}
	else
	{
		ActionValue number2 = r.toFloat();
		ActionValue number1 = toFloat();
		if (number2.isNumeric() && number1.isNumeric())
		{
			float n2 = number2.getFloat();
			float n1 = number1.getFloat();
			return ActionValue(n1 + n2);
		}
		else
			return ActionValue();
	}
}

ActionValue ActionValue::operator - (const ActionValue& r) const
{
	if (r.isInteger() && isInteger())
	{
		int32_t n2 = r.getInteger();
		int32_t n1 = getInteger();
		return ActionValue(n1 - n2);
	}
	else if (isNumeric() && r.isNumeric())
		return ActionValue(getFloat() - r.getFloat());
	else
		return ActionValue();
}

ActionValue ActionValue::operator * (const ActionValue& r) const
{
	if (r.isInteger() && isInteger())
	{
		int32_t n2 = r.getInteger();
		int32_t n1 = getInteger();
		return ActionValue(n1 * n2);
	}
	else if (isNumeric() && r.isNumeric())
		return ActionValue(getFloat() * r.getFloat());
	else
		return ActionValue();
}

ActionValue ActionValue::operator / (const ActionValue& r) const
{
	if (isNumeric() && r.isNumeric())
		return ActionValue(getFloat() / r.getFloat());
	else
		return ActionValue();
}

bool ActionValue::operator == (const ActionValue& r) const
{
	if (!r.isUndefined() && !isUndefined())
	{
		ActionValue::Type predicateType = std::max(r.getType(), getType());
		if (predicateType == ActionValue::AvtBoolean)
		{
			bool v2 = r.getBoolean();
			bool v1 = getBoolean();
			return v1 == v2;
		}
		else if (predicateType == ActionValue::AvtInteger)
		{
			int32_t v2 = r.getInteger();
			int32_t v1 = getInteger();
			return v1 == v2;
		}
		else if (predicateType == ActionValue::AvtFloat)
		{
			float v2 = r.getFloat();
			float v1 = getFloat();
			return v1 == v2;
		}
		else if (predicateType == ActionValue::AvtString)
		{
			std::string v2 = r.getString();
			std::string v1 = getString();
			return v1 == v2;
		}
		else	// AvtObject
		{
			if (r.isObject() && isObject())
			{
				Ref< ActionObject > object2 = r.getObject();
				Ref< ActionObject > object1 = getObject();
				return object1 == object2;
			}
			else
				return false;
		}
	}
	else if (isObject() && r.isUndefined())
	{
		ActionObject* object1 = getObject();
		return object1 == nullptr;
	}
	else if (isUndefined() && r.isObject())
	{
		ActionObject* object2 = r.getObject();
		return object2 == nullptr;
	}
	else if (isUndefined() && r.isUndefined())
		return true;
	else
		return false;
}

void ActionValue::disposeReference(Collectable* collectable)
{
	T_ASSERT(m_type == AvtObjectWeak);
	T_ASSERT(m_value.o == collectable);
	m_value.o = nullptr;
}

	}
}
