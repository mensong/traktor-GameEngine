#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <lmcons.h>
#include <shlobj.h>
#include <tchar.h>
#include "Core/Io/IVolume.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/OS.h"
#include "Core/System/Win32/ProcessWin32.h"
#include "Core/System/Win32/SharedMemoryWin32.h"

namespace traktor
{
	namespace
	{

typedef HRESULT STDAPICALLTYPE IEISPROTECTEDMODEPROCESSPROC (BOOL* pbResult);
typedef HRESULT STDAPICALLTYPE IEGETWRITEABLEFOLDERPATHPROC (REFGUID clsidFolderID, LPWSTR *lppwstrPath);

HINSTANCE s_hIeFrameLib = 0;
IEISPROTECTEDMODEPROCESSPROC* s_IEIsProtectedModeProcess = 0;
IEGETWRITEABLEFOLDERPATHPROC* s_IEGetWriteableFolderPath = 0;

	}

OS& OS::getInstance()
{
	static OS* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new OS();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

uint32_t OS::getCPUCoreCount() const
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

std::wstring OS::getComputerName() const
{
#if !defined(WINCE)
	wchar_t buf[MAX_COMPUTERNAME_LENGTH + 1];

	DWORD size = sizeof_array(buf);
	if (!GetComputerName(buf, &size))
		return L"Unavailable";

	return buf;
#else
	return L"Unavailable";
#endif
}

std::wstring OS::getCurrentUser() const
{
#if !defined(WINCE)
	wchar_t buf[UNLEN + 1];
	
	DWORD size = sizeof_array(buf);
	if (!GetUserName(buf, &size))
		return L"Unavailable";

	return buf;
#else
	return L"Unavailable";
#endif
}

std::wstring OS::getUserHomePath() const
{
#if !defined(WINCE)
	TCHAR szPath[MAX_PATH];
	HRESULT hr;

	hr = SHGetFolderPath(
		NULL,
		CSIDL_PERSONAL,
		NULL,
		SHGFP_TYPE_CURRENT,
		szPath
	);
	if (FAILED(hr))
		return L"";

	return replaceAll(tstows(szPath), L'\\', L'/');
#else
	return L"";
#endif
}

std::wstring OS::getUserApplicationDataPath() const
{
#if !defined(WINCE)
	TCHAR szPath[MAX_PATH];
	HRESULT hr;

	hr = SHGetFolderPath(
		NULL,
		CSIDL_LOCAL_APPDATA,
		NULL,
		SHGFP_TYPE_CURRENT,
		szPath
	);
	if (FAILED(hr))
		return L"";

	return replaceAll(tstows(szPath), L'\\', L'/');
#else
	return L"";
#endif
}

std::wstring OS::getWritableFolderPath() const
{
#if !defined(WINCE)
	if (s_IEIsProtectedModeProcess && s_IEGetWriteableFolderPath)
	{
		HRESULT hr;
		BOOL pm;

		hr = (*s_IEIsProtectedModeProcess)(&pm);
		if (FAILED(hr))
			pm = FALSE;

		if (pm)
		{
			LPWSTR pwstrPath = 0;
			hr = (*s_IEGetWriteableFolderPath)(
				FOLDERID_LocalAppDataLow,
				&pwstrPath
			);
			if (FAILED(hr))
				return L"";

			std::wstring path = pwstrPath;
			path = replaceAll(path, L'\\', L'/');

			CoTaskMemFree(pwstrPath);
			return path;
		}
	}
#endif
	return getUserApplicationDataPath();
}

bool OS::editFile(const Path& file) const
{
#if !defined(WINCE)
	Path absoluteFile = FileSystem::getInstance().getAbsolutePath(file);
	HINSTANCE hInstance = ShellExecute(
		NULL,
		_T("edit"),
		wstots(absoluteFile.getPathName()).c_str(),
		NULL,
		NULL,
		SW_SHOWDEFAULT
	);
	return int(hInstance) > 32;
#else
	return false;
#endif
}

bool OS::exploreFile(const Path& file) const
{
#if !defined(WINCE)
	Path absoluteFile = FileSystem::getInstance().getAbsolutePath(file.getPathOnly());
	HINSTANCE hInstance = ShellExecute(
		NULL,
		_T("explore"),
		wstots(absoluteFile.getPathName()).c_str(),
		NULL,
		NULL,
		SW_SHOWDEFAULT
	);
	return int(hInstance) > 32;
#else
	return false;
#endif
}

OS::envmap_t OS::getEnvironment() const
{
	envmap_t envmap;

#if !defined(WINCE)
	LPTCH lpEnv = GetEnvironmentStrings();
	if (lpEnv)
	{
		TCHAR* p = lpEnv;
		while (*p != 0)
		{
			TCHAR* key = p;
			TCHAR* sep = _tcschr(p, '=');
			if (!sep)
				break;
			TCHAR* val = sep + 1;

			if (key < sep)
			{
				envmap.insert(std::make_pair(
					tstows(tstring(key, sep)),
					tstows(val)
				));
			}

			p = val + _tcslen(val) + 1;
		}
	}
#endif

	return envmap;
}

bool OS::getEnvironment(const std::wstring& name, std::wstring& outValue) const
{
	const char* env = getenv(wstombs(name).c_str());
	if (!env)
		return false;

	outValue = mbstows(env);
	return true;
}

Ref< IProcess > OS::execute(
	const Path& file,
	const std::wstring& commandLine,
	const Path& workingDirectory,
	const envmap_t* envmap,
	bool redirect,
	bool mute,
	bool detach
) const
{
	TCHAR cmd[32768], cwd[MAX_PATH];
	HANDLE hStdInRead = 0, hStdInWrite = 0;
	HANDLE hStdOutRead = 0, hStdOutWrite = 0;
	HANDLE hStdErrRead = 0, hStdErrWrite = 0;
	AutoArrayPtr< wchar_t > environment;

	Path fileAbs = FileSystem::getInstance().getAbsolutePath(file);
	Path workingDirectoryAbs = FileSystem::getInstance().getAbsolutePath(workingDirectory);

	StringOutputStream ss;
	ss << L"\"" << fileAbs.getPathName() << L"\"";
	if (!commandLine.empty())
		ss << L" " << commandLine;

#if !defined(WINCE)
	_tcscpy_s(cmd, wstots(ss.str()).c_str());
	_tcscpy_s(cwd, wstots(workingDirectoryAbs.getPathName()).c_str());
#else
	_tcscpy_s(cmd, sizeof_array(cmd), wstots(ss.str()).c_str());
	_tcscpy_s(cwd, sizeof_array(cwd), wstots(workingDirectoryAbs.getPathName()).c_str());
#endif

	// Create environment variables.
	if (envmap)
	{
		// Calculate how much space we need to allocate.
		uint32_t size = 0;
		for (envmap_t::const_iterator i = envmap->begin(); i != envmap->end(); ++i)
			size += i->first.length() + 1 + i->second.length() + 1;
		size += 1;

		environment.reset(new wchar_t [size]);

		wchar_t* p = environment.ptr(); *p = 0;
		for (envmap_t::const_iterator i = envmap->begin(); i != envmap->end(); ++i)
		{
			wcscpy(p, i->first.c_str());
			wcscat(p, L"=");
			wcscat(p, i->second.c_str());
			p += i->first.length() + 1 + i->second.length() + 1;
		}
		*p++ = 0;

		T_ASSERT (size_t(p - environment.ptr()) == size);
	}

#if !defined(WINCE)
	if (redirect)
	{
		// Create IO pipes.
		SECURITY_DESCRIPTOR sd;
		InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&sd, true, NULL, false);
	
		SECURITY_ATTRIBUTES sa;
		std::memset(&sa, 0, sizeof(sa));
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = &sd;
		sa.bInheritHandle = true;
	
		CreatePipe(
			&hStdInRead,
			&hStdInWrite,
			&sa,
			0
		);
	
		CreatePipe(
			&hStdOutRead,
			&hStdOutWrite,
			&sa,
			0
		);
	
		CreatePipe(
			&hStdErrRead,
			&hStdErrWrite,
			&sa,
			0
		);
	}
#endif

	STARTUPINFO si;
	std::memset(&si, 0, sizeof(si));
	si.cb = sizeof(STARTUPINFO);
#if !defined(WINCE)
	si.dwFlags = redirect ? STARTF_USESTDHANDLES : 0;
#else
	si.dwFlags = 0;
#endif
	si.hStdInput = hStdInRead;
	si.hStdOutput = hStdOutWrite;
	si.hStdError = hStdErrWrite;

	PROCESS_INFORMATION pi;
	std::memset(&pi, 0, sizeof(pi));

	DWORD dwCreationFlags = 0;
#if !defined(WINCE)
	if (detach)
		dwCreationFlags = DETACHED_PROCESS;
	else if (mute)
		dwCreationFlags = CREATE_NO_WINDOW;
	else
		dwCreationFlags = CREATE_NEW_CONSOLE;

	if (environment.ptr())
		dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
#else
	dwCreationFlags = mute ? 0 : CREATE_NEW_CONSOLE;
#endif

	BOOL result = CreateProcess(
		NULL,
		cmd,
		NULL,
		NULL,
		TRUE,
		dwCreationFlags,
		environment.ptr(),
		(cwd[0] != L'\0' ? cwd : NULL),
		&si,
		&pi
	);
	if (result == FALSE)
	{
		log::debug << L"Unable to create process, error = " << (int32_t)GetLastError() << Endl;
		return 0;
	}

	return new ProcessWin32(
		pi,
		hStdInRead,
		hStdInWrite,
		hStdOutRead,
		hStdOutWrite,
		hStdErrRead,
		hStdErrWrite
	);
}

Ref< ISharedMemory > OS::createSharedMemory(const std::wstring& name, uint32_t size) const
{
	Ref< SharedMemoryWin32 > sharedMemory = new SharedMemoryWin32();
	if (!sharedMemory->create(name, size))
		return 0;
	return sharedMemory;
}

OS::OS()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

#if !defined(WINCE)
	// Load IEFrame library; only available on Vista.
	s_hIeFrameLib = LoadLibrary(L"ieframe.dll");
	if (s_hIeFrameLib)
	{
		s_IEIsProtectedModeProcess = (IEISPROTECTEDMODEPROCESSPROC*)GetProcAddress(s_hIeFrameLib, "IEIsProtectedModeProcess");
		T_ASSERT (s_IEIsProtectedModeProcess);
		s_IEGetWriteableFolderPath = (IEGETWRITEABLEFOLDERPATHPROC*)GetProcAddress(s_hIeFrameLib, "IEGetWriteableFolderPath");
		T_ASSERT (s_IEGetWriteableFolderPath);
	}
#endif
}

OS::~OS()
{
	if (s_hIeFrameLib)
		FreeLibrary(s_hIeFrameLib);

	CoUninitialize();
}

void OS::destroy()
{
	delete this;
}

}
