#ifndef traktor_Writer_H
#define traktor_Writer_H

#include <string>
#include "Core/Heap/Ref.h"
#include "Core/Io/Stream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Primitive writer class.
 * \ingroup Core
 */
class T_DLLCLASS Writer : public Object
{
	T_RTTI_CLASS(Writer)

public:
	Writer(Stream* stream);
	
	Writer& operator << (bool v);
	
	Writer& operator << (int8_t c);
	
	Writer& operator << (uint8_t uc);
	
	Writer& operator << (int16_t i);
	
	Writer& operator << (uint16_t ui);
	
	Writer& operator << (int32_t i);
	
	Writer& operator << (uint32_t i);
	
	Writer& operator << (int64_t i);

	Writer& operator << (uint64_t i);
	
	Writer& operator << (float f);
	
	Writer& operator << (double f);
	
	Writer& operator << (const std::wstring& s);
	
	Writer& operator << (const wchar_t* s);
	
	int write(const void* block, int nbytes);
	
	int write(const void* block, int count, int size);
	
private:
	Ref< Stream > m_stream;
};
	
}

#endif	// traktor_Writer_H
