#if !defined(_WIN32)
#include <cassert>
#endif
#include <iostream>
#include "Core/Debug/Debugger.h"

namespace traktor
{

Debugger& Debugger::getInstance()
{
	static Debugger instance;
	return instance;
}

void Debugger::assertionFailed(const std::string& expression, const std::string& file, int line, const std::wstring& message)
{
	std::cerr << "ASSERT FAILED! " << expression << ", " << file << " (" << line << ")" << std::endl;
	if (!message.empty())
		std::wcerr << L"               " << message << std::endl;

#if defined(_WIN32)
	__debugbreak();
#else
	// Fall back on assert macro
	assert (0);
#endif
}

void Debugger::breakDebugger()
{
#if defined(_WIN32)
	__debugbreak();
#else
	// Fall back on assert macro
	assert (0);
#endif
}

}
