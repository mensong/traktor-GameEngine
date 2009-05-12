#include <cstring>
#include "Core/Io/DynamicMemoryStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.DynamicMemoryStream", DynamicMemoryStream, Stream)

DynamicMemoryStream::DynamicMemoryStream(std::vector< uint8_t >& buffer, bool readAllowed, bool writeAllowed)
:	m_buffer(buffer)
,	m_bufferIter(buffer.begin())
,	m_readAllowed(readAllowed)
,	m_writeAllowed(writeAllowed)
{
}

void DynamicMemoryStream::close()
{
	m_readAllowed =
	m_writeAllowed = false;
}

bool DynamicMemoryStream::canRead() const
{
	return m_readAllowed;
}

bool DynamicMemoryStream::canWrite() const
{
	return m_writeAllowed;
}

bool DynamicMemoryStream::canSeek() const
{
	return false;
}

int DynamicMemoryStream::tell() const
{
	return 0;
}

int DynamicMemoryStream::available() const
{
	return int(std::distance(m_bufferIter, m_buffer.end()));
}

int DynamicMemoryStream::seek(SeekOriginType origin, int offset)
{
	return 0;
}

int DynamicMemoryStream::read(void* block, int nbytes)
{
	if (!m_readAllowed)
		return 0;

	if (nbytes <= 0)
		return 0;

	int read = std::min< int >(nbytes, available());
	std::memcpy(block, &(*m_bufferIter), read);
	m_bufferIter += read;

	return read;
}

int DynamicMemoryStream::write(const void* block, int nbytes)
{
	if (!m_writeAllowed)
		return 0;

	if (nbytes <= 0)
		return 0;

	size_t org = m_buffer.size();
	m_buffer.resize(org + nbytes);
	std::memcpy(&m_buffer[org], block, nbytes);

	return nbytes;
}

void DynamicMemoryStream::flush()
{
}

}

