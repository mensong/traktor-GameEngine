#include <sstream>
#if defined(T_BIG_ENDIAN)
#	include <algorithm>
#endif
#include <limits>
#include <algorithm>
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{
	namespace
	{

#if defined(T_LITTLE_ENDIAN)

template < typename T > bool read_primitive(const Ref< IStream >& stream, T& value)
{
	return stream->read(&value, sizeof(T)) == sizeof(T);
}

template < typename T > bool write_primitive(const Ref< IStream >& stream, T v)
{
	return stream->write(&v, sizeof(T)) == sizeof(T);
}

inline bool read_block(const Ref< IStream >& stream, void* block, int count, int size)
{
	return stream->read(block, count * size) == count * size;
}

inline bool write_block(const Ref< IStream >& stream, const void* block, int count, int size)
{
	return stream->write(block, count * size) == count * size;
}

#elif defined(T_BIG_ENDIAN)

template < typename T > bool read_primitive(const Ref< IStream >& stream, T& value)
{
	switch (sizeof(T))
	{
	case 1:
		{
			if (stream->read(&value, sizeof(T)) == sizeof(T))
				return true;
		}
	default:
		{
			std::vector< uint8_t > tmp(sizeof(T));
			if (stream->read(&tmp[0], sizeof(T)) == sizeof(T))
			{
				std::reverse(tmp.begin(), tmp.end());
				value = *(T*)&tmp[0];
				return true;
			}
			break;
		}
	}
	return false;
}

template < typename T > bool write_primitive(const Ref< IStream >& stream, T v)
{
	switch (sizeof(T))
	{
	case 1:
		if (stream->write(&v, sizeof(v)) == sizeof(v))
			return true;
	default:
		std::vector< uint8_t > tmp(sizeof(T));
		std::memcpy(&tmp.front(), &v, sizeof(T));
		std::reverse(tmp.begin(), tmp.end());
		if (stream->write(&tmp.front(), sizeof(T)) == sizeof(T))
			return true;
	}
	return false;
}

bool read_block(const Ref< IStream >& stream, void* block, int count, int size)
{
	int result = stream->read(block, count * size);
	if (result > 0 && size > 1)
	{
		uint8_t* p = static_cast< uint8_t* >(block);
		for (int i = 0; i < result; i += size)
		{
			for (int j = 0; j < size >> 1; ++j)
				std::swap(p[j], p[size - j - 1]);
			p += size;
		}
	}
	return result == count * size;
}

bool write_block(const Ref< IStream >& stream, const void* block, int count, int size)
{
	if (size > 1)
	{
		const uint8_t* p = static_cast< const uint8_t* >(block);
		std::vector< uint8_t > tmp(size);

		for (int i = 0; i < count; ++i, p += size)
		{
			std::memcpy(&tmp.front(), p, size);
			std::reverse(tmp.begin(), tmp.end());
			if (stream->write(&tmp.front(), size) != size)
				return false;
		}

		return true;
	}
	else
	{
		return stream->write(block, count * size) == count * size;
	}
}

#endif

template < >
bool read_primitive(const Ref< IStream >& stream, bool& value)
{
	uint8_t tmp;
	if (stream->read(&tmp, 1) != 1)
		return false;

	value = bool(tmp != 0);
	return true;
}

template < >
bool write_primitive(const Ref< IStream >& stream, bool v)
{
	uint8_t tmp = v ? 0xff : 0x00;
	if (stream->write(&tmp, 1) == 1)
		return true;
	return false;
}

bool read_string(const Ref< IStream >& stream, uint32_t u8len, std::wstring& outString)
{
	wchar_t wc;

	outString.clear();

	if (u8len > 0)
	{
		AutoArrayPtr< uint8_t > u8str(new uint8_t [u8len]);

		if (!read_block(stream, u8str.ptr(), u8len, sizeof(uint8_t)))
			return false;

		Utf8Encoding utf8enc;
		for (uint32_t i = 0; i < u8len; )
		{
			int n = utf8enc.translate(u8str.ptr() + i, u8len - i, wc);
			if (n > 0)
			{
				outString.push_back(wc);
				i += n;
			}
			else
				return false;
		}
	}

	return true;
}

bool read_string(const Ref< IStream >& stream, std::wstring& outString)
{
	uint32_t u8len;

	if (!read_primitive< uint32_t >(stream, u8len))
		return false;

	return read_string(stream, u8len, outString);
}

bool write_string(const Ref< IStream >& stream, const std::wstring& str)
{
	T_ASSERT (str.length() <= std::numeric_limits< uint16_t >::max());
	
	uint32_t length = uint32_t(str.length());
	if (length > 0)
	{
		AutoArrayPtr< uint8_t > u8str(new uint8_t [length * 4]);
		uint32_t u8len;
		
		Utf8Encoding utf8enc;
		u8len = utf8enc.translate(str.c_str(), length, u8str.ptr());

		if (!write_primitive< uint32_t >(stream, u8len))
			return false;

		return write_block(stream, u8str.ptr(), u8len, sizeof(uint8_t));
	}
	else
	{
		if (!write_primitive< uint32_t >(stream, 0))
			return false;

		return true;
	}
}

bool read_string(const Ref< IStream >& stream, std::string& outString)
{
	std::wstring ws;
	if (!read_string(stream, ws))
		return false;

	outString = wstombs(ws);
	return true;
}

bool write_string(const Ref< IStream >& stream, const std::string& str)
{
	std::wstring ws = mbstows(str);
	return write_string(stream, ws);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.BinarySerializer", BinarySerializer, Serializer);

// Disable "Unusual use of Boolean expression"
/*lint -e514*/

BinarySerializer::BinarySerializer(IStream* stream)
:	m_stream(stream)
,	m_direction(m_stream->canRead() ? SdRead : SdWrite)
,	m_nextCacheId(1)
,	m_nextTypeCacheId(0)
{
}

Serializer::Direction BinarySerializer::getDirection()
{
	return m_direction;
}

bool BinarySerializer::operator >> (const Member< bool >& m)
{
	if (m_direction == SdRead)
		return read_primitive< bool >(m_stream, m);
	else
		return write_primitive< bool >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< int8_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< int8_t >(m_stream, m);
	else
		return write_primitive< int8_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< uint8_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< uint8_t >(m_stream, m);
	else
		return write_primitive< uint8_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< int16_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< int16_t >(m_stream, m);
	else
		return write_primitive< int16_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< uint16_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< uint16_t >(m_stream, m);
	else
		return write_primitive< uint16_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< int32_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< int32_t >(m_stream, m);
	else
		return write_primitive< int32_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< uint32_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< uint32_t >(m_stream, m);
	else
		return write_primitive< uint32_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< int64_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< int64_t >(m_stream, m);
	else
		return write_primitive< int64_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< uint64_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< uint64_t >(m_stream, m);
	else
		return write_primitive< uint64_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< float >& m)
{
	if (m_direction == SdRead)
		return read_primitive< float >(m_stream, m);
	else
		return write_primitive< float >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< double >& m)
{
	if (m_direction == SdRead)
		return read_primitive< double >(m_stream, m);
	else
		return write_primitive< double >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< std::string >& m)
{
	if (m_direction == SdRead)
		return read_string(m_stream, m);
	else
		return write_string(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< std::wstring >& m)
{
	if (m_direction == SdRead)
		return read_string(m_stream, m);
	else
		return write_string(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< Guid >& m)
{
	Guid& guid = m;
	if (m_direction == SdRead)
	{
		bool validGuid;
		
		if (!read_primitive< bool >(m_stream, validGuid))
			return false;

		if (validGuid)
		{
			uint8_t data[16];
			if (!read_block(m_stream, data, 16, 1))
				return false;
			guid = Guid(data);
		}
		else
			guid = Guid();
	}
	else
	{
		bool validGuid = guid.isValid();
		if (!write_primitive< bool >(m_stream, validGuid))
			return false;

		if (validGuid)
		{
			if (!write_block(m_stream, static_cast< const uint8_t* >(guid), 16, 1))
				return false;
		}
	}
	return true;
}

bool BinarySerializer::operator >> (const Member< Path >& m)
{
	if (m_direction == SdRead)
	{
		std::wstring path;
		if (!read_string(m_stream, path))
			return false;
		*m = Path(path);
	}
	else
	{
		std::wstring path = m->getOriginal();
		if (!write_string(m_stream, path))
			return false;
	}
	return true;
}

bool BinarySerializer::operator >> (const Member< Color >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		result &= read_primitive< uint8_t >(m_stream, m->r);
		result &= read_primitive< uint8_t >(m_stream, m->g);
		result &= read_primitive< uint8_t >(m_stream, m->b);
		result &= read_primitive< uint8_t >(m_stream, m->a);
	}
	else
	{
		result &= write_primitive< uint8_t >(m_stream, m->r);
		result &= write_primitive< uint8_t >(m_stream, m->g);
		result &= write_primitive< uint8_t >(m_stream, m->b);
		result &= write_primitive< uint8_t >(m_stream, m->a);
	}
	return result;
}

bool BinarySerializer::operator >> (const Member< Scalar >& m)
{
	Scalar& v = m;
	if (m_direction == SdRead)
	{
		float tmp;
		if (!read_primitive< float >(m_stream, tmp))
			return false;
		v = Scalar(tmp);
	}
	else
	{
		if (!write_primitive< float >(m_stream, float(v)))
			return false;
	}
	return true;
}

bool BinarySerializer::operator >> (const Member< Vector2 >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		result &= read_primitive< float >(m_stream, m->x);
		result &= read_primitive< float >(m_stream, m->y);
	}
	else
	{
		result &= write_primitive< float >(m_stream, m->x);
		result &= write_primitive< float >(m_stream, m->y);
	}
	return result;
}

bool BinarySerializer::operator >> (const Member< Vector4 >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		float x, y, z, w;
		result &= read_primitive< float >(m_stream, x);
		result &= read_primitive< float >(m_stream, y);
		result &= read_primitive< float >(m_stream, z);
		result &= read_primitive< float >(m_stream, w);
		if (result)
			m->set(x, y, z, w);
	}
	else
	{
		result &= write_primitive< float >(m_stream, m->x());
		result &= write_primitive< float >(m_stream, m->y());
		result &= write_primitive< float >(m_stream, m->z());
		result &= write_primitive< float >(m_stream, m->w());
	}
	return result;
}

bool BinarySerializer::operator >> (const Member< Matrix33 >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		for (int i = 0; i < 3 * 3; ++i)
			result &= read_primitive< float >(m_stream, m->m[i]);
	}
	else
	{
		for (int i = 0; i < 3 * 3; ++i)
			result &= write_primitive< float >(m_stream, m->m[i]);
	}
	return result;
}

bool BinarySerializer::operator >> (const Member< Matrix44 >& m)
{
	float T_MATH_ALIGN16 values[16];
	bool result = true;

	if (m_direction == SdRead)
	{
		for (int i = 0; i < 16; ++i)
			result &= read_primitive< float >(m_stream, values[i]);
		(*m) = Matrix44::loadAligned(values);
	}
	else
	{
		(*m).storeAligned(values);
		for (int i = 0; i < 16; ++i)
			result &= write_primitive< float >(m_stream, values[i]);
	}
	return result;
}

bool BinarySerializer::operator >> (const Member< Quaternion >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		float x, y, z, w;
		result &= read_primitive< float >(m_stream, x);
		result &= read_primitive< float >(m_stream, y);
		result &= read_primitive< float >(m_stream, z);
		result &= read_primitive< float >(m_stream, w);
		if (result)
			m->e.set(x, y, z, w);
	}
	else
	{
		result &= write_primitive< float >(m_stream, m->e.x());
		result &= write_primitive< float >(m_stream, m->e.y());
		result &= write_primitive< float >(m_stream, m->e.z());
		result &= write_primitive< float >(m_stream, m->e.w());
	}
	return result;
}

