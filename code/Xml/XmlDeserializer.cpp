#include <sstream>
#include "Xml/XmlDeserializer.h"
#include "Core/Io/Stream.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Misc/SplitString.h"
#include "Core/Misc/StringUtils.h"
#include "Core/Misc/Base64.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace xml
	{
		namespace
		{

inline
XmlPullParser::Attributes::const_iterator findAttribute(const XmlPullParser::Attributes& attr, const std::wstring& name)
{
	for (XmlPullParser::Attributes::const_iterator i = attr.begin(); i != attr.end(); ++i)
	{
		if (i->first == name)
			return i;
	}
	return attr.end();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.XmlDeserializer", XmlDeserializer, Serializer)

XmlDeserializer::XmlDeserializer(Stream* stream)
:	m_xpp(stream)
{
	T_ASSERT_M (stream->canRead(), L"Incorrect direction on input stream");
}

Serializer::Direction XmlDeserializer::getDirection()
{
	return Serializer::SdRead;
}

bool XmlDeserializer::operator >> (const Member< bool >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	m = bool(value == L"true");
	
	return true;
}

bool XmlDeserializer::operator >> (const Member< int8_t >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;
	
	m = parseString< int32_t >(value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< uint8_t >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	m = parseString< uint32_t >(value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< int16_t >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	m = parseString< int16_t >(value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< uint16_t >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	m = parseString< uint16_t >(value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< int32_t >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	m = parseString< int32_t >(value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< uint32_t >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	m = parseString< uint32_t >(value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< int64_t >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	m = parseString< int64_t >(value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< uint64_t >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	m = parseString< uint64_t >(value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< float >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	m = parseString< float >(value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< double >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	m = parseString< double >(value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< std::string >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	m = wstombs(value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< std::wstring >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	m = value;
	return true;
}

bool XmlDeserializer::operator >> (const Member< Guid >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	Guid& guid = m;
	if (!guid.create(value))
		return false;

	return true;
}

bool XmlDeserializer::operator >> (const Member< Color >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	std::vector< float > v;
	if (Split< std::wstring, float >::any(value, L",", v) != 4)
		return false;

	m->r = uint8_t(v[0]);
	m->g = uint8_t(v[1]);
	m->b = uint8_t(v[2]);
	m->a = uint8_t(v[3]);

	return true;
}

bool XmlDeserializer::operator >> (const Member< Scalar >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	m = Scalar(parseString< float >(value));
	return true;
}

bool XmlDeserializer::operator >> (const Member< Vector2 >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;
	
	std::vector< float > v;
	if (Split< std::wstring, float >::any(value, L",", v) != 2)
		return false;
		
	m->x = v[0];
	m->y = v[1];
	
	return true;
}

bool XmlDeserializer::operator >> (const Member< Vector4 >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	std::vector< float > v;
	if (Split< std::wstring, float >::any(value, L",", v) != 4)
		return false;
		
	m->set(v[0], v[1], v[2], v[3]);
	return true;
}

bool XmlDeserializer::operator >> (const Member< Matrix33 >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;
		
	std::vector< float > v;
	if (Split< std::wstring, float >::any(value, L",", v) != 3 * 3)
		return false;

	for (int r = 0; r < 3; ++r)
	{
		for (int c = 0; c < 3; ++c)
		{
			m->e[r][c] = v[r * 3 + c];
		}
	}

	return true;
}

bool XmlDeserializer::operator >> (const Member< Matrix44 >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	std::vector< float > v;
	if (Split< std::wstring, float >::any(value, L",", v) != 4 * 4)
		return false;

	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
		{
			m->e[r][c] = v[r * 4 + c];
		}
	}

	return true;
}

bool XmlDeserializer::operator >> (const Member< Quaternion >& m)
{
	std::wstring value;
	if (!nextElementValue(m.getName(), value))
		return false;

	std::vector< float > v;
	if (Split< std::wstring, float >::any(value, L",", v) != 4)
		return false;
		
	m->x = v[0];
	m->y = v[1];
	m->z = v[2];
	m->w = v[3];

	return true;
}

bool XmlDeserializer::operator >> (const Member< Serializable >& m)
{
	Serializable* o = &static_cast< Serializable& >(m);

	if (!enterElement(m.getName()))
		return false;

	const XmlPullParser::Attributes& attr = m_xpp.getEvent().attr;
	XmlPullParser::Attributes::const_iterator a = findAttribute(attr, L"version");
	int version = a != attr.end() ? parseString< int >(a->second) : 0;
	T_ASSERT_M (version <= o->getVersion(), L"Serialized data has a higher version number than supported by the code");
	
	rememberObject(o);
	if (!serialize(o, version))
		return false;

	if (!leaveElement(m.getName()))
		return false;

	return true;
}

bool XmlDeserializer::operator >> (const Member< Serializable* >& m)
{
	if (!enterElement(m.getName()))
		return false;

	const XmlPullParser::Attributes& attr = m_xpp.getEvent().attr;
	XmlPullParser::Attributes::const_iterator a;

	if ((a = findAttribute(attr, L"ref")) != attr.end())
	{
		std::map< std::wstring, Object* >::iterator i = m_refs.find(a->second);
		if (i == m_refs.end())
		{
			log::error << L"No such reference, \"" << a->second << L"\"" << Endl;
			return 0;
		}
		m = static_cast< Serializable* >(m_refs[a->second]);
	}
	else if ((a = findAttribute(attr, L"type")) != attr.end())
	{
		std::wstring typeName = a->second;

		const Type* type = Type::find(typeName);
		if (!type)
		{
			log::error << L"No such type, \"" << typeName << L"\"" << Endl;
			return false;
		}

		Ref< Serializable > o = dynamic_type_cast< Serializable* >(type->newInstance());
		if (!o)
		{
			log::error << L"Unable to instanciate type \"" << typeName << L"\"" << Endl;
			return false;
		}

		int version = 0;
		if ((a = findAttribute(attr, L"version")) != attr.end())
		{
			version = parseString< int >(a->second);
			T_ASSERT_M (version <= o->getVersion(), L"Serialized data has a higher version number than supported by the code");
		}

		rememberObject(o);
		if (serialize(o, version))
			m = o;
		else
		{
			log::error << L"Unable to serialize object of type \"" << typeName << L"\"" << Endl;
			return false;
		}
	}
	else
	{
		rememberObject(0);
		m = 0;
	}

	if (!leaveElement(m.getName()))
		return false;

	return true;
}

bool XmlDeserializer::operator >> (const Member< void* >& m)
{
	if (!enterElement(m.getName()))
		return false;

	if (m_xpp.next() != XmlPullParser::EtText)
		return false;

	std::vector< uint8_t > data = Base64().decode(m_xpp.getEvent().value);

	if (!data.empty())
	{
		m.setBlobSize(uint32_t(data.size()));
		memcpy(m.getBlob(), &data[0], data.size());
	}
	else
		m.setBlobSize(0);

	if (!leaveElement(m.getName()))
		return false;

	return true;
}

bool XmlDeserializer::operator >> (const MemberArray& m)
{
	if (!enterElement(m.getName()))
		return false;

	m.reserve(0);

	for (size_t i = 0;; ++i)
	{
		while (m_xpp.next() != XmlPullParser::EtEndDocument)
		{
			if (
				m_xpp.getEvent().type == XmlPullParser::EtStartElement &&
				m_xpp.getEvent().value == L"item"
			)
				break;
			if (
				m_xpp.getEvent().type == XmlPullParser::EtEndElement &&
				m_xpp.getEvent().value == m.getName()
			)
				break;
		}

		if (!(
			m_xpp.getEvent().type == XmlPullParser::EtStartElement &&
			m_xpp.getEvent().value == L"item"
		))
			break;

		m_xpp.push();

		if (!m.serialize(*this, i))
			return false;
	}

	m_stack.pop_back();

	return true;
}

bool XmlDeserializer::operator >> (const MemberComplex& m)
{
	if (m.getCompound() == true)
	{
		if (!enterElement(m.getName()))
			return false;
	}

	if (!m.serialize(*this))
		return false;
	
	if (m.getCompound() == true)
	{
		if (!leaveElement(m.getName()))
			return false;
	}

	return true;
}

std::wstring XmlDeserializer::stackPath()
{
	std::wstringstream ss;
	for (std::list< Entry >::const_iterator i = m_stack.begin(); i != m_stack.end(); ++i)
	{
		ss << L'/' << i->name;
		if (i->index > 0)
			ss << L'[' << i->index << L']';
	}
	return ss.str();
}

bool XmlDeserializer::enterElement(const std::wstring& name)
{
	Entry e =
	{
		name,
		m_stack.empty() ? 0 : m_stack.back().dups[name]++
	};
	m_stack.push_back(e);

	XmlPullParser::EventType eventType;
	while ((eventType = m_xpp.next()) != XmlPullParser::EtEndDocument)
	{
		if (
			eventType == XmlPullParser::EtStartElement &&
			m_xpp.getEvent().value == name
		)
			return true;
		else if (eventType == XmlPullParser::EtInvalid)
		{
			log::error << L"Invalid response from parser when entering element \"" << name << L"\"" << Endl;
			break;
		}
	}

	return false;
}

bool XmlDeserializer::leaveElement(const std::wstring& name)
{
	T_ASSERT (m_stack.back().name == name);
	m_stack.pop_back();

	while (m_xpp.next() != XmlPullParser::EtEndDocument)
	{
		if (
			m_xpp.getEvent().type == XmlPullParser::EtEndElement &&
			m_xpp.getEvent().value == name
		)
			return true;
	}

	return false;
}

void XmlDeserializer::rememberObject(Object* object)
{
	m_refs[stackPath()] = object;
}

bool XmlDeserializer::nextElementValue(const std::wstring& name, std::wstring& value)
{
	if (!enterElement(name))
		return false;

	m_xpp.next();
	if (m_xpp.getEvent().type == XmlPullParser::EtText)
		value = m_xpp.getEvent().value;
	else
	{
		m_xpp.push();
		value = L"";
	}

	if (!leaveElement(name))
		return false;

	return true;
}
	
	}
}
