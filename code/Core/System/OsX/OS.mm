#include <CoreFoundation/CFBundle.h>#include <Foundation/Foundation.h>#include <crt_externs.h>#include <glob.h>#include <pwd.h>#include <spawn.h>#include <unistd.h>#include <sys/param.h>#include <sys/syslimits.h>#include "Core/Io/Path.h"#include "Core/Log/Log.h"#include "Core/Misc/String.h"#include "Core/Misc/StringSplit.h"#include "Core/Misc/TString.h"#include "Core/Singleton/SingletonManager.h"#include "Core/System/OS.h"#include "Core/System/OsX/ProcessOsX.h"#include "Core/System/OsX/SharedMemoryOsX.h"namespace traktor{OS& OS::getInstance(){	static OS* s_instance = 0;	if (!s_instance)	{		s_instance = new OS();		SingletonManager::getInstance().add(s_instance);	}	return *s_instance;}uint32_t OS::getCPUCoreCount() const{#if TARGET_OS_IPHONE	return 1;#else	return 4;#endif}std::wstring OS::getComputerName() const{	char name[MAXHOSTNAMELEN];	if (gethostname(name, sizeof_array(name)) != -1)		return mbstows(name);	return L"Unavailable";}std::wstring OS::getCurrentUser() const{	passwd* pwd = getpwuid(geteuid());	if (!pwd)		return L"Unavailable";	const char* who = pwd->pw_name;	if (!who)		return L"Unavailable";	return mbstows(who);}std::wstring OS::getUserHomePath() const{	char path[] = { "~" };	glob_t globbuf;		if (glob(path, GLOB_TILDE, NULL, &globbuf) == 0)	{		char* ep = globbuf.gl_pathv[0];		return mbstows(ep);	}		return L"";}std::wstring OS::getUserApplicationDataPath() const{	return getUserHomePath();}std::wstring OS::getWritableFolderPath() const{#if TARGET_OS_IPHONE	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);	NSString* documentsDirectory = [paths objectAtIndex: 0];	return mbstows([documentsDirectory UTF8String]);#else	return getUserHomePath();#endif}bool OS::editFile(const Path& file) const{	return false;}bool OS::exploreFile(const Path& file) const{	return false;}OS::envmap_t OS::getEnvironment() const{	envmap_t envmap;		char** environ = *_NSGetEnviron();	for (char** e = environ; *e; ++e)	{		std::wstring pair(mbstows(*e));		size_t p = pair.find('=');		if (p != pair.npos)		{			std::wstring key = pair.substr(0, p);			std::wstring value = pair.substr(p + 1);			envmap[key] = value;		}	}		return envmap;}bool OS::getEnvironment(const std::wstring& name, std::wstring& outValue) const{	CFBundleRef mainBundle = CFBundleGetMainBundle();	if (mainBundle)	{		bool gotValue = false;				if (name == L"BUNDLE_PATH")	// /path-to-bundle/MyApplication.app		{			if (mainBundle)			{				CFURLRef appUrl = CFBundleCopyBundleURL(mainBundle);				if (appUrl)				{					char bundlePath[PATH_MAX];					CFURLGetFileSystemRepresentation(appUrl, TRUE, (uint8_t*)bundlePath, PATH_MAX);					CFRelease(appUrl);									outValue = mbstows(bundlePath);					gotValue = true;				}			}		}				if (gotValue)			return true;		// Try to get value from bundle.		CFStringRef keyStr = CFStringCreateWithCString(kCFAllocatorDefault, wstombs(name).c_str(), kCFStringEncodingMacRoman);		if (keyStr)		{			CFStringRef valueRef = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(mainBundle, keyStr);			if (valueRef)			{				const char* valueStr = CFStringGetCStringPtr(valueRef, kCFStringEncodingMacRoman);				if (valueStr)				{					outValue = mbstows(valueStr);										// \hack Replace "\$" sequences with plain "$" as Info.plist preprocessor					// seems to intercept and remove those.					outValue = replaceAll< std::wstring >(outValue, L"\\$", L"$");										gotValue = true;				}			}			CFRelease(keyStr);		}				if (gotValue)			return true;	}	// Ordinary variables; read from process environment.	const char* env = getenv(wstombs(name).c_str());	if (!env)		return false;			outValue = mbstows(env);	return true;}Ref< IProcess > OS::execute(	const Path& file,	const std::wstring& commandLine,	const Path& workingDirectory,	const envmap_t* envmap,	bool redirect,	bool mute,	bool detach) const{	if (compareIgnoreCase< std::wstring >(file.getExtension(), L"sh") == 0)	{		char cwd[512];		char* envv[256];		char* argv[64];		int envc = 0;		int argc = 0;		int err;				// Convert all arguments.		argv[argc++] = strdup("/bin/sh");		argv[argc++] = strdup(wstombs(file.getPathNameNoVolume()).c_str());				StringSplit< std::wstring > s(commandLine, L" ");		for (StringSplit< std::wstring >::const_iterator i = s.begin(); i != s.end(); ++i)			argv[argc++] = strdup(wstombs(*i).c_str());				// Convert environment variables.		if (envmap)		{			for (envmap_t::const_iterator i = envmap->begin(); i != envmap->end(); ++i)				envv[envc++] = strdup(wstombs(i->first + L"=" + i->second).c_str());		}				// Terminate argument and environment vectors.		envv[envc] = 0;		argv[argc] = 0;				// Spawned process inherit working directory from our process; thus		// we need to temporarily change directory.		getcwd(cwd, sizeof(cwd));		chdir(wstombs(workingDirectory.getPathNameNoVolume()).c_str());		// Redirect standard IO.		/*		posix_spawn_file_actions_t fileActions;		posix_spawn_file_actions_init(&fileActions);		posix_spawn_file_actions_adddup2(&fileActions, 1, 2);		*/				// Spawn process.		pid_t pid;		err = posix_spawn(&pid, argv[0], 0, 0, argv, envv);				// Restore our working directory before returning.		chdir(cwd);				if (err == 0)			return new ProcessOsX(pid);	}	return 0;}Ref< ISharedMemory > OS::createSharedMemory(const std::wstring& name, uint32_t size) const{	return new SharedMemoryOsX(size);}OS::OS(){}OS::~OS(){}void OS::destroy(){	delete this;}}