bool BinarySerializer::operator >> (const Member< ISerializable >& m)
{
	ISerializable* object = static_cast< ISerializable* >(&(*m));
	if (m_direction == SdRead)
	{
		int32_t version;
		if (!read_primitive< int32_t >(m_stream, version))
			return false;
		return serialize(object, version, 0);
	}
	else
	{
		int32_t version = type_of(object).getVersion();
		if (!write_primitive< int32_t >(m_stream, version))
			return false;
		return serialize(object, version, 0);
	}
}

bool BinarySerializer::operator >> (const Member< ISerializable* >& m)
{
	if (m_direction == SdRead)
	{
		bool reference;
		uint64_t hash;

		if (!read_primitive< bool >(m_stream, reference))
			return false;
		if (!read_primitive< uint64_t >(m_stream, hash))
			return false;

		ISerializable* object = 0;

		if (hash)
		{
			if (!reference)
			{
				uint32_t typeHashOrLen;
				std::wstring typeName;
				int32_t version;

				if (!read_primitive< uint32_t >(m_stream, typeHashOrLen))
					return false;

				if ((typeHashOrLen & 0x80000000) == 0x80000000)
					typeName = m_typeReadCache[typeHashOrLen & 0x7fffffff];
				else
				{
					if (!read_string(m_stream, typeHashOrLen, typeName))
						return false;

					m_typeReadCache.push_back(typeName);
				}
				
				const TypeInfo* type = TypeInfo::find(typeName);
				if (type == 0)
				{
					log::error << L"Unable to serialize \"" << m.getName() << L"\"; no such type \"" << typeName << L"\"" << Endl;
					return false;
				}
					
				if (!(object = checked_type_cast< ISerializable* >(type->createInstance())))
				{
					log::error << L"Unable to serialize \"" << m.getName() << L"\"; unable to create type \"" << typeName << L"\"" << Endl;
					return false;
				}

				if (!read_primitive< int32_t >(m_stream, version))
					return false;

				if (!serialize(object, version, 0))
					return false;

				m_readCache[hash] = object;
			}
			else
			{
				object = static_cast< ISerializable* >(m_readCache[hash]);
				if (!object)
					return false;
			}
		}

		m = object;
	}
	else
	{
		ISerializable* object = *m;

		if (object)
		{
			std::map< ISerializable*, uint64_t >::iterator i = m_writeCache.find(object);
			if (i != m_writeCache.end())
			{
				if (!write_primitive< bool >(m_stream, true))
					return false;
				if (!write_primitive< uint64_t >(m_stream, i->second))
					return false;
			}
			else
			{
				int32_t version = type_of(object).getVersion();
				uint64_t hash = m_nextCacheId++;

				if (!write_primitive< bool >(m_stream, false))
					return false;
				if (!write_primitive< uint64_t >(m_stream, hash))
					return false;

				std::map< const TypeInfo*, uint32_t >::const_iterator i = m_typeWriteCache.find(&type_of(object));
				if (i != m_typeWriteCache.end())
					write_primitive< uint32_t >(m_stream, 0x80000000 | i->second);
				else
				{
					if (!write_string(m_stream, type_name(object)))
						return false;

					m_typeWriteCache.insert(std::make_pair(&type_of(object), m_nextTypeCacheId++));
				}

				if (!write_primitive< int32_t >(m_stream, version))
					return false;

				m_writeCache[object] = hash;
				if (!serialize(object, version, 0))
					return false;
			}
		}
		else
		{
			if (!write_primitive< bool >(m_stream, false))
				return false;
			if (!write_primitive< uint64_t >(m_stream, 0ULL))
				return false;
		}
	}
	return true;
}

bool BinarySerializer::operator >> (const Member< void* >& m)
{
	if (m_direction == SdRead)
	{
		uint32_t size;
			
		if (!read_primitive< uint32_t >(m_stream, size))
			return false;
		
		if (size > m.getBlobSize())
			return false;

		m.setBlobSize(size);
		return read_block(m_stream, m.getBlob(), size, 1);
	}
	else
	{
		if (!write_primitive< uint32_t >(m_stream, uint32_t(m.getBlobSize())))
			return false;

		return write_block(m_stream, m.getBlob(), m.getBlobSize(), 1);
	}
}

bool BinarySerializer::operator >> (const MemberArray& m)
{
	if (m_direction == SdRead)
	{
		uint32_t size;

		if (!read_primitive< uint32_t >(m_stream, size))
			return false;
		
		m.reserve(size);
		for (uint32_t i = 0; i < size; ++i)
		{
			if (!m.serialize(*this, i))
				return false;
		}
	}
	else
	{
		uint32_t size = uint32_t(m.size());

		if (!write_primitive< uint32_t >(m_stream, size))
			return false;

		for (uint32_t i = 0; i < size; ++i)
		{
			if (!m.serialize(*this, i))
				return false;
		}
	}
	return true;
}

bool BinarySerializer::operator >> (const MemberComplex& m)
{
	return m.serialize(*this);
}

bool BinarySerializer::operator >> (const MemberEnumBase& m)
{
	return this->operator >> (*(MemberComplex*)(&m));
}

/*lint -restore*/

}
